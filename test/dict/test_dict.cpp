#include <sys/time.h>
#include <gtest/gtest.h>
#include <test.h>

extern "C"
{
#include "gp.h"
#include "sds.h"

uint64_t hashCallback(const void *key) {
    return dictGenHashFunction((unsigned char *) key, sdslen((char *) key));
}

uint64_t hashCallback_u64(const void *key) {
    return dictGenHashFunction((unsigned char *) key, sizeof(unsigned long));
}

int compareCallback_u64(void *privdata, const void *key1, const void *key2) {
    unsigned long l1, l2;
    DICT_NOTUSED(privdata);

    l1 = (unsigned long) key1;
    l2 = (unsigned long) key2;
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}

void freeCallback_ptr(void *privdata, void *val) {
    DICT_NOTUSED(privdata);
    val = NULL;

}

int compareCallback(void *privdata, const void *key1, const void *key2) {
    int l1, l2;
    DICT_NOTUSED(privdata);

    l1 = sdslen((sds) key1);
    l2 = sdslen((sds) key2);
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}

void print_keys(dict *dict) {
    dictIterator *di;
    dictEntry *de;

    di = dictGetSafeIterator(dict);

    while ((de = dictNext(di)) != NULL) {
        printf("%p\n", dictGetKey(de));
    }
}

void freeCallback(void *privdata, void *val) {
    DICT_NOTUSED(privdata);

    sdsfree((char *) val);
}

dictType PtrDictType = {
        hashCallback_u64,
        NULL,
        NULL,
        compareCallback_u64,
        freeCallback_ptr,
        NULL
};

dictType BenchmarkDictType = {
        hashCallback,
        NULL,
        NULL,
        compareCallback,
        freeCallback,
        NULL
};

}

using namespace std;

int a[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

class TestDictTime : public ::testing::Test {
protected:
    virtual void TearDown() {

    }

    virtual void SetUp() {
        string_dict = dictCreate(STRING_DICT, 0);
        int_dict = dictCreate(INT_DICT, 0);
    }

    dict *string_dict;
    dict *int_dict;
};

TEST_F(TestDictTime, TestDictStringAddTime) {
    start_benchmark()
    int j = 0;
    for (j = 0; j < 5000000; j++) {
        dictAdd(string_dict, sdsfromlonglong(j), (void *)&j);
    }
    end_benchmark("TestDictStringAdd")
}

TEST_F(TestDictTime, TestDictStringFindTime) {
    int j = 0;
    for (j = 0; j < 5000000; j++) {
        dictAdd(string_dict, sdsfromlonglong(j), (void *)&j);
    }
    start_benchmark()
    for (j = 0; j < 5000000; j++) {
        sds key = sdsfromlonglong(j);
        dictFind(string_dict,key);
    }
    end_benchmark("TestDictStringFind")
}

TEST_F(TestDictTime, TestDictIntAddTime) {
    start_benchmark()
    int j = 0;
    for (j = 0; j < 5000000; j++) {
        dictAdd(int_dict, (void *)&j, (void *)&j);
    }
    end_benchmark("TestDictIntAdd")
}

TEST_F(TestDictTime, TestDictIntFindTime) {
    int j = 0;
    for (j = 0; j < 5000000; j++) {
        dictAdd(int_dict, (void *)&j, (void *)&j);
    }
    start_benchmark()
    for (j = 0; j < 5000000; j++) {
        dictFind(int_dict,(void *)&j);
    }
    end_benchmark("TestDictIntFind")
}

class TestDict : public ::testing::Test {
protected:
    virtual void TearDown() {

    }

    virtual void SetUp() {
        string_int_dict = dictCreate(STRING_DICT, INT_DICT);
        int_string_dict = dictCreate(INT_DICT, STRING_DICT);
        string_string_dict = dictCreate(STRING_DICT, STRING_DICT);
        int_int_dict = dictCreate(INT_DICT, INT_DICT);
        string_dict = dictCreate(STRING_DICT, 0);
        int_dict = dictCreate(INT_DICT, 0);

        for (int i = 0; i < 10; i++) {
            dictAdd(string_int_dict, sdsfromlonglong(i), (void *) &a[i]);
        }
        for (int i = 0; i < 10; i++) {
            dictAdd(int_string_dict, (void *) &a[i], sdsfromlonglong(i));
        }
        for (int i = 0; i < 10; i++) {
            dictAdd(string_string_dict, sdsfromlonglong(i), sdsfromlonglong(i));
        }
        for (int i = 0; i < 10; i++) {
            dictAdd(int_int_dict, (void *) &a[i], (void *) &a[i]);
        }
    }

    dict *string_int_dict;
    dict *int_string_dict;
    dict *string_string_dict;
    dict *int_int_dict;
    dict *string_dict;
    dict *int_dict;
};


struct test {
    int a;
    int b;
    int c;
    int d;

    ~test() {
    }
};

TEST_F(TestDict, TestDictSize) {
    int i = 0;
    i = dictSize(string_int_dict);
    EXPECT_EQ(i,10);
    i = dictSize(string_string_dict);
    EXPECT_EQ(i,10);
    i = dictSize(int_string_dict);
    EXPECT_EQ(i,10);
    i = dictSize(int_int_dict);
    EXPECT_EQ(i,10);
}

TEST_F(TestDict, TestDictFetchValue) {
    void *ret;
    int i = 0;
    ret = dictFetchValue(string_int_dict, sdsfromlonglong(i));
    EXPECT_EQ(i, *(int *) ret);

    ret = dictFetchValue(int_string_dict, (void *) &i);
    EXPECT_STREQ(sdsfromlonglong(i), (char *) ret);

    ret = dictFetchValue(string_string_dict, sdsfromlonglong(i));
    EXPECT_STREQ(sdsfromlonglong(i), (char *) ret);

    ret = dictFetchValue(int_int_dict, (void *) &i);
    EXPECT_EQ(i, *(int *) ret);

    {
        {
            struct test t;
            t.a = 1;
            t.b = 2;
            t.c = 3;
            t.d = 4;
            dictAdd(string_dict, sdsfromlonglong(i), (void *) &t);
        }
        ret = dictFetchValue(string_dict, sdsfromlonglong(i));
        struct test *tmp = (struct test *) ret; //已经是野指针了
        EXPECT_EQ(1, 1 == tmp->a);
        EXPECT_EQ(1, 2 == tmp->b);
        EXPECT_EQ(1, 3 != tmp->c);
        EXPECT_EQ(1, 4 != tmp->d);
    }

}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

