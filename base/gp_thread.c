#include <signal.h>
#include "gp.h"

static void gp_tmr_wrap(void *arg)
{
	gp_thread_manager * gp_tmr = arg;
	int start;

	gp_mtx_lock(&gp_tmr->mtx);
	while(((start = gp_tmr->start) == 0) && (gp_tmr->stop == 0)) {
		gp_cv_wait(&gp_tmr->cv);
	}
	gp_mtx_unlock(&gp_tmr->mtx);
	
	if((start) && (gp_tmr->gp_tmr_fn != NULL)){
		gp_tmr->gp_tmr_fn(gp_tmr->arg);
	}
	
	gp_mtx_lock(&gp_tmr->mtx);
	gp_tmr->done = 1;
	gp_cv_wake(&gp_tmr->cv);
	gp_mtx_unlock(&gp_tmr->mtx);
	
}

static void * gp_thread_main(void *arg)
{
	pthread_detach(pthread_self());

	gp_thread * gp_thr = arg;
	sigset_t	set;

	sigemptyset(&set);
	sigaddset(&set, SIGPIPE);
	(void) pthread_sigmask(SIG_BLOCK, &set, NULL);

	gp_thr->gp_thr_fn(gp_thr->arg);

	return NULL;
}

void create_gp_thr_manager(gp_thread_manager **gp_tmr, gp_thread_fn fn, void *arg)
{
	gp_thread_manager * tmr = malloc(sizeof(gp_thread_manager));
	init_gp_thr_manager(tmr, fn, arg);
	*gp_tmr = tmr;
}

int init_gp_thr_manager(gp_thread_manager *gp_tmr, gp_thread_fn fn, void *arg)
{
	int rv;
	gp_tmr->done = 0;
	gp_tmr->start = 0;
	gp_tmr->stop = 0;
	gp_tmr->gp_tmr_fn = fn;
	gp_tmr->arg = arg;
	gp_tmr->ops = &gp_tmr_ops;

	init_gp_mtx(&gp_tmr->mtx);
	init_gp_cv(&gp_tmr->cv, &gp_tmr->mtx);

	if(fn == NULL) {
		gp_tmr->init = 1;
		gp_tmr->done = 1;
		return -1;
	}
	if((rv = init_gp_thr(&gp_tmr->rthread, gp_tmr_wrap, gp_tmr) != 0)){
		gp_tmr->done = 1;
		destroy_gp_cv(&gp_tmr->cv);
		destroy_gp_mtx(&gp_tmr->mtx);
		return rv;
	}
	gp_tmr->init = 1;
	return 0;

}

void destroy_gp_thr_manager(gp_thread_manager *gp_tmr)
{
	gp_tmr_finish(gp_tmr);
	free(gp_tmr);
}

void gp_thr_manager_wait(void *gp_thr_manager)
{
	gp_thread_manager *gp_tmr = gp_thr_manager;

	if(!gp_tmr->init){
		return;
	}
	gp_mtx_lock(&gp_tmr->mtx);
	gp_tmr->stop = 1;
	gp_cv_wake(&gp_tmr->cv);
	while(!gp_tmr->done){
		gp_cv_wait(&gp_tmr->cv);
	}
	gp_mtx_unlock(&gp_tmr->mtx);
}

void gp_thr_manager_run(void *gp_thr_manager)
{
	gp_thread_manager *gp_tmr = gp_thr_manager;

	gp_mtx_lock(&gp_tmr->mtx);
	gp_tmr->start = 1;
	gp_cv_wake(&gp_tmr->cv);
	gp_mtx_unlock(&gp_tmr->mtx);
}

void gp_thr_manager_finish(void *gp_thr_manager)
{
	gp_thread_manager *gp_tmr = gp_thr_manager;

	if(!gp_tmr->init){
		return;
	}
	gp_mtx_lock(&gp_tmr->mtx);
	gp_tmr->stop = 1;
	gp_cv_wake(&gp_tmr->cv);
	while(!gp_tmr->done){
		gp_cv_wait(&gp_tmr->cv);
	}
	gp_mtx_unlock(&gp_tmr->mtx);

	destroy_gp_cv(&gp_tmr->cv);
	destroy_gp_mtx(&gp_tmr->mtx);
	gp_tmr->init = 0;
}

int init_gp_thr(gp_thread *gp_thr, gp_thread_fn fn, void *arg)
{
	int rv;

	gp_thr->gp_thr_fn = fn;
	gp_thr->arg = arg; //arg is  gp_tmr

	rv = pthread_create(&gp_thr->id, NULL, gp_thread_main, gp_thr);
	if(rv != 0) {
		return 1;
	}
	return 0;
}

void init_gp_mtx(gp_mtx *mtx)
{
	pthread_mutex_init(&mtx->mtx, NULL);
}

void destroy_gp_mtx(gp_mtx *mtx)
{
	(void) pthread_mutex_destroy(&mtx->mtx);
}
	
void init_gp_cv(gp_cv *cv, gp_mtx *mtx)
{
	pthread_cond_init(&cv->cv, NULL);
	cv->mtx = mtx;
}

void destroy_gp_cv(gp_cv *cv)
{
	pthread_cond_destroy(&cv->cv);
	cv->mtx = NULL;
}

gp_thread_mgr_operations gp_tmr_ops = {
	.run = gp_thr_manager_run,
	.wait = gp_thr_manager_wait,
	.finish = gp_thr_manager_finish,
};
