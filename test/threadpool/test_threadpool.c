#include "gp.h"
#include "proto.h"


void func2(void *arg)
{
	Name *a = arg;
	printf("thread id:%lu b class name: %s\n",pthread_self(), a->base.descriptor->name);
}

void func1(void *arg)
{
	Name *a = arg;
	printf("thread id:%lu a class name: %s\n",pthread_self(), a->base.descriptor->name);
}

int main()
{
	printf("main thread:%lu \n", pthread_self());
	Name a;
	name__init(&a);
	a.name = malloc(10);
	strcpy(a.name, "i am a");
	
	unsigned char *c =NULL;
	int size = encode((ProtobufCMessage *)&a, &c);
	
	ProtobufCMessage * p = decode("Name", size, c);
	printf("name :%s\n", ((Name *)p)->name);

	free(c);

	Name b;
	name__init(&b);
	a.name = malloc(10);
	strcpy(a.name, "i am b");
	
	/*
	gp_task_processor *gp_tp;
	create_task_processor(&gp_tp,2);
	destroy_task_processor(gp_tp);
	*/

	gp_task_processor *gp_tp2;
	create_task_processor(&gp_tp2,2);

	gp_task *task;
	create_task(&task, gp_tp2, func1, &a);
	run_task(task);

	gp_task *task2;
	create_task(&task2, gp_tp2, func2, &b);
	run_task(task2);

	destroy_task(task);
	destroy_task(task2);

	destroy_task_processor(gp_tp2);

	free(a.name);
	return 0;
}
