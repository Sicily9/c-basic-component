#include "gp.h"
#include <unistd.h>

void gp_thread_1(void * arg)
{
	printf("thread1: thread start\n");
	printf("thread1: thread finish\n");
	sleep(10);
}

int main()
{
	gp_thread_manager *gp_tmr;
	create_gp_thr_manager(&gp_tmr, gp_thread_1, NULL);
	printf("main: create the thread\n");
	gp_tmr_run(gp_tmr);
	printf("main: run the thread\n");
	printf("main: wait thread finish...\n");
	gp_tmr_wait(gp_tmr);
	printf("main: thread has been finished, continue\n");
	
	destroy_gp_thr_manager(gp_tmr);
	printf("main: destroy the thread\n");
	return 0;
}

