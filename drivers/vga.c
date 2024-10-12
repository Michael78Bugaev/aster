/*------------------------------------------------------------------------------
*	Guide:	01-KERNEL
*	File:	ex01 / drivers / screen.c
*	Title:	Функции работы с экраном
* ------------------------------------------------------------------------------
*	Description:
* ----------------------------------------------------------------------------*/


#include <vga.h>
#include <io/iotools.h>
#include <stdint.h>


void	kprint(uint8_t *str)
{
	/* Функция печати строки */
	
	// uint8_t *str: указатель на строку (на первый символ строки). Строка должна
	// быть null-terminated.

	while (*str)
	{
		putchar(*str, 0x07);
		str++;
	}
}

void kprintc(uint8_t *str, uint8_t attr)
{
    while (*str)
	{
		putchar(*str, attr);
		str++;
	}
}
void	putchar(uint8_t character, uint8_t attribute_byte)
{
	/* Более высокоуровневая функция печати символа */

	// uint8_t character: байт, соответствующий символу
	// uint8_t attribute_byte: байт, соответствующий цвету текста/фона символа

	uint16_t offset;

	offset = get_cursor();
	if (character == '\n')
	{
		// Переводим строку.
		if ((offset / 2 / MAX_COLS) == (MAX_ROWS - 1)) 
			scroll_line();
		else
			set_cursor((offset - offset % (MAX_COLS*2)) + MAX_COLS*2);
	}
	else 
	{
		if (offset == (MAX_COLS * MAX_ROWS * 2)) scroll_line();
		write(character, attribute_byte, offset);
		set_cursor(offset+2);
	}
}

void	scroll_line()
{
	/* Функция скроллинга */

	uint8_t i = 1;		// Начинаем со второй строки.
	uint16_t last_line;	// Начало последней строки.

	while (i < MAX_ROWS)
	{
		memcpy(
			(uint8_t *)(VIDEO_ADDRESS + (MAX_COLS * i * 2)),
			(uint8_t *)(VIDEO_ADDRESS + (MAX_COLS * (i-1) * 2)),
			(MAX_COLS*2)
		);
		i++;
	}

	last_line = (MAX_COLS*MAX_ROWS*2) - MAX_COLS*2;
	i = 0;
	while (i < MAX_COLS)
	{
		write('\0', WHITE_ON_BLACK, (last_line + i * 2));
		i++;
	}
	set_cursor(last_line);
}

void	clear_screen()
{
	/* Функция очистки экрана */

	uint16_t	offset = 0;
	while (offset < (MAX_ROWS * MAX_COLS * 2))
	{
		write('\0', WHITE_ON_BLACK, offset);
		offset += 2;
	}
	set_cursor(0);//                                                                         "
    kprintc("                                 Origin Aster                                   \n", 0x70);
}

void	write(uint8_t character, uint8_t attribute_byte, uint16_t offset)
{
	/* Функция печати символа на экран с помощью VGA по адресу 0xb8000 */

	// uint8_t character: байт, соответствующий символу
	// uint8_t attribute_byte: байт, соответствующий цвету текста/фона символа
	// uint16_t offset: смещение (позиция), по которому нужно распечатать символ
	
	uint8_t *vga = (uint8_t *) VIDEO_ADDRESS;
	vga[offset] = character;
	vga[offset + 1] = attribute_byte;
}

uint16_t		get_cursor()
{
	/* Функция, возвращающая позицию курсора (char offset). */

	port_byte_out(REG_SCREEN_CTRL, 14);				// Запрашиваем верхний байт
	uint8_t high_byte = port_byte_in(REG_SCREEN_DATA);	// Принимаем его
	port_byte_out(REG_SCREEN_CTRL, 15);				// Запрашиваем нижний байт
	uint8_t low_byte = port_byte_in(REG_SCREEN_DATA);	// Принимаем и его
	// Возвращаем смещение умножая его на 2, т.к. порты возвращают смещение в
	// клетках экрана (cell offset), а нам нужно в символах (char offset), т.к.
	// на каждый символ у нас 2 байта
	return (((high_byte << 8) + low_byte) * 2);
}

void	set_cursor(uint16_t pos)
{
	/* Функция, устаналивающая курсор по смещнию (позиции) pos */
	/* Поиграться с битами можно тут http://bitwisecmd.com/ */

	// конвертируем в cell offset (в позицию по клеткам, а не символам)
	pos /= 2;

	// Указываем, что будем передавать верхний байт
	port_byte_out(REG_SCREEN_CTRL, 14);
	// Передаем верхний байт
	port_byte_out(REG_SCREEN_DATA, (uint8_t)(pos >> 8));
	// Указываем, что будем передавать нижний байт
	port_byte_out(REG_SCREEN_CTRL, 15);
	// Передаем нижний байт
	port_byte_out(REG_SCREEN_DATA, (uint8_t)(pos & 0xff));
}