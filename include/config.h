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

#define ALL_OK 0
#define IO_ERROR 1
#define INVALID_ARGUMENT_ERROR 2
#define NO_MEMORY_ERROR 3
#define BAD_PATH_ERROR 4 
#define FILE_SYSTEM_NOT_US_ERROR 5
#define READ_ONLY_ERROR 6
#define UNIMPLEMENTED_ERROR 7
#define INITIALIZE_SLOT_TAKEN 8
#define INVALID_FORMAT_ERROR 9

#define ROOT_DIR "/"

#define OS_VERSION "Aster 32-bit kernel 0.07"

#define VAR_MAXCOUNT 512

typedef struct page_dir_entry {
    unsigned int present    : 1;
    unsigned int rw         : 1;
    unsigned int user       : 1;
    unsigned int w_through  : 1;
    unsigned int cache      : 1;
    unsigned int access     : 1;
    unsigned int reserved   : 1;
    unsigned int page_size  : 1;
    unsigned int global     : 1;
    unsigned int available  : 3;
    unsigned int frame      : 20;
}page_dir_entry_t;

typedef struct page_table_entry {
    unsigned int present    : 1;
    unsigned int rw         : 1;
    unsigned int user       : 1;
    unsigned int reserved   : 2;
    unsigned int accessed   : 1;
    unsigned int dirty      : 1;
    unsigned int reserved2  : 2;
    unsigned int available  : 3;
    unsigned int frame      : 20;
}page_table_entry_t;


typedef struct page_table
{
    page_table_entry_t pages[1024];
} page_table_t;

typedef struct page_directory
{
    // The actual page directory entries(note that the frame number it stores is physical address)
    page_dir_entry_t tables[1024];
    // We need a table that contains virtual address, so that we can actually get to the tables
    page_table_t * ref_tables[1024];
} page_directory_t;

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
void start_global_config();
void execute_init(const char *filename);
void DEBUG(uint8_t *msg);
void INFO(uint8_t *msg);

#endif