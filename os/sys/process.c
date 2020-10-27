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

#include <vcrtos/assert.h>
#include <vcrtos/cpu.h>

#include "sys/process.h"

custom_event_t _process_events[PROCESS_MAX_EVENTS];

void *process_instance = NULL;

static process_event_t lastevent = PROCESS_EVENT_LAST;

struct process *process_list = NULL;
struct process *process_current = NULL;

void process_init(void *instance)
{
    process_instance = instance;

    unsigned state = cpu_irq_disable();

    for (int i = 0; i < PROCESS_MAX_EVENTS; i++)
    {
        event_init((event_t *)&_process_events[i]);
        _process_events[i].id = PROCESS_EVENT_NONE;
        _process_events[i].data = NULL;
    }

    cpu_irq_restore(state);
}

void process_start(struct process *p, process_data_t data)
{
    vcassert(p != NULL && process_instance != NULL);

    struct process *q;

    /* First make sure that we don't try to start a process that is already
     * running */
    for (q = process_list; q != p && q != NULL; q = q->next);

    /* If we found the process on the process list, we bail out */
    if (q == p)
    {
        return;
    }

    if (p->instance == NULL)
    {
        p->instance = process_instance;
    }

    /* Put on the process list */
    p->next = process_list;
    process_list = p;

    p->state = PROCESS_STATE_RUNNING;
    p->lc = 0;

    p->pid = thread_create(p->instance, p->stack, p->stack_size, KERNEL_THREAD_PRIORITY_MAIN,
                           THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                           p->thread_handler, (void *)data, p->process_name);
}

int process_post(struct process *p, process_event_t event, process_data_t data)
{
    vcassert((p != NULL) && (p->pid != KERNEL_PID_UNDEF) && (process_instance != NULL));

    custom_event_t *ev = NULL;

    unsigned i;

    unsigned state = cpu_irq_disable();

    for (i = 0; i < PROCESS_MAX_EVENTS; i++)
    {
        if (_process_events[i].super.list_node.next == NULL)
        {
            ev = &_process_events[i];
            ev->id = event;
            ev->data = data;
            break;
        }
    }

    vcassert((i < PROCESS_MAX_EVENTS) && (ev != NULL));

    cpu_irq_restore(state);

    event_post(&p->event_queue, (event_t *)ev, p->thread);

    return 0;
}

void process_post_synch(struct process *p, process_event_t event, process_data_t data)
{
    struct process *caller = process_current;

    unsigned ret = process_call(p, event, data);

    /* Note: don't exit process using process post synch */

    vcassert(ret != PT_EXITED && ret != PT_ENDED);

    process_current = caller;
}

process_event_t process_alloc_event(void)
{
    return lastevent++;
}

int process_is_running(struct process *p)
{
    return p->state != PROCESS_STATE_NONE;
}

static void exit_process(struct process *p, struct process *fromprocess)
{
    register struct process *q;

    struct process *old_current = process_current;

    /* make sure the process is in the process list before we try to exit it */
    for (q = process_list; q != p && q != NULL; q = q->next);

    if (q == NULL)
    {
        return;
    }

    if (process_is_running(p))
    {
        /* process was running */
        p->state = PROCESS_STATE_NONE;

        /* post synchronous event to all processes to inform them that this
         * process is about to exit. This will allow services to deallocate
         * state associated with this process. */
        for (q = process_list; q != NULL; q = q->next)
        {
            if (p != q)
            {
                process_call(q, PROCESS_EVENT_EXITED, (process_data_t)p);
            }
        }

        if (p->process_func != NULL && p != fromprocess)
        {
            process_current = p;
            p->process_func(p, PROCESS_EVENT_EXIT, NULL);

            /* post the exited event to make vcrtos thread to exit */
            process_post(p, PROCESS_EVENT_EXITED, NULL);
        }
    }

    if (p == process_list)
    {
        process_list = process_list->next;
    }
    else
    {
        for (q = process_list; q != NULL; q = q->next)
        {
            if (q->next == p)
            {
                q->next = p->next;
                break;
            }
        }
    }

    process_current = old_current;
}

unsigned process_call(struct process *p, process_event_t ev, process_data_t data)
{
    unsigned ret = PT_UNKNOWN;

    if ((p->state & PROCESS_STATE_RUNNING) && p->process_func != NULL)
    {
        process_current = p;

        p->state = PROCESS_STATE_CALLED;

        ret = p->process_func(p, ev, data);

        if (ret == PT_EXITED || ret == PT_ENDED || ev == PROCESS_EVENT_EXIT)
        {
            exit_process(p, p);

            if (ev == PROCESS_EVENT_EXIT && ret != PT_EXITED)
            {
                ret = PT_EXITED;
            }
        }
        else
        {
            p->state = PROCESS_STATE_RUNNING;
        }
    }

    return ret;
}

void process_exit(struct process *p)
{
    exit_process(p, PROCESS_CURRENT());
}
