#include "ctimer.h"

PROCESS(ctimer_process, "ctimer-process", VCRTOS_CONFIG_MAIN_THREAD_STACK_SIZE);

static process_event_t ctimer_callback_event;

PROCESS_THREAD(ctimer_process, ev, data)
{
    PROCESS_BEGIN();

    ctimer_callback_event = process_alloc_event(PROCESS_EVENT_PRIO_MEDIUM);

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == ctimer_callback_event);
        struct ctimer *ct = (struct ctimer *)data;
        PROCESS_CONTEXT_BEGIN(ct->p);
        if (ct->cb != NULL)
        {
            ct->cb(ct->arg);
        }
        PROCESS_CONTEXT_END(ct->p);
    }

    PROCESS_END();
}

static void _ctimer_callback(void *arg)
{
    process_post(&ctimer_process, ctimer_callback_event, arg);
}

void ctimer_init(void)
{
    process_start(&ctimer_process, NULL);
}

static void _ctimer_set(struct ctimer *ct, clock_time_t interval, void (*cb)(void *), void *arg)
{
    ct->etimer.super.callback = _ctimer_callback;
    ct->etimer.super.arg = (void *)ct;
    timer_set(&ct->etimer.timer, interval);
    ct->cb = cb;
    ct->arg = arg;
    ztimer_set(ZTIMER_USEC, &ct->etimer.super, (uint32_t)interval);
}

void ctimer_set(struct ctimer *ct, clock_time_t interval, void (*cb)(void *), void *arg)
{
    ct->etimer.p = PROCESS_CURRENT();
    _ctimer_set(ct, interval, cb, arg);
}

void ctimer_reset(struct ctimer *ct)
{
    ztimer_remove(ZTIMER_USEC, &ct->etimer.super);
    _ctimer_set(ct, ct->etimer.timer.interval, ct->cb, ct->arg);
}

void ctimer_set_with_process(struct ctimer *ct, clock_time_t interval,
                             void (*cb)(void *), void *arg, struct process *p)
{
    ct->etimer.p = p;
    _ctimer_set(ct, interval, cb, arg);
}

void ctimer_stop(struct ctimer *ct)
{
    ztimer_remove(ZTIMER_USEC, &ct->etimer.super);
    ct->etimer.timer.start = 0;
}

int ctimer_expired(struct ctimer *ct)
{
    return etimer_expired(&ct->etimer);
}
