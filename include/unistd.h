#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>
#include <sys/types.h>
#include <fs/file.h>
#include <stdint.h>

#define STDIN  0
#define STDOUT 1
#define STDERR 2

File* current_file;

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
pid_t fork(void);
int execve(const char *filename, char *const argv[], char *const envp[]);
int close(int fd);
int unlink(const char *pathname);

#endif // UNISTD_H