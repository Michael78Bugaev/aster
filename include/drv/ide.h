#include <stdint.h>
#include <stdbool.h>

#define IDE_PORT_BASE 0x1F0  // Базовый адрес для первого IDE-канала
#define IDE_CMD_READ 0x20     // Команда чтения
#define IDE_CMD_WRITE 0x30    // Команда записи
#define IDE_CMD_IDENTIFY 0xEC  // Команда идентификации

#define IDE_STATUS_BUSY 0x80   // Статус: занят
#define IDE_STATUS_DRQ 0x08     // Статус: данные готовы

// Структура для хранения информации о диске
typedef struct {
    uint16_t cylinders;
    uint16_t heads;
    uint16_t sectors;
    uint16_t signature;
    uint16_t reserved[3];
    uint16_t serial_number[10];
    char model[40];
    uint16_t firmware_version[4];
} ide_disk_info_t;

void ide_wait();
bool ide_read_sector(uint32_t lba, void *buffer);
bool ide_write_sector(uint32_t lba, const void *buffer);
bool ide_identify(ide_disk_info_t *info );