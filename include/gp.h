#ifndef __GP_H__
#define __GP_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>


typedef struct gp_loop_s gp_loop;
typedef struct gp_io_s gp_io;
typedef struct gp_list_s gp_list;
typedef struct gp_list_node_s gp_list_node;
typedef struct gp_mtx_s gp_mtx;
typedef struct gp_cv_s gp_cv;
typedef struct gp_thread_s gp_thread;
typedef struct gp_thread_manager_s gp_thread_manager;
typedef struct gp_thread_manager_ops_s gp_thread_mgr_operations;
typedef struct gp_task_s gp_task;
typedef struct gp_taskp_tmr_s gp_taskp_tmr;
typedef struct gp_task_processor_s gp_task_processor;
typedef struct dictEntry_s dictEntry;
typedef struct dictType_s dictType;
typedef struct dictht_s dictht;
typedef struct dict_s dict;
typedef struct dictIterator_s dictIterator;
typedef struct gp_module_desc_s gp_module_desc;
typedef enum gp_clocktype_s gp_clocktype;
typedef struct gp_timer_base_s gp_timer_base;
typedef struct gp_timer_list_s gp_timer_list;
typedef struct tvec_root_s tvec_root;
typedef struct tvec_s	tvec;

typedef void (*gp_io_cb)(gp_loop *loop, gp_io *w, unsigned int events);
typedef void (*gp_cb)(void *);
typedef void (*gp_thread_fn)(void *);


typedef enum {
    GP_RUN_DEFAULT = 0,
    GP_RUN_ONCE,
    GP_RUN_NOWAIT
}gp_run_mode;

//doubly list
struct gp_list_node_s {
	gp_list_node *ln_next;
    gp_list_node *ln_prev;
};

struct gp_list_s {
    gp_list_node ll_head; // head node of the list
    size_t ll_offset; // the relative locationof gp_list_node member in the node
};

struct gp_io_s {
    gp_io_cb cb; 
    gp_list_node pending_node;
    gp_list_node watcher_node;
    unsigned int pevents;
    unsigned int events;
    int fd; 
};

//TODO: add handle base class
struct gp_loop_s {
//  unsigned int active_handles;
//  gp_list handle_list;
//  unsigned int count;//active_request count
    unsigned int stop_flags;
    unsigned long flags;
    int backend_fd;
    gp_list pending_list;
    gp_list watcher_list;
    gp_io **watchers;
    unsigned long time;
    gp_timer_base *timer_base;
    unsigned int nwatchers;
    unsigned int nfds;
};



struct gp_mtx_s {
    pthread_mutex_t mtx;
};

struct gp_cv_s {
    pthread_cond_t cv; 
    gp_mtx * mtx;
};

struct gp_thread_s {
    pthread_t id; 
    gp_thread_fn gp_thr_fn;
    void *arg;
};

struct gp_thread_manager_s {
    gp_thread  rthread;
    gp_mtx mtx;
    gp_cv cv; 
    gp_thread_fn gp_tmr_fn;
    void *arg;
    int start;
    int stop;
    int done;
    int init;

    gp_thread_mgr_operations *ops;
};

struct gp_thread_manager_ops_s{
    void (*run)(void *); 
    void (*wait)(void *); 
    void (*finish)(void *); 
};

struct gp_task_s {
    gp_list_node      task_node;
    void *               task_arg;
    gp_cb                task_cb;
    gp_task_processor *task_tp;
    gp_thread_manager *task_tmr;
    unsigned         task_busy;
    //int            task_prep;
    //int            task_reap;
    gp_mtx        task_mtx;
    gp_cv             task_cv;
};

struct gp_taskp_tmr_s {
    gp_task_processor * tpt_tp;
    gp_thread_manager tpt_thread;
};

//TODO: priority queue
// a threadpool
struct gp_task_processor_s {
    gp_list       tp_tasks;
    gp_mtx        tp_mtx;
    gp_cv         tp_sched_cv;
    gp_cv         tp_wait_cv;
    gp_taskp_tmr *tp_threads;
    int          tp_nthreads;
    int          tp_run;
};

enum {
    INT_DICT = 1,
    STRING_DICT,
};

struct dictEntry_s {
    void *key;
    union {
        void *val;
        uint64_t u64;
        int64_t s64;
        double d;
    } v;
    dictEntry *next;
};

struct dictType_s {
    uint64_t (*hashFunction)(const void *key);
    void *(*keyDup)(void *privdata, const void *key);
    void *(*valDup)(void *privdata, const void *obj);
    int (*keyCompare)(void *privdata, const void *key1, const void *key2);
    void (*keyDestructor)(void *privdata, void *key);
    void (*valDestructor)(void *privdata, void *obj);
};

/* This is our hash table structure. Every dictionary has two of this as we
 * implement incremental rehashing, for the old to the new table. */
struct dictht_s {
    dictEntry **table;
    unsigned long size;
    unsigned long sizemask;
    unsigned long used;
};

struct dict_s {
    dictType *type;
    void *privdata;
    dictht ht[2];
    long rehashidx; /* rehashing not in progress if rehashidx == -1 */
    unsigned long iterators; /* number of iterators currently running */
};

struct dictIterator_s {
    dict *d;
    long index;
    int table, safe;
    dictEntry *entry, *nextEntry;
    /* unsafe iterator fingerprint for misuse detection. */
    long long fingerprint;
};

enum gp_module_init_type {
    MODULE_INIT_FIRST,
};

struct gp_module_desc_s {
    gp_list_node node;
    int type;

    int priority;
    char *name;
    void (*init)(void);
    void (*early_init)(void);
    //void (*destroy)(void);

    struct {
        int init;
        char *file;
    }stat;
};

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS) 
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)
#define MAX_TVAL ((unsigned long)((1ULL << (TVR_BITS + 4*TVN_BITS)) - 1))
#define INDEX(C,N) (((C)->timer_jiffies >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)
enum gp_clocktype_s{
    GP_CLOCK_PRECISE = 0,
    GP_CLOCK_FAST = 1
};

struct tvec_s {
    gp_list vec[TVN_SIZE];
};

struct tvec_root_s {
    gp_list vec[TVR_SIZE];
};

struct gp_timer_base_s {
    unsigned long timer_jiffies;
    unsigned long next_timer;
    tvec_root tv1;
    tvec tv2;
    tvec tv3;
    tvec tv4;
    tvec tv5;
};

struct gp_timer_list_s {
    gp_list_node node;
    gp_loop *loop;
    unsigned long expires;
    int interval;
    int repeat;
    void (*callback)(void *);
    void *data;
    unsigned int state;
};




/*-----------------------------------------------------------------------------------------------*/

extern void create_gp_io(gp_io **, gp_io_cb, int);
extern void init_gp_io(gp_io *, gp_io_cb, int);
extern void gp_io_stop(gp_loop *, gp_io *, unsigned int);
extern void gp_io_start(gp_loop *, gp_io *, unsigned int);
extern void gp_io_poll(gp_loop *, unsigned long);

/*-----------------------------------------------------------------------------------------------*/

extern int create_gp_loop(gp_loop **);
extern int init_gp_loop(gp_loop *); 
//extern int gp_loop_alive(gp_loop *loop);
extern int gp_loop_run(gp_loop *, gp_run_mode);

extern void gp_loop_timer_start(gp_loop *, void (*fn)(void *), void *, int , int );
extern void gp_loop_timer_stop(gp_timer_list *);
extern void gp_loop_timer_mod(gp_loop *, gp_timer_list *,unsigned long, int, int);
extern void gp_loop_run_timers(gp_loop *);
extern void gp_loop_update_time(gp_loop *);



/*-----------------------------------------------------------------------------------------------*/
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define GP_LIST_INIT(list, type, field) \
        gp_list_init_offset(list, offsetof(type, field))

#define GP_LIST_NODE_INIT(node)                       \
        {                                              \
                (node)->ln_prev = (node)->ln_next = 0; \
        }

extern void gp_list_init_offset(gp_list *list, size_t offset);
extern void *gp_list_first(const gp_list *); 
extern void *gp_list_last(const gp_list *); 
extern void  gp_list_append(gp_list *, void *); 
extern void  gp_list_prepend(gp_list *, void *); 
extern void  gp_list_insert_before(gp_list *, void *, void *); 
extern void  gp_list_insert_after(gp_list *, void *, void *); 
extern void  gp_list_replace(gp_list *, gp_list *); 
extern void *gp_list_next(const gp_list *, void *); 
extern void *gp_list_prev(const gp_list *, void *); 
extern void  gp_list_remove(gp_list *, void *); 
extern int   gp_list_active(gp_list *, void *); 
extern int   gp_list_empty(gp_list *); 
extern int   gp_list_node_active(gp_list_node *); 
extern void   gp_list_node_remove(gp_list_node *); 

#define GP_LIST_FOREACH(l, it) \
        for (it = gp_list_first(l); it != NULL; it = gp_list_next(l, it))

#define GP_LIST_FOREACH_SAFE(l, tmp, it) \
        for (it = gp_list_first(l), tmp = gp_list_next(l, it); it != NULL; it = tmp, tmp = gp_list_next(l, it))

/*-----------------------------------------------------------------------------------------------*/

#define gp_tmr_finish(m) \
do{\
    (m)->ops->finish(m);\
}while(0) \

#define gp_tmr_wait(m) \
do{\
    (m)->ops->wait(m);\
}while(0) \

#define gp_tmr_run(m) \
do{\
    (m)->ops->run(m);\
}while(0) \

#define gp_mtx_lock(m) \
do{\
    int rv;\
    if((rv = pthread_mutex_lock(&((m)->mtx))) != 0){ \
        printf("pthread_mutex_lock :%s", strerror(rv));\
    } \
}while(0) \

#define gp_mtx_unlock(m) \
do{ \
    int rv;\
    if((rv = pthread_mutex_unlock(&((m)->mtx))) != 0){ \
        printf("pthread_mutex_unlock :%s", strerror(rv));\
    } \
}while(0) \

#define gp_cv_signal(c) \
do { \
    int rv;\
    if((rv = pthread_cond_signal(&((c)->cv))) != 0){ \
        printf("pthread_cond_signal :%s", strerror(rv));\
    } \
}while(0) \

#define gp_cv_wait(c) \
do { \
    int rv;\
    if((rv = pthread_cond_wait(&((c)->cv), &((c)->mtx->mtx))) != 0){ \
        printf("pthread_cond_wait :%s", strerror(rv));\
    } \
}while(0) \

#define gp_cv_wake(c) \
do { \
    int rv;\
    if((rv = pthread_cond_broadcast(&((c)->cv)) != 0)){ \
        printf("pthread_cond_broadcast :%s", strerror(rv));\
    } \
}while(0) \


extern void create_gp_thr_manager(gp_thread_manager **gp_tmr, gp_thread_fn fn, void *arg);
extern int init_gp_thr_manager(gp_thread_manager *gp_tmr, gp_thread_fn fn, void *arg);
extern void destroy_gp_thr_manager(gp_thread_manager *gp_tmr);
extern int init_gp_thr(gp_thread *gp_thr, gp_thread_fn fn, void *arg);
extern void init_gp_mtx(gp_mtx *mtx);
extern void destroy_gp_mtx(gp_mtx *mtx);
extern void init_gp_cv(gp_cv *cv, gp_mtx *mtx);
extern void destroy_gp_cv(gp_cv *cv);
extern gp_thread_mgr_operations gp_tmr_ops;

/*-----------------------------------------------------------------------------------------------*/


extern void create_task_processor(gp_task_processor **gp_tp, int num);
extern void init_task_processor(gp_task_processor *gp_tp, int num);
extern void destroy_task_processor(gp_task_processor *gp_tp);
extern void create_task(gp_task ** task, gp_task_processor *gp_tp, gp_cb cb, void *arg);
extern void init_task(gp_task *task, gp_task_processor *gp_tp, gp_cb cb, void *arg);
extern void run_task(gp_task *task);
extern void wait_task(gp_task *task);
extern void destroy_task(gp_task *task);

/*-----------------------------------------------------------------------------------------------*/
#define DICT_OK 0
#define DICT_ERR 1
#define DICT_NOTUSED(V) ((void) V)
/* This is the initial size of every hash table */
#define DICT_HT_INITIAL_SIZE     4

typedef void (dictScanFunction)(void *privdata, const dictEntry *de);
typedef void (dictScanBucketFunction)(void *privdata, dictEntry **bucketref);

#define dictFreeVal(d, entry) \
    if ((d)->type->valDestructor) \
        (d)->type->valDestructor((d)->privdata, (entry)->v.val)

#define dictSetVal(d, entry, _val_) do { \
    if ((d)->type->valDup) \
        (entry)->v.val = (d)->type->valDup((d)->privdata, _val_); \
    else \
        (entry)->v.val = (_val_); \
} while(0)

#define dictSetSignedIntegerVal(entry, _val_) \
    do { (entry)->v.s64 = _val_; } while(0)

#define dictSetUnsignedIntegerVal(entry, _val_) \
    do { (entry)->v.u64 = _val_; } while(0)

#define dictSetDoubleVal(entry, _val_) \
    do { (entry)->v.d = _val_; } while(0)

#define dictFreeKey(d, entry) \
    if ((d)->type->keyDestructor) \
        (d)->type->keyDestructor((d)->privdata, (entry)->key)

#define dictSetKey(d, entry, _key_) do { \
    if ((d)->type->keyDup) \
        (entry)->key = (d)->type->keyDup((d)->privdata, _key_); \
    else \
        (entry)->key = (_key_); \
} while(0)

#define dictCompareKeys(d, key1, key2) \
    (((d)->type->keyCompare) ? \
        (d)->type->keyCompare((d)->privdata, key1, key2) : \
        (key1) == (key2))

#define dictHashKey(d, key) (d)->type->hashFunction(key)
#define dictGetKey(he) ((he)->key)
#define dictGetVal(he) ((he)->v.val)
#define dictGetSignedIntegerVal(he) ((he)->v.s64)
#define dictGetUnsignedIntegerVal(he) ((he)->v.u64)
#define dictGetDoubleVal(he) ((he)->v.d)
#define dictSlots(d) ((d)->ht[0].size+(d)->ht[1].size)
#define dictSize(d) ((d)->ht[0].used+(d)->ht[1].used)
#define dictIsRehashing(d) ((d)->rehashidx != -1)


extern dict *dictCreate(int key_type, int value_type);
extern dict *_dictCreate(dictType *type, void *privDataPtr);
extern int dictExpand(dict *d, unsigned long size);
extern int dictAdd(dict *d, void *key, void *val);
extern dictEntry *dictAddRaw(dict *d, void *key, dictEntry **existing);
extern dictEntry *dictAddOrFind(dict *d, void *key);
extern int dictReplace(dict *d, void *key, void *val);
extern int dictDelete(dict *d, const void *key);
extern dictEntry *dictUnlink(dict *ht, const void *key);
extern void dictFreeUnlinkedEntry(dict *d, dictEntry *he);
extern void dictRelease(dict *d);
extern dictEntry * dictFind(dict *d, const void *key);
extern void *dictFetchValue(dict *d, const void *key);
extern int dictResize(dict *d);
extern dictIterator *dictGetIterator(dict *d);
extern dictIterator *dictGetSafeIterator(dict *d);
extern dictEntry *dictNext(dictIterator *iter);
extern void dictReleaseIterator(dictIterator *iter);
extern dictEntry *dictGetRandomKey(dict *d);
extern dictEntry *dictGetFairRandomKey(dict *d);
extern unsigned int dictGetSomeKeys(dict *d, dictEntry **des, unsigned int count);
extern void dictGetStats(char *buf, size_t bufsize, dict *d);
extern uint64_t dictGenHashFunction(const void *key, int len);
extern uint64_t dictGenCaseHashFunction(const unsigned char *buf, int len);
extern void dictEmpty(dict *d, void(callback)(void*));
extern void dictEnableResize(void);
extern void dictDisableResize(void);
extern int dictRehash(dict *d, int n);
extern int dictRehashMilliseconds(dict *d, int ms);
extern void dictSetHashFunctionSeed(uint8_t *seed);
extern uint8_t *dictGetHashFunctionSeed(void);
extern unsigned long dictScan(dict *d, unsigned long v, dictScanFunction *fn, dictScanBucketFunction *bucketfn, void *privdata);
extern uint64_t dictGetHash(dict *d, const void *key);
extern dictEntry **dictFindEntryRefByPtrAndHash(dict *d, const void *oldptr, uint64_t hash);

/* Hash table types */
extern dictType dictTypeHeapStringCopyKey;
extern dictType dictTypeHeapStrings;
extern dictType dictTypeHeapStringCopyKeyValue;
/*-----------------------------------------------------------------------------------------------*/

extern int siptlw(int c);

#define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define U32TO8_LE(p, v)                                                        \
    (p)[0] = (uint8_t)((v));                                                   \
    (p)[1] = (uint8_t)((v) >> 8);                                              \
    (p)[2] = (uint8_t)((v) >> 16);                                             \
    (p)[3] = (uint8_t)((v) >> 24);

#define U64TO8_LE(p, v)                                                        \
    U32TO8_LE((p), (uint32_t)((v)));                                           \
    U32TO8_LE((p) + 4, (uint32_t)((v) >> 32));

#define U8TO64_LE(p) (*((uint64_t*)(p)))

#define U8TO64_LE_NOCASE(p)                                                    \
    (((uint64_t)(siptlw((p)[0]))) |                                           \
     ((uint64_t)(siptlw((p)[1])) << 8) |                                      \
     ((uint64_t)(siptlw((p)[2])) << 16) |                                     \
     ((uint64_t)(siptlw((p)[3])) << 24) |                                     \
     ((uint64_t)(siptlw((p)[4])) << 32) |                                              \
     ((uint64_t)(siptlw((p)[5])) << 40) |                                              \
     ((uint64_t)(siptlw((p)[6])) << 48) |                                              \
     ((uint64_t)(siptlw((p)[7])) << 56))

#define SIPROUND                                                               \
    do {                                                                       \
        v0 += v1;                                                              \
        v1 = ROTL(v1, 13);                                                     \
        v1 ^= v0;                                                              \
        v0 = ROTL(v0, 32);                                                     \
        v2 += v3;                                                              \
        v3 = ROTL(v3, 16);                                                     \
        v3 ^= v2;                                                              \
        v0 += v3;                                                              \
        v3 = ROTL(v3, 21);                                                     \
        v3 ^= v0;                                                              \
        v2 += v1;                                                              \
        v1 = ROTL(v1, 17);                                                     \
        v1 ^= v2;                                                              \
        v2 = ROTL(v2, 32);                                                     \
    } while (0)

uint64_t siphash(const uint8_t *in, const size_t inlen, const uint8_t *k);
uint64_t siphash_nocase(const uint8_t *in, const size_t inlen, const uint8_t *k);

/*-----------------------------------------------------------------------------------------------*/

extern void gp_register_module(gp_module_desc *desc);


#define gp_module_init(module) \
static void __attribute__((constructor)) do_vs_init_ ## module(void) {  \
        if(module.early_init){ \
        module.early_init(); \
    } \
    GP_LIST_NODE_INIT(&module.node) \
    module.stat.file = __FILE__;        \
        gp_register_module(&module);    \
}

extern int gp_init_modules(void);


/*-----------------------------------------------------------------------------------------------*/

extern void create_gp_timer(gp_timer_list **, void (*fn)(void *), void *, unsigned long, int, int);
extern void init_gp_timer(gp_timer_list *, void (*fn)(void *), void *, unsigned long, int, int);
extern void gp_timer_add(gp_timer_base *, gp_timer_list *);
extern void gp_timer_del(gp_timer_list *);
extern void gp_timer_mod(gp_timer_base *, gp_timer_list *, unsigned long, int, int);
extern void destruct_gp_timer(gp_timer_list *);
extern void create_gp_timer_base(gp_timer_base **, unsigned long);
extern void init_gp_timer_base(gp_timer_base *, unsigned long);
extern void gp_run_timers(gp_timer_base *, unsigned long);
extern void destruct_gp_timer_base(gp_timer_base *);
extern unsigned long gp_time(gp_clocktype);




#endif


