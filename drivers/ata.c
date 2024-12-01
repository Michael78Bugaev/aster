#include <drv/ata.h>
#include <config.h>
#include <stdint.h>
#include <io/iotools.h>
#include <stdio.h>

ata_device_t ata_devices[2];

void ata_init() {
    // Инициализация ATA-дисков
    ata_devices[0].base = ATA_PRIMARY;
    ata_devices[1].base = ATA_SECONDARY;

    // Проверка наличия устройств
    for (int i = 0; i < 2; i++) {
        port_byte_out(ata_devices[i].base + ATA_DEV_REG, 0);
        if (port_byte_in(ata_devices[i].base + ATA_STATUS_REG) != 0xFF) {
            printf("[INFO]: ATA device found on channel %d\n", i);
        }
    }
}

void ata_read(uint8_t drive, uint32_t lba, uint8_t sector_count, void *buffer) {
    port_byte_out(ata_devices[drive].base + ATA_SECTOR_COUNT_REG, sector_count);
    port_byte_out(ata_devices[drive].base + ATA_LBA_LOW_REG, lba & 0xFF);
    port_byte_out(ata_devices[drive].base + ATA_LBA_MID_REG, (lba >> 8) & 0xFF);
    port_byte_out(ata_devices[drive].base + ATA_LBA_HIGH_REG, (lba >> 16) & 0xFF);
    port_byte_out(ata_devices[drive].base + ATA_DEV_REG, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    port_byte_out(ata_devices[drive].base + ATA_CMD_READ, ATA_CMD_READ);

    // Ожидание завершения операции
    while (!(port_byte_in(ata_devices[drive].base + ATA_STATUS_REG) & 0x08));

    // Чтение данных
    for (int i = 0; i < sector_count * 256; i++) {
        ((uint16_t *)buffer)[i] = port_word_in(ata_devices[drive].base + ATA_DATA_REG);
    }
}

void ata_write(uint8_t drive, uint32_t lba, uint8_t sector_count, const void *buffer) {
    port_byte_out(ata_devices[drive].base + ATA_SECTOR_COUNT_REG, sector_count);
    port_byte_out(ata_devices[drive].base + ATA_LBA_LOW_REG, lba & 0xFF);
    port_byte_out(ata_devices[drive].base + ATA_LBA_MID_REG, (lba >> 8) & 0xFF);
    port_byte_out(ata_devices[drive].base + ATA_LBA_HIGH_REG, (lba >> 16) & 0xFF);
    port_byte_out(ata_devices[drive].base + ATA_DEV_REG, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    port_byte_out(ata_devices[drive].base + ATA_CMD_WRITE, ATA_CMD_WRITE);

    // Ожидание завершения операции
    while (!(port_byte_in(ata_devices[drive].base + ATA_STATUS_REG) & 0x08));

    // Запись данных
    for (int i = 0; i < sector_count * 256; i++) {
        port_word_out(ata_devices[drive].base + ATA_DATA_REG, ((const uint16_t *)buffer)[i]);
    }
}