#ifndef __GP_H__
#define __GP_H__

#define _GNU_SOURCE 1
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <endian.h>
#include <netdb.h>
#include <protobuf-c/protobuf-c.h>

#ifndef likely
	#define likely(x)     __builtin_expect((x), 1)
#endif

#ifndef unlikely
	#define unlikely(x)   __builtin_expect((x), 0)
#endif

typedef struct gp_atomic_s gp_atomic;
typedef struct gp_sock_address_s gp_sock_address;
typedef struct gp_socket_s gp_socket;
typedef struct gp_pending_task_s gp_pending_task;
typedef struct gp_loop_s gp_loop;
typedef struct gp_handler_s gp_handler;
typedef struct gp_epoller_s gp_epoller;
typedef struct gp_acceptor_s gp_acceptor;
typedef struct gp_server_s gp_server;
typedef struct gp_connection_s gp_connection;
typedef struct gp_buffer_s gp_buffer;
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
typedef struct gp_timer_base_s gp_timer_base;
typedef struct gp_timer_list_s gp_timer_list;
typedef struct tvec_root_s tvec_root;
typedef struct tvec_s	tvec;
typedef struct gp_ring_s gp_ring;
typedef struct gp_task_wait_strategy_s gp_task_wait_strategy;

typedef void (*gp_io_cb)(gp_loop *loop, gp_io *w, unsigned int events);
typedef void (*gp_cb)(void *);
typedef void (*gp_thread_fn)(void *);
typedef void (*gp_event_callback)(gp_handler *);
typedef void (*gp_connection_callback)(gp_connection *);
typedef void (*gp_close_callback)(gp_connection *);
typedef void (*gp_write_complete_callback)(gp_connection *);
typedef void (*gp_message_callback)(gp_connection *, gp_buffer*);
typedef void (*gp_new_connection_callback)(int32_t, struct sockaddr *);
typedef void (*gp_pending_func)(gp_server *, gp_connection *, char *, int);
typedef uint32_t (*gp_protobuf_msg_callback)(gp_connection *, ProtobufCMessage *);

enum gp_run_mode_s{
    GP_RUN_DEFAULT = 0,
    GP_RUN_ONCE,
    GP_RUN_NOWAIT
};
typedef enum gp_run_mode_s gp_run_mode;

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
    uint32_t pevents;
    uint32_t events;
    int fd; 
};

struct gp_atomic_s {
	volatile int64_t counter;
};

struct gp_loop_s {
    int8_t        quit;
    int8_t        looping;
    int8_t        calling_pending_functors;
    int32_t       wakeup_fd;
    int64_t       tid;
    uint32_t      flags;
    uint32_t      time;
	
    gp_timer_base *timer_base;
	gp_epoller    *epoller;
	gp_handler	  *wakeup_handler;

    gp_list       active_handler_list;
    gp_list       pending_list;
};

struct gp_handler_s{
    int8_t		  	  add_to_loop;
    int8_t		  	  event_handling;

	int32_t       	  index;
    int32_t       	  fd;
    int32_t       	  _events;
    int32_t       	  _revents;

    gp_loop       	  *loop;

	gp_list_node	  handler_node;
    gp_event_callback _read_callback;
    gp_event_callback _write_callback;
    gp_event_callback _close_callback;
    gp_event_callback _error_callback;
};

struct epoll_event;
typedef struct epoll_event* event_list;
struct gp_epoller_s{
	int32_t    epollfd;
	event_list events;
	int32_t    events_len;
	dict	   *handlers;
};

struct gp_acceptor_s{
	uint8_t		listenning;
	uint32_t	fd;
	gp_loop 	*loop;
	gp_handler  *accept_handler;
	gp_new_connection_callback new_connection_callback;
};

enum address_type {
    IP,
	IPV6,
	DOMAIN,
};

struct gp_sock_address_s{
	union {
		struct sockaddr_in  addr;
		struct sockaddr_in6 addr6;
		struct sockaddr_un  path;
	};
};

enum gp_state {
	k_disconnected,
	k_connecting,
	k_connected,
	k_disconnecting
};

struct gp_connection_s{
	gp_atomic ref;
	gp_loop *loop;
	gp_handler *handler;
	int32_t fd;
	gp_sock_address local_addr;
	gp_sock_address peer_addr;
	gp_buffer *input_buffer;
	gp_buffer *output_buffer;
	enum gp_state state;
	gp_connection_callback 		 connection_callback;
	gp_close_callback 		 	 close_callback;
	gp_message_callback    		 message_callback;
	gp_write_complete_callback   write_complete_callback;
};

struct gp_buffer_s{
	char 	*buffer;
	size_t  len;
	size_t  reader_index;
	size_t  writer_index;
};


enum{
    GP_RUN_IN_LOOP_TRANS = 0,
    GP_RUN_IN_LOOP_CONN,
    GP_RUN_IN_LOOP_REMOVE_CONN,
    GP_RUN_IN_LOOP_SERVER_START,
};

struct gp_pending_task_s{
	int8_t				type;
	gp_pending_func		pending_func;

	gp_server		*server;
	gp_connection	*conn;
	char				*msg;
	int					len;
	
	gp_list_node		pending_task_node;
};

struct gp_server_s{
	char					   name[15];
	char				       hostport[5];
	int32_t					   next_conn_id;
	int32_t					   started;
	gp_loop 				   *loop;
	gp_acceptor 			   *acceptor;
	dict 				       *connections;
	
	gp_message_callback 	   message_callback;
	gp_write_complete_callback write_complete_callback;
	gp_connection_callback 	   connection_callback;
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
    void *               task_arg; //task对应的 消息
    gp_cb                task_cb;  //task对应的 处理函数
    gp_task_processor *task_tp;    //task 管理结构
    gp_thread_manager *task_tmr;
    uint32_t         task_busy;
    gp_mtx        task_mtx;
    gp_cv             task_cv;
};

struct gp_taskp_tmr_s {
    gp_task_processor * tpt_tp;
    gp_thread_manager tpt_thread;
};


typedef int (*wait_for)(const void *, const void *);

struct gp_task_wait_strategy_s {
	wait_for	func;
};

//TODO: priority queue
// a threadpool
struct gp_task_processor_s {
    gp_list       tp_tasks;
    gp_ring      *tp_tasks_ring;
    gp_mtx        tp_mtx;
    gp_cv         tp_sched_cv;
    gp_cv         tp_wait_cv;
    gp_taskp_tmr *tp_threads;
	gp_task_wait_strategy *wait_strategy;
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

enum gp_module_init_type_s {
    MODULE_INIT_FIRST,
};
typedef enum gp_module_init_type_s gp_module_init_type;

struct gp_module_desc_s {
    gp_list_node node;
    int type;

    int priority;
    char name[20];
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
typedef enum gp_clocktype_s gp_clocktype;

struct tvec_s {
    gp_list vec[TVN_SIZE];
};

struct tvec_root_s {
    gp_list vec[TVR_SIZE];
};

struct gp_timer_base_s {
    uint32_t timer_jiffies;
    uint32_t next_timer;
    tvec_root tv1;
    tvec tv2;
    tvec tv3;
    tvec tv4;
    tvec tv5;
};

struct gp_timer_list_s {
    gp_list_node node;
    gp_loop *loop;
    uint32_t expires;
    int32_t interval;
    int32_t repeat;
    void (*callback)(void *);
    void *data;
    uint32_t state;
};

#define CACHE_LINE_SIZE 64
#define RBUFSIZE 1024
#define RBUFMASK RBUFSIZE-1

struct gp_ring_headtail_s {
	volatile uint32_t first;
	volatile uint32_t second;
};

struct gp_ring_s {
	char name[32] __attribute((aligned(CACHE_LINE_SIZE)));

	uint32_t size;// size of ring
	uint32_t mask;// mask (size - 1) of ring
	uint32_t capacity;// usable size of ring

    char pad0 __attribute((aligned(CACHE_LINE_SIZE)));// usable size of ring

	struct {
		volatile uint32_t first;//这里使用 first 和second的原因是因为 可以保证多生产和多消费同时进行,并且可以保证一个顺序
		volatile uint32_t second; //使用volatile是因为编译器的cas接口要求入参是volatile
	}head __attribute((aligned(CACHE_LINE_SIZE)));//cacheline 对齐
    char pad1 __attribute((aligned(CACHE_LINE_SIZE)));//

    struct {
		volatile uint32_t first;
		volatile uint32_t second;
	}tail __attribute((aligned(CACHE_LINE_SIZE)));
    char pad2 __attribute((aligned(CACHE_LINE_SIZE)));// 

    void *msgs[0];	
};


typedef struct gp_conf_node_ {
    char *name;
    char *val;

    int is_seq;

    /**< Flag that sets this nodes value as final. */
    int final;

    struct gp_conf_node_ *parent;
    TAILQ_HEAD(, gp_conf_node_) head;
    TAILQ_ENTRY(gp_conf_node_) next;
} gp_conf_node;


/*-----------------------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------------------*/
extern int32_t create_gp_loop(gp_loop **);
extern int32_t init_gp_loop(gp_loop *); 

extern int32_t gp_loop_run(gp_loop *, gp_run_mode);
extern void    gp_loop_timer_start(gp_loop *, void (*fn)(void *), void *, int32_t , int32_t );
extern void    gp_loop_timer_stop(gp_timer_list *);
extern void    gp_loop_timer_mod(gp_loop *, gp_timer_list *,uint32_t, int32_t, int32_t);
extern void    gp_loop_run_timers(gp_loop *);
extern void    gp_loop_update_time(gp_loop *);
extern void    gp_loop_quit(gp_loop*);
extern void    gp_run_in_loop(gp_loop *loop, gp_pending_task *task);
extern void    gp_queue_in_loop(gp_loop *loop, gp_pending_task *task);
extern int8_t  is_in_loop_thread(gp_loop *loop);
extern void    gp_loop_remove_handler(gp_loop*, gp_handler *);
extern void    gp_loop_update_handler(gp_loop*, gp_handler *);

/*-----------------------------------------------------------------------------------------------*/
extern void    create_gp_epoller(gp_epoller **);
extern void    init_gp_epoller(gp_epoller *);
extern void    poller_poll(gp_epoller *, int32_t, gp_list *);
extern void    remove_handler(gp_epoller *, gp_handler *);
extern void    update_handler(gp_epoller *, gp_handler *);

/*-----------------------------------------------------------------------------------------------*/
extern void    create_gp_handler(gp_handler **, gp_loop *, int fd);
extern void    init_gp_handler(gp_handler *, gp_loop *, int fd);
extern void    destruct_gp_handler(gp_handler *);
extern void    set_read_callback(gp_handler *, gp_event_callback);
extern void    set_write_callback(gp_handler *, gp_event_callback);
extern void    set_close_callback(gp_handler *, gp_event_callback);
extern void    set_error_callback(gp_handler *, gp_event_callback);
extern void    set_revents(gp_handler *, int revt);
extern void    enable_writing(gp_handler *);
extern void    enable_reading(gp_handler *);
extern void    disable_writing(gp_handler *);
extern void    disable_reading(gp_handler *);
extern void    disable_all(gp_handler *);
extern int8_t  is_reading(gp_handler *);
extern int8_t  is_writing(gp_handler *);
extern int8_t  is_none_event(gp_handler *);
extern void    handle_event(gp_handler *);
extern void    handler_remove(gp_handler *);

/*-----------------------------------------------------------------------------------------------*/
extern void    create_gp_acceptor(gp_acceptor **, gp_loop *, gp_sock_address *);
extern void    init_gp_acceptor(gp_acceptor *, gp_loop *, gp_sock_address *);
extern void    set_new_connection_callback(gp_acceptor *, gp_new_connection_callback);
extern void    acceptor_listen(gp_acceptor *);

/*-----------------------------------------------------------------------------------------------*/
extern void    create_gp_sock_address(gp_sock_address **, char *, uint16_t, uint8_t);
extern struct sockaddr* create_sockaddr(struct sockaddr *);
extern void    init_gp_sock_address(gp_sock_address *, char *, uint16_t, uint8_t);
extern void    init_gp_sock_address_by_sockaddr(gp_sock_address *, struct sockaddr *);
extern void    get_gp_sock_address(gp_sock_address *, char[], int);
extern int32_t get_gp_sock_len(gp_sock_address *);
extern int32_t get_gp_sock_len_by_sockaddr(struct sockaddr *);
extern int32_t get_gp_sock_len_by_fd(int32_t);

/*-----------------------------------------------------------------------------------------------*/
extern void    create_gp_pending_task(gp_pending_task **, int8_t, gp_pending_func, gp_server *, gp_connection *, char *, int );
extern void    init_gp_pending_task(gp_pending_task *, int8_t, gp_pending_func, gp_server *, gp_connection *, char *, int );
extern void    destruct_gp_pending_task(gp_pending_task *);

/*-----------------------------------------------------------------------------------------------*/
extern void	   create_gp_server(gp_server **, gp_loop *, gp_sock_address *, char *);
extern void	   init_gp_server(gp_server *, gp_loop *, gp_sock_address *, char *);
extern void    server_set_write_complete_callback(gp_server *, gp_write_complete_callback);
extern void    server_set_message_callback(gp_server *, gp_message_callback);
extern void    server_set_connection_callback(gp_server *, gp_connection_callback);
extern void    start_server(gp_server *);
extern gp_server *   get_server(void);

/*-----------------------------------------------------------------------------------------------*/
extern void	   create_gp_connection(gp_connection **, gp_loop *, int32_t, struct sockaddr *, struct sockaddr *);
extern void	   init_gp_connection(gp_connection *, gp_loop *, int32_t, struct sockaddr *, struct sockaddr *);
extern void	   destruct_gp_connection(gp_connection *);
extern void    conn_set_write_complete_callback(gp_connection *, gp_write_complete_callback);
extern void    conn_set_message_callback(gp_connection *, gp_message_callback);
extern void    conn_set_connection_callback(gp_connection *, gp_connection_callback);
extern void    conn_set_close_callback(gp_connection *, gp_close_callback);
extern void    conn_force_close(gp_connection *);
extern void    conn_shutdown(gp_connection *);
extern void    conn_send(gp_connection *, char *, int);
extern void    conn_send_in_loop(gp_connection *, char *, int);
extern void    connection_established(gp_connection *);
extern void    connection_destroyed(gp_connection *);
extern void    conn_ref_inc(gp_connection **);
extern void    conn_ref_dec(gp_connection **);

/*-----------------------------------------------------------------------------------------------*/
extern void	   create_gp_buffer(gp_buffer **);
extern void	   init_gp_buffer(gp_buffer *);
extern void    destruct_gp_buffer(gp_buffer *);
extern size_t  readable_bytes(gp_buffer *);
extern size_t  writable_bytes(gp_buffer *);
extern size_t  prepenable_bytes(gp_buffer *);
extern char*   peek(gp_buffer *);
extern void	   retrieve(gp_buffer *, size_t);
extern void	   retrieve_all(gp_buffer *);
extern char*   retrieve_as_string(gp_buffer *, size_t);
extern char*   retrieve_all_as_string(gp_buffer *);
extern void	   buffer_append(gp_buffer *, char *, size_t);
extern size_t  buffer_read_fd(gp_buffer *, int32_t, int *);

/*-----------------------------------------------------------------------------------------------*/
extern void	   register_name_pb_map(char *, const ProtobufCMessageDescriptor *desc);
extern size_t encode(ProtobufCMessage *, uint8_t **);
extern ProtobufCMessage* decode(gp_buffer *);

/*-----------------------------------------------------------------------------------------------*/
extern void  get_local_address(int32_t, char a[], int *, int);
extern void  get_peer_address(int32_t, char a[], int *, int);

extern void  register_msg_callback(char *name, gp_protobuf_msg_callback);
extern gp_protobuf_msg_callback  get_msg_callback(const char *name);

/*-----------------------------------------------------------------------------------------------*/
extern void  gp_list_init_offset(gp_list *, size_t);
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

//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define GP_LIST_INIT(list, type, field) \
        gp_list_init_offset(list, offsetof(type, field))

#define GP_LIST_NODE_INIT(node)                       \
        {                                              \
                (node)->ln_prev = (node)->ln_next = NULL; \
        }

#define GP_LIST_FOREACH(l, it) \
        for (it = gp_list_first(l); it != NULL; it = gp_list_next(l, it))

//gp_list_foreach_safe 是因为 如果要遍历删除的话, 删除后的节点p  p->next ==
//NULL了 因此不会执行下一次循环了,  所以得先有一个指针指着下一个 然后删除上一个
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

/*----------------------------------------------gp_task-----------------------------------------------*/


extern void create_task_processor(gp_task_processor **gp_tp, int num);
extern void init_task_processor(gp_task_processor *gp_tp, int num);
extern void destroy_task_processor(gp_task_processor *gp_tp);
extern void create_task(gp_task ** task, gp_task_processor *gp_tp, gp_cb cb, void *arg);
extern void init_task(gp_task *task, gp_task_processor *gp_tp, gp_cb cb, void *arg);
extern void run_task(gp_task *task);
extern void wait_task(gp_task *task);
extern void destroy_task(gp_task *task);

/*----------------------------------------------gp_dict-----------------------------------------------*/
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
/*-------------------------------------------gp_hash---------------------------------------------------*/


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

extern int siptlw(int c);
extern uint64_t siphash(const uint8_t *in, const size_t inlen, const uint8_t *k);
extern uint64_t siphash_nocase(const uint8_t *in, const size_t inlen, const uint8_t *k);

/*---------------------------------------------gp_module------------------------------------------------*/
#define gp_module_init(module, arg1, arg2, func1, func2) \
static void __attribute__((constructor)) do_gp_init_ ## module(void) {  \
		gp_module_desc module;\
		module.type = arg1; \
        strcpy(module.name, arg2); \
        module.init = func1; \
        module.early_init = func2; \
        \
        if(module.early_init){ \
        module.early_init(); \
    } \
    GP_LIST_NODE_INIT(&module.node) \
    module.stat.file = __FILE__;        \
        gp_register_module(&module);    \
}

extern int gp_init_modules(void);
extern void gp_register_module(gp_module_desc *desc);


/*---------------------------------------------gp_timer-------------------------------------------------*/

extern void create_gp_timer(gp_timer_list **, void (*fn)(void *), void *, uint32_t, int32_t, int32_t);
extern void init_gp_timer(gp_timer_list *, void (*fn)(void *), void *, uint32_t, int32_t, int32_t);
extern void gp_timer_add(gp_timer_base *, gp_timer_list *);
extern void gp_timer_del(gp_timer_list *);
extern void gp_timer_mod(gp_timer_base *, gp_timer_list *, uint32_t, int32_t, int32_t);
extern void destruct_gp_timer(gp_timer_list *);
extern void create_gp_timer_base(gp_timer_base **, uint32_t);
extern void init_gp_timer_base(gp_timer_base *, uint32_t);
extern void gp_run_timers(gp_timer_base *, uint32_t);
extern void destruct_gp_timer_base(gp_timer_base *);
extern uint64_t gp_time(gp_clocktype);


/*----------------------------------------------gp_ring---------------------------------------------*/
extern void create_gp_ring(gp_ring **, int);
extern void init_gp_ring(gp_ring *, int);
extern int gp_ring_push(gp_ring *, int, void **);
extern int gp_ring_pop(gp_ring *, int, void **);


/*-----------------------------------------------gp_conf--------------------------------------------*/
void gp_conf_init(void);
void gp_conf_deinit(void);
gp_conf_node *gp_conf_get_root_node(void);
int gp_conf_get(const char *name, const char **vptr);
int gp_conf_get_value(const char *name, const char **vptr);
int gp_conf_get_int(const char *name, int *val);
int gp_conf_get_bool(const char *name, int *val);
int gp_conf_get_double(const char *name, double *val);
int gp_conf_get_float(const char *name, float *val);
int gp_conf_set(const char *name, const char *val);
int gp_conf_set_from_string(const char *input, int final);
int gp_conf_set_final(const char *name, const char *val);
void gp_conf_dump(void);
void gp_conf_node_dump(const gp_conf_node *node, const char *prefix);
gp_conf_node *gp_conf_node_new(void);
void gp_conf_node_free(gp_conf_node *);
gp_conf_node *gp_conf_get_node(const char *key);
gp_conf_node *gp_conf_get_node_in_array(const char *head_name, int i, const char *name);
void gp_conf_create_context_backup(void);
void gp_conf_restore_context_backup(void);
gp_conf_node *gp_conf_node_lookup_child(const gp_conf_node *node, const char *key);
const char *gp_conf_node_lookup_child_value(const gp_conf_node *node, const char *key);
void gp_conf_node_remove(gp_conf_node *);
void gp_conf_register_tests(void);
int gp_conf_node_child_value_is_true(const gp_conf_node *node, const char *key);
int gp_conf_val_is_true(const char *val);
int gp_conf_val_is_false(const char *val);
void gp_conf_node_prune(gp_conf_node *node);
int gp_conf_remove(const char *name);
int gp_conf_node_has_children(const gp_conf_node *node);
int gp_conf_get_siblings_num(gp_conf_node * node);

gp_conf_node *gp_conf_get_child_with_default(const gp_conf_node *base, const gp_conf_node *dflt, const char *name);
gp_conf_node *gp_conf_node_lookup_key_value(const gp_conf_node *base, const char *key, const char *value);
int gp_conf_get_child_value(const gp_conf_node *base, const char *name, const char **vptr);
int gp_conf_get_child_value_int(const gp_conf_node *base, const char *name, intmax_t *val);
int gp_conf_get_child_value_bool(const gp_conf_node *base, const char *name, int *val);
int gp_conf_get_child_value_with_default(const gp_conf_node *base, const gp_conf_node *dflt, const char *name, const char **vptr);
int gp_conf_get_child_value_int_with_default(const gp_conf_node *base, const gp_conf_node *dflt, const char *name, intmax_t *val);
int gp_conf_get_child_value_bool_with_default(const gp_conf_node *base, const gp_conf_node *dflt, const char *name, int *val);
char *gp_conf_load_complete_include_path(const char *);
int gp_conf_node_is_sequence(const gp_conf_node *node);
int gp_conf_yaml_load_file(const char *);                                  
int gp_conf_yaml_load_string(const char *, size_t);
int gp_conf_yaml_load_file_with_prefix(const char *filename, const char *prefix);

size_t strlcpy(char *dst, const char *src, size_t siz);
size_t strlcat(char *dst, const char *src, size_t siz);

/*-----------------------------------------------gp_atomic--------------------------------------------*/

#define LOCK_PREFIX_HERE \
        ".section .smp_locks,\"a\"\n"   \
        ".balign 4\n"           \
        ".long 671f - .\n" /* offset */ \
        ".previous\n"           \
        "671:"
#define LOCK_PREFIX LOCK_PREFIX_HERE "\n\tlock; "


#define gp_atomic_get x86_64_atomic64_get
#define gp_atomic_set(ptr, value)   x86_64_atomic64_set(ptr, value)
#define gp_atomic_add(ptr, value)   x86_64_atomic64_add(ptr, value)
#define gp_atomic_inc(ptr)  x86_64_atomic64_inc(ptr)
#define gp_atomic_dec(ptr)  x86_64_atomic64_dec(ptr)
#define gp_atomic_add_and_test(ptr, value)  x86_64_atomic64_add_and_test(ptr, value)
#define gp_atomic_dec_and_test(ptr) x86_64_atomic64_dec_and_test(ptr)
#define gp_atomic_inc_and_test(ptr) x86_64_atomic64_inc_and_test(ptr)

static inline void x86_64_atomic64_init(gp_atomic *ptr)
{   
    ptr->counter = 0;
}
/**
 * atomic_get - read atomic variable
 * param @v: pointer of type atomic32_t 
 *
 * Atomically reads the value of @v
 */
static inline int64_t x86_64_atomic64_get(gp_atomic *ptr)
{   
    return (*(volatile int64_t *)&(ptr)->counter);
}

/**
 * atomic_set - set atomic variable
 * param @v: pointer of type atomic32_t 
 * param @i: integer or long interger of required value
 *
 * Atomically sets the value of @v to @i
 */
static inline void x86_64_atomic64_set(gp_atomic *ptr, int64_t i)
{   
    (ptr)->counter = i;
}

/**
 * atomic_add - add integer to atomic vatiable
 * param @v: pointer of type atomic32_t 
 * param @i: interger value to add
 *
 * Atomically adds @i to @v
 */
static inline void x86_64_atomic64_add(gp_atomic *v, long i)
{
    __asm__ __volatile__ (
        LOCK_PREFIX "addq %1,%0 \n\t"
        :"+m"(v->counter)
        :"ir"((long)i)
        );
}

/**
 * atomic_inc - increment atomic variable
 * param @v: pointer of type atomic32_t 
 * 
 * Atomically increments @v by 1
 */
static inline void x86_64_atomic64_inc(gp_atomic *v)
{
    __asm__ __volatile__ (
        LOCK_PREFIX "incq %0    \n\t"
        :"+m"(v->counter)
        );
}

/**
 * atomic_dec - decrement atomic variable
 * param @v: pointer of type of atomic32_t 
 * 
 * Atomically decrements @v by 1
 */
static inline void x86_64_atomic64_dec(gp_atomic *v)
{
    __asm__ __volatile__ (
        LOCK_PREFIX "decq %0    \n\t"
        :"+m"(v->counter)
        );
}
/**
 * atomic_inc_and_test - increment and test
 * param @v: pointer of type of atomic64_t 
 *
 * Atomically increment @v by 1
 * if (atomic value == 0) return true;
 * else return false
 */
static inline int x86_64_atomic64_inc_and_test(gp_atomic *v)
{
    __asm__ __volatile__ (
        LOCK_PREFIX "incq %0    \n\t"
        :"+m"(v->counter)
        :
        :"memory");
    return v->counter == 0;
}

/**
 * atomic_dec_and_test -  decrement and test
 * param @v: pointer of type of atomic64_t 
 *
 * Atomically decrement @v by 1
 * if (atomic value == 0) return true;
 * else return false;
 */
static inline int x86_64_atomic64_dec_and_test(gp_atomic *v)
{
    __asm__ __volatile__ (
        LOCK_PREFIX "decq %0    \n\t" 
        :"+m"(v->counter)
        :
        :"memory"
        );
    return v->counter == 0;
}

/**
 * atomic64_add_and_test - add and test
 * param @v: pointer of type of atomic64_t 
 * param @i:integer value to add
 * 
 * Atomically adds @i to @v
 * if (atomic value == 0) return true;
 * else return false
 */
    
static inline int x86_64_atomic64_add_and_test(gp_atomic *v, long i)
{
    __asm__ __volatile__ (
        LOCK_PREFIX "addq %1, %0    \n\t"
        :"+m"(v->counter)
        :"ir"((long)i)
        :"memory"
        );
    return v->counter == 0;
}



#endif


