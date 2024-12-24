#include <unistd.h>
#include <fs/file.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Функция для чтения из файла
ssize_t read(int fd, void *buf, size_t count) {
    // Проверяем, что дескриптор файла валиден
    if (fd < 0) {
        return -1; // Ошибка: неверный дескриптор
    }

    // Получаем файл по дескриптору
    File *file = current_directory->files[fd];
    if (!file) {
        return -1; // Ошибка: файл не найден
    }

    // Ограничиваем количество читаемых байт
    size_t bytes_to_read = (count < file->size) ? count : file->size;

    // Копируем данные в буфер
    memcpy(buf, file->data, bytes_to_read);
    return bytes_to_read; // Возвращаем количество прочитанных байт
}

// Функция для записи в файл
ssize_t write(int fd, const void *buf, size_t count) {
    // Проверяем, что дескриптор файла валиден
    if (fd < 0) {
        return -1; // Ошибка: неверный дескриптор
    }

    // Получаем файл по дескриптору
    File *file = current_directory->files[fd];
    if (!file) {
        return -1; // Ошибка: файл не найден
    }

    // Ограничиваем количество записываемых байт
    size_t bytes_to_write = (count < file->size) ? count : file->size;

    // Копируем данные из буфера в файл
    memcpy(file->data, buf, bytes_to_write);
    return bytes_to_write; // Возвращаем количество записанных байт
}

// Функция для создания нового процесса
pid_t fork(void) {
    // Реализация создания нового процесса
    pid_t new_pid = create_process(); // Ваша функция создания процесса
    return new_pid; // Возвращаем PID нового процесса
}

// Функция для выполнения программы
int execve(const char *filename, char *const argv[], char *const envp[]) {
    // Реализация загрузки и выполнения программы
    // Здесь вы можете использовать вашу файловую систему для открытия файла
    File *file = find_file(filename, current_directory);
    if (!file) {
        return -1; // Ошибка: файл не найден
    }
    // Загрузка и выполнение файла
    // ...
    return 0; // Успех
}

// Функция для закрытия файла
int close(int fd) {
    // Проверяем, что дескриптор файла валиден
    if (fd < 0) {
        return -1; // Ошибка: неверный дескриптор
    }

    // Освобождаем ресурсы файла
    current_directory->files[fd] = NULL; // Удаляем файл из текущей директории
    return 0; // Успех
}

// Функция для удаления файла
int unlink(const char *pathname) {
    // Удаляем файл из текущей директории
    if (delete_file(pathname, current_directory) == 0) {
        return 0; // Успех
    }
    return -1; // Ошибка: файл не найден
}