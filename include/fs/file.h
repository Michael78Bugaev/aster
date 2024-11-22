#ifndef FILE_H
#define FILE_H

#include <fs/initrd.h>

void read_file(const char *name, Directory *dir);
void write_file(const char *name, const uint8_t *data, uint32_t size, Directory *dir);

#endif // FILE_H