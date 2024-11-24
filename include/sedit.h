#ifndef SEDIT_H
#define SEDIT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io/kb.h>
#include <io/iotools.h>
#include <fs/file.h>

#define MAX_LINES 100
#define MAX_LINE_LENGTH 256

typedef struct {
    char *lines[MAX_LINES];
    int line_count;
    char input_buffer[MAX_LINE_LENGTH];
    int input_index;
} EditorBuffer;

// Функции редактора
void init_editor(EditorBuffer *buffer);
void load_file(EditorBuffer *buffer, const char *filename);
void save_file(EditorBuffer *buffer, const char *filename);
void display_editor(EditorBuffer *buffer);
void edit_file(EditorBuffer *buffer);
void free_editor(EditorBuffer *buffer);
void keyboard_irq_handler(struct InterruptRegisters *regs);

#endif // SEDIT_H