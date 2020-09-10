#include "sys/ctimer.h"
#include "contiki.h"
#include "lib/list.h"

LIST(ctimer_list);

static char initialized;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

PROCESS(ctimer_process, "ctimer-process", VCRTOS_CONFIG_MAIN_THREAD_STACK_SIZE);

PROCESS_THREAD(ctimer_process, ev, data)
{
    struct ctimer *c;

    PROCESS_BEGIN();

    for (c = list_head(ctimer_list); c != NULL; c = c->next)
    {
        etimer_set(&c->etimer, c->etimer.timer.interval);
    }

    initialized = 1;

    while (1)
    {
        PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_TIMER);
        for (c = list_head(ctimer_list); c != NULL; c = c->next)
        {
            if (&c->etimer == data)
            {
                list_remove(ctimer_list, c);
                PROCESS_CONTEXT_BEGIN(c->p);
                if (c->f != NULL)
                {
                    c->f(c->ptr);
                }
                PROCESS_CONTEXT_END(c->p);
                break;
            }
        }
    }

    PROCESS_END();
}

void ctimer_init(void)
{
    initialized = 0;
    list_init(ctimer_list);
    process_start(&ctimer_process, NULL);
}

void ctimer_set(struct ctimer *c, clock_time_t t, void (*f)(void *), void *ptr)
{
    ctimer_set_with_process(c, t, f, ptr, PROCESS_CURRENT());
}

void ctimer_set_with_process(struct ctimer *c, clock_time_t t,
	                         void (*f)(void *), void *ptr, struct process *p)
{
    PRINTF("ctimer_set %p %lu in %s\n", c, (unsigned long)t, p->process_name);

    c->p = p;
    c->f = f;
    c->ptr = ptr;

    if (initialized)
    {
        PROCESS_CONTEXT_BEGIN(&ctimer_process);
        etimer_set(&c->etimer, t);
        PROCESS_CONTEXT_END(&ctimer_process);
    }
    else
    {
        c->etimer.timer.interval = t;
    }

    list_add(ctimer_list, c);
}

void ctimer_reset(struct ctimer *c)
{
    if (initialized)
    {
        PROCESS_CONTEXT_BEGIN(&ctimer_process);
        etimer_reset(&c->etimer);
        PROCESS_CONTEXT_END(&ctimer_process);
    }

    list_add(ctimer_list, c);
}

void ctimer_restart(struct ctimer *c)
{
    if (initialized)
    {
        PROCESS_CONTEXT_BEGIN(&ctimer_process);
        etimer_restart(&c->etimer);
        PROCESS_CONTEXT_END(&ctimer_process);
    }

    list_add(ctimer_list, c);
}

void ctimer_stop(struct ctimer *c)
{
    if (initialized)
    {
        etimer_stop(&c->etimer);
    }
    else
    {
        //c->etimer.next = NULL;
        c->etimer.p = PROCESS_NONE;
    }

    list_remove(ctimer_list, c);
}

int ctimer_expired(struct ctimer *c)
{
    struct ctimer *t;

    if (initialized)
    {
        return etimer_expired(&c->etimer);
    }

    for (t = list_head(ctimer_list); t != NULL; t = t->next)
    {
        if (t == c)
        {
            return 0;
        }
    }

    return 1;
}
