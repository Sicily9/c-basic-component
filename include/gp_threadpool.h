#ifndef __GP_THREADPOOL_H__
#define __GP_THREADPOOL_H__

#include "gp_list.h"
#include "gp_thread.h"

typedef void (*gp_cb)(void *);
struct gp_taskq;

struct gp_task {
	struct gp_list_node      task_node;
	void *	     		 task_arg;
	gp_cb	     		 task_cb;
	struct gp_taskq		 task_tq;
	struct gp_thread_manager *task_tmr;
	unsigned		 task_busy;
	int			 task_prep;
	int			 task_reap;
	struct gp_mtx		 task_mtx;
	struct gp_cv	         task_cv;
};

struct gp_taskq_tmr {
	struct gp_taskq * tqt_tq;
	struct gp_thread_manager tqt_thread;
};

struct gp_taskq {
	struct gp_list	     tq_tasks;
	struct gp_mtx	     tq_mtx;
	struct gp_cv	     tq_sched_cv;
	struct gp_cv	     tq_wait_cv;
	struct gp_taskq_tmr *tq_threads;
	int		     tq_nthreads;
	int		     tq_run;
};


#endif
