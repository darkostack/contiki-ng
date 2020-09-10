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

static process_event_t lastevent = PROCESS_EVENT_LAST;
void *process_instance = NULL;
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
    vcassert(p != NULL && p->pid == KERNEL_PID_UNDEF && process_instance != NULL);

    p->instance = process_instance;

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
    process_post(p, event, data);
}

process_event_t process_alloc_event(void)
{
    return lastevent++;
}
