#include <fs/file.h>
#include <stdio.h>
#include <string.h>

uint8_t* read_file(const char *name, Directory *dir) {
    File *file = find_file(name, dir);
    if (file) {
        //remove_null_chars(file->data);
        return file->data;
    } else {
        printf("%s: file not found\n", name);
        return NULL;
    }
}

void write_file(const char *name, const uint8_t *data, uint32_t size, Directory *dir) {
    create_file(name, data, size, dir);
}