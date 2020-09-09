#include "contiki.h"

#include "sys/clock.h"
#include "sys/timer.h"

void timer_set(struct timer *t, clock_time_t interval)
{
    t->interval = interval;
    t->start = clock_time();
}

void timer_reset(struct timer *t)
{
    if (timer_expired(t))
    {
        t->start += t->interval;
    }
}

void timer_restart(struct timer *t)
{
    t->start = clock_time();
}

int timer_expired(struct timer *t)
{
    clock_time_t diff = (clock_time() - t->start) + 1;
    return t->interval < diff;
}

clock_time_t timer_remaining(struct timer *t)
{
    return t->start + t->interval - clock_time();
}
