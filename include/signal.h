#ifndef SIGNAL_H
#define SIGNAL_H

#include <stdint.h>

#define SIGINT 2
#define SIGTERM 15


typedef void (*sighandler_t)(int);

sighandler_t signal(int signum, sighandler_t handler);
void raise(int signum);

#endif // SIGNAL_H