/*------------------------------------------------------------------------------
*	Guide:	01-KERNEL
*	File:	ex01 / drivers / screen.h
*	Title:	Заголовочный файл для screen.c
* ------------------------------------------------------------------------------
*	Description:
* ----------------------------------------------------------------------------*/

#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VIDEO_ADDRESS 0xb8000	// Адрес начала VGA для печати символов
#define MAX_ROWS 25				// макс. строк
#define MAX_COLS 80				// макс. столбцов

#define WHITE_ON_BLACK 0x07		// 0x0 == white fg, 0xf == black bg

// Адреса I/O портов для взаимодействия с экраном.
#define REG_SCREEN_CTRL 0x3d4	// этот порт для описания данных
#define REG_SCREEN_DATA 0x3d5	// а этот порт для самих данных

void	kprint(uint8_t *str);
void kprintc(uint8_t *str, uint8_t attr);
void kprinti(int number);
void kprintci(int number, uint8_t attr);
void kprinti_vidmem(int number, int offset);
void kprintci_vidmem(int number, uint8_t attr, int offset);
void	putchar(uint8_t character, uint8_t attribute_byte);
void	clear_screen();
void	write(uint8_t character, uint8_t attribute_byte, uint16_t offset);
void	scroll_line();
uint16_t		get_cursor();
void	set_cursor(uint16_t pos);
// Получение координаты X курсора
uint8_t get_cursor_x();
uint8_t get_cursor_y();

void set_cursor_xy(uint8_t x, uint8_t y);
void disable_cursor();

void kprint_hex(uint32_t value);

#endif