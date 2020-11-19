#include <time.h>
#include "gp.h"

uint64_t gp_time(gp_clocktype type)
{
	static clock_t fast_clock_id = -1;
	struct timespec t;
	clock_t clock_id;

	if(type == GP_CLOCK_FAST && fast_clock_id == -1) {
		if(clock_getres(CLOCK_MONOTONIC_COARSE, &t) == 0 &&
				t.tv_nsec <= 1 * 1000 * 1000) {
			fast_clock_id = CLOCK_MONOTONIC_COARSE;
		}else {
			fast_clock_id = CLOCK_MONOTONIC;
		}
	}

	clock_id = CLOCK_MONOTONIC;
	if(type == GP_CLOCK_FAST)
		clock_id = fast_clock_id;

	if(clock_gettime(clock_id, &t))
		return 0;

	return (t.tv_sec * (uint64_t) 1e9 + t.tv_nsec)/1000000; //1ms resolution
}

/*---------------------------------------gp_timer_list----------------------------------------*/

void create_gp_timer(gp_timer_list **timer, void (*fn)(void *), void *data, uint32_t expires, int32_t interval, int32_t repeat)
{
	gp_timer_list *tmp = malloc(sizeof(gp_timer_list));
	memset(tmp, 0, sizeof(*tmp));
	init_gp_timer(tmp, fn, data, expires, interval, repeat);
	*timer = tmp;
}

void init_gp_timer(gp_timer_list *timer, void (*fn)(void *), void *data, uint32_t expires, int32_t interval, int32_t repeat)
{
	GP_LIST_NODE_INIT(&timer->node);
	timer->loop = NULL;
	timer->expires = expires;
	timer->interval = interval;
	timer->repeat = repeat;
	timer->state = 0;
	timer->callback = fn;
	timer->data = data;
	
}

void destruct_gp_timer(gp_timer_list *timer)
{
	gp_list_node_remove(&timer->node);
	timer->loop = NULL;
	timer->expires = 0;
	timer->interval = 0;
	timer->repeat = 0;
	timer->state = 0;
	timer->callback = NULL;
	timer->data = NULL;

	free(timer);
}


static uint32_t gp_next_timer(gp_timer_base *base)
{
	uint32_t timer_jiffies = base->timer_jiffies;
	uint32_t expires = MAX_TVAL;
	
	int index, slot, array, found = 0;
	gp_timer_list *timer;
	tvec *varray[4];

	index = slot = timer_jiffies & TVR_MASK;
	do {
		GP_LIST_FOREACH(base->tv1.vec+slot, timer){
			found = 1;
			expires =timer->expires;

			if(!index || slot < index)
				goto cascade;
			return expires;
		}
		slot = (slot + 1) & TVR_MASK;
	}while(slot != index);

cascade:
	if (index)
       timer_jiffies += TVR_SIZE - index;
    timer_jiffies >>= TVR_BITS;

    /* Check tv2-tv5. */
    varray[0] = &base->tv2;
    varray[1] = &base->tv3;
    varray[2] = &base->tv4;
    varray[3] = &base->tv5;

    for (array = 0; array < 4; array++) {
        tvec *varp = (tvec *)varray[array];
        index = slot = timer_jiffies & TVN_MASK;
        do {
            GP_LIST_FOREACH(varp->vec+slot, timer) {
                found = 1; 
                if (((int64_t)timer->expires - (int64_t)expires) < 0)
                    expires = timer->expires;
            }
            if (found) {
                if (!index || slot < index)
                    break;
                return expires;
            }
            slot = (slot + 1) & TVN_MASK;
        } while (slot != index);

        if (index)
            timer_jiffies += TVN_SIZE - index;
        timer_jiffies >>= TVN_BITS;
    }
    return expires;
}

static void gp_timer_internal_add(gp_timer_base *base, gp_timer_list *timer)
{
	uint32_t expires = timer->expires ;
    uint32_t idx = expires - base->timer_jiffies;
    gp_list *vec;

	int32_t i = 0;
    if (idx < TVR_SIZE) {
        i = expires & TVR_MASK;
        vec = base->tv1.vec + i; 
    } else if (idx < 1 << (TVR_BITS + TVN_BITS)) {
        i = (expires >> TVR_BITS) & TVN_MASK;
        vec = base->tv2.vec + i; 
    } else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) {
        i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
        vec = base->tv3.vec + i; 
    } else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) {
        i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
        vec = base->tv4.vec + i; 
    } else if ((int64_t) idx < 0) { 
        vec = base->tv1.vec + (base->timer_jiffies & TVR_MASK);
	} else {
		#if 1
        if (idx > MAX_TVAL) {
            idx = MAX_TVAL;
            expires = idx + base->timer_jiffies;
        }
		#endif
        i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
        vec = base->tv5.vec + i;
    }

    gp_list_append(vec, timer);
	base->next_timer = gp_next_timer(base);
}

void gp_timer_add(gp_timer_base *base, gp_timer_list *timer)
{
	gp_list_node_remove(&timer->node);
	gp_timer_internal_add(base, timer);
}

void gp_timer_del(gp_timer_list *timer)
{
	gp_list_node_remove(&timer->node);
}

void gp_timer_mod(gp_timer_base *base, gp_timer_list *timer, uint32_t expires, int32_t interval, int32_t repeat)
{
	gp_timer_del(timer);
	timer->expires = expires;
	timer->interval = interval;
	timer->repeat = repeat;
	gp_timer_add(base, timer);
}

/*---------------------------------------gp_timer_base----------------------------------------*/


void create_gp_timer_base(gp_timer_base **base, uint32_t jiffies)
{
	gp_timer_base *base_t = malloc(sizeof(*base_t));
	memset(base_t, 0, sizeof(*base_t));
	init_gp_timer_base(base_t, jiffies);
	*base = base_t;
}

void init_gp_timer_base(gp_timer_base *base, uint32_t jiffies)
{
	int i = 0;
	base->timer_jiffies = jiffies;
	base->next_timer = MAX_TVAL;
	for (i = 0; i < TVR_SIZE; i++) {
		GP_LIST_INIT(&base->tv1.vec[i], gp_timer_list, node);
	}

	for (i = 0; i < TVN_SIZE; i++) {
		GP_LIST_INIT(&base->tv2.vec[i], gp_timer_list, node);
		GP_LIST_INIT(&base->tv3.vec[i], gp_timer_list, node);
		GP_LIST_INIT(&base->tv4.vec[i], gp_timer_list, node);
		GP_LIST_INIT(&base->tv5.vec[i], gp_timer_list, node);
	}
}

void destruct_gp_timer_base(gp_timer_base *base)
{
	int32_t i;
	gp_timer_list *tmp;
	gp_list *root = NULL;
	for (i = 0; i < TVR_SIZE; i++) {
		root = &(base->tv1.vec[i]);
		GP_LIST_FOREACH(root, tmp){
			destruct_gp_timer(tmp);
		}
	}

	for (i = 0; i < TVN_SIZE; i++) {
		root =&(base->tv2.vec[i]);
		GP_LIST_FOREACH(root, tmp){
			destruct_gp_timer(tmp);
		}

		root = &(base->tv3.vec[i]);
		GP_LIST_FOREACH(root, tmp){
			destruct_gp_timer(tmp);
		}

		root = &(base->tv4.vec[i]);
		GP_LIST_FOREACH(root, tmp){
			destruct_gp_timer(tmp);
		}

		root = &(base->tv5.vec[i]);
		GP_LIST_FOREACH(root, tmp){
			destruct_gp_timer(tmp);
		}
	}
	free(base);
}

static void cascade(tvec *tv, int32_t index)
{
	gp_timer_list *timer = NULL;
	gp_timer_list *tmp =NULL;
	GP_LIST_FOREACH_SAFE(tv->vec+index, tmp, timer){
		gp_list_node_remove(&timer->node);
		gp_timer_internal_add(timer->loop->timer_base, timer);
	}
}

void gp_run_timers(gp_timer_base *base, uint32_t jiffies)
{
	gp_timer_list *timer = NULL;
	gp_timer_list *tmp = NULL;
	while( ((int32_t)jiffies - (int32_t)base->timer_jiffies) >= 0){
		int32_t index = base->timer_jiffies & TVR_MASK;
		if (index == 0) {
			int32_t i = INDEX(base, 0);
			cascade(&base->tv2, i);
			if (i == 0) {
				i = INDEX(base, 1);
				cascade(&base->tv3, i);
				if (i == 0) {
					i = INDEX(base, 2);
					cascade(&base->tv4, i);
					if (i == 0) {
						i = INDEX(base, 3);
						cascade(&base->tv5, i);
					}
				}
			}
		}
		base->timer_jiffies++;
		
		GP_LIST_FOREACH_SAFE(base->tv1.vec + index, tmp, timer){
			if (timer->callback){
				timer->callback(timer->data);
			}
			gp_timer_del(timer);
			if(timer->repeat == 1){
				timer->expires += timer->interval;
				gp_timer_add(base, timer);		
			}else{
				destruct_gp_timer(timer);
			}
			base->next_timer = gp_next_timer(base);
		}

	}
}


