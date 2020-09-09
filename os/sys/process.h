#ifndef PROCESS_H
#define PROCESS_H

#include <vcrtos/config.h>
#include <vcrtos/instance.h>
#include <vcrtos/thread.h>
#include <vcrtos/mutex.h>
#include <vcrtos/event.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PROCESS_MAX_EVENTS
#define PROCESS_MAX_EVENTS KERNEL_MAXTHREADS
#endif

#define PROCESS_NONE NULL

#define PT_WAITING 0
#define PT_YIELDED 1
#define PT_EXITED  2
#define PT_ENDED   3

typedef unsigned char process_event_t; /* event id */
typedef void *process_data_t;

typedef enum
{
    PROCESS_EVENT_PRIO_HIGH,
    PROCESS_EVENT_PRIO_MEDIUM,
    PROCESS_EVENT_PRIO_LOW,
    PROCESS_EVENT_PRIO_NUMOF
} process_event_prio_t;

#define PROCESS_EVENT_NONE 0
#define PROCESS_EVENT_INIT 1
#define PROCESS_EVENT_POLL 2
#define PROCESS_EVENT_EXIT 3
#define PROCESS_EVENT_SERVICE_REMOVED 4
#define PROCESS_EVENT_CONTINUE 5
#define PROCESS_EVENT_MSG 6
#define PROCESS_EVENT_EXITED 7
#define PROCESS_EVENT_TIMER 8
#define PROCESS_EVENT_COM 9

#define PROCESS_EVENT_LAST 10

struct process
{
    unsigned short lc;
    kernel_pid_t pid;
    mutex_t mutex;
    process_event_t event;
    process_data_t data;
    char *stack;
    size_t stack_size;
    const char *process_name;
    thread_handler_func_t thread_handler;
    void *instance;
};

typedef struct
{
    event_t super;
    struct process *process;
    process_event_prio_t priority;
    unsigned char event_id;
    void *data;
} process_custom_event_t;

#define PROCESS(name, strname, size) \
    static char process_stack_##name[size]; \
    void *process_handler_##name(void *arg); \
    struct process name = {  \
        .pid = KERNEL_PID_UNDEF, \
        .stack = process_stack_##name, \
        .stack_size = size, \
        .process_name = strname, \
        .thread_handler = process_handler_##name, \
        .instance = NULL \
    }; \
    static unsigned process_func_##name(struct process *p, process_event_t ev, process_data_t data)

#define PROCESS_NAME(name) extern struct process name

#define PROCESS_THREAD(name, ev, data) \
    void *process_handler_##name(void *arg) \
    { \
        process_event_t _event = PROCESS_EVENT_INIT; \
        process_data_t _data = (process_data_t *)arg; \
        struct process *p = &name; \
        unsigned state = process_func_##name(p, _event, _data); \
        while (state != PT_ENDED) { \
            if (state == PT_WAITING || state == PT_YIELDED) { \
                mutex_lock(&p->mutex); \
                _event = p->event; \
                _data = p->data; \
                state = process_func_##name(p, _event, _data); \
            } \
        } \
        thread_exit(p->instance); \
        return NULL; \
    } \
    static unsigned process_func_##name(struct process *p, process_event_t ev, process_data_t data)

#define PROCESS_BEGIN() \
    do { \
        if (ev == PROCESS_EVENT_INIT) \
        { \
            p->event = ev; \
            p->data = data; \
            mutex_init(p->instance, &p->mutex); \
            if (p->mutex.queue.next == NULL) mutex_lock(&p->mutex); \
        } \
    } while(0); \
    { char PT_YIELD_FLAG = 1; if (PT_YIELD_FLAG) {;} process_current = p;  switch(p->lc) { case 0:

#define PROCESS_YIELD() \
    do { \
        PT_YIELD_FLAG = 0; \
        p->lc = __LINE__; case __LINE__: \
        if (PT_YIELD_FLAG == 0) { \
            return PT_YIELDED; \
        } \
    } while (0)

#define PROCESS_YIELD_UNTIL(condition) \
    do { \
        PT_YIELD_FLAG = 0; \
        p->lc = __LINE__; case __LINE__: \
        if ((PT_YIELD_FLAG == 0) || !(condition)) { \
            return PT_YIELDED; \
        } \
    } while (0)

#define PROCESS_END()  } PT_YIELD_FLAG = 0; process_current = NULL; return PT_ENDED; } \

#define PROCESS_WAIT_EVENT() PROCESS_YIELD()

#define PROCESS_WAIT_EVENT_UNTIL(condition) PROCESS_YIELD_UNTIL(condition)

#define PROCESS_CURRENT() process_current

#define PROCESS_CONTEXT_BEGIN(p) { \
    struct process *tmp_current = PROCESS_CURRENT(); \
    process_current = p

#define PROCESS_CONTEXT_END(p) process_current = tmp_current; }

void process_init(void *instance);

void process_start(struct process *p, process_data_t data);

int process_post(struct process *p, process_event_t event, process_data_t data);

void process_post_synch(struct process *p, process_event_t event, process_data_t data);

process_event_t process_alloc_event(process_event_prio_t prio);

extern process_custom_event_t _process_events[KERNEL_MAXTHREADS];
extern void *process_instance;
extern struct process *process_current;

#ifdef __cplusplus
}
#endif

#endif /* PROCESS_H */
