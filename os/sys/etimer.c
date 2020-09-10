#include "etimer.h"

static void _etimer_callback(void *arg)
{
    struct etimer *et = (struct etimer *)arg;
    process_post(et->p, PROCESS_EVENT_TIMER, et);
}

static void _etimer_set(struct etimer *et, clock_time_t interval)
{
    et->super.callback = _etimer_callback;
    et->super.arg = (void *)et;
    timer_set(&et->timer, interval);
    ztimer_set(ZTIMER_USEC, &et->super, (uint32_t)interval);
}

void etimer_set(struct etimer *et, clock_time_t interval)
{
    et->p = PROCESS_CURRENT();
    _etimer_set(et, interval);
}

void etimer_reset(struct etimer *et)
{
    _etimer_set(et, et->timer.interval);
}

void etimer_reset_with_new_interval(struct etimer *et, clock_time_t interval)
{
    etimer_set(et, interval);
}

void etimer_restart(struct etimer *et)
{
    etimer_reset(et);
}

clock_time_t etimer_expiration_time(struct etimer *et)
{
    return et->timer.start + et->timer.interval;
}

clock_time_t etimer_start_time(struct etimer *et)
{
    return et->timer.start;
}

static int _etimer_strictly_before(uint32_t time_a, uint32_t time_b)
{
    uint32_t diff = time_a - time_b;
    return ((diff & (1UL << 31)) != 0);
}

int etimer_expired(struct etimer *et)
{
    return _etimer_strictly_before(etimer_expiration_time(et), ztimer_now(ZTIMER_USEC));
}

void etimer_stop(struct etimer *et)
{
    ztimer_remove(ZTIMER_USEC, &et->super);
    et->timer.start = 0;
}
