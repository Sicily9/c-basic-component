#include "gp_module.h"


static gp_list first_list;
static void __attribute__((constructor)) init_moudle_list(void)
{
    GP_LIST_INIT(&first_list, gp_module_desc, node);
}


static void gp_init_module(gp_list *list)
{
	gp_module_desc *pos;
	GP_LIST_FOREACH(list, pos){
		if(pos->init){
			pos->stat.init = 1;
			pos->init();
		}
	}

}

static void gp_register_module_main(gp_list* list, gp_module_desc *module)
{
	gp_module_desc *pos;
	GP_LIST_FOREACH(list, pos){
		if(pos->priority > module->priority) {
			gp_list_insert_after(list, module, pos);
			return;
		}
	}
	gp_list_append(list, module);
}

void gp_register_module(gp_module_desc * module)
{
	switch(module->type){
	case MODULE_INIT_FIRST:
		gp_register_module_main(&first_list, module);
		break;
	default:
		break;
	}

}

int gp_init_modules(void)
{
    gp_init_module(&first_list);
    return 0;
}


