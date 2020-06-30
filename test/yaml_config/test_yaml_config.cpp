
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

class TestModule : public ::testing::Test{
protected:
    virtual void TearDown() {

    }

    virtual void SetUp() {
    }
};

TEST(TestModule, TestSequence) {
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

	gp_conf_create_context_backup();
	gp_conf_init();
	gp_conf_yaml_load_string(input, strlen(input));

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

int main(int argc, char **argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
