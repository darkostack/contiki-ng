#include <vcrtos/assert.h>
#include <vcrtos/cpu.h>

#include "sys/process.h"

extern event_queue_t event_queue_highest;
extern event_queue_t event_queue_medium;
extern event_queue_t event_queue_lowest;

process_custom_event_t _process_events[KERNEL_MAXTHREADS];

static process_event_t lastevent = PROCESS_EVENT_LAST;
void *process_instance = NULL;
struct process *process_current = NULL;

void _process_event_default_handler(event_t *event)
{
    process_custom_event_t *custom_event = (process_custom_event_t *)event;

    unsigned state = cpu_irq_disable();

    custom_event->process->event = custom_event->event_id;
    custom_event->process->data = custom_event->data;

    cpu_irq_restore(state);

    mutex_unlock(&custom_event->process->mutex);
}

void process_init(void *instance)
{
    process_instance = instance;

    for (int i = 0; i < KERNEL_MAXTHREADS; i++)
    {
        _process_events[i].super.list_node.next = NULL;
        _process_events[i].super.handler = _process_event_default_handler;
        _process_events[i].process = NULL;
        _process_events[i].priority = PROCESS_EVENT_PRIO_MEDIUM;
        _process_events[i].event_id = i;
        _process_events[i].data = NULL;
    }
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
    vcassert(_process_events[event].event_id == event);

    process_event_prio_t prio = _process_events[event].priority;

    unsigned state = cpu_irq_disable();

    _process_events[event].process = p;
    _process_events[event].data = data;

    cpu_irq_restore(state);

    switch (prio)
    {
    case PROCESS_EVENT_PRIO_HIGH:
        event_post(&event_queue_highest, (event_t *)&_process_events[event]);
        break;

    case PROCESS_EVENT_PRIO_MEDIUM:
        event_post(&event_queue_medium, (event_t *)&_process_events[event]);
        break;

    case PROCESS_EVENT_PRIO_LOW:
        event_post(&event_queue_lowest, (event_t *)&_process_events[event]);
        break;

    default:
        return -1;
    }

    return 0;
}

void process_post_synch(struct process *p, process_event_t event, process_data_t data)
{
    process_post(p, event, data);
}

process_event_t process_alloc_event(process_event_prio_t prio)
{
    vcassert(lastevent < KERNEL_MAXTHREADS);
    unsigned state = cpu_irq_disable();
    unsigned char event_id = lastevent++;
    _process_events[event_id].priority = prio;
    cpu_irq_restore(state);
    return event_id;
}
