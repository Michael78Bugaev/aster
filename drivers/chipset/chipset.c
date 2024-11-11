#include <chset/chipset.h>
#include <io/iotools.h>
#include <stdio.h>
#include <cpu/pit.h>

// Функция инициализации чипсета
void init_chipset() {
    // Настройка базовых регистров чипсета
    port_byte_out(CHIPS_BASE_ADDRESS + CHIPS_CONFIG_REGISTER, 0x01); // Пример настройки
    port_byte_out(CHIPS_BASE_ADDRESS + CHIPS_STATUS_REGISTER, 0x00); // Сброс статуса
    printf("\nUniversal chipset driver UCD v.1.0.0\nInfo: Chipset initialized successfully.\n\n");
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