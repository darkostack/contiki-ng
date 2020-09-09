#ifndef AUTOSTART_H_
#define AUTOSTART_H_

#include "contiki.h"
#include "sys/process.h"

#define AUTOSTART_PROCESSES(...) \
    struct process * const autostart_processes[] = { __VA_ARGS__, NULL }

extern struct process * const autostart_processes[];

void autostart_start(struct process * const processes[]);
void autostart_exit(struct process * const processes[]);

#endif /* AUTOSTART_H_ */
