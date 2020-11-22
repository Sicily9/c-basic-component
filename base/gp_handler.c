#include "gp.h"
#include <poll.h>


int knone_event = 0; int kread_event = POLLIN | POLLPRI;
int kwrite_event = POLLOUT;

static void update(gp_handler *handler){
	handler->add_to_loop = 1;
	gp_loop_update_handler(handler->loop, handler);
}

void init_gp_handler(gp_handler *tmp, gp_loop *loop, int fd)
{
	tmp->loop = loop;
	tmp->fd = fd;
	tmp->index = -1;
	tmp->_events = 0;
	tmp->_revents = 0;
	tmp->event_handling = 0;
	tmp->add_to_loop = 0;
	GP_LIST_NODE_INIT(&tmp->handler_node);
}

void create_gp_handler(gp_handler **handler, gp_loop *loop, int fd){
	gp_handler *tmp = malloc(sizeof(gp_handler));
	memset(tmp, 0, sizeof(*tmp));
	init_gp_handler(tmp, loop, fd);
	*handler = tmp;
}

void set_read_callback(gp_handler *handler, gp_event_callback cb){
	handler->_read_callback = cb;	
}

void set_write_callback(gp_handler *handler, gp_event_callback cb){
	handler->_write_callback = cb;	
}

void set_close_callback(gp_handler *handler, gp_event_callback cb){
	handler->_close_callback = cb;	
}

void set_error_callback(gp_handler *handler, gp_event_callback cb){
	handler->_error_callback = cb;	
}

void set_revents(gp_handler *handler, int revt){
	handler->_revents = revt;
	update(handler);
}

void enable_reading(gp_handler *handler){
	handler->_events |= kread_event;
	update(handler);
}

void enable_writing(gp_handler *handler){
	handler->_events |= kwrite_event;
	update(handler);
}

void disable_reading(gp_handler *handler){
	handler->_events &= ~kread_event;
	update(handler);
}

void disable_writing(gp_handler *handler){
	handler->_events &= ~kwrite_event;
	update(handler);
}

void disable_all(gp_handler *handler){
	handler->_events = knone_event;
	update(handler);
}

int8_t is_writing(gp_handler *handler){
	return handler->_events & kwrite_event;
}

int8_t is_reading(gp_handler *handler){
	return handler->_events & kread_event;
}

int8_t is_none_event(gp_handler *handler){
	return handler->_events == knone_event;
}

void handle_event(gp_handler *handler){
	handler->event_handling = 1;

	if (unlikely((handler->_revents & POLLHUP) && !(handler->_revents & POLLIN))){
		if(handler->_close_callback)
			handler->_close_callback(handler);
	}

	if (unlikely(handler->_revents & (POLLERR | POLLNVAL))){
		if(handler->_error_callback)
			handler->_error_callback(handler);
	}

	if ( handler->_revents & (POLLIN | POLLPRI | POLLRDHUP)){
		if(handler->_read_callback)
			handler->_read_callback(handler);
	}

	if ( handler->_revents & POLLOUT){
		if(handler->_write_callback)
			handler->_write_callback(handler);
	}

	handler->event_handling = 0;
}

void handler_remove(gp_handler *handler)
{
	handler->add_to_loop = 0;
	gp_loop_remove_handler(handler->loop, handler);
}



