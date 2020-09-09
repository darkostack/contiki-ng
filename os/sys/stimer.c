#include "contiki.h"
#include "sys/clock.h"
#include "sys/stimer.h"

#define SCLOCK_GEQ(a, b)	((unsigned long)((a) - (b)) < ((unsigned long)(~((unsigned long)0)) >> 1))

void stimer_set(struct stimer *t, unsigned long interval)
{
    t->interval = interval;
    t->start = clock_seconds();
}

void stimer_reset(struct stimer *t)
{
    if (stimer_expired(t))
    {
        t->start += t->interval;
    }
}

void stimer_restart(struct stimer *t)
{
    t->start = clock_seconds();
}

int stimer_expired(struct stimer *t)
{
    return SCLOCK_GEQ(clock_seconds(), t->start + t->interval);
}

unsigned long stimer_remaining(struct stimer *t)
{
    return t->start + t->interval - clock_seconds();
}

unsigned long stimer_elapsed(struct stimer *t)
{
    return clock_seconds() - t->start;
}
