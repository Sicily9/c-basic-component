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
	conn_ref_inc(&task->conn);
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
				conn_ref_inc(&task->conn);
            	task->pending_func(NULL, task->conn, task->msg, task->len);
				conn_ref_dec(&task->conn);
            	break;
        	}
        	case GP_RUN_IN_LOOP_CONN:
        	{
				printf("run in loop conn_pending_task, fd:%d\n", task->conn->fd);
				conn_ref_inc(&task->conn);
            	task->pending_func(NULL, task->conn, NULL, 0);
				conn_ref_dec(&task->conn);
            	break;
        	}
        	case GP_RUN_IN_LOOP_REMOVE_CONN:
        	{
				conn_ref_inc(&task->conn);
            	task->pending_func(task->tcp_server, task->conn, NULL, 0);
				conn_ref_dec(&task->conn);
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
		destruct_gp_pending_task(task);
	}else{
		gp_queue_in_loop(loop, task);
	}
}

static void do_pending_functors(gp_loop *loop)
{
	loop->calling_pending_functors = 1;
	gp_pending_task *task = NULL;
	gp_pending_task *tmp = NULL;
	GP_LIST_FOREACH_SAFE(&loop->pending_list, tmp, task)
	{
    	switch(task->type)
    	{
        	case GP_RUN_IN_LOOP_TRANS:
        	{
				printf("trans_pending_task, send msg\n");
            	task->pending_func(NULL, task->conn, task->msg, task->len);
				conn_ref_dec(&task->conn);
            	break;
        	}
        	case GP_RUN_IN_LOOP_CONN:
        	{
				printf("conn_pending_task, fd:%d\n", task->conn->fd);
            	task->pending_func(NULL, task->conn, NULL, 0);
				conn_ref_dec(&task->conn);
            	break;
        	}
        	case GP_RUN_IN_LOOP_REMOVE_CONN:
        	{
				printf("remove conn pending_task, fd:%d\n", task->conn->fd);
            	task->pending_func(task->tcp_server, task->conn, NULL, 0);
				conn_ref_dec(&task->conn);
            	break;
        	}
        	case GP_RUN_IN_LOOP_SERVER_START:
        	{
				printf("conn_server_start\n");
            	task->pending_func(task->tcp_server, NULL, NULL, 0);
            	break;
        	}
        	default:
        	{
				break;
        	}
    	}
		destruct_gp_pending_task(task);
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
	printf("wakeup fd: %d\n", loop->wakeup_fd);
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
	gp_handler *tmp2;

	loop->looping = 1;
	loop->quit 	  = 0;
	while(loop->quit == 0){
		gp_loop_run_timers(loop);
		timeout = 0;

		if((mode == GP_RUN_ONCE) || mode == GP_RUN_DEFAULT)
			timeout = loop->timer_base->next_timer - loop->time;

		poller_poll(loop->epoller, timeout, &loop->active_handler_list);
		gp_loop_update_time(loop);

		GP_LIST_FOREACH_SAFE(&loop->active_handler_list, tmp2, tmp){
			handle_event(tmp);
			gp_list_node_remove(&tmp->handler_node);
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
