//
// Created by xc on 2019/12/12.
//
#include <iostream>
#include "gtest/gtest.h"

extern "C"{
#include "gp.h"
}

using namespace std;

#define N 5
struct student{
    int id;
    gp_list_node st_node;
};

struct student s[N];
struct student t[N];
struct student a;

class TestList : public ::testing::Test{

protected:
    virtual void TearDown() {

    }

    virtual void SetUp() {

        a.id = 10;
        GP_LIST_NODE_INIT(&a.st_node);

        GP_LIST_INIT(&list_append, struct student, st_node);
        GP_LIST_INIT(&list_prepend, struct student, st_node);

        for(int i = 0; i < N; i++){
            s[i].id = i;
            GP_LIST_NODE_INIT(&s[i].st_node);
        }

        for(int i = 0; i < N; i++){
            t[i].id = i;
            GP_LIST_NODE_INIT(&t[i].st_node);
        }

        for(int i = 0; i < N; i++){
            gp_list_append(&list_append, &s[i]);
        }

        for(int i = 0; i < N; i++){
            gp_list_prepend(&list_prepend, &t[i]);
        }

    }

    gp_list list_append;
    gp_list list_prepend;
};

TEST_F(TestList, TestListFirst) {
    struct student *tmp;
    tmp = (struct student *)gp_list_first(&list_append);
    EXPECT_EQ(0,tmp->id);
}

TEST_F(TestList, TestListNext) {

    struct student *tmp = NULL;
    tmp = (struct student *)gp_list_first(&list_append);
    EXPECT_EQ(0,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(1,tmp->id);

    tmp = (struct student *)gp_list_next(&list_append, tmp);
    gp_list_remove(&list_append, &s[2]);
    gp_list_remove(&list_append, &s[3]);
    EXPECT_EQ(2,tmp->id);

    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_TRUE(tmp == NULL);

}

TEST_F(TestList, TestListLast) {
    struct student *tmp; tmp = (struct student *) gp_list_last(&list_append);
    EXPECT_EQ(4, tmp->id);
}

TEST_F(TestList, TestListPrev) {
    struct student *tmp;
    tmp = (struct student *) gp_list_last(&list_append);
    EXPECT_EQ(4,tmp->id);
    tmp = (struct student *)gp_list_prev(&list_append, tmp);
    EXPECT_EQ(3,tmp->id);
    tmp = (struct student *)gp_list_prev(&list_append, tmp);
    EXPECT_EQ(2,tmp->id);
    tmp = (struct student *)gp_list_prev(&list_append, tmp);
    EXPECT_EQ(1,tmp->id);
    tmp = (struct student *)gp_list_prev(&list_append, tmp);
    EXPECT_EQ(0,tmp->id);

}

TEST_F(TestList, TestListAppend) {
    struct student *tmp;
    tmp = (struct student *)gp_list_first(&list_append);
    EXPECT_EQ(0,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(1,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(2,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(3,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(4,tmp->id);
}

TEST_F(TestList, TestListPrepend) {
    struct student *tmp;
    tmp = (struct student *)gp_list_first(&list_prepend);
    EXPECT_EQ(4,tmp->id);
    tmp = (struct student *)gp_list_next(&list_prepend, tmp);
    EXPECT_EQ(3,tmp->id);
    tmp = (struct student *)gp_list_next(&list_prepend, tmp);
    EXPECT_EQ(2,tmp->id);
    tmp = (struct student *)gp_list_next(&list_prepend, tmp);
    EXPECT_EQ(1,tmp->id);
    tmp = (struct student *)gp_list_next(&list_prepend, tmp);
    EXPECT_EQ(0,tmp->id);
}

TEST_F(TestList, TestListInsertAfter) {
    gp_list_insert_after(&list_append, &a, &s[2]);

    struct student *tmp;
    tmp = (struct student *)gp_list_first(&list_append);
    EXPECT_EQ(0,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(1,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(2,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(10,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(3,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(4,tmp->id);

}

TEST_F(TestList, TestListInsertBefore) {
    gp_list_insert_before(&list_append, &a, &s[2]);

    struct student *tmp;
    tmp = (struct student *)gp_list_first(&list_append);
    EXPECT_EQ(0,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(1,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(10,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(2,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(3,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(4,tmp->id);

}

TEST_F(TestList, TestListRemove) {

    struct student *tmp = NULL;
    tmp = (struct student *)gp_list_first(&list_append);
    EXPECT_EQ(0,tmp->id);

    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(1,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    gp_list_remove(&list_append, &s[2]);
    gp_list_remove(&list_append, &s[3]);
    EXPECT_EQ(2,tmp->id);

    tmp = (struct student *)gp_list_first(&list_append);
    EXPECT_EQ(0,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(1,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_EQ(4,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_TRUE(tmp == NULL);

}

TEST_F(TestList, TestListReplace) {
    gp_list_replace(&list_prepend, &list_append);
                        //old         new
    struct student *tmp;
    tmp = (struct student *) gp_list_first(&list_append);
    EXPECT_EQ(4, tmp->id);
    tmp = (struct student *) gp_list_next(&list_append, tmp);
    EXPECT_EQ(3, tmp->id);
    tmp = (struct student *) gp_list_next(&list_append, tmp);
    EXPECT_EQ(2, tmp->id);
    tmp = (struct student *) gp_list_next(&list_append, tmp);
    EXPECT_EQ(1, tmp->id);
    tmp = (struct student *) gp_list_next(&list_append, tmp);
    EXPECT_EQ(0, tmp->id);

    tmp = (struct student *)gp_list_first(&list_prepend);
    EXPECT_EQ(4,tmp->id);
    tmp = (struct student *)gp_list_next(&list_prepend, tmp);
    EXPECT_EQ(3,tmp->id);
    tmp = (struct student *)gp_list_next(&list_prepend, tmp);
    EXPECT_EQ(2,tmp->id);
    tmp = (struct student *)gp_list_next(&list_prepend, tmp);
    EXPECT_EQ(1,tmp->id);
    tmp = (struct student *)gp_list_next(&list_prepend, tmp);
    EXPECT_EQ(0,tmp->id);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
