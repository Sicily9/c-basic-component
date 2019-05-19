#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gp_threadpool.h"
#include "task.pb-c.h"


void func1(void *arg)
{
	Name *a = arg;
	printf("%s\n", a->name);
}

int main()
{
	Name a;
	name__init(&a);

	printf("class name: %s\n", a.base.descriptor->name);

	a.name = malloc(10);
	strcpy(a.name, "hello");
	
	gp_task_processor *gp_tp;
	create_task_processor(&gp_tp,2);
	destroy_task_processor(gp_tp);

	gp_task_processor *gp_tp2;
	create_task_processor(&gp_tp2,2);

	gp_task *task;
	create_task(&task, gp_tp2, func1, &a);
	run_task(task);

	destroy_task(task);
	destroy_task_processor(gp_tp2);

	free(a.name);
	return 0;
}
