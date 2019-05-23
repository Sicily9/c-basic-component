#include "gp_list.h"

enum gp_module_init_type {
	MODULE_INIT_FIRST,
};

typedef struct gp_module_desc {
	gp_list_node node;
	int type;

	int priority;
	char *name;
	void (*init)(void);
	void (*early_init)(void);
	//void (*destroy)(void);

	struct {
		int init;
		char *file;
	}stat;
}gp_module_desc;


extern void gp_register_module(gp_module_desc *desc);


#define gp_module_init(module) \
static void __attribute__((constructor)) do_vs_init_ ## module(void) {  \
    	if(module.early_init){ \
		module.early_init(); \
	} \
	GP_LIST_NODE_INIT(&module.node) \
	module.stat.file = __FILE__;        \
    	gp_register_module(&module);    \
}

extern int gp_init_modules(void);

//extern void gp_cleanup_modules(void);


