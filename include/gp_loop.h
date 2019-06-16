#ifndef __GP_LOOP_H__
#define __GP_LOOP_H__

#include "gp_list.h"
#include "gp_thread.h"
#include "gp_timer.h"

struct gp_loop_s;
struct gp_io_s;

typedef void (*gp_io_cb)(struct gp_loop_s *loop, struct gp_io_s *w, unsigned int events);

typedef enum {
	GP_RUN_DEFAULT = 0,
	GP_RUN_ONCE,
	GP_RUN_NOWAIT
}gp_run_mode;

typedef struct gp_io_s {
	gp_io_cb cb;
	gp_list_node pending_node;
	gp_list_node watcher_node;
	unsigned int pevents;
	unsigned int events;
	int fd;
}gp_io;

//TODO: add handle base class
typedef struct gp_loop_s {
//	unsigned int active_handles;
//	gp_list	handle_list;
//	unsigned int count;//active_request count
	unsigned int stop_flags;
	unsigned long flags;
	int backend_fd;
	gp_list pending_list;
	gp_list watcher_list;
	gp_io **watchers;
	unsigned long time;
	gp_timer_base *timer_base;
	unsigned int nwatchers;
	unsigned int nfds;
}gp_loop;

extern void create_gp_io(gp_io **w, gp_io_cb cb, int fd);
extern void init_gp_io(gp_io *w, gp_io_cb cb, int fd);
extern void gp_io_stop(gp_loop *loop, gp_io *w, unsigned int events);
extern void gp_io_start(gp_loop *loop, gp_io *w, unsigned int events);
extern void gp_io_poll(gp_loop *loop, unsigned long timeout);


extern int create_gp_loop(gp_loop **loop);
extern int init_gp_loop(gp_loop *loop);
//extern int gp_loop_alive(gp_loop *loop);
extern int gp_loop_run(gp_loop *loop, gp_run_mode mode);

extern void gp_loop_timer_start(gp_loop *loop, void (*fn)(void *), void *data, unsigned long expires);
extern void gp_loop_timer_stop(gp_loop *loop, gp_timer_list *timer);
extern void gp_loop_timer_mod(gp_loop *loop, gp_timer_list *timer, unsigned long expires);
extern void gp_loop_run_timers(gp_loop *loop);
extern void gp_loop_update_time(gp_loop *loop);

#endif

