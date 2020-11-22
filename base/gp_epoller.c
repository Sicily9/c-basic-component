#include "gp.h"
#include <sys/epoll.h>
#include <poll.h>


int k_new = -1;
int k_added = 1;
int k_deleted = 2;

void init_gp_epoller(gp_epoller *epoller)
{
	epoller->epollfd    = epoll_create1(EPOLL_CLOEXEC);
	epoller->events_len = 1024;
	epoller->events     = malloc(epoller->events_len * sizeof(struct epoll_event));
	epoller->handlers   = dictCreate(INT_DICT, 0);
}

void create_gp_epoller(gp_epoller **epoller)
{
	gp_epoller *tmp = (gp_epoller *)malloc(sizeof(gp_epoller));
	memset(tmp, 0, sizeof(*tmp));
	init_gp_epoller(tmp);
	*epoller = tmp;
}

static void poller_update(gp_epoller* epoller, int operation, gp_handler *handler)
{
	struct epoll_event event;
	bzero(&event, sizeof event);
	event.events = handler->_events;
	event.data.ptr = handler;
	int32_t fd = handler->fd;

	if(epoll_ctl(epoller->epollfd, operation, fd, &event) < 0){
	
	}

}

static void fill_active_handlers(gp_epoller *epoller, int32_t num_events, event_list *active_handlers)
{
	for (int i = 0; i < num_events; ++i){
		gp_handler *handler = (gp_handler *)(epoller->events[i].data.ptr);

		//int32_t fd = handler->fd;

		set_revents(handler, epoller->events[i].events);

		//add to _active_handlers 
	//	gp_list_append(active_handlers, handler);
	}

}

void update_handler(gp_epoller *epoller, gp_handler *handler)
{
	int32_t index = handler->index;

	if(index == k_new || index == k_deleted){
		int fd = handler->fd;
		if(index == k_new){
			dictAdd(epoller->handlers, (void *)&fd, handler);
		}else {
			
		}
		poller_update(epoller, EPOLL_CTL_ADD, handler);
		handler->index = k_added;
	}else{
		if(is_none_event(handler)) {
			poller_update(epoller, EPOLL_CTL_DEL, handler);
			handler->index = k_deleted;
		}else{
			poller_update(epoller, EPOLL_CTL_MOD, handler);
		}
	}
}

void poller_poll(gp_epoller *epoller, int32_t timeout_ms, event_list* active_handlers)
{
	int32_t num_events = epoll_wait(epoller->epollfd, epoller->events, epoller->events_len, timeout_ms);

	int32_t saved_errno = errno;
	if(num_events > 0){
		fill_active_handlers(epoller, num_events, active_handlers);
		//TODO:连接超过1024时的处理
	}else if (num_events == 0){
		//
	}else{
		if (saved_errno != EINTR){
			char *msg = strerror(saved_errno);
		}
	}
}


void remove_handler(gp_epoller *epoller, gp_handler* handler)
{
	int32_t fd = handler->fd;

	int32_t index = handler->index;
	dictDelete(epoller->handlers, &fd);
	if(index == k_added){
		poller_update(epoller, EPOLL_CTL_DEL, handler);
	}

	handler->index = k_new;
}



