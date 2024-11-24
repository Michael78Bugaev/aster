#include <fs/initrd.h>
#include <stdio.h>
#include <cpu/mem.h>
#include <string.h>
#include <stdlib.h>

Directory* fscreate_directory(const char *name, Directory *parent);

void init_vfs() {
    root = (Directory *)malloc(sizeof(Directory));
    strncpy(root->name, "/", MAX_FILENAME_LENGTH);
    root->parent = NULL;
    root->file_count = 0;
    root->dir_count = 0;
    current_directory = root; // Устанавливаем текущую директорию как корневую
    execfs = create_directory("/exec");
    tmpfs = create_directory("/temp");
    devfs = create_directory("/dev");
    userfs = create_directory("/user");
    sysfs = create_directory("/sys");

    uint8_t *data = "lalalaaaa\nalalaaaa";

    File *help_bin = new_file("/exec/help.bin", NULL, NULL);
    File *clear_bin = new_file("/exec/clear.bin", NULL, NULL);
    File *ls_bin = new_file("/exec/ls.bin", NULL, NULL);
    File *cd_bin = new_file("/exec/cd.bin", NULL, NULL);
    File *mkdir_bin = new_file("/exec/mkdir.bin", NULL, NULL);
    File *rm_bin = new_file("/exec/rm.bin", NULL, NULL);
    File *cp_bin = new_file("/exec/cp.bin", NULL, NULL);
    File *mv_bin = new_file("/exec/mv.bin", NULL, NULL);
    File *cat_bin = new_file("/exec/cat.bin", NULL, NULL);
    File *echo_bin = new_file("/exec/echo.bin", NULL, NULL);
    File *mkdirp_bin = new_file("/exec/mkdirp.bin", NULL, NULL);
    File *touch_bin = new_file("/exec/touch.bin", NULL, NULL);
    File *rmrf_bin = new_file("/exec/rmrf.bin", NULL, NULL);
    File *chmod_bin = new_file("/exec/chmod.bin", NULL, NULL);
    File *chown_bin = new_file("/exec/chown.bin", NULL, NULL);
    File *chgrp_bin = new_file("/exec/chgrp.bin", data, sizeof(data));
    File *ln_bin = new_file("/exec/ln.bin", NULL, NULL);
}

Directory* get_root_directory() {
    return root;
}

Directory* create_directory(const char *path) {
    char *components[128];
    int count = 0;
    split_path(path, components, &count);

    Directory *target_directory = root; // Начинаем с корневой директории

    for (int i = 0; i < count; i++) {
        if (strcmp(components[i], "") == 0) {
            // Игнорируем пустые компоненты (например, при начале с /)
            continue;
        }

        Directory *existing_dir = find_directory(components[i], target_directory);
        if (existing_dir) {
            target_directory = existing_dir; // Переход в подкаталог
        } else {
            // Создаем новую директорию, если она не существует
            target_directory = fscreate_directory(components[i], target_directory);
        }
    }

    return target_directory; // Возвращаем созданную или найденную директорию
}

Directory* fscreate_directory(const char *name, Directory *parent) {
    if (parent->dir_count >= MAX_DIRS) {
        return NULL; // Превышено максимальное количество подкаталогов
    }

    Directory *new_dir = (Directory *)malloc(sizeof(Directory));
    strncpy(new_dir->name, name, MAX_FILENAME_LENGTH);
    new_dir->parent = parent;
    new_dir->file_count = 0;
    new_dir->dir_count = 0;

    parent->subdirs[parent->dir_count++] = new_dir;
    return new_dir;
}

File* create_file(const char *name, const uint8_t *data, uint32_t size, Directory *dir) {
    if (dir->file_count >= MAX_FILES) {
        return NULL; // Превышено максимальное количество файлов
    }

    File *new_file = (File *)malloc(sizeof(File));
    strncpy(new_file->name, name, MAX_FILENAME_LENGTH);
    new_file->size = size;
    new_file->data = (uint8_t *)malloc(size);
    strncpy(new_file->data, data, size);

    dir->files[dir->file_count++] = new_file;
    return new_file;
}

File* find_file(const char *name, Directory *dir) {
    for (uint32_t i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->name, name) == 0) {
            return dir->files[i];
        }
    }
    return NULL;
}

Directory* find_directory(const char *name, Directory *dir) {
    for (uint32_t i = 0; i < dir->dir_count; i++) {
        if (strcmp(dir->subdirs[i]->name, name) == 0) {
            return dir->subdirs[i];
        }
    }
    return NULL;
}

void delete_file(const char *name, Directory *dir) {
    for (uint32_t i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i]->name, name) == 0) {
            mfree(dir->files[i]-> data);
            mfree(dir->files[i]);
            dir->files[i] = dir->files[--dir->file_count]; // Удаление файла
            return;
        }
    }
}

void delete_directory(const char *name, Directory *parent) {
    for (uint32_t i = 0; i < parent->dir_count; i++) {
        if (strcmp(parent->subdirs[i]->name, name) == 0) {
            // Рекурсивно удаляем все файлы и подкаталоги
            for (uint32_t j = 0; j < parent->subdirs[i]->file_count; j++) {
                mfree(parent->subdirs[i]->files[j]->data);
                mfree(parent->subdirs[i]->files[j]);
            }
            for (uint32_t j = 0; j < parent->subdirs[i]->dir_count; j++) {
                delete_directory(parent->subdirs[i]->subdirs[j]->name, parent->subdirs[i]);
            }
            mfree(parent->subdirs[i]);
            parent->subdirs[i] = parent->subdirs[--parent->dir_count]; // Удаление директории
            return;
        }
    }
}

void rename_file(const char *old_name, const char *new_name, Directory *dir) {
    File *file = find_file(old_name, dir);
    if (file) {
        strncpy(file->name, new_name, MAX_FILENAME_LENGTH);
    }
}

void rename_directory(const char *old_name, const char *new_name, Directory *parent) {
    Directory *dir = find_directory(old_name, parent);
    if (dir) {
        strncpy(dir->name, new_name, MAX_FILENAME_LENGTH);
    }
}

void list_directory(Directory *dir) {
    if (dir == NULL) {
        printf("ls: error: directory not found.\n");
        return;
    }
    int dir_count = 0, qfile_count = 0;
    for (uint32_t i = 0; i < dir->file_count; i++) {
        printf(" f %s\n", dir->files[i]->name);
        qfile_count++;
    }
    for (uint32_t i = 0; i < dir->dir_count; i++) {
        printf(" d %s\n", dir->subdirs[i]->name);
        dir_count++;
    }
    printf("total %d\n", dir_count + qfile_count);
}

Directory* find_or_create_directory(const char *path) {
    char *components[128];
    int count = 0;
    split_path(path, components, &count);

    Directory *target_directory = root; // Начинаем с корневой директории

    for (int i = 0; i < count; i++) {
        if (strcmp(components[i], "") == 0) {
            // Игнорируем пустые компоненты (например, при начале с /)
            continue;
        }

        Directory *existing_dir = find_directory(components[i], target_directory);
        if (existing_dir) {
            target_directory = existing_dir; // Переход в подкаталог
        } else {
            // Создаем новую директорию, если она не существует
            target_directory = fscreate_directory(components[i], target_directory);
        }
    }

    return target_directory; // Возвращаем найденную или созданную директорию
}

File* new_file(const char *path, const uint8_t *data, uint32_t size) {
    // Получаем имя файла из пути
    char *path_copy = strdup(path);
    char *last_slash = strrchr(path_copy, '/');
    if (last_slash) {
        *last_slash = '\0'; // Разделяем путь и имя файла
        char *filename = last_slash + 1; // Имя файла

        // Находим или создаем директорию
        Directory *target_directory = find_or_create_directory(path_copy);

        // Создаем файл в целевой директории
        File *new_file = create_file(filename, data, size, target_directory);
        mfree(path_copy);
        return new_file;
    }
    mfree(path_copy);
    return NULL; // Если путь некорректен
}
