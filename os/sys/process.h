/*
 * Copyright (c) 2020, Vertexcom Technologies, Inc.
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Vertexcom Technologies, Inc. and its suppliers,
 * if any. The intellectual and technical concepts contained
 * herein are proprietary to Vertexcom Technologies, Inc.
 * and may be covered by U.S. and Foreign Patents, patents in process,
 * and protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Vertexcom Technologies, Inc.
 *
 * Authors: Darko Pancev <darko.pancev@vertexcom.com>
 */

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

#define PROCESS_NONE NULL

#define PT_WAITING 0
#define PT_YIELDED 1
#define PT_EXITED  2
#define PT_ENDED   3

typedef unsigned char process_event_t;
typedef void *process_data_t;

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

#define PROCESS_MAX_EVENTS 32

struct process
{
    unsigned short lc;
    kernel_pid_t pid;
    event_queue_t event_queue;
    char *stack;
    size_t stack_size;
    const char *process_name;
    thread_t *thread;
    thread_handler_func_t thread_handler;
    void *instance;
};

typedef struct
{
    event_t super;
    process_event_t id;
    process_data_t data;
} custom_event_t;

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
        struct process *p = &name; \
        p->thread = thread_get_from_scheduler(p->instance, p->pid); \
        event_queue_init(p->instance, &p->event_queue); \
        process_current = p; \
        unsigned state = process_func_##name(p, PROCESS_EVENT_INIT, (process_data_t *)arg); \
        while (state != PT_ENDED) { \
            if (state == PT_WAITING || state == PT_YIELDED) { \
                custom_event_t *event = (custom_event_t *)event_wait(&p->event_queue); \
                process_current = p; \
                state = process_func_##name(p, event->id, event->data); \
                event_release((event_t *)event); \
            } \
        } \
        process_current = NULL; \
        thread_exit(p->instance); \
        return NULL; \
    } \
    static unsigned process_func_##name(struct process *p, process_event_t ev, process_data_t data)

#define PROCESS_BEGIN() \
    { char PT_YIELD_FLAG = 1; if (PT_YIELD_FLAG) { (void)data; } switch(p->lc) { case 0:

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

#define PROCESS_END()  } PT_YIELD_FLAG = 0; return PT_ENDED; } \

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

process_event_t process_alloc_event(void);

extern void *process_instance;
extern struct process *process_current;

#ifdef __cplusplus
}
#endif

#endif /* PROCESS_H */
