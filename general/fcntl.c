#include <fcntl.h>
#include <fs/file.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Функция для открытия файла
int open(const char *pathname, int flags) {
    // Ищем файл в текущей директории
    File *file = find_file(pathname, current_directory);
    if (!file) {
        return -1; // Ошибка: не удалось открыть файл
    }

    // Возвращаем дескриптор файла (в данном случае просто индекс в массиве файлов)
    return 0; // Предполагаем, что дескриптор - это индекс в массиве файлов
}

// Функция для закрытия файла
int close(int fd) {
    // Здесь можно добавить логику для закрытия файла, если это необходимо
    // В данной реализации просто возвращаем 0, так как у нас нет явного управления дескрипторами
    return 0; // Успех
}

// Функция для чтения из файла
ssize_t read(int fd, void *buf, size_t count) {
    // Ищем файл в текущей директории
    File *file = current_directory->files[fd];

    // Ограничиваем количество читаемых байт
    size_t bytes_to_read = (count < file->size) ? count : file->size;

    // Копируем данные в буфер
    memcpy(buf, file->data, bytes_to_read);
    return bytes_to_read; // Возвращаем количество прочитанных байт
}

// Функция для записи в файл
ssize_t write(int fd, const void *buf, size_t count) {
    // Ищем файл в текущей директории
    File *file = current_directory->files[fd];

    // Ограничиваем количество записываемых байт
    size_t bytes_to_write = (count < file->size) ? count : file->size;

    // Копируем данные из буфера в файл
    memcpy(file->data, buf, bytes_to_write);
    return bytes_to_write; // Возвращаем количество записанных байт
}