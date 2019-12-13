#include "gp.h"
#include <unistd.h>

void gp_thread_1(void * arg)
{
	printf("thread1: thread start\n");
	printf("thread1: thread finish\n");
	sleep(1000);
}
// for testing struct pthread (glibc pthread struct). tid and kernel pid
typedef union dtv 
{
  size_t counter;
  struct
  {
    void *val;
    char is_static;
  } pointer;
} dtv_t;

typedef struct
{
  int i[4];
} __128bits;


typedef struct
{
  void *tcb;        /* Pointer to the TCB.  Not necessarily the
               thread descriptor used by libpthread.  */
  dtv_t *dtv;
  void *self;       /* Pointer to the thread descriptor.  */
  int multiple_threads;
  int gscope_flag;
  uintptr_t sysinfo;
  uintptr_t stack_guard;
  uintptr_t pointer_guard;
  unsigned long int vgetcpu_cache[2];
  int __unused1;
  int rtld_must_xmm_save;
  void *__private_tm[4];
  void *__private_ss;
  long int __unused2;
  __128bits n[8][4] __attribute__ ((aligned (32)));

  void *__padding[8];
} tcbhead_t;


struct pthread_fake{
		tcbhead_t t;
		void *nothing[2];
		pid_t tid;
};
int main()
{
	gp_thread_manager *gp_tmr;
	create_gp_thr_manager(&gp_tmr, gp_thread_1, NULL);
	printf("main tid:%d and create the thread %d\n", ((struct pthread_fake *)(pthread_self()))->tid, ((struct pthread_fake *)(gp_tmr->rthread.id))->tid);
	gp_tmr_run(gp_tmr);
	printf("main: run the thread\n");
	printf("main: wait thread finish...\n");
	gp_tmr_wait(gp_tmr);
	printf("main: thread has been finished, continue\n");
	
	destroy_gp_thr_manager(gp_tmr);
	printf("main: destroy the thread\n");
	return 0;
}

