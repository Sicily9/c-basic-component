#include "gp.h"
#include <errno.h>
#include <sys/epoll.h>
/*-----------------------------timer------------------------------------*/ 
void gp_loop_timer_start(gp_loop *loop, void (*fn)(void *), void *data, int interval, int repeat)
{
	gp_timer_list *timer = NULL;
	create_gp_timer(&timer, fn, data, loop->time + interval, interval, repeat);
	timer->loop = loop;
	gp_timer_add(loop->timer_base, timer);
}


void gp_loop_timer_stop(gp_timer_list *timer)
{
	gp_timer_del(timer);

}

void gp_loop_timer_mod(gp_loop *loop, gp_timer_list *timer, unsigned long expires, int interval, int repeat)
{
	gp_timer_mod(loop->timer_base, timer, expires, interval, repeat);
}

void gp_loop_run_timers(gp_loop *loop)
{
	gp_run_timers(loop->timer_base, loop->time);
}

void gp_loop_update_time(gp_loop *loop)
{
	loop->time = gp_time(GP_CLOCK_PRECISE);
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

	if(watchers == NULL)
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
		gp_list_append(&loop->watcher_list, w);
	
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
			loop->watchers[w->fd] = NULL;
			loop->nfds--;
			w->events = 0;
		}
	}else if(gp_list_node_active(&w->watcher_node))
		gp_list_append(&loop->watcher_list, w);
}

void gp_io_poll(gp_loop *loop, unsigned long timeout)
{
	static const int max_safe_timeout = MAX_TVAL;
 	struct epoll_event events[1024];
  	struct epoll_event* pe;
  	struct epoll_event e;
  	int real_timeout;
  	gp_io* w;
  	uint64_t base;
  	int nevents;
  	int count;
  	int nfds;
  	int fd;
  	int op;
  	int i;

  	if (loop->nfds == 0) { 
    		return;
  	}

  	memset(&e, 0, sizeof(e));

	//take out the epoll event from the loop watcher list and insert into
	//the rb-tree
	GP_LIST_FOREACH(&loop->watcher_list, w){
    	e.events = w->pevents;
    	e.data.fd = w->fd;
		if (w->events == 0)
 			op = EPOLL_CTL_ADD;
    	else
      		op = EPOLL_CTL_MOD;

    	if (epoll_ctl(loop->backend_fd, op, w->fd, &e)) {
      		if (errno != EEXIST)
        		abort();

      		if (epoll_ctl(loop->backend_fd, EPOLL_CTL_MOD, w->fd, &e))
        		abort();
		}

		w->events = w->pevents;
	}

	base = loop->time;
	count = 48;
	real_timeout = timeout;

	for(;;) {
		if(sizeof(int) == sizeof(long) && timeout >= max_safe_timeout)
			timeout = max_safe_timeout;

		nfds = epoll_wait(loop->backend_fd, 
				   events, 
				   1024,
				   timeout
				   );

		gp_loop_update_time(loop);

		if (nfds == 0) { 
      			if (timeout == 0)
        			return;
      			goto update_timeout;
    		}

    		if (nfds == -1) {
      			if (errno != EINTR){
					perror("epoll_wait");
					printf("%d\n", errno);
        			abort();
				}

      			if (timeout == -1)
        			continue;

      			if (timeout == 0)
        			return;

      			goto update_timeout;
    		}

	}
	nevents = 0;
	loop->watchers[loop->nwatchers] = (void*) events;
    loop->watchers[loop->nwatchers + 1] = (void*) (uintptr_t) nfds;
    for (i = 0; i < nfds; i++) {
      	pe = events + i;
      	fd = pe->data.fd;

      	if (fd == -1)
        	continue;


 	    w = loop->watchers[fd];

  	    if (w == NULL) {
        	epoll_ctl(loop->backend_fd, EPOLL_CTL_DEL, fd, pe);
        	continue;
      	}
		
		pe->events &= w->pevents | EPOLLERR | EPOLLHUP;
		if (pe->events == EPOLLERR || pe->events == EPOLLHUP){
        	pe->events |= w->pevents & 
				(EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI);
		}

    	if (pe->events != 0) {
          	w->cb(loop, w, pe->events);
        	nevents++;
		}

    	loop->watchers[loop->nwatchers] = NULL;
    	loop->watchers[loop->nwatchers + 1] = NULL;


    	if (nevents != 0) {
      		if (nfds == sizeof(events)/sizeof(events[0]) && --count != 0) {
        		timeout = 0;
        		continue;
      		}
      		return;
    	}

    	if (timeout == 0)
      		return;

    	if (timeout == -1)
      		continue;

update_timeout:
    	real_timeout -= (loop->time - base);
    	if (real_timeout <= 0)
      		return;

    	timeout = real_timeout;
  }
}


/*-------------------------------------gp_loop-------------------------------------*/
int create_gp_loop(gp_loop **loop)
{
	gp_loop *loop_t = malloc(sizeof(gp_loop));
	memset(loop_t, 0, sizeof(*loop_t));
	init_gp_loop(loop_t);
	*loop = loop_t;
	return 0;
}

int init_gp_loop(gp_loop *loop)
{
	gp_loop_update_time(loop);
	loop->timer_base = NULL;
	create_gp_timer_base(&loop->timer_base, loop->time);
	loop->watchers = NULL;
	loop->nfds = 0;
	loop->nwatchers = 0;
	loop->stop_flags = 0;
	loop->backend_fd = -1;

	loop->backend_fd = epoll_create1(EPOLL_CLOEXEC);

	GP_LIST_INIT(&loop->watcher_list, gp_io, watcher_node);
	GP_LIST_INIT(&loop->pending_list, gp_io, pending_node);
	return 0;
}

int gp_loop_run(gp_loop *loop, gp_run_mode mode)
{
	unsigned long timeout;
	while(loop->stop_flags == 0){
		gp_loop_run_timers(loop);
		timeout = 0;

		if((mode == GP_RUN_ONCE) || mode == GP_RUN_DEFAULT)
			timeout = loop->timer_base->next_timer - loop->time;
		gp_io_poll(loop, timeout);
		
		if(mode == GP_RUN_ONCE) {
			gp_loop_update_time(loop);
			gp_loop_run_timers(loop);
		}
		if(mode == GP_RUN_ONCE || mode == GP_RUN_NOWAIT)
			break;
	}

	if(loop->stop_flags != 0)
		loop->stop_flags = 0;
	return 0;
}
