#include <cpu/pit.h>
#include <vga.h>
#include <io/iotools.h>
#include <io/idt.h>
#include <stdbool.h>
#include <stdint.h>

uint64_t ticks;
const uint32_t freq = 1000;
int target = 0;
int old_ticks;
bool end = false;

static bool cursor_visible = true; // Состояние видимости курсора
static const unsigned int blink_interval = 500; // Интервал мигания в миллисекундах
static unsigned int last_blink_time = 0; // Время последнего мигания

void inwait(struct InterruptRegisters *regs);

void init_pit()
{
    ticks = 0;
    target = 0;
    irq_install_handler(0, &pit_handler);

    uint32_t div = 1193180 / freq;

    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, (uint8_t)div & 0xFF);
    port_byte_out(0x40, (uint8_t)((div >> 8) & 0xFF));
}
void pit_handler(struct InterruptRegisters *regs)
{
    ticks += 1;

    // if (ticks * (1000 / TICKS_PER_SECOND) - last_blink_time >= blink_interval) {
    //     cursor_visible = !cursor_visible; // Меняем состояние видимости курсора
    //     last_blink_time = ticks * (1000 / TICKS_PER_SECOND); // Обновляем время последнего мигания
    // }

    // // Здесь вы можете добавить код для отрисовки курсора, если он видим
    // if (cursor_visible) {
    //     unsigned char *vga = (unsigned char*)VIDEO_ADDRESS;
    //     vga[get_cursor() * 2] = ' ';
    //     vga[get_cursor() * 2 + 1] = 0x70;
    // } else {
    //     unsigned char *vga = (unsigned char*)VIDEO_ADDRESS;
    //     vga[get_cursor() * 2] = ' ';
    //     vga[get_cursor() * 2 + 1] = 0x00;
    // }
}

void inwait(struct InterruptRegisters *regs)
{
    if (end = false)
    {
        if (ticks < target)
        {
            ticks += 1;
        }
        else
        {
            kprint("timer ended!");
            end = true;
            irq_install_handler(0, &pit_handler);
        }
    }
}

void wait(int ms)
{
    old_ticks = ticks;
    target = ms + ticks;

    while (ticks < target)
    {
        
    }
    
}

uint64_t get_ticks()
{
    return ticks;
}