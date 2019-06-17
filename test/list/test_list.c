#include "gp.h"
#include <unistd.h>


struct student{
	int id;
	gp_list_node st_node;
};

int main()
{
	gp_list st_list;

	GP_LIST_INIT(&st_list, struct student, st_node);

	struct student a;
	a.id = 1;
	GP_LIST_NODE_INIT(&a.st_node);

	struct student b;
	b.id = 2;
	GP_LIST_NODE_INIT(&b.st_node);

	struct student c;
	c.id = 3;
	GP_LIST_NODE_INIT(&c.st_node);

	gp_list_append(&st_list, &a);
	gp_list_append(&st_list, &b);
	gp_list_append(&st_list, &c);

	struct student *tmp;
	GP_LIST_FOREACH(&st_list, tmp){
		printf("id:%d\n", tmp->id);
	}
	return 0;
}

