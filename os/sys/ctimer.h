#ifndef CTIMER_H
#define CTIMER_H

#include "sys/process.h"
#include "sys/etimer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ctimer
{
    struct ctimer *next;
    struct etimer etimer;
    struct process *p;
    void (*f)(void *);
    void *ptr;
};

void ctimer_reset(struct ctimer *c);

void ctimer_restart(struct ctimer *c);

void ctimer_set(struct ctimer *c, clock_time_t t, void (*f)(void *), void *ptr);

void ctimer_set_with_process(struct ctimer *c, clock_time_t t,
		                     void (*f)(void *), void *ptr, struct process *p);

void ctimer_stop(struct ctimer *c);

int ctimer_expired(struct ctimer *c);

void ctimer_init(void);

#ifdef __cplusplus
}
#endif

#endif /* CTIMER_H */
