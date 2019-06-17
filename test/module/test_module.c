#include "gp.h"

void early_init_test(void){
	printf("early_init_test\n");
}

void init_test(void){
	printf("init_test\n");
}
gp_module_desc test_module = {
	.name = "test",
	.type = MODULE_INIT_FIRST,
	.init = init_test,
	.early_init = early_init_test,
};

gp_module_init(test_module);


int main()
{
	printf("main...\n");
	return 0;
}
