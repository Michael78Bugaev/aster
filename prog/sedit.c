#include <sedit.h>
#include <stdio.h>
#include <string.h>
#include <fs/file.h>
#include <fs/initrd.h>
#include <io/kb.h> // Для работы с клавиатурой

bool enter = false;
void keyboard_handler(struct InterruptRegisters *regs);

void init_editor(EditorBuffer *editor_buffer) {
    memset(editor_buffer->buffer, 0, MAX_BUFFER_SIZE);
    editor_buffer->cursor_position = 0;
    editor_buffer->length = 0;
}

void load_file(EditorBuffer *editor_buffer, const char *filename) {
    File *file = read_file(filename, current_directory);
    if (file) {
        strncpy(editor_buffer->buffer, (char *)file->data, MAX_BUFFER_SIZE);
        editor_buffer->length = strlen(editor_buffer->buffer);
        editor_buffer->cursor_position = editor_buffer->length; // Устанавливаем курсор в конец
    } else {
        printf("Error: Could not load file %s\n", filename);
    }
}

void save_file(EditorBuffer *editor_buffer, const char *filename) {
    if (editor_buffer->length > 0) {
        write_file(filename, (uint8_t *)editor_buffer->buffer, editor_buffer->length, current_directory);
    } else {
        printf("Error: Buffer is empty, nothing to save.\n");
    }
}

void display_editor(EditorBuffer *editor_buffer) {
    clear_screen();
    printf("Editor - Press Ctrl+S to save, Ctrl+Q to quit\n");
    printf("%s\n", editor_buffer->buffer);
    // Отображаем курсор
    set_cursor(editor_buffer->cursor_position);
}

void handle_input(EditorBuffer *editor_buffer) {
    char scanCode;
    char press;

    while (1) {
        // Устанавливаем обработчик прерываний для клавиатуры
        irq_install_handler(1, &keyboard_handler); // keyboard_handler - ваш обработчик клавиатуры

        // Ждем, пока не будет нажата клавиша
        while (!enter) {
            // Ожидание нажатия клавиши
        }

        // Получаем код нажатой клавиши
        scanCode = port_byte_in(0x60) & 0x7F; // Получаем код нажатой клавиши
        press = port_byte_in(0x60) & 0x80; // Проверяем, была ли клавиша нажата или отпущена

        if (press == 0) { // Если клавиша нажата
            if (scanCode == 0x1C) { // Enter
                editor_buffer->buffer[editor_buffer->length] = '\n'; // Добавляем новую строку
                editor_buffer->length++;
                editor_buffer->cursor_position++;
            } else if (scanCode == 0x0E) { // Backspace
                if (editor_buffer->length > 0) {
                    editor_buffer->length--;
                    editor_buffer->cursor_position--;
                    editor_buffer->buffer[editor_buffer->length] = '\0'; // Удаляем символ
                }
            } else if (scanCode == 0x3B) { // Ctrl + S
                save_file(editor_buffer, "output.txt"); // Сохраняем в файл
            } else if (scanCode == 0x3A) { // Ctrl + Q
                break; // Выход из редактора
            } else {
                // Добавляем символ в буфер
                char character = get_acsii_low(scanCode); // Получаем символ из кода
                if (editor_buffer->length < MAX_BUFFER_SIZE - 1) {
                    editor_buffer->buffer[editor_buffer->length] = character; // Добавляем символ
                    editor_buffer->length++;
                    editor_buffer->cursor_position++;
                }
            }
        }

        enter = false; // Сбрасываем флаг нажатия клавиши
        display_editor(editor_buffer); // Обновляем отображение редактора
    }
}

void free_editor_resources(EditorBuffer *editor_buffer) {
    // Освобождение ресурсов, если необходимо
}

void keyboard_handler(struct InterruptRegisters *regs) {
    char scanCode = port_byte_in(0x60) & 0x7F; // Получаем код нажатой клавиши
    char press = port_byte_in(0x60) & 0x80; // Проверяем, была ли клавиша нажата или отпущена

    if (press == 0) { // Если клавиша нажата
        // Устанавливаем флаг нажатия клавиши
        enter = true;

        // Обработка специальных клавиш
        if (scanCode == 0x1C) { // Enter
            // Здесь можно добавить дополнительную логику, если нужно
        } else if (scanCode == 0x0E) { // Backspace
            // Здесь можно добавить дополнительную логику, если нужно
        } else if (scanCode == 0x3B) { // Ctrl + S
            // Здесь можно добавить дополнительную логику, если нужно
        } else if (scanCode == 0x3A) { // Ctrl + Q
            // Здесь можно добавить дополнительную логику, если нужно
        } else {
            // Обработка обычных символов
            char character = get_acsii_low(scanCode); // Получаем символ из кода
            // Здесь можно добавить дополнительную логику для обработки символов
        }
    }
}