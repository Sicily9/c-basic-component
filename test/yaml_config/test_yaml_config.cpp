
//
// Created by xc on 2019/12/15.
//
#include <iostream>
#include <sys/queue.h>
#include "test.h"
#include "gtest/gtest.h"


using namespace std;
extern "C" {
#include "gp.h"
}

class TestYamlConfig : public ::testing::Test{
protected:
    virtual void TearDown() {

    }

    virtual void SetUp() {
    }
};

TEST(TestYamlConfig, TestParse1) {
/*	char input[] = "\
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
*/
	//gp_conf_yaml_load_string(input, strlen(input));
	gp_conf_create_context_backup();
	gp_conf_init();
	gp_conf_yaml_load_file("/home/xiaobear/code/github/c-basic-component/test/yaml_config/config.yaml");
	gp_conf_dump();

	gp_conf_node *outputs;
	outputs = gp_conf_get_node("logging.output");

	gp_conf_node *output;
	gp_conf_node *output_param;

    output = TAILQ_FIRST(&outputs->head);
	EXPECT_STREQ(output->name, "0");
	output_param = TAILQ_FIRST(&output->head);
	EXPECT_STREQ(output_param->name, "interface");
	EXPECT_STREQ(output_param->val, "console");

	output_param = TAILQ_NEXT(output_param, next);
	EXPECT_STREQ(output_param->name, "log-level");
	EXPECT_STREQ(output_param->val, "error");

	output = TAILQ_NEXT(output, next);
	EXPECT_STREQ(output->name, "1");

	output_param = TAILQ_FIRST(&output->head);
	EXPECT_STREQ(output_param->name, "interface");
	EXPECT_STREQ(output_param->val, "syslog");

	output_param = TAILQ_NEXT(output_param, next);
	EXPECT_STREQ(output_param->name, "facility");
	EXPECT_STREQ(output_param->val, "local4");

	output_param = TAILQ_NEXT(output_param, next);
	EXPECT_STREQ(output_param->name, "log-level");
	EXPECT_STREQ(output_param->val, "info");
	gp_conf_deinit();
	gp_conf_restore_context_backup();

}

TEST(TestYamlConfig, TestParse2) {
	gp_conf_create_context_backup();
	gp_conf_init();

	gp_conf_yaml_load_file("/home/xiaobear/code/github/c-basic-component/test/yaml_config/config2.yaml");
	gp_conf_dump();
	gp_conf_node *outputs;
	gp_conf_node *outputs2;
	gp_conf_node *output_param;
    gp_conf_node *root = gp_conf_get_root_node();
	printf("%s:%s\n", root->name, root->val);
	
    outputs = TAILQ_FIRST(&root->head);
	printf("%s:%s\n", outputs->name, outputs->val);

    output_param = TAILQ_FIRST(&outputs->head);
	printf("%s:%s\n", output_param->name, output_param->val);

    output_param = TAILQ_FIRST(&output_param->head);
	printf("%s:%s\n", output_param->name, output_param->val);

    output_param = TAILQ_FIRST(&output_param->head);
	printf("%s:%s\n", output_param->name, output_param->val);

    outputs2 = TAILQ_NEXT(outputs, next);
	printf("%s:%s\n", outputs2->name, outputs2->val);
	gp_conf_deinit();
	gp_conf_restore_context_backup();
}
int main(int argc, char **argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
