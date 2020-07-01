//
// Created by xc on 2019/12/12.
//
#include <iostream>
#include <sys/time.h>
#include "test.h"
#include "gtest/gtest.h"

extern "C"{
#include "sds.h"
#include "gp.h"
}

using namespace std;

#define N 5
struct student{
    int id;
    char name[21];
    gp_list_node st_node;
};
struct student s[N];
struct student t[N];
struct student a;
struct student b[5000000];

class TestListTime : public ::testing::Test{
protected:
    virtual void TearDown() {

    }
    virtual void SetUp() {
        GP_LIST_INIT(&list_for_time, struct student, st_node);

        for(long long i = 0; i < 5000000; i++){
            memset(&b[i], 0, sizeof(struct student));
            b[i].id = i;
            strcpy(b[i].name, sdsfromlonglong(i));
            GP_LIST_NODE_INIT(&b[i].st_node);
        }
    }

    gp_list list_for_time;
};

TEST_F(TestListTime, TestListAddTime) {
    start_benchmark()
    for(int i=0; i < 5000000; i++){
        gp_list_append(&list_for_time, &b[i]);
    }
    end_benchmark("TestListAddTime")
}

struct student *find_student_string(char * str, gp_list * list){
    void * t;
    GP_LIST_FOREACH(list, t){
        if(!strcmp(str, ((struct student *)t)->name)) {
            struct student * tmp = (struct student *)t;
            return tmp;
        }
    }
    return NULL;
}

struct student *find_student_id(int id, gp_list * list){
    void * t;
    GP_LIST_FOREACH(list, t){
        if( id == ((struct student *)t)->id ) {
            struct student * tmp = (struct student *)t;
            return tmp;
        }
    }
    return NULL;
}

#define NUM 5000

TEST_F(TestListTime, TestListFindByIDTime) {
    start_benchmark()
    for (long long i = 0; i < NUM; i++) {
        gp_list_append(&list_for_time, &b[i]);
    }

    for (long long i = 0; i < NUM; i++) {
        find_student_id(i, &list_for_time);
		if(i == NUM/10){
    		end_benchmark("1/10 total")
		}
    }
    end_benchmark("TestListFindByIDTime")
}

TEST_F(TestListTime, TestListFindByStringTime) {
    start_benchmark()
    for (long long i = 0; i < NUM; i++) {
        gp_list_append(&list_for_time, &b[i]);
    }

    for (long long i = 0; i < NUM; i++) {
        find_student_string(sdsfromlonglong(i), &list_for_time);
		if(i == NUM/10){
    		end_benchmark("1/10 total")
		}
    }
    end_benchmark("TestListFindByStringTime")
}

class TestList : public ::testing::Test{
protected:
    virtual void TearDown() {

    }

    virtual void SetUp() {

        GP_LIST_INIT(&list_append, struct student, st_node);
        GP_LIST_INIT(&list_prepend, struct student, st_node);

        a.id = 10;
        GP_LIST_NODE_INIT(&a.st_node);

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

TEST_F(TestList, TestListEmpty){
    struct student *tmp = NULL;
    tmp = (struct student *)gp_list_first(&list_append);
    EXPECT_EQ(0,tmp->id);
    int flag = gp_list_empty(&list_append);
    EXPECT_EQ(0,flag);

    gp_list_remove(&list_append, &s[1]);
    gp_list_remove(&list_append, &s[2]);
    gp_list_remove(&list_append, &s[3]);
    gp_list_remove(&list_append, &s[4]);
    flag = gp_list_empty(&list_append);
    EXPECT_EQ(0,flag);

    gp_list_remove(&list_append, tmp);
    flag = gp_list_empty(&list_append);
    EXPECT_EQ(1,flag);
}

TEST_F(TestList, TestListActive){
    struct student *tmp = NULL;
    tmp = (struct student *)gp_list_first(&list_append);
    EXPECT_EQ(0,tmp->id);
    tmp = (struct student *)gp_list_next(&list_append, tmp);
    tmp = (struct student *)gp_list_next(&list_append, tmp);

    EXPECT_TRUE(1 == gp_list_active(&list_append, tmp));
    gp_list_remove(&list_append, &s[2]);
    gp_list_remove(&list_append, &s[3]);
    EXPECT_TRUE(0 == gp_list_active(&list_append, tmp));

    tmp = (struct student *)gp_list_next(&list_append, tmp);
    EXPECT_TRUE(tmp == NULL);
}

TEST_F(TestList, TestListForeach){
    void *tmp = NULL;
    int i = 0;
    GP_LIST_FOREACH(&list_append, tmp){
        EXPECT_EQ(i++,((struct student *)tmp)->id);
    }

    GP_LIST_FOREACH(&list_append, tmp){
        gp_list_remove(&list_append, tmp);
    }
    EXPECT_EQ(0, gp_list_empty(&list_append));

    tmp = gp_list_first(&list_append);
    EXPECT_EQ(1,((struct student *)tmp)->id);
    tmp = gp_list_next(&list_append, tmp);
    EXPECT_EQ(2,((struct student *)tmp)->id);
    tmp = gp_list_next(&list_append, tmp);
    EXPECT_EQ(3,((struct student *)tmp)->id);
    tmp = gp_list_next(&list_append, tmp);
    EXPECT_EQ(4,((struct student *)tmp)->id);
}

TEST_F(TestList, TestListForeachSafe){
    void *first = NULL;
    void *second = NULL;
    int i = 0;
    GP_LIST_FOREACH_SAFE(&list_append, second, first){
        EXPECT_EQ(i++,((struct student *)first)->id);
    }

    GP_LIST_FOREACH_SAFE(&list_append, second, first){
        gp_list_remove(&list_append, first);
    }
    EXPECT_EQ(1, gp_list_empty(&list_append));
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
