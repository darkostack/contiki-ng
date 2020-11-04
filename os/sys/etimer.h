#ifndef ETIMER_H
#define ETIMER_H

#include "contiki.h"

#ifdef __cplusplus
extern "C" {
#endif

struct etimer
{
    struct timer timer;
    struct etimer *next;
    struct process *p;
};

void etimer_set(struct etimer *et, clock_time_t interval);

void etimer_reset(struct etimer *et);

void etimer_reset_with_new_interval(struct etimer *et, clock_time_t interval);

void etimer_restart(struct etimer *et);

void etimer_adjust(struct etimer *et, int td);

clock_time_t etimer_expiration_time(struct etimer *et);

clock_time_t etimer_start_time(struct etimer *et);

int etimer_expired(struct etimer *et);

void etimer_stop(struct etimer *et);

void etimer_request_poll(void);

int etimer_pending(void);

clock_time_t etimer_next_expiration_time(void);

PROCESS_NAME(etimer_process);

#ifdef __cplusplus
}
#endif

#endif /* ETIMER_H */
