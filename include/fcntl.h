#ifndef FCNTL_H
#define FCNTL_H

#include <stdint.h>
#include <sys/types.h>

#define O_RDONLY  0x0000
#define O_WRONLY  0x0001
#define O_RDWR    0x0002
#define O_CREAT   0x0200
#define O_TRUNC   0x2000

#define MAX_PROCESSES 256

int open(const char *pathname, int flags);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

#endif // FCNTL_H