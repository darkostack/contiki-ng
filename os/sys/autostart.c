#include "sys/autostart.h"

void autostart_start(struct process *const processes[])
{
    for (int i = 0; processes[i] != NULL; ++i)
    {
        process_start(processes[i], NULL);
    }
}

void autostart_exit(struct process *const processes[])
{
    (void) processes;
}
