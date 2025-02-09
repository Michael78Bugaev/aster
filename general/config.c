#include <config.h>
#include <stdint.h>
#include <drv/vbe.h>
#include <string.h>
#include <stdio.h>
#include <sash.h>
#include <vga.h>
#include <cpu/mem.h>

uint64_t var_count = 0;
int _current_ata_num = 0;

struct global_variable* find_variable(const char *name) {
    for (int i = 0; i < VAR_MAXCOUNT; i++) {
        if (strcmp(var[i].name, name) == 0) {
            return &var[i];
        }
    }
    return NULL; // Если переменная не найдена
}

void init_variable(const char *name, const char *value, int type) {
    struct global_variable new_var;
    new_var.name = strdup(name); // Копируем имя переменной
    new_var.type = type;

    if (isdigit(name[0])) {
        kprint("error: variable name cannot start with a digit\n");
        return;
    }

    if (type == TYPE_INT) {
        // Проверка на наличие букв в строке
        for (const char *p = value; *p; p++) {
            if (isalpha(*p)) {
                kprint("error: variable contains letters\n");
                return; // Выход из функции, если ошибка
            }
        }
        new_var.data.int_value = atoi(value); // Преобразуем строку в int
    } else if (type == TYPE_STR) {
        new_var.data.str_value = strdup(value); // Копируем строку
    }
    var[var_count++] = new_var; // Сохраняем переменную
}

void assign_variable(const char *var_name, const char *value) {
    struct global_variable *var_to_assign = find_variable(var_name);
    if (var_to_assign != NULL) {
        if (var_to_assign->type == TYPE_INT) {
            var_to_assign->data.int_value = atoi(value);
        } else if (var_to_assign->type == TYPE_STR) {
            mfree(var_to_assign->data.str_value); // Освобождаем старую строку
            var_to_assign->data.str_value = strdup(value); // Копируем новое значение
        }
    } else {
        kprint("syntax error: variable ");
        kprint(var_name);
        kprint(" not found\n");
    }
}

void free_variables() {
    for (int i = 0; i < VAR_MAXCOUNT; i++) {
        mfree(var[i].name);
        if (var[i].type == TYPE_STR) {
            mfree(var[i].data.str_value);
        }
    }
}

int get_var_count()
{
    return var_count;
}

void start_global_config()
{
    get_full_cpu_name();      
}

void execute_init(const char *filename) {
    
}

void DEBUG(uint8_t *msg)
{
    printf("[   DBG  ]: ");
    printf(msg); // [  <(0b)>INFO<(07)>  ]:
    printf("\n");
}

// Функция INFO для форматированного вывода
void INFO(const char* format, ...) {
    char c;
    uint8_t current_color = 0x07; // По умолчанию светло-серый текст на черном фоне
    char **arg = (char **) &format; // Указатель на аргументы
    int *int_arg;
    char *str_arg;
    char num_buf[32];
    unsigned int uint_arg;

    arg++; // Переходим к первому аргументу после format

    // Печатаем префикс

    char log_buf[1024]; // Буфер для логирования
    int log_len = 0; // Длина логирования

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
            log_buf[log_len++] = c; // Добавляем символ в логирование
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
                // Преобразуем число в строку вручную
                int num = *int_arg;
                int i = 0;
                if (num < 0) {
                    putchar('-', current_color);
                    log_buf[log_len++] = '-'; // Добавляем символ в логирование
                    num = -num;
                }
                do {
                    num_buf[i++] = (num % 10) + '0';
                    num /= 10;
                } while (num > 0);
                // Выводим число в обратном порядке
                for (int j = i - 1; j >= 0; j--) {
                    putchar(num_buf[j], current_color);
                    log_buf[log_len++] = num_buf[j]; // Добавляем символ в логирование
                }
                break;
            case 's':
                str_arg = *(char **)arg++;
                while (*str_arg) {
                    putchar(*str_arg, current_color);
                    log_buf[log_len++] = *str_arg; // Добавляем символ в логирование
                    str_arg++;
                }
                break;
            case 'x': {
                int_arg = (int *)arg++;
                hex_to_str(*int_arg, num_buf);
                int len = strlen(num_buf);
                // Добавляем пробелы для выравнивания
                for (int i = 0; i < width - len; i++) {
                    putchar('0', current_color); // Заполняем нулями
                    log_buf[log_len++] = '0'; // Добавляем символ в логирование
                }
                for (char *ptr = num_buf; *ptr; ptr++) {
                    putchar(*ptr, current_color);
                    log_buf[log_len++] = *ptr; // Добавляем символ в логирование
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
                    log_buf[log_len++] = '0'; // Добавляем символ в логирование
                }
                for (char *ptr = num_buf; *ptr; ptr++) {
                    putchar(*ptr, current_color);
                    log_buf[log_len++] = *ptr; // Добавляем символ в логирование
                }
                break;
            }
            case 'c':
                int_arg = (int *)arg++;
                putchar((char)(*int_arg), current_color);
                log_buf[log_len++] = (char)(*int_arg); // Добавляем символ в логирование
                break;

            default:
                putchar(c, current_color);
                log_buf[log_len++] = c; // Добавляем символ в логирование
                break;
        }
    }
    putchar('\n', current_color); // Переход на новую строку
}

void ERRORf(const char* format, ...)
{
    char c;
    uint8_t current_color = 0x07; // По умолчанию светло-серый текст на черном фоне
    char **arg = (char **) &format; // Указатель на аргументы
    int *int_arg;
    char *str_arg;
    char num_buf[32];
    unsigned int uint_arg;

    arg++; // Переходим к первому аргументу после format

    // Печатаем префикс

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
                // Преобразуем число в строку вручную
                int num = *int_arg;
                int i = 0;
                if (num < 0) {
                    putchar('-', current_color);
                    num = -num;
                }
                do {
                    num_buf[i++] = (num % 10) + '0';
                    num /= 10;
                } while (num > 0);
                // Выводим число в обратном порядке
                for (int j = i - 1; j >= 0; j--) {
                    putchar(num_buf[j], current_color);
                }
                break;
            case 's':
                str_arg = *(char **)arg++;
                while (*str_arg) {
                    putchar(*str_arg++, current_color);
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

            default:
                putchar(c, current_color);
                break;
        }
    }
    putchar('\n', current_color); // Переход на новую строку
}