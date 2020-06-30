#ifndef __CONF_H__
#define __CONF_H__

#include "queue.h"

/**
 * Structure of a gp_configuration parameter.
 */
typedef struct gp_conf_node_ {
    char *name;
    char *val;

    int is_seq;

    /**< Flag that sets this nodes value as final. */
    int final;

    struct gp_conf_node_ *parent;
    TAILQ_HEAD(, gp_conf_node_) head;
    TAILQ_ENTRY(gp_conf_node_) next;
} gp_conf_node;


void gp_conf_init(void);
void gp_conf_deinit(void);
gp_conf_node *gp_conf_get_root_node(void);
int gp_conf_get(const char *name, const char **vptr);
int gp_conf_get_value(const char *name, const char **vptr);
int gp_conf_get_Int(const char *name, intmax_t *val);
int gp_conf_get_bool(const char *name, int *val);
int gp_conf_get_double(const char *name, double *val);
int gp_conf_get_float(const char *name, float *val);
int gp_conf_set(const char *name, const char *val);
int gp_conf_set_from_string(const char *input, int final);
int gp_conf_set_final(const char *name, const char *val);
void gp_conf_dump(void);
void gp_conf_node_dump(const gp_conf_node *node, const char *prefix);
gp_conf_node *gp_conf_node_new(void);
void gp_conf_node_free(gp_conf_node *);
gp_conf_node *gp_conf_get_node(const char *key);
void gp_conf_create_context_backup(void);
void gp_conf_restore_context_backup(void);
gp_conf_node *gp_conf_node_lookup_child(const gp_conf_node *node, const char *key);
const char *gp_conf_node_lookup_child_value(const gp_conf_node *node, const char *key);
void gp_conf_node_remove(gp_conf_node *);
void gp_conf_register_tests(void);
int gp_conf_node_child_value_is_true(const gp_conf_node *node, const char *key);
int gp_conf_val_is_true(const char *val);
int gp_conf_val_is_false(const char *val);
void gp_conf_node_prune(gp_conf_node *node);
int gp_conf_remove(const char *name);
bool gp_conf_node_has_children(const gp_conf_node *node);

gp_conf_node *gp_conf_get_child_with_default(const gp_conf_node *base, const gp_conf_node *dflt, const char *name);
gp_conf_node *gp_conf_node_lookup_key_value(const gp_conf_node *base, const char *key, const char *value);
int gp_conf_get_child_value(const gp_conf_node *base, const char *name, const char **vptr);
int gp_conf_get_child_value_int(const gp_conf_node *base, const char *name, intmax_t *val);
int gp_conf_get_child_value_bool(const gp_conf_node *base, const char *name, int *val);
int gp_conf_get_child_value_with_default(const gp_conf_node *base, const gp_conf_node *dflt, const char *name, const char **vptr);
int gp_conf_get_child_value_int_with_default(const gp_conf_node *base, const gp_conf_node *dflt, const char *name, intmax_t *val);
int gp_conf_get_child_value_bool_with_default(const gp_conf_node *base, const gp_conf_node *dflt, const char *name, int *val);
char *gp_conf_load_complete_include_path(const char *);
int gp_conf_node_is_sequence(const gp_conf_node *node);

#endif /* ! __CONF_H__ */
