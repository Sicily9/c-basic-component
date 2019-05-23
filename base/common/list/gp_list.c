#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gp_list.h"

#define NODE(list, item) \
        (gp_list_node *) (void *)(((char *) item) + list->ll_offset)
#define ITEM(list, node) (void *) (((char *) node) - list->ll_offset)

void gp_list_init_offset(gp_list *list, size_t offset)
{
        list->ll_offset       = offset;
        list->ll_head.ln_next = &list->ll_head;
        list->ll_head.ln_prev = &list->ll_head;
}

void * gp_list_first(const gp_list *list)
{
        gp_list_node *node = list->ll_head.ln_next;

        if (node == &list->ll_head) {
                return (NULL);
        }
        return (ITEM(list, node));
}

void * gp_list_last(const gp_list *list)
{
        gp_list_node *node = list->ll_head.ln_prev;

        if (node == &list->ll_head) {
                return (NULL);
        }
        return (ITEM(list, node));
}

void gp_list_append(gp_list *list, void *item)
{
        gp_list_node *node = NODE(list, item);

        if ((node->ln_next != NULL) || (node->ln_prev != NULL)) {
                printf("appending node already on a list or not inited\n");
        }
        node->ln_prev          = list->ll_head.ln_prev;
        node->ln_next          = &list->ll_head;
        node->ln_next->ln_prev = node;
        node->ln_prev->ln_next = node;
}


void gp_list_prepend(gp_list *list, void *item)
{
        gp_list_node *node = NODE(list, item);

        if ((node->ln_next != NULL) || (node->ln_prev != NULL)) {
                printf("prepending node already on a list or not inited\n");
        }
        node->ln_next          = list->ll_head.ln_next;
        node->ln_prev          = &list->ll_head;
        node->ln_next->ln_prev = node;
        node->ln_prev->ln_next = node;
}

void gp_list_insert_before(gp_list *list, void *item, void *before)
{
        gp_list_node *node  = NODE(list, item);
        gp_list_node *where = NODE(list, before);

        if ((node->ln_next != NULL) || (node->ln_prev != NULL)) {
                printf("inserting node already on a list or not inited\n");
        }
        node->ln_next          = where;
        node->ln_prev          = where->ln_prev;
        node->ln_next->ln_prev = node;
        node->ln_prev->ln_next = node;
}

void gp_list_insert_after(gp_list *list, void *item, void *after)
{
        gp_list_node *node  = NODE(list, item);
        gp_list_node *where = NODE(list, after);

        if ((node->ln_next != NULL) || (node->ln_prev != NULL)) {
                printf("inserting node already on a list or not inited\n");
        }
        node->ln_prev          = where;
        node->ln_next          = where->ln_next;
        node->ln_next->ln_prev = node;
        node->ln_prev->ln_next = node;
}

void * gp_list_next(const gp_list *list, void *item)
{
        gp_list_node *node = NODE(list, item);

        if ((node = node->ln_next) == &list->ll_head) {
                return (NULL);
        }
        return (ITEM(list, node));
}

void * gp_list_prev(const gp_list *list, void *item)
{
        gp_list_node *node = NODE(list, item);

        if ((node = node->ln_prev) == &list->ll_head) {
                return (NULL);
        }
        return (ITEM(list, node));
}

void gp_list_remove(gp_list *list, void *item)
{
        gp_list_node *node = NODE(list, item);

        node->ln_prev->ln_next = node->ln_next;
        node->ln_next->ln_prev = node->ln_prev;
        node->ln_next          = NULL;
        node->ln_prev          = NULL;
}

int gp_list_active(gp_list *list, void *item)
{
        gp_list_node *node = NODE(list, item);

        return (node->ln_next == NULL ? 0 : 1);
}

int gp_list_empty(gp_list *list)
{
        // The first check ensures that we treat an uninitialized list
        // as empty.  This use useful for statically initialized lists.
        return ((list->ll_head.ln_next == NULL) ||
            (list->ll_head.ln_next == &list->ll_head));
}

int gp_list_node_active(gp_list_node *node)
{
        return (node->ln_next == NULL ? 0 : 1);
}

void gp_list_node_remove(gp_list_node *node)
{
        if (node->ln_next != NULL) {
                node->ln_prev->ln_next = node->ln_next;
                node->ln_next->ln_prev = node->ln_prev;
                node->ln_next          = NULL;
                node->ln_prev          = NULL;
        }
}

