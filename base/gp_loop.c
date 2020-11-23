#include "gp.h"
#include <errno.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <stdarg.h>
/*-----------------------------timer------------------------------------*/ 
void gp_loop_timer_start(gp_loop *loop, void (*fn)(void *), void *data, int32_t interval, int32_t repeat)
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
void gp_loop_timer_mod(gp_loop *loop, gp_timer_list *timer, uint32_t expires, int32_t interval, int32_t repeat)
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

#if 0
void gp_io_poll(gp_loop *loop, uint32_t timeout)
{
	static const int32_t max_safe_timeout = MAX_TVAL;
 	struct epoll_event events[1024];
  	struct epoll_event* pe;
  	struct epoll_event e;
  	int32_t real_timeout;
  	gp_io* w, *tmp;
  	uint32_t base;
  	int32_t nevents;
  	int32_t count;
  	int32_t nfds;
  	int32_t fd;
  	int32_t op;
  	int32_t i;

  	if (loop->nfds == 0) { 
    		return;
  	}

  	memset(&e, 0, sizeof(e));

	//take out the epoll event from the loop watcher list and insert into
	//the rb-tree
	GP_LIST_FOREACH_SAFE(&loop->watcher_list, tmp, w){
		gp_list_node_remove(&w->watcher_node);
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
				   sizeof(events)/sizeof(events[0]),
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
	}

    loop->watchers[loop->nwatchers] = NULL;
    loop->watchers[loop->nwatchers + 1] = NULL;

	printf("hahaha timeout:%u, nfds:%d\n", timeout, nfds);

    if (nevents != 0) {
      	if (nfds == 1024 && --count != 0) {
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
#endif

/*-------------------------------------gp_loop-------------------------------------*/

static int32_t create_eventfd(void){
	int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0)
	{
		abort();
	}
	return evtfd;
}

static void handler_wakeup_read(gp_handler *handler)
{
	uint64_t one = 1;
	ssize_t n = read(handler->fd, &one, sizeof one);
	if (n != sizeof one)
	{

	}
}

static void wakeup(int32_t fd)
{
	uint64_t one = 1;
	ssize_t n = write(fd, &one, sizeof one);
	if (n != sizeof one)
	{

	}
}

void wakeup_handler_read_callback(gp_handler *handler)
{
	handler_wakeup_read(handler);
}

int32_t create_gp_loop(gp_loop **loop)
{
	gp_loop *loop_t = malloc(sizeof(gp_loop));
	memset(loop_t, 0, sizeof(*loop_t));
	init_gp_loop(loop_t);
	*loop = loop_t;
	return 0;
}

int8_t is_in_loop_thread(gp_loop *loop)
{
	return loop->tid == (int64_t)syscall(SYS_gettid);
}

void gp_queue_in_loop(gp_loop *loop, gp_pending_task *task)
{
	//TODO:多线程 per loop per thread时 得上锁
	gp_list_append(&loop->pending_list, task);

	if(!is_in_loop_thread(loop) || loop->calling_pending_functors)
	{
		wakeup(loop->wakeup_fd);
	}
}

void gp_run_in_loop(gp_loop *loop, gp_pending_task *task)
{
	if(is_in_loop_thread(loop)){
    	switch(task->type)
    	{
        	case GP_RUN_IN_LOOP_TRANS:
        	{
            	task->pending_func(NULL, task->conn, task->msg, task->len);
            	break;
        	}
        	case GP_RUN_IN_LOOP_CONN:
        	{
            	task->pending_func(NULL, task->conn, NULL, 0);
            	break;
        	}
        	case GP_RUN_IN_LOOP_REMOVE_CONN:
        	{
            	task->pending_func(task->tcp_server, task->conn, NULL, 0);
            	break;
        	}
        	case GP_RUN_IN_LOOP_SERVER_START:
        	{
            	task->pending_func(task->tcp_server, NULL, NULL, 0);
            	break;
        	}
        	default:
        	{
				break;
        	}
    	}
	}else{
		gp_queue_in_loop(loop, task);
	}
}

static void do_pending_functors(gp_loop *loop)
{
	loop->calling_pending_functors = 1;
	gp_pending_task *task = NULL;
	GP_LIST_FOREACH(&loop->pending_list, task)
	{
    	switch(task->type)
    	{
        	case GP_RUN_IN_LOOP_TRANS:
        	{
            	task->pending_func(NULL, task->conn, task->msg, task->len);
            	break;
        	}
        	case GP_RUN_IN_LOOP_CONN:
        	{
            	task->pending_func(NULL, task->conn, NULL, 0);
            	break;
        	}
        	case GP_RUN_IN_LOOP_REMOVE_CONN:
        	{
            	task->pending_func(task->tcp_server, task->conn, NULL, 0);
            	break;
        	}
        	case GP_RUN_IN_LOOP_SERVER_START:
        	{
            	task->pending_func(task->tcp_server, NULL, NULL, 0);
            	break;
        	}
        	default:
        	{
				break;
        	}
    	}
	}
	loop->calling_pending_functors = 0;
}

void gp_loop_update_handler(gp_loop *loop, gp_handler *handler)
{
	update_handler(loop->epoller, handler);
}

void gp_loop_remove_handler(gp_loop *loop, gp_handler *handler)
{
	remove_handler(loop->epoller, handler);
}

void gp_loop_quit(gp_loop *loop)
{
	loop->quit = 1;
	if(!is_in_loop_thread(loop))
	{
		wakeup(loop->wakeup_fd);
	}
}

int32_t init_gp_loop(gp_loop *loop)
{
	gp_loop_update_time(loop);
	loop->timer_base = NULL;

	loop->quit = 0;
	loop->tid = syscall(SYS_gettid);
	loop->looping = 0;
	loop->calling_pending_functors = 0;

	create_gp_timer_base(&loop->timer_base, loop->time);
	create_gp_epoller(&loop->epoller);
	loop->wakeup_fd = create_eventfd();
	create_gp_handler(&loop->wakeup_handler, loop, loop->wakeup_fd);
	set_read_callback(loop->wakeup_handler, wakeup_handler_read_callback);
	enable_reading(loop->wakeup_handler);

	GP_LIST_INIT(&loop->active_handler_list, gp_handler, handler_node);
	GP_LIST_INIT(&loop->pending_list, gp_pending_task, pending_task_node);
	return 0;
}

int32_t gp_loop_run(gp_loop *loop, gp_run_mode mode)
{
	uint32_t timeout;
	gp_handler *tmp;

	loop->looping = 1;
	loop->quit 	  = 0;
	while(loop->quit == 0){
		printf("timer_base:%u time:%u\n", loop->timer_base->next_timer, loop->time);
		gp_loop_run_timers(loop);
		timeout = 0;

		if((mode == GP_RUN_ONCE) || mode == GP_RUN_DEFAULT)
			timeout = loop->timer_base->next_timer - loop->time;

		poller_poll(loop->epoller, timeout, &loop->active_handler_list);
		gp_loop_update_time(loop);

		GP_LIST_FOREACH(&loop->active_handler_list, tmp){
			handle_event(tmp);
		}
		do_pending_functors(loop);
		
		if(mode == GP_RUN_ONCE) {
			gp_loop_update_time(loop);
			gp_loop_run_timers(loop);
		}
		if(mode == GP_RUN_ONCE || mode == GP_RUN_NOWAIT)
			break;
	}
	loop->looping = 0;
	return 0;
}
