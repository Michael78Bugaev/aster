#include <stdint.h>
#include <config.h>
#include <string.h>
#include <drv/vbe.h>
#include <cpu/pit.h>

static bool cursor_visible = true; // Состояние видимости курсора
static const unsigned int blink_interval = 500; // Интервал мигания в миллисекундах
static unsigned int last_blink_time = 0; // Время последнего мигания
static int cursor_x = 0; // Позиция курсора по X
static int cursor_y = 0; // Позиция курсора по Y
bool newline = false;

void pixel(struct multiboot_info* mboot, int x, int y, uint8_t color) {
    unsigned char *pixel = mboot->framebuffer_addr + y * mboot->framebuffer_pitch + x;
    *pixel = color;
}

void print_char(struct multiboot_info* boot_info, char c, int x, int y, uint16_t color) {
    if (c < 0 || c > 127) return; // Проверка на допустимый ASCII диапазон

    // Получаем данные шрифта для символа
    const uint8_t *char_data = font[(uint8_t)c];

    // Рисуем символ на экране
    for (int row = 0; row < FONT_HEIGHT; row++) {
        for (int col = 0; col < FONT_WIDTH; col++) {
            // Проверяем, установлен ли бит для текущего пикселя
            if (char_data[row] & (1 << col)) {
                // Устанавливаем цвет пикселя на основе предоставленного атрибута цвета
                pixel(boot_info, x + col, y + row, color); // Рисуем пиксель
            }
            //else pixel(boot_info, x + col, y + row, 0x00);
        }
    }
}

void display_text(struct multiboot_info* boot_info, const char *text, int start_x, int start_y, uint16_t color) {
    int x = start_x;
    int y = start_y;

    while (*text) {
        print_char(boot_info, *text, x, y, color); // Печатаем символ с цветом
        x += FONT_WIDTH; // Переход к следующей позиции символа
        if (x >= boot_info->framebuffer_width) { // Если достигнут конец строки
            x = start_x; // Сбрасываем x
            y += FONT_HEIGHT; // Переход на следующую строку
            newline = true;
        }
        text++;
    }
}

void vbe_printf(struct multiboot_info* boot_info, char *text, int x, int y, uint16_t color) {
    int posX = x * 8;
    int posY = y * 8;
    display_text(boot_info, text, posX, posY, color);
}

void update_cursor(struct multiboot_info* boot_info, int x, int y) {
    cursor_x = x;
    cursor_y = y;
    print_char(boot_info, '_', (x * 8), (y * 8) + 8, 0x07);
}

void blink_cursor(struct multiboot_info* boot_info, int x, int y, int blink_interval) {
    unsigned int current_time = get_ticks(); // Получаем текущее время
    if (current_time - last_blink_time >= blink_interval) {
        cursor_visible = !cursor_visible; // Переключаем видимость курсора
        last_blink_time = current_time; // Обновляем время последнего мигания

        if (cursor_visible) {
            // Отображаем курсор
            update_cursor(boot_info, cursor_x, cursor_y);
        } else {
            // Скрываем курсор
            update_cursor(boot_info, cursor_x, cursor_y); // Можно использовать цвет фона
        }
    }
}

void init_vbe_terminal(struct multiboot_info* boot_info) {
    // Инициализация VBE терминала
    // Здесь можно добавить код для настройки экрана и начального состояния терминала
    _GLOBAL_MBOOT = boot_info;
    display_text(boot_info, "[INFO]: VBE Terminal enabled", 0, 0, 0xFFFFFF); // Пример текста
}

void vbe_screen_clear(struct multiboot_info* boot_info, uint8_t color)
{
    for (int x = 0; x < boot_info->framebuffer_width; x++)
    {
        for (int y = 0; y < boot_info->framebuffer_height; y++)
        {
            pixel(boot_info, x, y, color);
        } 
    }
}

void _print(struct _vbe_cursor cursor_t, char *text) {
    vbe_printf(_GLOBAL_MBOOT, text, cursor_t.x, cursor_t.y, 0x07);
    cursor_t.x += strlen(text); // Увеличиваем x на длину текста

    // Если x выходит за пределы ширины экрана, сбрасываем его и увеличиваем y
    if (cursor_t.x >= _GLOBAL_MBOOT->framebuffer_width / FONT_WIDTH) {
        cursor_t.x = 0; // Сбрасываем x
        cursor_t.y++; // Переход на следующую строку
    }
}