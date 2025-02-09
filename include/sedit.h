#ifndef SEDIT_H
#define SEDIT_H

#include <stdint.h>

#define MAX_BUFFER_SIZE 1024

typedef struct {
    char buffer[MAX_BUFFER_SIZE];  // Буфер для текста
    uint32_t cursor_position;       // Позиция курсора
    uint32_t length;                // Длина текста
} EditorBuffer;

// Инициализация редактора
void init_editor(EditorBuffer *editor_buffer);

// Загрузка файла в редактор
void load_file(EditorBuffer *editor_buffer, const char *filename);

// Сохранение файла из редактора
void save_file(EditorBuffer *editor_buffer, const char *filename);

// Отображение содержимого редактора
void display_editor(EditorBuffer *editor_buffer);

// Обработка ввода пользователя
void handle_input(EditorBuffer *editor_buffer);

// Освобождение ресурсов редактора
void free_editor_resources(EditorBuffer *editor_buffer);

#endif // SEDIT_H