#include <drv/sata.h>
#include <io/iotools.h>
#include <cpu/achi.h>
#include <stdio.h>

// Ожидание готовности устройства
void sata_wait_ready(sata_device_t *device) {
    while (port_byte_in(device->base + SATA_STATUS) & SATA_STATUS_BSY);
}

// Мягкий сброс устройства
void sata_soft_reset(sata_device_t *device) {
    port_byte_out(device->control, 0x04);
    for(int i = 0; i < 1000; i++); // Небольшая задержка
    port_byte_out(device->control, 0x00);
    sata_wait_ready(device);
}

// Инициализация устройства SATA
bool sata_initialize(sata_device_t *device, uint16_t base, uint16_t control, uint16_t bm_ide, bool master) {
    device->base = base;
    device->control = control;
    device->bm_ide = bm_ide;
    device->master = master;

    // Выполняем мягкий сброс
    sata_soft_reset(device);

    // Проверяем наличие устройства
    uint8_t status = port_byte_in(device->base + SATA_STATUS);
    if (status == 0xFF) {
        printf("No SATA device detected\n");
        return false;
    }

    // Идентифицируем устройство
    return sata_identify(device);
}

// Идентификация устройства SATA
bool sata_identify(sata_device_t *device) {
    uint16_t identify_data[256];

    // Выбираем устройство
    port_byte_out(device->base + SATA_DEVHEAD, device->master ? 0xA0 : 0xB0);
    sata_wait_ready(device);

    // Отправляем команду IDENTIFY
    port_byte_out(device->base + SATA_COMMAND, SATA_CMD_IDENTIFY);
    sata_wait_ready(device);

    // Проверяем статус
    uint8_t status = port_byte_in(device->base + SATA_STATUS);
    if (status == 0) {
        printf("Device does not exist\n");
        return false;
    }

    // Ждем данные
    while(1) {
        status = port_byte_in(device->base + SATA_STATUS);
        // if (status & SATA_STATUS_ERR) {
        //     printf("Error during identify\n");
        //     //return false;
        // }
        if (!(status & SATA_STATUS_BSY) && (status & SATA_STATUS_DRQ)) {
            break;
        }
    }

    // Читаем идентификационные данные
    for (int i = 0; i < 256; i++) {
        identify_data[i] = port_word_in(device->base + SATA_DATA);
    }

    // Сохраняем важную информацию
    device->capabilities = *((uint32_t*)&identify_data[49]);
    device->command_sets = *((uint32_t*)&identify_data[82]);
    device->size = *((uint32_t*)&identify_data[60]);

    printf("SATA device initialized. Size: %d sectors\n", device->size);
    return true;
}

// Чтение секторов
bool sata_read_sectors(sata_device_t *device, uint32_t lba, uint8_t sectors, void *buffer) {
    printf("SATA: Reading sector %d, count: %d\n", lba, sectors);
    if (!sectors) return true;

    sata_wait_ready(device);

    // Выбираем устройство и устанавливаем режим LBA
    port_byte_out(device->base + SATA_DEVHEAD, (device->master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
    
    // Устанавливаем параметры чтения
    port_byte_out(device->base + SATA_FEATURES, 0);
    port_byte_out(device->base + SATA_SECCOUNT, sectors);
    port_byte_out(device->base + SATA_SECNUM, lba & 0xFF);
    port_byte_out(device->base + SATA_CYLLOW, (lba >> 8) & 0xFF);
    port_byte_out(device->base + SATA_CYLHIGH, (lba >> 16) & 0xFF);

    // Отправляем команду чтения
    port_byte_out(device->base + SATA_COMMAND, SATA_CMD_READ_DMA_EXT);

    // Читаем данные
    uint16_t *buf = (uint16_t*)buffer;
    for (int i = 0; i < sectors * 256; i++) {
        sata_wait_ready(device);
        buf[i] = port_word_in(device->base + SATA_DATA);
    }

    return true;
}

// Запись секторов
bool sata_write_sectors(sata_device_t *device, uint32_t lba, uint8_t sectors, const void *buffer) {
    if (!sectors) return true;

    sata_wait_ready(device);

    // Выбираем устройство и устанавливаем режим LBA
    port_byte_out(device->base + SATA_DEVHEAD, (device->master ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
    
    // Устанавливаем параметры записи
    port_byte_out(device->base + SATA_FEATURES, 0);
    port_byte_out(device->base + SATA_SECCOUNT, sectors);
    port_byte_out(device->base + SATA_SECNUM, lba & 0xFF);
    port_byte_out(device->base + SATA_CYLLOW, (lba >> 8) & 0xFF);
    port_byte_out(device->base + SATA_CYLHIGH, (lba >> 16) & 0xFF);

    // Отправляем команду записи
    port_byte_out(device->base + SATA_COMMAND, SATA_CMD_WRITE_DMA_EXT);

    // Пишем данные
    const uint16_t *buf = (const uint16_t*)buffer;
    for (int i = 0; i < sectors * 256; i++) {
        port_word_out(device->base + SATA_DATA, buf[i]);
    }

    return true;
}

int sata_init_device(sata_device_t *device) {
    // Сброс порта
    device->port.cmd &= ~HBA_PxCMD_ST;
    device->port.cmd &= ~HBA_PxCMD_FRE;
    
    // Ждем остановки
    uint32_t spin = 0;
    while ((device->port.cmd & HBA_PxCMD_CR) || (device->port.cmd & HBA_PxCMD_FR)) {
        spin++;
        if (spin > 1000000) {
            printf("SATA: Port reset timeout\n");
            return -1;
        }
    }

    // Проверяем тип устройства
    uint32_t ssts = device->port.ssts;
    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;
    
    if (det != SATA_DEV_PRESENT || ipm != SATA_DEV_IPM_ACTIVE) {
        printf("SATA: No active device present\n");
        return -2;
    }

    // Инициализация порта
    device->port.serr = 0xFFFFFFFF; // Очищаем все ошибки
    device->port.is = 0xFFFFFFFF;   // Очищаем все прерывания

    // Теперь можно включить порт
    device->port.cmd |= HBA_PxCMD_FRE;
    device->port.cmd |= HBA_PxCMD_ST;

    printf("SATA: Device initialized successfully\n");
    return 0;
}

int sata_format(sata_device_t *device, uint32_t start_lba, uint32_t count) {
    // HBA_PORT *port = (HBA_PORT*)device->port;
    // uint32_t spin = 0;
    
    // // Ждем готовности порта
    // while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
    //     spin++;
    // }
    // if (spin == 1000000) {
    //     return -1; // Timeout
    // }

    // // Настраиваем командный заголовок
    // HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(uint64_t)(port->clb);
    // cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t); // Command FIS size
    // cmdheader->w = 1;  // Write operation
    // cmdheader->prdtl = 0; // No data transfer

    // // Настраиваем Command FIS
    // FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)((uint64_t)(port->fb));
    // memset(cmdfis, 0, sizeof(FIS_REG_H2D));

    // cmdfis->fis_type = FIS_TYPE_REG_H2D;
    // cmdfis->command = ATA_CMD_FORMAT_TRACK;  // Format track command
    // cmdfis->device = 0x40;  // Set LBA mode

    // // Устанавливаем LBA
    // cmdfis->lba0 = (uint8_t)start_lba;
    // cmdfis->lba1 = (uint8_t)(start_lba >> 8);
    // cmdfis->lba2 = (uint8_t)(start_lba >> 16);
    // cmdfis->lba3 = (uint8_t)(start_lba >> 24);
    // cmdfis->lba4 = 0;
    // cmdfis->lba5 = 0;

    // // Устанавливаем количество секторов
    // cmdfis->countl = count & 0xFF;
    // cmdfis->counth = (count >> 8) & 0xFF;

    // cmdfis->c = 1; // Command bit

    // // Отправляем команду
    // port->ci = 1;

    // // Ждем завершения
    // while (1) {
    //     if ((port->ci & 1) == 0) {
    //         break;
    //     }
    //     if (port->is & HBA_PxIS_TFES) {
    //         return -1; // Task file error
    //     }
    // }

    // // Проверяем наличие ошибок
    // if (port->is & HBA_PxIS_TFES) {
    //     return -1;
    // }

    // // Выполняем flush после форматирования
    // if (sata_flush(&device) != 0) {
    //     return -1;
    // }

    return 0;
}

bool format_drive(sata_device_t* device) {
    printf("Starting drive format...\n");

    // 1. Очистка первых нескольких секторов
    uint8_t empty_sector[512];
    memset(empty_sector, 0, 512);

    for (int i = 0; i < 64; i++) {  // Очищаем первые 64 сектора
        if (!sata_write_sectors(device, i, 1, empty_sector)) {
            printf("Failed to clear sector %d\n", i);
            return false;
        }
    }

    // 2. Создание MBR (Master Boot Record)
    uint8_t mbr[512];
    memset(mbr, 0, 512);

    // Устанавливаем сигнатуру загрузочного сектора
    mbr[510] = 0x55;
    mbr[511] = 0xAA;

    // Создаем одну основную партицию, занимающую весь диск
    uint32_t total_sectors = device->size;
    
    // Начало партиции (обычно с сектора 2048 для выравнивания)
    uint32_t start_sector = 2048;
    
    // Размер партиции
    uint32_t partition_size = total_sectors - start_sector;

    // Заполняем запись о партиции
    mbr[446] = 0x80;  // Загрузочный флаг
    mbr[447] = 0;     // Начальная голова
    mbr[448] = 0;     // Начальный сектор
    mbr[449] = 0;     // Начальный цилиндр
    mbr[450] = 0x0B;  // Тип системы (0x0B для FAT32)
    mbr[451] = 0;     // Конечная голова
    mbr[452] = 0;     // Конечный сектор
    mbr[453] = 0;     // Конечный цилиндр

    // Записываем начальный сектор и размер партиции
    *(uint32_t*)&mbr[454] = start_sector;
    *(uint32_t*)&mbr[458] = partition_size;

    // Записываем MBR
    if (!sata_write_sectors(device, 0, 1, mbr)) {
        printf("Failed to write MBR\n");
        return false;
    }

    // 3. Инициализация FAT32
    if (!init_fat32_filesystem(device)) {
        printf("Failed to initialize FAT32 filesystem\n");
        return false;
    }

    printf("Drive format completed successfully\n");
    return true;
}