#ifndef STIMER_H_
#define STIMER_H_

#include "sys/clock.h"

struct stimer {
    unsigned long start;
    unsigned long interval;
};

void stimer_set(struct stimer *t, unsigned long interval);
void stimer_reset(struct stimer *t);
void stimer_restart(struct stimer *t);
int stimer_expired(struct stimer *t);
unsigned long stimer_remaining(struct stimer *t);
unsigned long stimer_elapsed(struct stimer *t);

#endif /* STIMER_H_ */
