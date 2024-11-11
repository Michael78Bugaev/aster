#include <io/iotools.h>
#include <stdint.h>

unsigned char   port_byte_in(unsigned short port)
{
    /* Функция-обертка над assembly, читающая 1 байт из параметра port */
    /* unsigned short port: адрес регистра какого-либо девайса, из которого */
    /* мы что-то прочтем. */

    /* Используется другой синтаксис ассембли (GAS). Обратите внимание, что */
    /* выражение "mov dest, src" в GAS мы запишем как "mov src, dest", т.е. */
    /* "in dx, al" означает прочитать содержимое порта (адрес которого */
    /* находится в DX) и положить в AL. */
    /* Символ % означает регистр, а т.к. % - escape symbol, то мы */
    /* пишем еще один %. */
    /* Перемещаем результат в регистр AL т.к. размер AL == 1 байт */
    unsigned char result;
	__asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
	/* разберем только что вызванную функцию: */
	/* "in %%dx, %%al"		- Прочитать содержимое порта и положить это в AL */
	/* : "=a" (result)		- Положить значение AL в переменную result */
	/* : "d" (port)			- Загрузить port в регистр EDX (extended DX: 32b) */
    return (result);		/* Возвращаем прочитанное содержимое из port */
}


void    port_byte_out(unsigned short port, unsigned char data)
{
    /* Функция-обертка над assembly, пишущая data (1 байт) в port */
    /* unsigned short port: адрес регистра девайса, в который что-то запишем */
    /* unsigned char data: 1 байт какой-то информации (например, символ) */
	__asm__("out %%al, %%dx" : : "a" (data), "d" (port));
	/* разберем только что вызванную функцию: */
	/* "out %%al, %%dx"		- Записать data в port */
	/* : : "a" (data)		- Загрузить data в регистр EAX */
	/* : "d" (port)			- Загрузить port в регистр EDX */
}


unsigned char   port_word_in(unsigned short port)
{
    /* Функция-обертка над assembly, читающая 2 байта из параметра port */
    /* Перемещаем результат в регистр AX т.к. размер AX == 2 байта */
    unsigned short result;
    __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return (result);
}


void port_word_out(unsigned short port, unsigned short data)
{
    /* Функция-обертка над assembly, пишущая data (2 байта, т.е. word) в port */
    __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}

void	memcpy(int *src, int *dest, int bytes)
{
	int i;

	i = 0;
	while (i < bytes)
	{
		dest[i] = src[i];
		i++;
	}
}

void memset(void *dest, char val, uint32_t count)
{
    char *temp = (char*)dest;
    for (; count != 0; count--)
    {
        *temp++=val;
    }

}

void insw(uint16_t port, void *addr, unsigned long count)
{
    asm volatile ("cld; rep insw"
                  : "+D" (addr), "+c" (count)
                  : "d" (port)
                  : "memory");
}

void outsw(uint16_t port, const void *addr, unsigned long count)
{
    asm volatile ("cld; rep outsw"
                  : "+S" (addr), "+c" (count)
                  : "d" (port)
                  : "memory");
}

void insb(uint16_t port, void *addr, unsigned long count) {
    // Используем ассемблер для выполнения операции ввода
    asm volatile (
        "cld; rep insb"  // cld - устанавливает направление чтения в памяти
        : "+D" (addr), "+c" (count)  // Указываем, что addr и count будут изменены
        : "d" (port)  // Указываем порт, из которого будем читать
        : "memory"  // Указываем, что память может быть изменена
    );
}

int k_toupper(int c) {
    if(c >= 97 && c <= 122) {
        return c - 32;
    }
    return c;
}

uint32_t port_dword_in(unsigned short port)
{
    /* Функция-обертка над assembly, читающая 4 байта (dword) из параметра port */
    uint32_t result;
    __asm__("inl %%dx, %%eax" : "=a" (result) : "d" (port));
    return result;
}

void port_dword_out(unsigned short port, uint32_t data)
{
    /* Функция-обертка над assembly, пишущая data (4 байта, т.е. dword) в port */
    __asm__("outl %%eax, %%dx" : : "a" (data), "d" (port));
}

void reboot_system() {
    uint8_t good = 0x02;
    while (good & 0x02)
        good = port_byte_in(0x64);
    port_byte_out(0x64, 0xFE);
    __asm__ volatile ("hlt");
}

void shutdown_system() {
    // Метод ACPI
    port_word_out(0xB004, 0x2000);

    // Метод APM
    port_word_out(0x604, 0x2000);

    // Если предыдущие методы не сработали, попробуем метод Bochs/QEMU
    port_word_out(0x4004, 0x3400);

    // Если ничего не сработало, просто зациклимся
    while(1) {
        __asm__ volatile ("hlt");
    }
}