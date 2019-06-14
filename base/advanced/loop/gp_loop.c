#include "gp_loop.h"
#include "stdlib.h"
#include "string.h"
/*-----------------------------timer------------------------------------*/

void gp_loop_timer_start(gp_loop *loop, gp_timer_list *timer, unsigned long expires)
{
	gp_timer_add(loop->timer_base, timer, expires);
}


void gp_loop_timer_stop(gp_loop *loop, gp_timer_list *timer)
{
	gp_timer_del(loop->timer_base, timer);

}

void gp_loop_timer_mod(gp_loop *loop, gp_timer_list *timer, unsigned long expires)
{
	gp_timer_mod(loop->timer_base, timer, expires);
}

void gp_loop_run_timers(gp_loop *loop)
{
	gp_run_timers(loop->timer_base, loop->time);
}

void gp_loop_update_time(gp_loop *loop)
{
	loop->time = gp_update_time(GP_CLOCK_FAST);
}

/*--------------------------------------gp_io---------------------------------------*/

static unsigned int next_power_of_two(unsigned int val)
{
	val -= 1;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	val += 1;
	return val;
}

static void maybe_resize(gp_loop *loop, unsigned int len) 
{
 	gp_io** watchers;
  	void* fake_watcher_list;
  	void* fake_watcher_count;
        unsigned int nwatchers;
  	unsigned int i;

  	if (len <= loop->nwatchers)
    		return;

  /* Preserve fake watcher list and count at the end of the watchers */
  	if (loop->watchers != NULL) {
    		fake_watcher_list = loop->watchers[loop->nwatchers];
    		fake_watcher_count = loop->watchers[loop->nwatchers + 1];
  	} else {
    		fake_watcher_list = NULL;
    		fake_watcher_count = NULL;
  	}

  	nwatchers = next_power_of_two(len + 2) - 2; 
  	watchers = realloc(loop->watchers,
           (nwatchers + 2) * sizeof(loop->watchers[0]));

	if(watcher == NULL)
		return;

  	for (i = loop->nwatchers; i < nwatchers; i++) 
    		watchers[i] = NULL;
  	
	watchers[nwatchers] = fake_watcher_list;
  	watchers[nwatchers + 1] = fake_watcher_count;

  	loop->watchers = watchers;
  	loop->nwatchers = nwatchers;

}

void create_gp_io(gp_io **w, gp_io_cb cb, int fd)
{
	gp_io *w_t = malloc(sizeof(gp_io));
	memset(w_t, 0, sizeof(gp_io));
	init_gp_io(w_t, cb, fd);
	*w = w_t;
}

void init_gp_io(gp_io *w, gp_io_cb cb, int fd)
{
	GP_LIST_NODE_INIT(&w->pending_node);
	GP_LIST_NODE_INIT(&w->watcher_node);
	w->cb = cb;
	w->fd = fd;
	w->events = 0;
	w->pevents = 0;
}

void gp_io_start(gp_loop *loop, gp_io *w, unsigned int events)
{
	w->pevents |= events;
	maybe_resize(loop, w->fd + 1);

	if(gp_list_node_active(&w->watcher_node))
		gp_list_append(&loop->watcher_list, &w->watcher_node);
	
	if(loop->watchers[w->fd] == NULL) {
		loop->watchers[w->fd] = w;
		loop->nfds++;
	}
}

void gp_io_stop(gp_loop *loop, gp_io *w, unsigned int events)
{
	if((unsigned int) w->fd >= loop->nwatchers)
		return;

	w->pevents &= ~events;

	if(w->pevents == 0){
		gp_list_node_remove(&w->watcher_node);
		if(loop->watchers[w->fd] != NULL){
			loop->watchers[w->fd] == NULL;
			loop->nfds--;
			w->events = 0;
		}
	}else if(gp_list_node_active(&w->watcher_node))
		gp_list_append(&loop->watcher_list, &w->watcher_node);
}

void gp_io_poll(gp_loop *loop, unsigned long timeout)
{


}


/*-------------------------------------gp_loop-------------------------------------*/
int create_gp_loop(gp_loop **loop)
{
	gp_loop *loop_t = malloc(sizeof(gp_loop));
	memset(loop_t, 0, sizeof(*loop_t));
	init_gp_loop(loop_t);
	*loop = loop_t;
}

int init_gp_loop(gp_loop *loop)
{
	gp_loop_update_time(loop);
	loop->timer_base = NULL;
	loop->timer_base = create_gp_timer_base(&loop->timer_base, loop->time);
	loop->watchers = NULL;
	loop->nfds = 0;
	loop->nwatchers = 0;
	loop->stop_flag = 0;
	GP_LIST_INIT(&loop->watcher_list, gp_io, watcher_node);
	GP_LIST_INIT(&loop->pending_list, gp_io, pending_node);
}

int gp_loop_run(gp_loop *loop, gp_run_mode mode)
{
	int ran_pending;
	unsigned long timeout;
	while(loop->stop_flag == 0){
		gp_loop_update_time(loop); 
		gp_loop_run_timers(loop);
		timeout = 0;
		if((mode == GP_RUN_ONCE && !ran_pending) || mode == GP_RUN_DEFAULT)
			timeout = loop->timer_base->next_timer - loop->timer_base;

		gp_io_poll(loop, timeout);
		
		if(mode == GP_RUN_ONCE) {
			gp_update_time(loop);
			gp_loop_run_timers(loop);
		}
		if(mode == GP_RUN_ONCE || mode == GP_RUN_NOWAIT)
			break;
	}

	if(loop->stop_flag != 0)
		loop->stop_flag = 0;
}
