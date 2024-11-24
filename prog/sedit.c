#include <sedit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <cpu/mem.h>
#include <fs/initrd.h>
#include <fs/file.h>
#include <fs/dir.h>
#include <io/kb.h>
#include <io/iotools.h>

static struct Key key;

static EditorBuffer editor_buffer;

void init_editor(EditorBuffer *buffer) {
    buffer->line_count = 0;
    buffer->input_index = 0; // Инициализация индекса ввода
    for (int i = 0; i < MAX_LINES; i++) {
        buffer->lines[i] = (char *)malloc(MAX_LINE_LENGTH);
    }

    // Устанавливаем обработчик для клавиатуры
    irq_install_handler(1, keyboard_irq_handler); // IRQ 1 - клавиатура
}

void load_file(EditorBuffer *buffer, const char *filename) {
    File *file = find_file(filename, current_directory);
    if (file) {
        buffer->line_count = 0;
        char *data = (char *)file->data;
        char *line = strtok(data, "\n");
        while (line != NULL && buffer->line_count < MAX_LINES) {
            strncpy(buffer->lines[buffer->line_count++], line, MAX_LINE_LENGTH);
            line = strtok(NULL, "\n");
        }
    } else {
        printf("File not found: %s\n", filename);
    }
}

void save_file(EditorBuffer *buffer, const char *filename) {
    char *data = (char *)malloc(MAX_LINES * MAX_LINE_LENGTH);
    data[0] = '\0'; // Обнуляем строку

    for (int i = 0; i < buffer->line_count; i++) {
        strcat(data, buffer->lines[i]);
        strcat(data, "\n");
    }

    create_file(filename, (uint8_t *)data, strlen(data), current_directory);
    mfree(data);
    printf("File saved: %s\n", filename);
}

void display_editor(EditorBuffer *buffer) {
    printf("\n--- Text Editor ---\n");
    for (int i = 0; i < buffer->line_count; i++) {
        printf("%d: %s\n", i + 1, buffer->lines[i]);
    }
    printf("-------------------\n");
    printf("Input: %s", buffer->input_buffer); // Отображаем текущий ввод
}

void keyboard_irq_handler(struct InterruptRegisters *regs) {
    key.scancode = port_byte_in(0x60) & 0x7F; // Получаем скан-код
    key.press = port_byte_in(0x60) & 0x80;    // Получаем состояние нажатия

    // Обработка нажатия клавиш
    if (!key.press) { // Если клавиша нажата
        if (key.scancode == 0x1C) { // Enter
            if (editor_buffer.input_index > 0) {
                editor_buffer.input_buffer[editor_buffer.input_index] = '\0'; // Завершаем строку
                if (strcmp(editor_buffer.input_buffer, "SAVE") == 0) {
                    char filename[MAX_LINE_LENGTH];
                    printf("\nEnter filename to save: ");
                    // Здесь можно использовать метод для получения имени файла
                } else if (strcmp(editor_buffer.input_buffer, "EXIT") == 0) {
                    // Здесь можно добавить логику для выхода из редактора
                    printf("\nExiting editor...\n");
                    return;
                } else {
                    if (editor_buffer.line_count < MAX_LINES) {
                        strncpy(editor_buffer.lines[editor_buffer.line_count++], editor_buffer.input_buffer, MAX_LINE_LENGTH);
                    }
                }
                display_editor(&editor_buffer);
                editor_buffer.input_index = 0; // Сбрасываем индекс ввода
            }
        } else if (key.scancode >= 0x20 && key.scancode <= 0x7E) { // Проверяем, если это символ
            if (editor_buffer.input_index < MAX_LINE_LENGTH - 1) {
                editor_buffer.input_buffer[editor_buffer.input_index++] = key.scancode; // Добавляем символ в буфер ввода
                editor_buffer.input_buffer[editor_buffer.input_index] = '\0'; // Обновляем строку
                display_editor(&editor_buffer); // Обновляем отображение редактора
            }
        }
    }
}