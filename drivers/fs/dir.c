#include <fs/dir.h>
#include <fs/initrd.h>
#include <stdio.h>
#include <string.h>
#include <cpu/mem.h>
#include <stdlib.h>

void split_path(const char *path, char **components, int *count) {
    char *token;
    char *path_copy = strdup(path); // Создаем копию пути, чтобы не изменять оригинал
    *count = 0;

    token = strtok(path_copy, "/");
    while (token != NULL) {
        components[(*count)++] = token; // Сохраняем компоненты пути
        token = strtok(NULL, "/");
    }
    mfree(path_copy);
}

void change_directory(const char *path) {
    if (strcmp(path, ".") == 0) {
        return; // Остаемся в текущей директории
    } else if (strcmp(path, "..") == 0) {
        if (current_directory->parent) {
            current_directory = current_directory->parent; // Переход к родительской директории
            // remove_null_chars(current_directory->parent->name);
            // strcpy(current_directory->name, current_directory->parent->name);
        }
        return;
    }

    // Разделяем путь на компоненты
    char *components[128];
    int count = 0;
    split_path(path, components, &count);

    Directory *target_directory = current_directory;

    for (int i = 0; i < count; i++) {
        if (strcmp(components[i], "") == 0) {
            // Игнорируем пустые компоненты (например, при начале с /)
            continue;
        }
        Directory *new_dir = find_directory(components[i], target_directory);
        if (new_dir) {
            target_directory = new_dir; // Переход в подкаталог
        } else {
            printf("%s: No such directory or this is a file\n", components[i]);
            return;
        }
    }

    current_directory = target_directory; // Устанавливаем текущую директорию
}

void list_current_directory() {
    list_directory(current_directory);
}