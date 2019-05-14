#ifndef GP_LIST_H
#define GP_LIST_H

#include <stdint.h>

//doubly list
typedef struct gp_list_node {
        struct gp_list_node *ln_next;
        struct gp_list_node *ln_prev;
} gp_list_node;

typedef struct gp_list {
        struct gp_list_node ll_head; // head node of the list
        size_t              ll_offset; // the relative locationof gp_list_node member in the node
} gp_list;

extern void gp_list_init_offset(gp_list *list, size_t offset);

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define GP_LIST_INIT(list, type, field) \
        gp_list_init_offset(list, offsetof(type, field))

#define GP_LIST_NODE_INIT(node)                       \
        {                                              \
                (node)->ln_prev = (node)->ln_next = 0; \
        }

extern void *gp_list_first(const gp_list *); 
extern void *gp_list_last(const gp_list *); 
extern void  gp_list_append(struct gp_list *, void *); 
extern void  gp_list_prepend(struct gp_list *, void *); 
extern void  gp_list_insert_before(struct gp_list *, void *, void *); 
extern void  gp_list_insert_after(struct gp_list *, void *, void *); 
extern void *gp_list_next(const struct gp_list *, void *); 
extern void *gp_list_prev(const struct gp_list *, void *); 
extern void  gp_list_remove(struct gp_list *, void *); 
extern int   gp_list_active(struct gp_list *, void *); 
extern int   gp_list_empty(struct gp_list *); 
extern int   gp_list_node_active(struct gp_list_node *); 

#define GP_LIST_FOREACH(l, it) \
        for (it = gp_list_first(l); it != NULL; it = gp_list_next(l, it))

#endif // GP_LIST_H

