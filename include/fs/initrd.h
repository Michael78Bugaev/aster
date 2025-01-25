#ifndef INITRD_H
#define INITRD_H

#include <stdint.h>

#define MAX_FILES 128
#define MAX_FILENAME_LENGTH 32
#define MAX_DIRS 128
#define MAX_PATH_LENGTH 1024

typedef struct File {
    char name[MAX_FILENAME_LENGTH];
    uint32_t size;
    uint8_t *data; // Указатель на данные файла
} File;

typedef struct Directory {
    char name[MAX_FILENAME_LENGTH];
    char full_name[MAX_PATH_LENGTH]; // Полное имя директории
    struct Directory *parent; // Родительская директория
    struct Directory *subdirs[MAX_DIRS]; // Подкаталоги
    File *files[MAX_FILES]; // Файлы в директории
    uint32_t file_count;
    uint32_t dir_count;
} Directory;

typedef struct vfs_node {
    // Baisc information about a file(note: in linux, everything is file, so the vfs_node could be used to describe a file, directory or even a device!)
    char name[256];
    void * device;
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t inode_num;
    uint32_t size;
    uint32_t fs_type;
    uint32_t open_flags;
    // Time
    uint32_t create_time;
    uint32_t access_time;
    uint32_t modified_time;

    uint32_t offset;
    unsigned nlink;
    int refcount;
}vfs_node_t;

Directory *root;

Directory *current_directory;

Directory *execfs;
Directory *tmpfs;
Directory *devfs;
Directory *userfs;
Directory *sysfs; 

void init_vfs();
void add_IDE_dev(char *dev_type, int drive, uint8_t *data);
Directory* find_or_create_directory(const char *path);
Directory* create_directory(const char *path);
File* create_file(const char *name, const uint8_t *data, uint32_t size, Directory *dir);
File* find_file(const char *name, Directory *dir);
Directory* find_directory(const char *name, Directory *dir);
int delete_file(const char *name, Directory *dir);
void delete_directory(const char *name, Directory *parent);
void rename_file(const char *old_name, const char *new_name, Directory *dir);
void rename_directory(const char *old_name, const char *new_name, Directory *parent);
void list_directory(Directory *dir);
Directory* get_root_directory();
File* new_file(const char *path, const uint8_t *data, uint32_t size);

#endif // INITRD_H