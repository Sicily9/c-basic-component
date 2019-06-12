#ifndef __GP_TIMER_H__
#define __GP_TIMER_H__

#include "gp_list.h"

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS) 
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)
#define MAX_TVAL ((unsigned long)((1ULL << (TVR_BITS + 4*TVN_BITS)) - 1))

#define INDEX(C,N) (((C)->timer_jiffies >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)


typedef enum {
	GP_CLOCK_PRECISE = 0, GP_CLOCK_FAST = 1
}gp_clocktype_t;

typedef struct tvec_t {
	gp_list vec[TVN_SIZE];
}tvec;

typedef struct tvec_root_t {
	gp_list vec[TVR_SIZE];
}tvec_root;

typedef struct gp_timer_base_t {
	unsigned long timer_jiffies;
	unsigned long next_timer;
	tvec_root tv1;
	tvec tv2;
	tvec tv3;
	tvec tv4;
	tvec tv5;
}gp_timer_base;

typedef struct gp_timer_list_t {
	gp_list_node node;
	unsigned long expires;
	gp_timer_base *base;
	void (*callback)(void *);
	void *data;
	unsigned int state;
}gp_timer_list;

extern void create_gp_timer(gp_timer_list **timer, void (*fn)(void *), void *data);
extern void init_gp_timer(gp_timer_list *timer, void (*fn)(void *), void *data);
extern void gp_timer_add(gp_timer_base *base, gp_timer_list *timer, unsigned long expires);
extern void gp_timer_del(gp_timer_base *base, gp_timer_list *timer);
extern void gp_timer_mod(gp_timer_base *base, gp_timer_list *timer, unsigned long expires);
extern void destruct_gp_timer(gp_timer_list *timer);


extern void create_gp_timer_base(gp_timer_base **base, unsigned long jiffies);
extern void init_gp_timer_base(gp_timer_base *base, unsigned long jiffies);
extern void gp_run_timers(gp_timer_base *base, unsigned long jiffies);
extern void destruct_gp_timer_base(gp_timer_base *base);

extern unsigned long gp_time(gp_clocktype_t);

#endif
