// progress.h
#ifndef PROGRESS_H
#define PROGRESS_H

#include <stdint.h>

#define PROGRESS_BAR_WIDTH 50
#define PROGRESS_BAR_CHAR '#'
#define PROGRESS_BAR_EMPTY '-'

typedef struct {
    uint8_t current_step;
    uint8_t total_steps;
    uint16_t position;
    uint8_t color;
    char title[32];
} ProgressBar;

void init_progress_bar(ProgressBar* bar, const char* title, uint8_t color, uint8_t total_steps);
void update_progress_bar(ProgressBar* bar);
void finish_progress_bar(ProgressBar* bar);

#endif