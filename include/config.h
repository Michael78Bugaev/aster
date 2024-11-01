#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>

#define FAT16_MAX_FILESYSTEMS 10
#define FAT16_MAX_HEAP_ALLOCATIONS 56
#define FAT16_MEMORY_BLOCK_SIZE 512
#define FAT16_STACK_END_ADDRESS 29184
#define FAT16_MAX_DISKS 4
#define FAT16_FORCE_MEMORY_ALIGNMENT 1
#define FAT16_SECTOR_SIZE 512
#define FAT16_MAX_PATH 108

#define FAT16_ALL_OK 0
#define FAT16_FS_IO_ERROR 1
#define FAT16_FS_FILE_NOT_FOUND_ERROR 2
#define FAT16_FS_FILE_READ_ONLY_ERROR 3
#define FAT16_FS_NOT_US 4
#define FAT16_BAD_PATH 5
#define FAT16_INVALID_DRIVE 6

#define VAR_MAXCOUNT 512

struct global_variable {
    char *name; // имя переменной
    enum { TYPE_INT, TYPE_STR } type; // тип переменной
    union {
        int int_value; // значение для int
        char *str_value; // значение для str
    } data; // данные переменной
};

struct global_variable var[];

struct global_variable* find_variable(const char *name);
void init_variable(const char *name, const char *value, int type);
void free_variables();
int get_var_count();

#endif