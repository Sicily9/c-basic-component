#include <yaml.h>
#include <sys/stat.h>
#include <limits.h>
#include "gp.h"

#define YAML_VERSION_MAJOR 1
#define YAML_VERSION_MINOR 1

/* The maximum level of recursion allowed while parsing the YAML
 * file. */
#define RECURSION_LIMIT 128

/* Sometimes we'll have to create a node name on the fly (integer
 * conversion, etc), so this is a default length to allocate that will
 * work most of the time. */
#define DEFAULT_NAME_LEN 16

#define MANGLE_ERRORS_MAX 10
static int mangle_errors = 0;

static char *conf_dirname = NULL;

static int gp_conf_yaml_parse(yaml_parser_t *parser, gp_conf_node *parent, int inseq, int rlevel);

/* gp_configuration processing states. */
enum conf_state {
    CONF_KEY = 0,
    CONF_VAL,
    CONF_INCLUDE,
};

/**
 * \brief mangle unsupported characters.
 *
 * \param string A pointer to an null terminated string.
 *
 * \retval none
 */
#if 0
static void
mangle(char *string)
{
    char *c;

    while ((c = strchr(string, '_')))
        *c = '-';

    return;
}
#endif

/**
 * \brief Set the directory name of the configuration file.
 *
 * \param filename The configuration filename.
 */
static void
gp_conf_yaml_set_conf_dirname(const char *filename)
{
    char *ep;

    ep = strrchr(filename, '\\');
    if (ep == NULL)
        ep = strrchr(filename, '/');

    if (ep == NULL) {
        conf_dirname = strdup(".");
        if (conf_dirname == NULL) {
            exit(EXIT_FAILURE);
        }
    }
    else {
        conf_dirname = strdup(filename);
        if (conf_dirname == NULL) {
            exit(EXIT_FAILURE);
        }
        conf_dirname[ep - filename] = '\0';
    }
}

/**
 * \brief Include a file in the configuration.
 *
 * \param parent The configuration node the included configuration will be
 *          placed at.
 * \param filename The filename to include.
 *
 * \retval 0 on success, -1 on failure.
 */
static int
gp_conf_yaml_handle_include(gp_conf_node *parent, const char *filename)
{
    yaml_parser_t parser;
    char include_filename[PATH_MAX];
    FILE *file = NULL;
    int ret = -1;

    if (yaml_parser_initialize(&parser) != 1) {
        return -1;
    }

    if (strlen(filename) > 1 && filename[0] == '/') {
        strlcpy(include_filename, filename, sizeof(include_filename));
    }
    else {
        snprintf(include_filename, sizeof(include_filename), "%s/%s",
            conf_dirname, filename);
    }

    file = fopen(include_filename, "r");
    if (file == NULL) {
        goto done;
    }

    yaml_parser_set_input_file(&parser, file);

    if (gp_conf_yaml_parse(&parser, parent, 0, 0) != 0) {
        goto done;
    }

    ret = 0;

done:
    yaml_parser_delete(&parser);
    if (file != NULL) {
        fclose(file);
    }

    return ret;
}

/**
 * \brief Parse a YAML layer.
 *
 * \param parser A pointer to an active yaml_parser_t.
 * \param parent The parent configuration node.
 *
 * \retval 0 on success, -1 on failure.
 */
static int
gp_conf_yaml_parse(yaml_parser_t *parser, gp_conf_node *parent, int inseq, int rlevel)
{
    gp_conf_node *node = parent;
    yaml_event_t event;
    memset(&event, 0, sizeof(event));
    int done = 0;
    int state = 0;
    int seq_idx = 0;
    int retval = 0;

    if (rlevel++ > RECURSION_LIMIT) {
        return -1;
    }

    while (!done) {
        if (!yaml_parser_parse(parser, &event)) {
            retval = -1;
            break;
        }

        if (event.type == YAML_DOCUMENT_START_EVENT) {
            /* Verify YAML version - its more likely to be a valid
             * Suricata configuration file if the version is
             * correct. */
            yaml_version_directive_t *ver =
                event.data.document_start.version_directive;
            if (ver == NULL) {
                goto fail;
            }
            int major = ver->major;
            int minor = ver->minor;
            if (!(major == YAML_VERSION_MAJOR && minor == YAML_VERSION_MINOR)) {
                goto fail;
            }
        }
        else if (event.type == YAML_SCALAR_EVENT) {
            char *value = (char *)event.data.scalar.value;
            char *tag = (char *)event.data.scalar.tag;

            /* Skip over empty scalar values while in KEY state. This
             * tends to only happen on an empty file, where a scalar
             * event probably shouldn't fire anyways. */
            if (state == CONF_KEY && strlen(value) == 0) {
                goto next;
            }

            if (inseq) {
                char sequence_node_name[DEFAULT_NAME_LEN];
                snprintf(sequence_node_name, DEFAULT_NAME_LEN, "%d", seq_idx++);
                gp_conf_node *seq_node = gp_conf_node_lookup_child(parent,
                    sequence_node_name);
                if (seq_node != NULL) {
                    /* The sequence node has already been set, probably
                     * from the command line.  Remove it so it gets
                     * re-added in the expected order for iteration.
                     */
                    TAILQ_REMOVE(&parent->head, seq_node, next);
                }
                else {
                    seq_node = gp_conf_node_new();
                    if (unlikely(seq_node == NULL)) {
                        goto fail;
                    }
                    seq_node->name = strdup(sequence_node_name);
                    if (unlikely(seq_node->name == NULL)) {
                        free(seq_node);
                        goto fail;
                    }
                    seq_node->val = strdup(value);
                    if (unlikely(seq_node->val == NULL)) {
                        free(seq_node->name);
                        goto fail;
                    }
                }
                TAILQ_INSERT_TAIL(&parent->head, seq_node, next);
            }
            else {
                if (state == CONF_INCLUDE) {
                    if (gp_conf_yaml_handle_include(parent, value) != 0) {
                        goto fail;
                    }
                    state = CONF_KEY;
                }
                else if (state == CONF_KEY) {

                    if (strcmp(value, "include") == 0) {
                        state = CONF_INCLUDE;
                        goto next;
                    }

                    if (parent->is_seq) {
                        if (parent->val == NULL) {
                            parent->val = strdup(value);
                            if (parent->val && strchr(parent->val, '_'))
							{}
                              //  mangle(parent->val);
                        }
                    }
                    gp_conf_node *existing = gp_conf_node_lookup_child(parent, value);
                    if (existing != NULL) {
                        if (!existing->final) {
                            gp_conf_node_prune(existing);
                        }
                        node = existing;
                    }
                    else {
                        node = gp_conf_node_new();
                        node->name = strdup(value);
                        if (node->name && strchr(node->name, '_')) {
                            if (!(parent->name &&
                                   ((strcmp(parent->name, "address-groups") == 0) ||
                                    (strcmp(parent->name, "port-groups") == 0)))) {
                              //  mangle(node->name);
                                if (mangle_errors < MANGLE_ERRORS_MAX) {
                              //      mangle_errors++;
                                }
                            }
                        }
                        TAILQ_INSERT_TAIL(&parent->head, node, next);
                    }
                    state = CONF_VAL;
                }
                else {
                    if ((tag != NULL) && (strcmp(tag, "!include") == 0)) {
                        if (gp_conf_yaml_handle_include(node, value) != 0)
                            goto fail;
                    }
                    else if (!node->final) {
                        if (node->val != NULL)
                            free(node->val);
                        node->val = strdup(value);
                    }
                    state = CONF_KEY;
                }
            }
        }
        else if (event.type == YAML_SEQUENCE_START_EVENT) {
            if (gp_conf_yaml_parse(parser, node, 1, rlevel) != 0)
                goto fail;
            node->is_seq = 1;
            state = CONF_KEY;
        }
        else if (event.type == YAML_SEQUENCE_END_EVENT) {
            done = 1;
        }
        else if (event.type == YAML_MAPPING_START_EVENT) {
            if (inseq) {
                char sequence_node_name[DEFAULT_NAME_LEN];
                snprintf(sequence_node_name, DEFAULT_NAME_LEN, "%d", seq_idx++);
                gp_conf_node *seq_node = gp_conf_node_lookup_child(node,
                    sequence_node_name);
                if (seq_node != NULL) {
                    /* The sequence node has already been set, probably
                     * from the command line.  Remove it so it gets
                     * re-added in the expected order for iteration.
                     */
                    TAILQ_REMOVE(&node->head, seq_node, next);
                }
                else {
                    seq_node = gp_conf_node_new();
                    if (unlikely(seq_node == NULL)) {
                        goto fail;
                    }
                    seq_node->name = strdup(sequence_node_name);
                    if (unlikely(seq_node->name == NULL)) {
                        free(seq_node);
                        goto fail;
                    }
                }
                seq_node->is_seq = 1;
                TAILQ_INSERT_TAIL(&node->head, seq_node, next);
                if (gp_conf_yaml_parse(parser, seq_node, 0, rlevel) != 0)
                    goto fail;
            }
            else {
                if (gp_conf_yaml_parse(parser, node, inseq, rlevel) != 0)
                    goto fail;
            }
            state = CONF_KEY;
        }
        else if (event.type == YAML_MAPPING_END_EVENT) {
            done = 1;
        }
        else if (event.type == YAML_STREAM_END_EVENT) {
            done = 1;
        }

    next:
        yaml_event_delete(&event);
        continue;

    fail:
        yaml_event_delete(&event);
        retval = -1;
        break;
    }

    rlevel--;
    return retval;
}

/**
 * \brief Load configuration from a YAML file.
 *
 * This function will load a configuration file.  On failure -1 will
 * be returned and it is suggested that the program then exit.  Any
 * errors while loading the configuration file will have already been
 * logged.
 *
 * \param filename Filename of configuration file to load.
 *
 * \retval 0 on success, -1 on failure.
 */
int
gp_conf_yaml_load_file(const char *filename)
{
    FILE *infile;
    yaml_parser_t parser;
    int ret;
    gp_conf_node *root = gp_conf_get_root_node();

    if (yaml_parser_initialize(&parser) != 1) {
        return -1;
    }

    struct stat stat_buf;
    if (stat(filename, &stat_buf) == 0) {
        if (stat_buf.st_mode & S_IFDIR) {
            yaml_parser_delete(&parser);
            return -1;
        }
    }

    // coverity[toctou : FALSE]
    infile = fopen(filename, "r");
    if (infile == NULL) {
        yaml_parser_delete(&parser);
        return -1;
    }

    if (conf_dirname == NULL) {
        gp_conf_yaml_set_conf_dirname(filename);
    }

    yaml_parser_set_input_file(&parser, infile);
    ret = gp_conf_yaml_parse(&parser, root, 0, 0);
    yaml_parser_delete(&parser);
    fclose(infile);

    return ret;
}

/**
 * \brief Load configuration from a YAML string.
 */
int
gp_conf_yaml_load_string(const char *string, size_t len)
{
    gp_conf_node *root = gp_conf_get_root_node();
    yaml_parser_t parser;
    int ret;

    if (yaml_parser_initialize(&parser) != 1) {
        fprintf(stderr, "Failed to initialize yaml parser.\n");
        exit(EXIT_FAILURE);
    }
    yaml_parser_set_input_string(&parser, (const unsigned char *)string, len);
    ret = gp_conf_yaml_parse(&parser, root, 0, 0);
    yaml_parser_delete(&parser);

    return ret;
}

/**
 * \brief Load configuration from a YAML file, insert in tree at 'prefix'
 *
 * This function will load a configuration file and insert it into the
 * config tree at 'prefix'. This means that if this is called with prefix
 * "abc" and the file contains a parameter "def", it will be loaded as
 * "abc.def".
 *
 * \param filename Filename of configuration file to load.
 * \param prefix Name prefix to use.
 *
 * \retval 0 on success, -1 on failure.
 */
int
gp_conf_yaml_load_file_with_prefix(const char *filename, const char *prefix)
{
    FILE *infile;
    yaml_parser_t parser;
    int ret;
    gp_conf_node *root = gp_conf_get_node(prefix);

    if (yaml_parser_initialize(&parser) != 1) {
        return -1;
    }

    struct stat stat_buf;
    /* coverity[toctou] */
    if (stat(filename, &stat_buf) == 0) {
        if (stat_buf.st_mode & S_IFDIR) {
            return -1;
        }
    }

    /* coverity[toctou] */
    infile = fopen(filename, "r");
    if (infile == NULL) {
        yaml_parser_delete(&parser);
        return -1;
    }

    if (conf_dirname == NULL) {
        gp_conf_yaml_set_conf_dirname(filename);
    }

    if (root == NULL) {
        /* if node at 'prefix' doesn't yet exist, add a place holder */
        gp_conf_set(prefix, "<prefix root node>");
        root = gp_conf_get_node(prefix);
        if (root == NULL) {
            fclose(infile);
            yaml_parser_delete(&parser);
            return -1;
        }
    }
    yaml_parser_set_input_file(&parser, infile);
    ret = gp_conf_yaml_parse(&parser, root, 0, 0);
    yaml_parser_delete(&parser);
    fclose(infile);

    return ret;
}

#if 0

static int
gp_confYamlSequenceTest(void)
{
    char input[] = "\
%YAML 1.1\n\
---\n\
rule-files:\n\
  - netbios.rules\n\
  - x11.rules\n\
\n\
default-log-dir: /tmp\n\
";

    gp_confCreateContextBackup();
    gp_confInit();

    gp_confYamlLoadString(input, strlen(input));

    gp_conf_node *node;
    node = gp_confGetNode("rule-files");
    if (node == NULL)
        return 0;
    if (!gp_conf_nodeIsSequence(node))
        return 0;
    if (TAILQ_EMPTY(&node->head))
        return 0;
    int i = 0;
    gp_conf_node *filename;
    TAILQ_FOREACH(filename, &node->head, next) {
        if (i == 0) {
            if (strcmp(filename->val, "netbios.rules") != 0)
                return 0;
            if (gp_conf_nodeIsSequence(filename))
                return 0;
            if (filename->is_seq != 0)
                return 0;
        }
        else if (i == 1) {
            if (strcmp(filename->val, "x11.rules") != 0)
                return 0;
            if (gp_conf_nodeIsSequence(filename))
                return 0;
        }
        else {
            return 0;
        }
        i++;
    }

    gp_confDeInit();
    gp_confRestoreContextBackup();

    return 1;
}

static int
gp_confYamlLoggingOutputTest(void)
{
    char input[] = "\
%YAML 1.1\n\
---\n\
logging:\n\
  output:\n\
    - interface: console\n\
      log-level: error\n\
    - interface: syslog\n\
      facility: local4\n\
      log-level: info\n\
";

    gp_confCreateContextBackup();
    gp_confInit();

    gp_confYamlLoadString(input, strlen(input));

    gp_conf_node *outputs;
    outputs = gp_confGetNode("logging.output");
    if (outputs == NULL)
        return 0;

    gp_conf_node *output;
    gp_conf_node *output_param;

    output = TAILQ_FIRST(&outputs->head);
    if (output == NULL)
        return 0;
    if (strcmp(output->name, "0") != 0)
        return 0;
    output_param = TAILQ_FIRST(&output->head);
    if (output_param == NULL)
        return 0;
    if (strcmp(output_param->name, "interface") != 0)
        return 0;
    if (strcmp(output_param->val, "console") != 0)
        return 0;
    output_param = TAILQ_NEXT(output_param, next);
    if (strcmp(output_param->name, "log-level") != 0)
        return 0;
    if (strcmp(output_param->val, "error") != 0)
        return 0;

    output = TAILQ_NEXT(output, next);
    if (output == NULL)
        return 0;
    if (strcmp(output->name, "1") != 0)
        return 0;
    output_param = TAILQ_FIRST(&output->head);
    if (output_param == NULL)
        return 0;
    if (strcmp(output_param->name, "interface") != 0)
        return 0;
    if (strcmp(output_param->val, "syslog") != 0)
        return 0;
    output_param = TAILQ_NEXT(output_param, next);
    if (strcmp(output_param->name, "facility") != 0)
        return 0;
    if (strcmp(output_param->val, "local4") != 0)
        return 0;
    output_param = TAILQ_NEXT(output_param, next);
    if (strcmp(output_param->name, "log-level") != 0)
        return 0;
    if (strcmp(output_param->val, "info") != 0)
        return 0;

    gp_confDeInit();
    gp_confRestoreContextBackup();

    return 1;
}

/**
 * Try to load something that is not a valid YAML file.
 */
static int
gp_confYamlNonYamlFileTest(void)
{
    gp_confCreateContextBackup();
    gp_confInit();

    if (gp_confYamlLoadFile("/etc/passwd") != -1)
        return 0;

    gp_confDeInit();
    gp_confRestoreContextBackup();

    return 1;
}

static int
gp_confYamlBadYamlVersionTest(void)
{
    char input[] = "\
%YAML 9.9\n\
---\n\
logging:\n\
  output:\n\
    - interface: console\n\
      log-level: error\n\
    - interface: syslog\n\
      facility: local4\n\
      log-level: info\n\
";

    gp_confCreateContextBackup();
    gp_confInit();

    if (gp_confYamlLoadString(input, strlen(input)) != -1)
        return 0;

    gp_confDeInit();
    gp_confRestoreContextBackup();

    return 1;
}

static int
gp_confYamlSecondLevelSequenceTest(void)
{
    char input[] = "\
%YAML 1.1\n\
---\n\
libhtp:\n\
  server-config:\n\
    - apache-php:\n\
        address: [\"192.168.1.0/24\"]\n\
        personality: [\"Apache_2_2\", \"PHP_5_3\"]\n\
        path-parsing: [\"compress_separators\", \"lowercase\"]\n\
    - iis-php:\n\
        address:\n\
          - 192.168.0.0/24\n\
\n\
        personality:\n\
          - IIS_7_0\n\
          - PHP_5_3\n\
\n\
        path-parsing:\n\
          - compress_separators\n\
";

    gp_confCreateContextBackup();
    gp_confInit();

    if (gp_confYamlLoadString(input, strlen(input)) != 0)
        return 0;

    gp_conf_node *outputs;
    outputs = gp_confGetNode("libhtp.server-config");
    if (outputs == NULL)
        return 0;

    gp_conf_node *node;

    node = TAILQ_FIRST(&outputs->head);
    if (node == NULL)
        return 0;
    if (strcmp(node->name, "0") != 0)
        return 0;
    node = TAILQ_FIRST(&node->head);
    if (node == NULL)
        return 0;
    if (strcmp(node->name, "apache-php") != 0)
        return 0;

    node = gp_conf_nodeLookupChild(node, "address");
    if (node == NULL)
        return 0;
    node = TAILQ_FIRST(&node->head);
    if (node == NULL)
        return 0;
    if (strcmp(node->name, "0") != 0)
        return 0;
    if (strcmp(node->val, "192.168.1.0/24") != 0)
        return 0;

    gp_confDeInit();
    gp_confRestoreContextBackup();

    return 1;
}

/**
 * Test file inclusion support.
 */
static int
gp_confYamlFileIncludeTest(void)
{
    int ret = 0;
    FILE *config_file;

    const char config_filename[] = "gp_confYamlFileIncludeTest-config.yaml";
    const char config_file_contents[] =
        "%YAML 1.1\n"
        "---\n"
        "# Include something at the root level.\n"
        "include: gp_confYamlFileIncludeTest-include.yaml\n"
        "# Test including under a mapping.\n"
        "mapping: !include gp_confYamlFileIncludeTest-include.yaml\n";

    const char include_filename[] = "gp_confYamlFileIncludeTest-include.yaml";
    const char include_file_contents[] =
        "%YAML 1.1\n"
        "---\n"
        "host-mode: auto\n"
        "unix-command:\n"
        "  enabled: no\n";

    gp_confCreateContextBackup();
    gp_confInit();

    /* Write out the test files. */
    if ((config_file = fopen(config_filename, "w")) == NULL) {
        goto cleanup;
    }
    if (fwrite(config_file_contents, strlen(config_file_contents), 1,
            config_file) != 1) {
        goto cleanup;
    }
    fclose(config_file);
    if ((config_file = fopen(include_filename, "w")) == NULL) {
        goto cleanup;
    }
    if (fwrite(include_file_contents, strlen(include_file_contents), 1,
            config_file) != 1) {
        goto cleanup;
    }
    fclose(config_file);

    /* Reset conf_dirname. */
    if (conf_dirname != NULL) {
        free(conf_dirname);
        conf_dirname = NULL;
    }

    if (gp_confYamlLoadFile("gp_confYamlFileIncludeTest-config.yaml") != 0)
        goto cleanup;

    /* Check values that should have been loaded into the root of the
     * configuration. */
    gp_conf_node *node;
    node = gp_confGetNode("host-mode");
    if (node == NULL)
        goto cleanup;
    if (strcmp(node->val, "auto") != 0)
        goto cleanup;
    node = gp_confGetNode("unix-command.enabled");
    if (node == NULL)
        goto cleanup;
    if (strcmp(node->val, "no") != 0)
        goto cleanup;

    /* Check for values that were included under a mapping. */
    node = gp_confGetNode("mapping.host-mode");
    if (node == NULL)
        goto cleanup;
    if (strcmp(node->val, "auto") != 0)
        goto cleanup;
    node = gp_confGetNode("mapping.unix-command.enabled");
    if (node == NULL)
        goto cleanup;
    if (strcmp(node->val, "no") != 0)
        goto cleanup;

    gp_confDeInit();
    gp_confRestoreContextBackup();

    ret = 1;

cleanup:
    unlink(config_filename);
    unlink(include_filename);

    return ret;
}

/**
 * Test that a configuration section is overridden but subsequent
 * occurrences.
 */
static int
gp_confYamlOverrideTest(void)
{
    char config[] =
        "%YAML 1.1\n"
        "---\n"
        "some-log-dir: /var/log\n"
        "some-log-dir: /tmp\n"
        "\n"
        "parent:\n"
        "  child0:\n"
        "    key: value\n"
        "parent:\n"
        "  child1:\n"
        "    key: value\n"
        ;
    const char *value;

    gp_confCreateContextBackup();
    gp_confInit();

    if (gp_confYamlLoadString(config, strlen(config)) != 0)
        return 0;
    if (!gp_confGet("some-log-dir", &value))
        return 0;
    if (strcmp(value, "/tmp") != 0)
        return 0;

    /* Test that parent.child0 does not exist, but child1 does. */
    if (gp_confGetNode("parent.child0") != NULL)
        return 0;
    if (!gp_confGet("parent.child1.key", &value))
        return 0;
    if (strcmp(value, "value") != 0)
        return 0;

    gp_confDeInit();
    gp_confRestoreContextBackup();

    return 1;
}

/**
 * Test that a configuration parameter loaded from YAML doesn't
 * override a 'final' value that may be set on the command line.
 */
static int
gp_confYamlOverrideFinalTest(void)
{
    gp_confCreateContextBackup();
    gp_confInit();

    char config[] =
        "%YAML 1.1\n"
        "---\n"
        "default-log-dir: /var/log\n";

    /* Set the log directory as if it was set on the command line. */
    if (!gp_confSetFinal("default-log-dir", "/tmp"))
        return 0;
    if (gp_confYamlLoadString(config, strlen(config)) != 0)
        return 0;

    const char *default_log_dir;

    if (!gp_confGet("default-log-dir", &default_log_dir))
        return 0;
    if (strcmp(default_log_dir, "/tmp") != 0) {
        fprintf(stderr, "final value was reassigned\n");
        return 0;
    }

    gp_confDeInit();
    gp_confRestoreContextBackup();

    return 1;
}

#endif /* UNITTESTS */

#if 0
void
gp_confYamlRegisterTests(void)
{
#ifdef UNITTESTS
    UtRegisterTest("gp_confYamlSequenceTest", gp_confYamlSequenceTest);
    UtRegisterTest("gp_confYamlLoggingOutputTest", gp_confYamlLoggingOutputTest);
    UtRegisterTest("gp_confYamlNonYamlFileTest", gp_confYamlNonYamlFileTest);
    UtRegisterTest("gp_confYamlBadYamlVersionTest", gp_confYamlBadYamlVersionTest);
    UtRegisterTest("gp_confYamlSecondLevelSequenceTest",
                   gp_confYamlSecondLevelSequenceTest);
    UtRegisterTest("gp_confYamlFileIncludeTest", gp_confYamlFileIncludeTest);
    UtRegisterTest("gp_confYamlOverrideTest", gp_confYamlOverrideTest);
    UtRegisterTest("gp_confYamlOverrideFinalTest", gp_confYamlOverrideFinalTest);
#endif /* UNITTESTS */
}

#endif
