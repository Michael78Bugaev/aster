#include <storage.h>
#include <drv/ata.h>
#include <drv/sata.h>
#include <stdio.h>

#define MAX_STORAGE_DEVICES 4

static storage_device_t devices[MAX_STORAGE_DEVICES];
static int device_count = 0;

// Прототипы внутренних функций
static int detect_ata_devices(void);
static int detect_sata_devices(void);
static void setup_ata_device(storage_device_t* dev, uint8_t port);
static void setup_sata_device(storage_device_t* dev, uint8_t port);

void storage_init(void) {
    printf("Initializing storage subsystem...\n");
    
    // Сначала попробуем обнаружить SATA устройства
    if (detect_sata_devices() == 0) {
        printf("No SATA devices found. Searching for ATA...\n");
        // Если SATA устройства не найдены, ищем ATA
        detect_ata_devices();
    }
    
    printf("Found %d storage devices\n", device_count);
    
    // Вывести информацию о найденных устройствах
    for (int i = 0; i < device_count + 1; i++) {
        printf("Device %d: %s - %s\n", 
               i,
               devices[i].type == STORAGE_TYPE_SATA ? "SATA" : "ATA",
               devices[i].model);
    }
}

static int detect_sata_devices(void) {
    // Проверяем наличие SATA контроллера
    if (!sata_detect()) {
        return 0;
    }
    
    // Инициализируем SATA контроллер
    sata_init();
    
    // Перебираем все порты
    for (uint8_t port = 0; port < sata_get_port_count(); port++) {
        if (sata_port_present(port)) {
            setup_sata_device(&devices[device_count], port);
            device_count++;
        }
    }
    
    return device_count;
}

static int detect_ata_devices(void) {
    // Пробуем определить ATA устройства
    if (!ata_detect()) {
        return 0;
    }
    
    // Инициализируем первичный канал ATA
    if (ata_identify(ATA_PRIMARY) == 1) {
        setup_ata_device(&devices[device_count], ATA_PRIMARY);
        device_count++;
    }
    
    // Проверяем вторичный канал
    if (ata_identify(ATA_SECONDARY) == 1) {
        setup_ata_device(&devices[device_count], ATA_SECONDARY);
        device_count++;
    }
    
    return device_count;
}

// Функции для настройки устройств
static void setup_ata_device(storage_device_t* dev, uint8_t port) {
    dev->type = STORAGE_TYPE_ATA;
    dev->port = port;
    dev->present = 1;
    
    // Получаем информацию об устройстве
    ata_get_model(port, dev->model);
    ata_get_serial(port, dev->serial);
    dev->size_sectors = ata_get_size(port);
    
    // Устанавливаем функции для работы с устройством
    dev->read = ata_read_sectors;
    dev->write = ata_write_sectors;
    dev->flush = ata_flush;
}

static void setup_sata_device(storage_device_t* dev, uint8_t port) {
    dev->type = STORAGE_TYPE_SATA;
    dev->port = port;
    dev->present = 1;
    
    // Получаем информацию об устройстве
    sata_get_model(port, dev->model);
    sata_get_serial(port, dev->serial);
    dev->size_sectors = sata_get_size(port);
    
    // Устанавливаем функции для работы с устройством
    dev->read = sata_read_sectors;
    dev->write = sata_write_sectors;
    dev->flush = sata_flush;
}

// Публичные функции для получения информации об устройствах
storage_device_t* storage_get_device(uint8_t index) {
    if (index >= device_count) {
        return NULL;
    }
    return &devices[index];
}

int storage_get_device_count(void) {
    return device_count;
}