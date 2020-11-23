#include <inttypes.h>
#include <errno.h>
#include "gp.h"

/** Maximum size of a complete domain name. */
#define NODE_NAME_MAX 1024

static gp_conf_node *root = NULL;
static gp_conf_node *root_backup = NULL;

size_t strlcat(dst, src, siz)                                                         
    char *dst;
    const char *src;
    size_t siz;
{
    register char *d = dst;
    register const char *s = src;
    register size_t n = siz;
    size_t dlen; 

    /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != '\0')
        d++;
    dlen = d - dst;
    n = siz - dlen;

    if (n == 0)
        return(dlen + strlen(s));
    while (*s != '\0') {
        if (n != 1) {
            *d++ = *s;
            n--;
        }       
        s++;
    }       
    *d = '\0';

    return(dlen + (s - src)); /* count does not include NUL */
}
size_t strlcpy(dst, src, siz)
    char *dst;
    const char *src;
    size_t siz;
{
    register char *d = dst;
    register const char *s = src;
    register size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0) {
        do {
            if ((*d++ = *s++) == 0)
                break;
        } while (--n != 0);
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0)
            *d = '\0'; /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return(s - src - 1); /* count does not include NUL */
}


static gp_conf_node *gp_conf_get_node_or_create(const char *name, int final)
		{
    gp_conf_node *parent = root;
    gp_conf_node *node = NULL;
    char node_name[NODE_NAME_MAX];
    char *key;
    char *next;

    if (strlcpy(node_name, name, sizeof(node_name)) >= sizeof(node_name)) {
        return NULL;
    }

    key = node_name;

    do {
        if ((next = strchr(key, '.')) != NULL)
            *next++ = '\0';
        if ((node = gp_conf_node_lookup_child(parent, key)) == NULL) {
            node = gp_conf_node_new();
            if (unlikely(node == NULL)) {
                goto end;
            }
            node->name = strdup(key);
            if (unlikely(node->name == NULL)) {
                gp_conf_node_free(node);
                node = NULL;
                goto end;
            }
            node->parent = parent;
            node->final = final;
            TAILQ_INSERT_TAIL(&parent->head, node, next);
        }
        key = next;
        parent = node;
    } while (next != NULL);

end:
    return node;
}

void gp_conf_init(void)
{
    if (root != NULL) {
        return;
    }
    root = gp_conf_node_new();
    if (root == NULL) {
        exit(EXIT_FAILURE);
    }
}

gp_conf_node *gp_conf_node_new(void)
{
    gp_conf_node *new;

    new = calloc(1, sizeof(*new));
    if (unlikely(new == NULL)) {
        return NULL;
    }
    TAILQ_INIT(&new->head);

    return new;
}

void gp_conf_node_free(gp_conf_node *node)
{
    gp_conf_node *tmp;

    while ((tmp = TAILQ_FIRST(&node->head))) {
        TAILQ_REMOVE(&node->head, tmp, next);
        gp_conf_node_free(tmp);
    }

    if (node->name != NULL)
        free(node->name);
    if (node->val != NULL)
        free(node->val);
    free(node);
}

int gp_conf_get_siblings_num(gp_conf_node * node)
{
    int i = 0;
	gp_conf_node *child;
    TAILQ_FOREACH(child, &node->head, next) { 			
        i++;
    }   
	return i;
}

gp_conf_node *gp_conf_get_node_in_array(const char *head_name, int i, const char *name)
{
    gp_conf_node *node = root;
    char tmp_name[NODE_NAME_MAX];
    char node_name[NODE_NAME_MAX];
    char *key;
    char *next;

	snprintf(tmp_name, NODE_NAME_MAX, "%s.%d.%s", head_name, i, name);

    if (strlcpy(node_name, tmp_name, sizeof(node_name)) >= sizeof(node_name)) {
        return NULL;
    }

    key = node_name;

    do {
        if ((next = strchr(key, '.')) != NULL)
            *next++ = '\0';
        node = gp_conf_node_lookup_child(node, key);
        key = next;
    } while (next != NULL && node != NULL);

    return node;
}

/**
 * \brief Get a gp_conf_node by name.
 *
 * \param name The full name of the configuration node to lookup.
 *
 * \retval A pointer to gp_conf_node is found or NULL if the configuration
 *    node does not exist.
 */
gp_conf_node *gp_conf_get_node(const char *name)
{
    gp_conf_node *node = root;
    char node_name[NODE_NAME_MAX];
    char *key;
    char *next;

    if (strlcpy(node_name, name, sizeof(node_name)) >= sizeof(node_name)) {
        return NULL;
    }

    key = node_name;
    do {
        if ((next = strchr(key, '.')) != NULL)
            *next++ = '\0';
        node = gp_conf_node_lookup_child(node, key);
        key = next;
    } while (next != NULL && node != NULL);

    return node;
}

gp_conf_node *gp_conf_get_root_node(void)
{
    return root;
}

int gp_conf_set(const char *name, const char *val)
{
    gp_conf_node *node = gp_conf_get_node_or_create(name, 0);
    if (node == NULL || node->final) {
        return 0;
    }
    if (node->val != NULL)
        free(node->val);
    node->val = strdup(val);
    if (unlikely(node->val == NULL)) {
        return 0;
    }
    return 1;
}

int gp_conf_set_from_string(const char *input, int final)
{
    int retval = 0;
    char *name = strdup(input), *val = NULL;
    if (unlikely(name == NULL)) {
        goto done;
    }
    val = strchr(name, '=');
    if (val == NULL) {
        goto done;
    }
    *val++ = '\0';

    while (isspace((int)name[strlen(name) - 1])) {
        name[strlen(name) - 1] = '\0';
    }

    while (isspace((int)*val)) {
        val++;
    }

    if (final) {
        if (!gp_conf_set_final(name, val)) {
            goto done;
        }
    }
    else {
        if (!gp_conf_set(name, val)) {
            goto done;
        }
    }

    retval = 1;
done:
    if (name != NULL) {
        free(name);
    }
    return retval;
}

int gp_conf_set_final(const char *name, const char *val)
{
    gp_conf_node *node = gp_conf_get_node_or_create(name, 1);
    if (node == NULL) {
        return 0;
    }
    if (node->val != NULL)
        free(node->val);
    node->val = strdup(val);
    if (unlikely(node->val == NULL)) {
        return 0;
    }
    node->final = 1;
    return 1;
}

int gp_conf_get(const char *name, const char **vptr)
{
    gp_conf_node *node = gp_conf_get_node(name);
    if (node == NULL) {
        return 0;
    }
    else {
        *vptr = node->val;
        return 1;
    }
}

int gp_conf_get_value(const char *name, const char **vptr)
{
    gp_conf_node *node;

    if (name == NULL) {
        return -2;
    }

    node = gp_conf_get_node(name);

    if (node == NULL) {
        return 0;
    }
    else {

        if (node->val == NULL) {
            return -1;
        }

        *vptr = node->val;
        return 1;
    }

}

int gp_conf_get_child_value(const gp_conf_node *base, const char *name, const char **vptr)
{
    gp_conf_node *node = gp_conf_node_lookup_child(base, name);

    if (node == NULL) {
        return 0;
    }
    else {
        *vptr = node->val;
        return 1;
    }
}

gp_conf_node *gp_conf_get_child_with_default(const gp_conf_node *base, const gp_conf_node *dflt,
    const char *name)
{
    gp_conf_node *node = gp_conf_node_lookup_child(base, name);
    if (node != NULL)
        return node;

    /* Get 'default' value */
    if (dflt) {
        return gp_conf_node_lookup_child(dflt, name);
    }
    return NULL;
}

int gp_conf_get_child_value_with_default(const gp_conf_node *base, const gp_conf_node *dflt,
    const char *name, const char **vptr)
{
    int ret = gp_conf_get_child_value(base, name, vptr);
    /* Get 'default' value */
    if (ret == 0 && dflt) {
        return gp_conf_get_child_value(dflt, name, vptr);
    }
    return ret;
}

int gp_conf_get_int(const char *name, int *val)
{
    const char *strval = NULL;
    intmax_t tmpint;
    char *endptr;

    if (gp_conf_get(name, &strval) == 0){	
		printf("name:%s\n",name);
        return 0;
	}

    if (strval == NULL) {
        return 0;
    }

    errno = 0;
    tmpint = strtoimax(strval, &endptr, 0);
	printf("%s",strval);
    if (strval[0] == '\0' || *endptr != '\0') {
        return 0;
    }
    if (errno == ERANGE && (tmpint == INTMAX_MAX || tmpint == INTMAX_MIN)) {
        return 0;
    }

    *val = tmpint;
    return 1;
}

int gp_conf_get_child_value_int(const gp_conf_node *base, const char *name, intmax_t *val)
{
    const char *strval = NULL;
    intmax_t tmpint;
    char *endptr;

    if (gp_conf_get_child_value(base, name, &strval) == 0)
        return 0;
    errno = 0;
    tmpint = strtoimax(strval, &endptr, 0);
    if (strval[0] == '\0' || *endptr != '\0') {
        return 0;
    }
    if (errno == ERANGE && (tmpint == INTMAX_MAX || tmpint == INTMAX_MIN)) {
        return 0;
    }

    *val = tmpint;
    return 1;

}

int gp_conf_get_child_value_int_with_default(const gp_conf_node *base, const gp_conf_node *dflt,
    const char *name, intmax_t *val)
{
    int ret = gp_conf_get_child_value_int(base, name, val);
    /* Get 'default' value */
    if (ret == 0 && dflt) {
        return gp_conf_get_child_value_int(dflt, name, val);
    }
    return ret;
}

int gp_conf_get_bool(const char *name, int *val)
{
    const char *strval = NULL;

    *val = 0;
    if (gp_conf_get_value(name, &strval) != 1)
        return 0;

    *val = gp_conf_val_is_true(strval);

    return 1;
}

int gp_conf_get_child_value_bool(const gp_conf_node *base, const char *name, int *val)
{
    const char *strval = NULL;

    *val = 0;
    if (gp_conf_get_child_value(base, name, &strval) == 0)
        return 0;

    *val = gp_conf_val_is_true(strval);

    return 1;
}

int gp_conf_get_child_value_bool_with_default(const gp_conf_node *base, const gp_conf_node *dflt,
    const char *name, int *val)
{
    int ret = gp_conf_get_child_value_bool(base, name, val);
    /* Get 'default' value */
    if (ret == 0 && dflt) {
        return gp_conf_get_child_value_bool(dflt, name, val);
    }
    return ret;
}


int gp_conf_val_is_true(const char *val)
{
    const char *trues[] = {"1", "yes", "true", "on"};
    size_t u;

    for (u = 0; u < sizeof(trues) / sizeof(trues[0]); u++) {
        if (strcasecmp(val, trues[u]) == 0) {
            return 1;
        }
    }

    return 0;
}

/**
 * \brief Check if a value is false.
 *
 * The value is considered false if it is a string with the value of 0,
 * no, false or off.  The test is not case sensitive, any other value
 * is not false.
 *
 * \param val The string to test for a false value.
 *
 * \retval 1 If the value is false, 0 if not.
 */
int gp_conf_val_is_false(const char *val)
{
    const char *falses[] = {"0", "no", "false", "off"};
    size_t u;

    for (u = 0; u < sizeof(falses) / sizeof(falses[0]); u++) {
        if (strcasecmp(val, falses[u]) == 0) {
            return 1;
        }
    }

    return 0;
}

int gp_conf_get_double(const char *name, double *val)
{
    const char *strval = NULL;
    double tmpdo;
    char *endptr;

    if (gp_conf_get(name, &strval) == 0)
        return 0;

    errno = 0;
    tmpdo = strtod(strval, &endptr);
    if (strval[0] == '\0' || *endptr != '\0')
        return 0;
    if (errno == ERANGE)
        return 0;

    *val = tmpdo;
    return 1;
}

int gp_conf_get_float(const char *name, float *val)
{
    const char *strval = NULL;
    double tmpfl;
    char *endptr;

    if (gp_conf_get(name, &strval) == 0)
        return 0;

    errno = 0;
    tmpfl = strtof(strval, &endptr);
    if (strval[0] == '\0' || *endptr != '\0')
        return 0;
    if (errno == ERANGE)
        return 0;

    *val = tmpfl;
    return 1;
}

void gp_conf_node_remove(gp_conf_node *node)
{
    if (node->parent != NULL)
        TAILQ_REMOVE(&node->parent->head, node, next);
    gp_conf_node_free(node);
}

int gp_conf_remove(const char *name)
{
    gp_conf_node *node;

    node = gp_conf_get_node(name);
    if (node == NULL)
        return 0;
    else {
        gp_conf_node_remove(node);
        return 1;
    }
}

void gp_conf_create_context_backup(void)
{
    root_backup = root;
    root = NULL;

    return;
}

void gp_conf_restore_context_backup(void)
{
    root = root_backup;
    root_backup = NULL;

    return;
}

void gp_conf_deinit(void)
{
    if (root != NULL) {
        gp_conf_node_free(root);
        root = NULL;
    }

}

static char *gp_conf_print_name_array(char **name_arr, int level)
{
    static char name[128*128];
    int i;

    name[0] = '\0';
    for (i = 0; i <= level; i++) {
        strlcat(name, name_arr[i], sizeof(name));
        if (i < level)
            strlcat(name, ".", sizeof(name));
    }

    return name;
}

/**
 * \brief Dump a configuration node and all its children.
 */
void gp_conf_node_dump(const gp_conf_node *node, const char *prefix)
{
    gp_conf_node *child;

    static char *name[128];
    static int level = -1;

    level++;
    TAILQ_FOREACH(child, &node->head, next) {
        name[level] = strdup(child->name);
        if (unlikely(name[level] == NULL)) {
            continue;
        }
        if (prefix == NULL) {
            printf("%s = %s\n", gp_conf_print_name_array(name, level),
                child->val);
        }
        else {
            printf("%s.%s = %s\n", prefix,
                gp_conf_print_name_array(name, level), child->val);
        }
        gp_conf_node_dump(child, prefix);
        free(name[level]);
    }
    level--;
}

/**
 * \brief Dump configuration to stdout.
 */
void gp_conf_dump(void)
{
    gp_conf_node_dump(root, NULL);
}

/**
 * \brief Check if a node has any children.
 *
 * Checks if the provided node has any children. Any node that is a
 * YAML map or array will have children.
 *
 * \param node The node to check.
 *
 * \retval true if node has children
 * \retval false if node does not have children
 */
int gp_conf_node_has_children(const gp_conf_node *node)
{
    if (TAILQ_EMPTY(&node->head)) {
        return 0;
    }
    return 1;
}

/**
 * \brief Lookup a child configuration node by name.
 *
 * Given a gp_conf_node this function will lookup an immediate child
 * gp_conf_node by name and return the child gp_conf_node.
 *
 * \param node The parent configuration node.
 * \param name The name of the child node to lookup.
 *
 * \retval A pointer the child gp_conf_node if found otherwise NULL.
 */
gp_conf_node *gp_conf_node_lookup_child(const gp_conf_node *node, const char *name)
{
    gp_conf_node *child;

    if (node == NULL || name == NULL) {
        return NULL;
    }

    TAILQ_FOREACH(child, &node->head, next) {
        if (child->name != NULL && strcmp(child->name, name) == 0)
            return child;
    }

    return NULL;
}

/**
 * \brief Lookup the value of a child configuration node by name.
 *
 * Given a parent gp_conf_node this function will return the value of a
 * child configuration node by name returning a reference to that
 * value.
 *
 * \param node The parent configuration node.
 * \param name The name of the child node to lookup.
 *
 * \retval A pointer the child gp_conf_nodes value if found otherwise NULL.
 */
const char *gp_conf_node_lookup_child_value(const gp_conf_node *node, const char *name)
{
    gp_conf_node *child;

    child = gp_conf_node_lookup_child(node, name);
    if (child != NULL)
        return child->val;

    return NULL;
}

/**
 * \brief Lookup for a key value under a specific node
 *
 * \return the gp_conf_node matching or NULL
 */

gp_conf_node *gp_conf_node_lookup_key_value(const gp_conf_node *base, const char *key,
    const char *value)
{
    gp_conf_node *child;

    TAILQ_FOREACH(child, &base->head, next) {
        if (!strncmp(child->val, key, strlen(child->val))) {
            gp_conf_node *subchild;
            TAILQ_FOREACH(subchild, &child->head, next) {
                if ((!strcmp(subchild->name, key)) && (!strcmp(subchild->val, value))) {
                    return child;
                }
            }
        }
    }

    return NULL;
}

/**
 * \brief Test if a configuration node has a true value.
 *
 * \param node The parent configuration node.
 * \param name The name of the child node to test.
 *
 * \retval 1 if the child node has a true value, otherwise 0 is
 *     returned, even if the child node does not exist.
 */
int gp_conf_node_child_value_is_true(const gp_conf_node *node, const char *key)
{
    const char *val;

    val = gp_conf_node_lookup_child_value(node, key);

    return val != NULL ? gp_conf_val_is_true(val) : 0;
}

char *gp_conf_load_complete_include_path(const char *file)
{
    const char *defaultpath = NULL;
    char *path = NULL;

    /* Path not specified */
    if (!( strlen(file) > 1 && file[0] == '/')) {
        if (gp_conf_get("include-path", &defaultpath) == 1) {
            size_t path_len = sizeof(char) * (strlen(defaultpath) +
                          strlen(file) + 2);
            path = malloc(path_len);
            if (unlikely(path == NULL))
                return NULL;
            strlcpy(path, defaultpath, path_len);
            if (path[strlen(path) - 1] != '/')
                strlcat(path, "/", path_len);
            strlcat(path, file, path_len);
       } else {
            path = strdup(file);
            if (unlikely(path == NULL))
                return NULL;
        }
    } else {
        path = strdup(file);
        if (unlikely(path == NULL))
            return NULL;
    }
    return path;
}

void gp_conf_node_prune(gp_conf_node *node)
{
    gp_conf_node *item, *it;

    for (item = TAILQ_FIRST(&node->head); item != NULL; item = it) {
        it = TAILQ_NEXT(item, next);
        if (!item->final) {
            gp_conf_node_prune(item);
            if (TAILQ_EMPTY(&item->head)) {
                TAILQ_REMOVE(&node->head, item, next);
                if (item->name != NULL)
                    free(item->name);
                if (item->val != NULL)
                    free(item->val);
                free(item);
            }
        }
    }

    if (node->val != NULL) {
        free(node->val);
        node->val = NULL;
    }
}

/**
 * \brief Check if a node is a sequence or node.
 *
 * \param node the node to check.
 *
 * \return 1 if node is a seuence, otherwise 0.
 */
int gp_conf_node_is_sequence(const gp_conf_node *node)
{
    return node->is_seq == 0 ? 0 : 1;
}

