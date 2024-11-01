#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>

// Типы устройств хранения
typedef enum {
    STORAGE_TYPE_UNKNOWN,
    STORAGE_TYPE_ATA,
    STORAGE_TYPE_SATA,
    STORAGE_TYPE_MAX
} storage_type_t;

// Структура для хранения информации об устройстве
typedef struct {
    storage_type_t type;
    uint8_t port;
    uint8_t present;
    char model[41];
    char serial[21];
    uint64_t size_sectors;
    
    // Функции для работы с устройством
    int (*read)(uint64_t lba, uint32_t count, void* buffer);
    int (*write)(uint64_t lba, uint32_t count, const void* buffer);
    int (*flush)(void);
} storage_device_t;

// Публичные функции
void storage_init(void);
storage_device_t* storage_get_device(uint8_t index);
int storage_get_device_count(void);

#endif