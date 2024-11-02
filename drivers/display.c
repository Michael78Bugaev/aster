/*------------------------------------------------------------------------------
*	Guide:	01-KERNEL
*	File:	ex01 / drivers / screen.c
*	Title:	Функции работы с экраном
* ------------------------------------------------------------------------------
*	Description:
* ----------------------------------------------------------------------------*/


#include <vga.h>
#include <io/iotools.h>
#include <cpu/pit.h>
#include <stdint.h>

void	kprint(uint8_t *str)
{
	/* Функция печати строки */
	
	// uint8_t *str: указатель на строку (на первый символ строки). Строка должна
	// быть null-terminated.

	while (*str)
	{
		putchar(*str, WHITE_ON_BLACK);
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
    else if (character == '\b')
    {
        set_cursor(get_cursor() - 1);
        kprint(" ");
        set_cursor(get_cursor() - 2);
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
	set_cursor(last_line - 160);
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
	set_cursor(0);
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

// Получение координаты X курсора
uint8_t get_cursor_x() {
    uint16_t pos = get_cursor();
    return pos % 80;  // 80 - стандартная ширина экрана в текстовом режиме
}

// Получение координаты Y курсора
uint8_t get_cursor_y() {
    uint16_t pos = get_cursor();
    return pos / 80;  // 80 - стандартная ширина экрана в текстовом режиме
}

void set_cursor_xy(uint8_t x, uint8_t y) {
    uint16_t pos;
    
    // Проверка границ
    if (x >= MAX_ROWS) x = MAX_ROWS - 1;
    if (y >= MAX_COLS) y = MAX_COLS - 1;
    
    pos = y * MAX_ROWS + x;
    set_cursor(pos);
}

void disable_cursor()
{
	// port_byte_out(REG_SCREEN_CTRL, 0x0A);
	// port_byte_out(REG_SCREEN_DATA, 0x20);
}
void kprint_hex(uint32_t value) {

    // Create a buffer for the hexadecimal digits
    char hex_str[9]; // 8 hex digits + null terminator
    hex_str[8] = '\0'; // Null-terminate the string

    // Fill the string with hexadecimal digits
    for (int i = 7; i >= 0; i--) {
        uint8_t digit = value & 0xF; // Get the last 4 bits
        if (digit < 10) {
            hex_str[i] = '0' + digit; // Convert to character '0'-'9'
        } else {
            hex_str[i] = 'A' + (digit - 10); // Convert to character 'A'-'F'
        }
        value >>= 4; // Shift right by 4 bits to process the next digit
    }

    // Print the hexadecimal string
    for (int i = 0; i < 8; i++) {
        putchar(hex_str[i],  0x07);
    }
}
void kprintc(uint8_t *str, uint8_t attr)
{
    while (*str)
	{
		if (*str == '\b')
		{
			putchar(' ', attr);
			set_cursor(get_cursor() - 2);
		}
		else {
			putchar(*str, attr);
            set_cursor(get_cursor()+1);
		}
		str++;
	}
}
void int_to_str(int num, char *str);
// Helper function to convert integer to string
void int_to_str(int num, char *str) {
    int i = 0;
    int is_negative = 0;

    // Handle 0 explicitly
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Handle negative numbers
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    // Process each digit
    while (num != 0) {
        int rem = num % 10;
        str[i++] = rem + '0';
        num = num / 10;
    }

    // If number is negative, append '-'
    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

// Prints an integer
void kprinti(int number) {
    char buffer[12]; // Enough to hold all int digits and a sign
    int_to_str(number, buffer);
	kprint(buffer);
    // Here you would call a function to print the string to the screen
    // For example: kprints(buffer);
}

// Prints a colored integer
void kprintci(int number, uint8_t attr) {
    char buffer[12];
	int_to_str(number, buffer);
    kprintc(buffer, attr);
    // Here you would call a function to print the string with color
    // For example: kprints_color(buffer, attr);
}

// Prints an integer directly into the video memory
void kprinti_vidmem(int number, int offset) {
    char buffer[12];
    int_to_str(number, buffer);
    volatile char *video = (volatile char *)0xB8000;
    for (int i = 0; buffer[i] != '\0'; i++) {
        video[offset + i * 2] = buffer[i];
        video[offset + i * 2 + 1] = 0x07; // Default attribute (white on black)
    }
}

// Prints a colored integer directly into the video memory
void kprintci_vidmem(int number, uint8_t attr, int offset) {
    
}
void kprint_hex_w(uint32_t value) {
    // Создаем буфер для шестнадцатеричных цифр
    char hex_str[5]; // 4 шестнадцатеричных символа + нулевой терминатор
    hex_str[4] = '\0'; // Нулевой терминатор строки

    // Заполняем строку шестнадцатеричными цифрами
    for (int i = 3; i >= 0; i--) {
        uint8_t digit = value & 0xF; // Получаем последние 4 бита
        if (digit < 10) {
            hex_str[i] = '0' + digit; // Преобразуем в символ '0'-'9'
        } else {
            hex_str[i] = 'A' + (digit - 10); // Преобразуем в символ 'A'-'F'
        }
        value >>= 4; // Сдвигаем вправо на 4 бита для обработки следующей цифры
    }

    // Печатаем шестнадцатеричную строку
    for (int i = 0; i < 4; i++) {
        putchar(hex_str[i], 0x07);
        set_cursor(get_cursor()+2);
    }
}