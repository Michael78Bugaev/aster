#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io/kb.h>
#include <vga.h>

void hex_to_str(uint32_t num, char *str);
void hex_to_str(uint32_t num, char *str) {
    int i = 0;
    
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    while (num != 0) {
        uint32_t rem = num % 16;
        if (rem < 10) {
            str[i++] = rem + '0';
        } else {
            str[i++] = (rem - 10) + 'A';
        }
        num = num / 16;
    }

    str[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

uint8_t parse_color_code(char bg, char fg);
uint8_t parse_color_code(char bg, char fg) {
    uint8_t background = 0;
    uint8_t foreground = 0;
    
    // Преобразование шестнадцатеричного символа в число
    if (bg >= '0' && bg <= '9') {
        background = bg - '0';
    } else if (bg >= 'a' && bg <= 'f') {
        background = bg - 'a' + 0xa;
    } else if (bg >= 'A' && bg <= 'F') {
        background = bg - 'A' + 0xa;
    }

    if (fg >= '0' && fg <= '9') {
        foreground = fg - '0';
    } else if (fg >= 'a' && fg <= 'f') {
        foreground = fg - 'a' + 0xa;
    } else if (fg >= 'A' && fg <= 'F') {
        foreground = fg - 'A' + 0xa;
    }
    
    return (background << 4) | foreground;
}

void printf(const char* format, ...) {
    char c;
    uint8_t current_color = 0x07; // По умолчанию светло-серый текст на черном фоне
    char **arg = (char **) &format;
    int *int_arg;
    char *str_arg;
    char num_buf[32];
    unsigned int uint_arg;

    arg++; // Переходим к первому аргументу после format

    while ((c = *format++) != 0) {
        if (c == '<' && *format == '(') {
            format++; // Пропускаем '('

            // Читаем два символа цветового кода
            char bg_color = *format++;
            char fg_color = *format++;

            if (*format == ')' && *(format + 1) == '>') {
                current_color = parse_color_code(bg_color, fg_color);
                format += 2; // Пропускаем ')>'
                continue;
            }
        }

        if (c != '%') {
            putchar(c, current_color);
            continue;
        }

        c = *format++;
        int width = 0; // Инициализируем ширину

        // Обработка ширины
        while (c >= '0' && c <= '9') {
            width = width * 10 + (c - '0'); // Собираем число
            c = *format++; // Переходим к следующему символу
        }

        switch (c) {
            case 'd':
                int_arg = (int *)arg++;
                intToString(*int_arg, num_buf);
                for (char *ptr = num_buf; *ptr; ptr++) {
                    putchar(*ptr, current_color);
                }
                break;

            case 'u':
                uint_arg = *(unsigned int *)arg++;
                intToString(uint_arg, num_buf);
                for (char *ptr = num_buf; *ptr; ptr++) {
                    putchar(*ptr, current_color);
                }
                break;

            case 'x': {
                int_arg = (int *)arg++;
                hex_to_str(*int_arg, num_buf);
                int len = strlen(num_buf);
                // Добавляем пробелы для выравнивания
                for (int i = 0; i < width - len; i++) {
                    putchar('0', current_color); // Заполняем нулями
                }
                for (char *ptr = num_buf; *ptr; ptr++) {
                    putchar(*ptr, current_color);
                }
                break;
            }

            case 'X': {
                int_arg = (int *)arg++;
                hex_to_str(*int_arg, num_buf);
                int len = strlen(num_buf);
                // Добавляем пробелы для выравнивания
                for (int i = 0; i < width - len; i++) {
                    putchar('0', current_color); // Заполняем нулями
                }
                for (char *ptr = num_buf; *ptr; ptr++) {
                    putchar(*ptr, current_color);
                }
                break;
            }

            case 'c':
                int_arg = (int *)arg++;
                putchar((char)(*int_arg), current_color);
                break;

            case 's':
                str_arg = *(char **)arg++;
                while (*str_arg) {
                    putchar(*str_arg++, current_color);
                }
                break;

            default:
                putchar(c, current_color);
                break;
        }
    }
}

char *scanf()
{
    char *to_return;
    get_string(to_return);
    return to_return;
}

