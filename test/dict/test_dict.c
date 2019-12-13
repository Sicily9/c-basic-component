#include <sys/time.h>

#include "gp.h"
#include "sds.h"

long long timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}


uint64_t hashCallback(const void *key) {
    return dictGenHashFunction((unsigned char*)key, sdslen((char*)key));
}

uint64_t hashCallback_u64(const void *key) {
    return dictGenHashFunction((unsigned char*)key, sizeof(unsigned long));
}

int compareCallback_u64(void *privdata, const void *key1, const void *key2) {
    unsigned long l1,l2;
    DICT_NOTUSED(privdata);

    l1 = (unsigned long)key1;
    l2 = (unsigned long)key2;
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}

void freeCallback_ptr(void *privdata, void *val) {
    DICT_NOTUSED(privdata);
	val = NULL;

}

int compareCallback(void *privdata, const void *key1, const void *key2) {
    int l1,l2;
    DICT_NOTUSED(privdata);

    l1 = sdslen((sds)key1);
    l2 = sdslen((sds)key2);
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}

void freeCallback(void *privdata, void *val) {
    DICT_NOTUSED(privdata);

    sdsfree(val);
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


#define start_benchmark() start = timeInMilliseconds()
#define end_benchmark(msg) do { \
    elapsed = timeInMilliseconds()-start; \
    printf(msg ": %ld items in %lld ms\n", count, elapsed); \
} while(0);

dict * dict2 = NULL;
dict * test_dict = NULL;


struct A{
	int a;
	int b;
};

struct A a[5] = {0};
void __attribute((constructor))__func(void){
	char a[10];
	strcpy(a, "123456");

	int b =3;

	dict2 =dictCreate(STRING_DICT, 0);
	int ret = dictAdd(dict2, a, &b);
	assert(ret == DICT_OK);

}

void func1(void){
    int a = *(int *)dictFetchValue(dict2, "123456");
    printf("a:%d\n", a);
}

void print_keys(dict *dict)
{
    dictIterator * di;
	dictEntry * de;

    di = dictGetSafeIterator(dict);

	while( (de = dictNext(di)) != NULL){
    	printf("%p\n", dictGetKey(de));
	}
}

/* dict-benchmark [count] */
int main(int argc, char **argv) {
	//func();
	func1();
    long j;
    long long start, elapsed;
    dict *dict = _dictCreate(&BenchmarkDictType,NULL);
    long count = 0; 

	test_dict =_dictCreate(&PtrDictType, NULL);
	for(int i = 0; i < 4; i++){
    	dictAdd(test_dict, (void *)&a[i], (void *)&i);
		printf("size:%ld %p\n",test_dict->ht[0].used, &a[i]);
	}
	
	print_keys(test_dict);
    

	for(int i = 0; i < 4; i++){
    	dictDelete(test_dict, (void *)&a[i]);
		printf("size:%ld\n",test_dict->ht[0].used);
	}
	print_keys(test_dict);

    if (argc == 2) { 
        count = strtol(argv[1],NULL,10);
    } else {
        count = 5000000;
    }    

    start_benchmark();
    for (j = 0; j < count; j++) {
        int retval = dictAdd(dict,sdsfromlonglong(j),(void*)j);
        assert(retval == DICT_OK);
    }    
    end_benchmark("Inserting");
    assert((long)dictSize(dict) == count);

    /* Wait for rehashing. */
    while (dictIsRehashing(dict)) {
        dictRehashMilliseconds(dict,100);
    }

    start_benchmark();
    for (j = 0; j < count; j++) {
        sds key = sdsfromlonglong(j);
        dictEntry *de = dictFind(dict,key);
        assert(de != NULL);
        sdsfree(key);
    }
    end_benchmark("Linear access of existing elements");

    start_benchmark();
    for (j = 0; j < count; j++) {
        sds key = sdsfromlonglong(j);
        dictEntry *de = dictFind(dict,key);
        assert(de != NULL);
        sdsfree(key);
    }
    end_benchmark("Linear access of existing elements (2nd round)");

    start_benchmark();
    for (j = 0; j < count; j++) {
        sds key = sdsfromlonglong(rand() % count);
        dictEntry *de = dictFind(dict,key);
        assert(de != NULL);
        sdsfree(key);
    }
    end_benchmark("Random access of existing elements");

    start_benchmark();
    for (j = 0; j < count; j++) {
        sds key = sdsfromlonglong(rand() % count);
        key[0] = 'X';
        dictEntry *de = dictFind(dict,key);
        assert(de == NULL);
        sdsfree(key);
    }
    end_benchmark("Accessing missing");

    start_benchmark();
    for (j = 0; j < count; j++) {
        sds key = sdsfromlonglong(j);
        int retval = dictDelete(dict,key);
        assert(retval == DICT_OK);
        key[0] += 17; /* Change first number to letter. */
        retval = dictAdd(dict,key,(void*)j);
        assert(retval == DICT_OK);
    }
    end_benchmark("Removing and adding");

    return 0;
}

