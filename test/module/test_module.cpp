//
// Created by xc on 2019/12/15.
//
#include <iostream>
#include "test.h"
#include "gtest/gtest.h"
#define _CRT_SECURE_NO_WARNINGS 

long long start = 0;
long long init = 0;
long long fuck_end = 0;

using namespace std;

extern "C" {

    #include "gp.h"
    void early_init_test2(void) {
	    printf("test2\n");
    }

    void early_init_test(void) {
        start = timeInMicroseconds();
    }

    void init_test(void) {
        init = timeInMicroseconds();
    }

    gp_module_init(test_module, MODULE_INIT_FIRST, "test", init_test, early_init_test) ;

    gp_module_init(test2_module, MODULE_INIT_FIRST, "222", init_test, early_init_test2) ;

}

class TestModule : public ::testing::Test{
protected:
    virtual void TearDown() {

    }

    virtual void SetUp() {
    }
};

TEST(TestModule, TestSequence) {
    gp_init_modules();
    EXPECT_TRUE(fuck_end - start > 0);
    EXPECT_TRUE(init - fuck_end > 0);
}

int main(int argc, char **argv){
    ::testing::InitGoogleTest(&argc, argv);
    fuck_end = timeInMicroseconds();
    return RUN_ALL_TESTS();
}
