#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "gp_timer.h"

unsigned long gp_time(gp_clocktype_t type)
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

	return (t.tv_sec * (unsigned long) 1e9 + t.tv_nsec)/1000000; //1ms resolution
}

/*---------------------------------------gp_timer_list----------------------------------------*/

void create_gp_timer(gp_timer_list **timer, void (*fn)(void *), void *data)
{
	gp_timer_list *tmp = malloc(sizeof(gp_timer_list));
	memset(tmp, 0, sizeof(*tmp));
	init_gp_timer(tmp, fn, data);
	*timer = tmp;
}

void init_gp_timer(gp_timer_list *timer, void (*fn)(void *), void *data)
{
	GP_LIST_NODE_INIT(&timer->node);
	timer->expires = 0;
	timer->state = 0;
	timer->callback = fn;
	timer->data = data;
	timer->base = NULL;
	
}

void destruct_gp_timer(gp_timer_list *timer)
{
	gp_list_node_remove(&timer->node);
	timer->base = NULL;
	timer->state = 0;
	timer->callback = NULL;
	timer->data = NULL;
	timer->expires = 0;
	free(timer);
}

static void gp_timer_internal_add(gp_timer_base *base, gp_timer_list *timer)
{
	unsigned long expires = timer->expires;
    unsigned long idx = expires - base->timer_jiffies;
    gp_list *vec;

    if (idx < TVR_SIZE) {
        int i = expires & TVR_MASK;
        vec = base->tv1.vec + i; 
    } else if (idx < 1 << (TVR_BITS + TVN_BITS)) {
        int i = (expires >> TVR_BITS) & TVN_MASK;
        vec = base->tv2.vec + i; 
    } else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) {
        int i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
        vec = base->tv3.vec + i; 
    } else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) {
        int i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
        vec = base->tv4.vec + i; 
    } else if ((signed long) idx < 0) { 
        vec = base->tv1.vec + (base->timer_jiffies & TVR_MASK);
	} else {
        int i;
        if (idx > MAX_TVAL) {
            idx = MAX_TVAL;
            expires = idx + base->timer_jiffies;
        }
        i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
        vec = base->tv5.vec + i;
    }


	if((long)(expires - base->timer_next) < 0){
		base->timer_next = expires;
	}

    gp_list_append(vec, &timer->node);

	timer->base = base;
}

void gp_timer_add(gp_timer_base *base, gp_timer_list *timer, unsigned long expires)
{
	gp_list_node_remove(&timer->node);
	timer->base = NULL;
	timer->expires = expires;
	
	gp_timer_internal_add(base, timer);
}

void gp_timer_del(gp_timer_base *base, gp_timer_list *timer)
{
	gp_list_node_remove(&timer->node);
	timer->base = NULL;
}

void gp_timer_mod(gp_timer_base *base, gp_timer_list *timer, unsigned long expires)
{
	gp_timer_del(base, timer);
	gp_timer_add(base, timer, expires);
}

/*---------------------------------------gp_timer_base----------------------------------------*/


void create_gp_timer_base(gp_timer_base **base, unsigned long jiffies)
{
	gp_timer_base *base_t = malloc(sizeof(*base_t));
	memset(base_t, 0, sizeof(*base_t));
	init_gp_timer_base(base_t, jiffies);
	*base = base_t;
}

void init_gp_timer_base(gp_timer_base *base, unsigned long jiffies)
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
	int i, j;
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

static void cascade(tvec *tv, int index)
{
	gp_list tmp;
	GP_LIST_INIT(&tmp, gp_timer_list, node);
	gp_list_replace(tv->vec+index, &tmp);

	gp_timer_list *timer =NULL;
	GP_LIST_FOREACH(&tmp, timer){
		gp_list_node_remove(&timer->node);
		gp_timer_internal_add(timer->base, timer);
	}
}

void gp_run_timers(gp_timer_base *base, unsigned long jiffies)
{
	gp_timer_list *timer =NULL;
	while( (long)(jiffies - base->timer_jiffies) >= 0){
		gp_list tmp;
		GP_LIST_INIT(&tmp, gp_timer_list, node);
		int index = base->timer_jiffies & TVR_MASK;
		if (index == 0) {
			int i = INDEX(base, 0);
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
		gp_list_replace(base->tv1.vec + index, &tmp);
		
		GP_LIST_FOREACH(&tmp, timer){
			gp_list_node_remove(&timer->node);
			timer->base = NULL;
			if (timer->callback) 
				timer->callback(timer->data);
		}

	}
}

