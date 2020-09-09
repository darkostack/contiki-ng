#ifndef INT_MASTER_H_
#define INT_MASTER_H_

#include "contiki.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef INT_MASTER_CONF_STATUS_DATATYPE
#define INT_MASTER_STATUS_DATATYPE INT_MASTER_CONF_STATUS_DATATYPE
#else
#define INT_MASTER_STATUS_DATATYPE uint32_t
#endif

typedef INT_MASTER_STATUS_DATATYPE int_master_status_t;

void int_master_enable(void);

int_master_status_t int_master_read_and_disable(void);

void int_master_status_set(int_master_status_t status);

bool int_master_is_enabled(void);

#endif /* INT_MASTER_H_ */
