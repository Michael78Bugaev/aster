#ifndef DIR_H
#define DIR_H

#include "initrd.h"

void split_path(const char *path, char **components, int *count);
void change_directory(const char *path);
void list_current_directory();

#endif // DIR_H