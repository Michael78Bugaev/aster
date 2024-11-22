#include <fs/file.h>
#include <stdio.h>

void read_file(const char *name, Directory *dir) {
    File *file = find_file(name, dir);
    if (file) {
        printf("Reading file: %s\n", file->name);
        // Здесь можно добавить код для обработки данных файла
    } else {
        printf("File not found: %s\n", name);
    }
}

void write_file(const char *name, const uint8_t *data, uint32_t size, Directory *dir) {
    create_file(name, data, size, dir);
}