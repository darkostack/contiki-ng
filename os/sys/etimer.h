#ifndef ETIMER_H
#define ETIMER_H

#include "sys/process.h"

#include <vcrtos/config.h>
#include <vcrtos/ztimer.h>

#include "sys/timer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct etimer
{
    ztimer_t super;
    struct timer timer;
    struct process *p;
};

void etimer_set(struct etimer *et, clock_time_t interval);

void etimer_reset(struct etimer *et);

void etimer_reset_with_new_interval(struct etimer *et, clock_time_t interval);

void etimer_restart(struct etimer *et);

clock_time_t etimer_expiration_time(struct etimer *et);

clock_time_t etimer_start_time(struct etimer *et);

int etimer_expired(struct etimer *et);

void etimer_stop(struct etimer *et);

#ifdef __cplusplus
}
#endif

#endif /* ETIMER_H */
