#include <chset/chipset.h>
#include <io/iotools.h>
#include <config.h>
#include <stdio.h>
#include <cpu/pit.h>

// Функция инициализации чипсета
void init_chipset() {
    // Настройка базовых регистров чипсета
    port_byte_out(CHIPS_BASE_ADDRESS + CHIPS_CONFIG_REGISTER, 0x01); // Пример настройки
    port_byte_out(CHIPS_BASE_ADDRESS + CHIPS_STATUS_REGISTER, 0x00); // Сброс статуса
    INFO("chipset detected (universal chipset driver)");
}

// Обработчик прерываний
void interrupt_handler(struct InterruptRegisters *regs) {
    // Обработка прерывания от чипсета
    uint8_t status = port_byte_in(CHIPS_BASE_ADDRESS + CHIPS_STATUS_REGISTER);
    if (status & 0x01) {
        printf("Interrupt from chipset detected.\n");
        // Обработка прерывания
    }
}

void get_full_cpu_name() {
    uint32_t eax, ebx, ecx, edx;

    // Запрашиваем базовую информацию о процессоре
    __asm__ volatile (
        "cpuid"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        : "a" (0) // Запрашиваем базовую информацию
    );

    // Получаем идентификатор производителя
    char vendor[13];
    *((uint32_t*)&vendor[0]) = ebx; // Записываем ebx в первые 4 байта
    *((uint32_t*)&vendor[4]) = edx; // Записываем edx во вторые 4 байта
    *((uint32_t*)&vendor[8]) = ecx; // Записываем ecx в последние 4 байта
    vendor[12] = '\0'; // Завершаем строку

    // Запрашиваем информацию о процессоре (полное имя)
    char cpu_name[49]; // Достаточно для хранения полного имени процессора
    *((uint32_t*)&cpu_name[0]) = 0; // Обнуляем
    *((uint32_t*)&cpu_name[4]) = 0; // Обнуляем
    *((uint32_t*)&cpu_name[8]) = 0; // Обнуляем
    *((uint32_t*)&cpu_name[12]) = 0; // Обнуляем

    // Получаем полное имя процессора
    for (int i = 0; i < 3; i++) {
        __asm__ volatile (
            "cpuid"
            : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
            : "a" (0x80000002 + i) // Запрашиваем имя процессора
        );
        *((uint32_t*)&cpu_name[i * 16]) = eax;
        *((uint32_t*)&cpu_name[i * 16 + 4]) = ebx;
        *((uint32_t*)&cpu_name[i * 16 + 8]) = ecx;
        *((uint32_t*)&cpu_name[i * 16 + 12]) = edx;
    }
    cpu_name[48] = '\0'; // Завершаем строку

    // Выводим информацию о процессоре
    printf("<(0f)>[INFO]:<(07)> cpu: vendor: %s\n", vendor);
    printf("<(0f)>[INFO]:<(07)> cpu: %s\n", cpu_name);
    strcpy(CPUNAME, cpu_name);
    //CPUNAME = cpu_name;
}