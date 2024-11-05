#include <chset/chipset.h>
#include <io/iotools.h>
#include <vga.h>
#include <stdio.h>
#include <cpu/mem.h>
#include <io/idt.h>

static ChipsetDevice chipset;

void chipset_initialize() {
    chipset.base_port = CHIPSET_BASE_PORT;
    chipset.initialized = false;

    // Инициализация регистров чипсета
    for (int i = 0; i < CHIPSET_MAX_REGISTERS; i++) {
        chipset.registers[i] = chipset_read_register(i);
    }

    // Отправка команды инициализации
    chipset_write_register(0, 0x01);

    // Настройка прерывания
    irq_install_handler(10, chipset_interrupt_handler); // Предполагаем, что чипсет использует IRQ 10

    chipset.initialized = true;
    printf("Chipset initialized at port: %x\n", chipset.base_port);
}

uint8_t chipset_read_register(uint8_t reg) {
    if (reg >= CHIPSET_MAX_REGISTERS) {
        printf("Error: Invalid register number\n");
        return 0xFF;
    }
    return port_byte_in(chipset.base_port + reg);
}

void chipset_write_register(uint8_t reg, uint8_t data) {
    if (reg >= CHIPSET_MAX_REGISTERS) {
        printf("Error: Invalid register number\n");
        return;
    }
    port_byte_out(chipset.base_port + reg, data);
    chipset.registers[reg] = data;
}

void chipset_enable_feature(uint8_t feature) {
    uint8_t control_reg = chipset_read_register(1); // Предполагаем, что регистр 1 - это регистр управления
    control_reg |= (1 << feature);
    chipset_write_register(1, control_reg);
    printf("Feature %d enabled\n", feature);
}

void chipset_disable_feature(uint8_t feature) {
    uint8_t control_reg = chipset_read_register(1);
    control_reg &= ~(1 << feature);
    chipset_write_register(1, control_reg);
    printf("Feature %d disabled\n", feature);
}

uint16_t chipset_get_status() {
    uint8_t status_low = chipset_read_register(2); // Предполагаем, что регистры 2 и 3 - статусные
    uint8_t status_high = chipset_read_register(3);
    return (status_high << 8) | status_low;
}

void chipset_interrupt_handler(struct InterruptRegisters *regs) {
    uint16_t status = chipset_get_status();
    printf("Chipset interrupt received. Status: %x\n", status);
    // Обработка различных состояний прерывания
    if (status & 0x0001) {
        printf("Data ready interrupt\n");
        // Обработка готовности данных
    }
    // Другие проверки статуса и соответствующие действия
}

void chipset_configure_dma(uint8_t channel, uint32_t address, uint32_t count) {
    // Настройка DMA канала
    // Это упрощенный пример, реальная настройка DMA может быть сложнее
    port_byte_out(0x0A, channel & 0x03); // Маскирование канала
    port_byte_out(0x0C, 0); // Сброс указателя флип-флопа
    port_byte_out(0x04 + (channel & 0x03) * 2, address & 0xFF);
    port_byte_out(0x04 + (channel & 0x03) * 2, (address >> 8) & 0xFF);
    port_byte_out(0x81 + (channel & 0x03), (address >> 16) & 0xFF);
    port_byte_out(0x0C, 0); // Сброс указателя флип-флопа
    port_byte_out(0x05 + (channel & 0x03) * 2, count & 0xFF);
    port_byte_out(0x05 + (channel & 0x03) * 2, (count >> 8) & 0xFF);
    port_byte_out(0x0A, channel & 0x03); // Размаскирование канала
    printf("DMA channel %d configured\n", channel);
}

void chipset_cleanup() {
    if (chipset.initialized) {
        // Отключение всех функций чипсета
        chipset_write_register(1, 0x00);
        
        // Удаление обработчика прерываний
        irq_uninstall_handler(10);

        chipset.initialized = false;
        printf("Chipset driver cleaned up\n");
    }
}

void chipset_run_diagnostics() {
    printf("Running chipset diagnostics...\n");
    
    // Проверка чтения/записи регистров
    for (int i = 0; i < CHIPSET_MAX_REGISTERS; i++) {
        uint8_t test_value = 0xAA;
        chipset_write_register(i, test_value);
        uint8_t read_value = chipset_read_register(i);
        if (read_value != test_value) {
            printf("Diagnostic failed for register %d: wrote %x, read %x\n", i, test_value, read_value);
        } else {
            printf("Register %d passed diagnostic: %x\n", i, read_value);
        }
    }
    
    // Дополнительные проверки и тесты могут быть добавлены здесь
    printf("Diagnostics complete.\n");
}