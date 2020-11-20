#include "gp.h"
#include <sys/epoll.h>
#include <poll.h>


int k_new = -1;
int k_added = 1;
int k_deleted = 2;

int32_t poll(gp_epoller *epoller, int32_t timeout_ms, event_list* active_handlers)
{
	int32_t num_events = epoll_wait(epoller->_epollfd, epoller->_events, epoller->_events_len, timeout_ms);

	int32_t saved_errno = errno;
	if(num_events > 0){
		fill_active_channels(epoller, num_events, active_handlers);
		//TODO:连接超过1024时的处理
	}else if (num_events == 0){
		//
	}else{
		if (savedErrno != EINTR){
			char *msg = strerror(savedErrno);
		}
	}
}

void update_handler(gp_epoller *epoll, gp_handler *handler)
{
	int32_t index = handler->index;

	if(index == k_new || index == k_deleted){
		int fd = handler->fd;
		if(index == k_new){
			
		}else {
			
		}
		update(EPOLL_CTL_ADD, handler);
		handler->index = k_added;
	}
	if(is_none_event(handler)) {
		update(EPOLL_CTL_DEL, handler);
		handler->index = k_deleted;
	}else{
		update(EPOLL_CTL_MOD, handler);
	}
}

void remove_handler(gp_epoller *epoller, gp_handler* handler)
{
	int32_t fd = handler->fd;

	int32_t index = handler->index;
	//erase
	
	if(index == k_added){
		update(EPOLL_CTL_DEL, handler);
	}

	handler->index = k_new;
}


void fill_active_channels(gp_epoller *epoller, int32_t num_events, handler_list *active_handlers)
{
	for (int i = 0; i < num_events; ++i){
		gp_handler *handler = (gp_handler *)(epoller->_events[i].data.ptr);

		int32_t fd = handler->fd;

		set_revents(handler, epoller->_events[i].events);

		//add to _active_handlers 

	}

}

void update(gp_epoller* epoller, int operation, gp_handler *handler)
{
	struct epoll_event event;
	bzero(&event, sizeof event);
	event.events = handler->_events;
	event.data.ptr = handler;
	int32_t fd = handler->fd;

	if(epoll_ctl(_epollfd, operation, fd, &event) < 0){
	
	}

}

