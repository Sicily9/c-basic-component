#ifndef __GP_THREADPOOL_H__
#define __GP_THREADPOOL_H__

#include "gp_list.h"
#include "gp_thread.h"

typedef void (*gp_cb)(void *);
struct gp_taskq;

typedef struct gp_task {
	struct gp_list_node      task_node;
	void *	     		 task_arg;
	gp_cb	     		 task_cb;
	struct gp_task_processor *task_tp;
	struct gp_thread_manager *task_tmr;
	unsigned		 task_busy;
	//int			 task_prep;
	//int			 task_reap;
	struct gp_mtx		 task_mtx;
	struct gp_cv	         task_cv;
}gp_task;

typedef struct gp_taskp_tmr {
	struct gp_task_processor * tpt_tp;
	struct gp_thread_manager tpt_thread;
}gp_taskp_tmr;


// a threadpool
typedef struct gp_task_processor {
	struct gp_list	     tp_tasks;
	struct gp_mtx	     tp_mtx;
	struct gp_cv	     tp_sched_cv;
	struct gp_cv	     tp_wait_cv;
	struct gp_taskp_tmr *tp_threads;
	int		     tp_nthreads;
	int		     tp_run;
} gp_task_processor;

extern void create_task_processor(gp_task_processor **gp_tp, int num);
extern void init_task_processor(gp_task_processor *gp_tp, int num);
extern void destroy_task_processor(gp_task_processor *gp_tp);

extern void create_task(gp_task ** task, gp_task_processor *gp_tp, gp_cb cb, void *arg);
extern void init_task(gp_task *task, gp_task_processor *gp_tp, gp_cb cb, void *arg);
extern void run_task(gp_task *task);
extern void wait_task(gp_task *task);
extern void destroy_task(gp_task *task);

#endif
