#ifndef PROC_H
#define PROC_H

#include <stdint.h>

typedef int pid_t;

void init_process_management();
pid_t create_process();
void terminate_process(pid_t pid);
void schedule();

#endif // PROC_H