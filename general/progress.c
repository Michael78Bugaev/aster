// progress.c
#include "progress.h"
#include <vga.h>
#include <string.h>
#include <stdio.h>

void init_progress_bar(ProgressBar* bar, const char* title, uint8_t color, uint8_t total_steps) {
    bar->current_step = 0;
    bar->total_steps = total_steps;
    bar->color = color;
    bar->position = get_cursor();
    strncpy(bar->title, title, 31);
    bar->title[31] = '\0';
    
    clear_screen();
    printf("%s: [", bar->title);
    bar->position = get_cursor();
    
    // Инициализация пустого прогресс-бара
    for (uint8_t i = 0; i < PROGRESS_BAR_WIDTH; i++) {
        putchar(PROGRESS_BAR_EMPTY, bar->color);
    }
    printf("]");
}

void update_progress_bar(ProgressBar* bar) {
    uint16_t old_pos = get_cursor();
    bar->current_step++;
    uint8_t percentage = (bar->current_step * 100) / bar->total_steps;
    uint8_t filled = (PROGRESS_BAR_WIDTH * percentage) / 100;
    
    set_cursor(bar->position);
    
    for (uint8_t i = 0; i < filled; i++) {
        putchar(PROGRESS_BAR_CHAR, bar->color);
    }
    
    set_cursor(bar->position + PROGRESS_BAR_WIDTH + 2);
    printf("%d%%", percentage);
    
    set_cursor(old_pos);
}

void finish_progress_bar(ProgressBar* bar) {
    update_progress_bar(bar);
    printf("\nInitialization complete!\n");
}