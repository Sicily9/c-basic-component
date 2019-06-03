#ifndef __GP_LOOP_H__
#define __GP_LOOP_H__

#include "gp_list.h"
#include "gp_thread.h"

struct gp_loop_s;
struct gp_io_s;

typedef void (*gp_io_cb)(struct gp_loop_s *loop, struct gp_io_s *w, unsigned int events);

typedef enum {
	GP_RUN_DEFAULT = 0,
	GP_RUN_ONCEM,
	GP_RUN_NOWAIT
}gp_run_mode;

typedef struct gp_io_s {
	gp_io_cb cb;
	gp_list pending_list;
	gp_list watcher_list;
	unsigned int pevents;
	unsigned int events;
	int fd;
}gp_io;


typedef struct gp_loop_s {
	void * data;
	unsigned int active_handles;
	gp_list	handle_list;
	union {
		void *unused[2];
		unsigned int count;
	}active_reqs;
	unsigned int stop_flags;
	unsigned long flags;
	int backend_fd;
	void *pending_list[2];
	void *watcher_list[2];
	gp_io **watchers;
	unsigned int nwatchers;
	unsigned int nfds;
}gp_loop;

extern void init_gp_io(gp_io **w, gp_io_cb cb, int fd);
extern void gp_io_stop(gp_loop *loop, gp_io *w, unsigned int events);
extern void gp_io_start(gp_loop *loop, gp_io *w, unsigned int events);
extern int  gp_io_poll(gp_loop *loop, int timeout);

extern int init_gp_loop(gp_loop **loop);
extern int gp_loop_alive(gp_loop *loop);
extern int gp_loop_run(gp_loop *loop, gp_run_mode mode);


#endif

