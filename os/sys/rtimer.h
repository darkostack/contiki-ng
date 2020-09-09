#ifndef RTIMER_H_
#define RTIMER_H_

#include "contiki.h"

#ifdef RTIMER_CONF_CLOCK_SIZE
#define RTIMER_CLOCK_SIZE RTIMER_CONF_CLOCK_SIZE
#else
#define RTIMER_CLOCK_SIZE 4
#endif

#if RTIMER_CLOCK_SIZE == 2
/* 16-bit rtimer */
typedef uint16_t rtimer_clock_t;
#define RTIMER_CLOCK_DIFF(a,b)     ((int16_t)((a)-(b)))

#elif RTIMER_CLOCK_SIZE == 4
/* 32-bit rtimer */
typedef uint32_t rtimer_clock_t;
#define RTIMER_CLOCK_DIFF(a, b)    ((int32_t)((a) - (b)))

#elif RTIMER_CLOCK_SIZE == 8
/* 64-bit rtimer */
typedef uint64_t rtimer_clock_t;
#define RTIMER_CLOCK_DIFF(a,b)     ((int64_t)((a)-(b)))

#else
#error Unsupported rtimer size (check RTIMER_CLOCK_SIZE)
#endif

#define RTIMER_CLOCK_MAX           ((rtimer_clock_t)-1)
#define RTIMER_CLOCK_LT(a, b)      (RTIMER_CLOCK_DIFF((a),(b)) < 0)

#define RTIMER_NOW() clock_time()
#define RTIMER_SECOND RTIMER_ARCH_SECOND

#ifndef RTIMER_BUSYWAIT_UNTIL_ABS
#define RTIMER_BUSYWAIT_UNTIL_ABS(cond, t0, max_time)                       \
  ({                                                                        \
    bool c;                                                                 \
    while(!(c = cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), (t0) + (max_time))); \
    c;                                                                      \
  })
#endif /* RTIMER_BUSYWAIT_UNTIL_ABS */

#define RTIMER_BUSYWAIT_UNTIL(cond, max_time)       \
  ({                                                \
    rtimer_clock_t t0 = RTIMER_NOW();               \
    RTIMER_BUSYWAIT_UNTIL_ABS(cond, t0, max_time);  \
  })

#define RTIMER_BUSYWAIT(duration) RTIMER_BUSYWAIT_UNTIL(0, duration)

#endif /* RTIMER_H_ */
