#include "contiki.h"

#include "sys/etimer.h"
#include "sys/process.h"

static struct etimer *timerlist;
static clock_time_t next_expiration;

PROCESS(etimer_process, "event timer", VCRTOS_CONFIG_MAIN_THREAD_STACK_SIZE);

static void update_time(void)
{
    clock_time_t tdist;
    clock_time_t now;
    struct etimer *t;

    if (timerlist == NULL)
    {
        next_expiration = 0;
    }
    else
    {
        now = clock_time();
        t = timerlist;
        tdist = t->timer.start + t->timer.interval - now;
        for (t = t->next; t != NULL; t = t->next)
        {
            if (t->timer.start + t->timer.interval - now < tdist)
            {
                tdist = t->timer.start + t->timer.interval - now;
            }
        }
        next_expiration = now + tdist;
    }
}

PROCESS_THREAD(etimer_process, ev, data)
{
    struct etimer *t, *u;

    PROCESS_BEGIN();

    timerlist = NULL;

    while (1)
    {
        PROCESS_YIELD();

        if (ev == PROCESS_EVENT_EXITED)
        {
            struct process *prc = data;
            while (timerlist != NULL && timerlist->p == prc)
            {
                timerlist = timerlist->next;
            }
            if (timerlist != NULL)
            {
                t = timerlist;
                while (t->next != NULL)
                {
                    if (t->next->p == prc)
                    {
                        t->next = t->next->next;
                    }
                    else
                    {
                        t = t->next;
                    }
                }
            }
            continue;
        }
        else if (ev != PROCESS_EVENT_POLL)
        {
            continue;
        }

again:

        u = NULL;

        for (t = timerlist; t != NULL; t = t->next)
        {
            if (timer_expired(&t->timer))
            {
                if (process_post(t->p, PROCESS_EVENT_TIMER, t) == PROCESS_ERR_OK)
                {
                    t->p = PROCESS_NONE;
                    if (u != NULL)
                    {
                        u->next = t->next;
                    }
                    else
                    {
                        timerlist = t->next;
                    }
                    t->next = NULL;
                    update_time();
                    goto again;
                }
                else
                {
                    etimer_request_poll();
                }
            }
            u = t;
        }
    }

    PROCESS_END();
}

void etimer_request_poll(void)
{
    process_poll(&etimer_process);
}

static void add_timer(struct etimer *timer)
{
    struct etimer *t;

    etimer_request_poll();

    if (timer->p != PROCESS_NONE)
    {
        for (t = timerlist; t != NULL; t = t->next)
        {
            if (t == timer)
            {
                /* timer already on list, bail out */
                timer->p = PROCESS_CURRENT();
                update_time();
                return;
            }
        }
    }

    timer->p = PROCESS_CURRENT();
    timer->next = timerlist;
    timerlist = timer;

    update_time();
}

void etimer_set(struct etimer *et, clock_time_t interval)
{
    timer_set(&et->timer, interval);
    add_timer(et);
}

void etimer_reset_with_new_interval(struct etimer *et, clock_time_t interval)
{
    timer_reset(&et->timer);
    et->timer.interval = interval;
    add_timer(et);
}

void etimer_reset(struct etimer *et)
{
    timer_reset(&et->timer);
    add_timer(et);
}

void etimer_restart(struct etimer *et)
{
    timer_restart(&et->timer);
    add_timer(et);
}

void etimer_adjust(struct etimer *et, int timediff)
{
    et->timer.start += timediff;
    update_time();
}

int etimer_expired(struct etimer *et)
{
    return et->p == PROCESS_NONE;
}

clock_time_t etimer_expiration_time(struct etimer *et)
{
    return et->timer.start + et->timer.interval;
}

clock_time_t etimer_start_time(struct etimer *et)
{
    return et->timer.start;
}

int etimer_pending(void)
{
    return timerlist != NULL;
}

clock_time_t etimer_next_expiration_time(void)
{
    return etimer_pending() ? next_expiration : 0;
}

void etimer_stop(struct etimer *et)
{
    struct etimer *t;

    if (et == timerlist)
    {
        timerlist = timerlist->next;
        update_time();
    }
    else
    {
        for (t = timerlist; t != NULL && t->next != et; t = t->next);

        if (t != NULL)
        {
            t->next = et->next;
            update_time();
        }
    }

    et->next = NULL;
    et->p = PROCESS_NONE;
}
