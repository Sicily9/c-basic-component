#include <stdlib.h>
#include "gp_threadpool.h"

void gp_task_thread(void *arg)
{
	gp_taskp_tmr *tp_tmr = arg;
	gp_task_processor *gp_tp = tp_tmr->tpt_tp;
	gp_task *task;

	gp_mtx_lock(&gp_tp->tp_mtx);
	for(;;){
		if((task = gp_list_first(&gp_tp->tp_tasks)) != NULL) {

			gp_mtx_lock(&task->task_mtx);
			gp_list_remove(&gp_tp->tp_tasks, task);
			task->task_tmr = &tp_tmr->tpt_thread;
			gp_mtx_unlock(&task->task_mtx);

			gp_mtx_unlock(&gp_tp->tp_mtx);
			
			task->task_cb(task->task_arg);

			gp_mtx_lock(&task->task_mtx);
			task->task_busy--;
			task->task_tmr = NULL;
			if(task->task_busy == 0) {
				gp_cv_wake(&task->task_cv);
			}
			gp_mtx_unlock(&task->task_mtx);

			gp_mtx_lock(&gp_tp->tp_mtx);
			
			continue;
		}

		if(!gp_tp->tp_run) {
			break;
		}
		gp_cv_wait(&gp_tp->tp_sched_cv);
	}
	gp_mtx_unlock(&gp_tp->tp_mtx);
}

void create_task_processor(gp_task_processor **gp_tp, int num)
{
	gp_task_processor *task_processor = (gp_task_processor *)malloc(sizeof(gp_task_processor));
	memset(task_processor, 0, sizeof(gp_task_processor));
	init_task_processor(task_processor, num);
	*gp_tp = task_processor;
}

void init_task_processor(gp_task_processor *gp_tp, int num)
{
	gp_tp->tp_threads = malloc(num*sizeof(struct gp_taskp_tmr));
	gp_tp->tp_nthreads= num;
	GP_LIST_INIT(&gp_tp->tp_tasks, gp_task, task_node);

	init_gp_mtx(&gp_tp->tp_mtx);
	init_gp_cv(&gp_tp->tp_sched_cv, &gp_tp->tp_mtx);
	init_gp_cv(&gp_tp->tp_wait_cv, &gp_tp->tp_mtx);

	for(int i = 0; i < num; i++){
		gp_tp->tp_threads[i].tpt_tp = gp_tp;
		init_gp_thr_manager(&gp_tp->tp_threads[i].tpt_thread, gp_task_thread,&gp_tp->tp_threads[i]);
	}

	gp_tp->tp_run = 1;

	for(int i = 0; i < num; i++){
		gp_tmr_run(&gp_tp->tp_threads[i].tpt_thread);
	}
}

void destroy_task_processor(gp_task_processor *gp_tp)
{
	if(gp_tp == NULL){
		return;
	}
	if(gp_tp->tp_run) {
		gp_mtx_lock(&gp_tp->tp_mtx);
		gp_tp->tp_run = 0;
		gp_cv_wake(&gp_tp->tp_sched_cv);
		gp_mtx_unlock(&gp_tp->tp_mtx);
	}
	for(int i = 0; i < gp_tp->tp_nthreads; i++){
		gp_tmr_finish(&gp_tp->tp_threads[i].tpt_thread);
	}
	destroy_gp_cv(&gp_tp->tp_wait_cv);
	destroy_gp_cv(&gp_tp->tp_sched_cv);
	destroy_gp_mtx(&gp_tp->tp_mtx);
	
	free(gp_tp->tp_threads);
	free(gp_tp);
}

void create_task(gp_task **tsk, gp_task_processor *gp_tp, gp_cb cb, void *arg)
{
	gp_task *task;
	task = malloc(sizeof(gp_task));
	init_task(task, gp_tp, cb, arg);
	*tsk = task;
}

void init_task(gp_task *task, gp_task_processor *gp_tp, gp_cb cb, void *arg)
{
	GP_LIST_NODE_INIT(&task->task_node);
	init_gp_mtx(&task->task_mtx);
	init_gp_cv(&task->task_cv, &task->task_mtx);
	
	task->task_busy = 0;
	task->task_cb = cb;
	task->task_arg = arg;
	task->task_tp = gp_tp;
}

void destroy_task(gp_task *task)
{
	gp_mtx_lock(&task->task_mtx);

	while(task->task_busy) {
		gp_cv_wait(&task->task_cv);
	}
	gp_mtx_unlock(&task->task_mtx);
	destroy_gp_cv(&task->task_cv);
	destroy_gp_mtx(&task->task_mtx);
	free(task);
}

void run_task(gp_task *task)
{
	gp_task_processor *gp_tp = task->task_tp;
	if(task->task_cb == NULL) {
		return;
	}

	gp_mtx_lock(&task->task_mtx);
	task->task_busy++;
	gp_mtx_unlock(&task->task_mtx);

	gp_mtx_lock(&gp_tp->tp_mtx);
	gp_list_append(&gp_tp->tp_tasks, task);
	gp_cv_signal(&gp_tp->tp_sched_cv);
	gp_mtx_unlock(&gp_tp->tp_mtx);
}

void wait_task(gp_task *task)
{
	gp_mtx_lock(&task->task_mtx);
	while (task->task_busy) {
		gp_cv_wait(&task->task_cv);
	}
	gp_mtx_lock(&task->task_mtx);

}









