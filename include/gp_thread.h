#ifndef __GP_THREAD_H__
#define __GP_THREAD_H__

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define gp_tmr_finish(m) \
do{\
	(m)->ops->finish(m);\
}while(0) \

#define gp_tmr_wait(m) \
do{\
	(m)->ops->wait(m);\
}while(0) \

#define gp_tmr_run(m) \
do{\
	(m)->ops->run(m);\
}while(0) \

#define gp_mtx_lock(m) \
do{\
	int rv;\
	if((rv = pthread_mutex_lock(&((m)->mtx))) != 0){ \
		printf("pthread_mutex_lock :%s", strerror(rv));\
	} \
}while(0) \

#define gp_mtx_unlock(m) \
do{ \
	int rv;\
	if((rv = pthread_mutex_unlock(&((m)->mtx))) != 0){ \
		printf("pthread_mutex_unlock :%s", strerror(rv));\
	} \
}while(0) \

#define gp_cv_signal(c) \
do { \
	int rv;\
	if((rv = pthread_cond_signal(&((c)->cv))) != 0){ \
		printf("pthread_cond_signal :%s", strerror(rv));\
	} \
}while(0) \

#define gp_cv_wait(c) \
do { \
	int rv;\
	if((rv = pthread_cond_wait(&((c)->cv), &((c)->mtx->mtx))) != 0){ \
		printf("pthread_cond_wait :%s", strerror(rv));\
	} \
}while(0) \

#define gp_cv_wake(c) \
do { \
	int rv;\
	if((rv = pthread_cond_broadcast(&((c)->cv)) != 0)){ \
		printf("pthread_cond_broadcast :%s", strerror(rv));\
	} \
}while(0) \

typedef void (*gp_thread_fn)(void *);


typedef struct gp_thread_manager_ops{
	void (*run)(void *);
	void (*wait)(void *);
	void (*finish)(void *);
}gp_thread_mgr_operations;

struct gp_mtx {
	pthread_mutex_t mtx;
};

struct gp_cv {
	pthread_cond_t cv;
	struct gp_mtx * mtx;
};

struct gp_thread {
	pthread_t id;
	gp_thread_fn gp_thr_fn;
	void *arg;
};

struct gp_thread_manager {
	struct gp_thread  rthread;
	struct gp_mtx mtx;
	struct gp_cv cv;
	gp_thread_fn gp_tmr_fn;
	void *arg;
	int start;
	int stop;
	int done;
	int init;

	gp_thread_mgr_operations *ops;
};

/*API*/
extern void create_gp_thr_manager(struct gp_thread_manager **gp_tmr, gp_thread_fn fn, void *arg);
extern int init_gp_thr_manager(struct gp_thread_manager *gp_tmr, gp_thread_fn fn, void *arg);
extern void destroy_gp_thr_manager(struct gp_thread_manager *gp_tmr);

extern int init_gp_thr(struct gp_thread *gp_thr, gp_thread_fn fn, void *arg);

extern void init_gp_mtx(struct gp_mtx *mtx);
extern void destroy_gp_mtx(struct gp_mtx *mtx);

extern void init_gp_cv(struct gp_cv *cv, struct gp_mtx *mtx);
extern void destroy_gp_cv(struct gp_cv *cv);

extern gp_thread_mgr_operations gp_tmr_ops;
#endif

