// sata.c
#include <drv/sata.h>
#include <string.h>
#include <drv/ata.h>
#include <storage.h>
#include <stdio.h>

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

#define HBA_PxCMD_CR 0x8000
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FR 0x4000

#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35

#define AHCI_BASE 0x400000 // This should be set to the actual AHCI base address

static SATA_HBA* hba;
static uint32_t port_count;

static int find_free_cmd_slot(SATA_PORT* port) {
    uint32_t slots = (port->sact | port->ci);
    for (int i = 0; i < 32; i++) {
        if ((slots & 1) == 0)
            return i;
        slots >>= 1;
    }
    return -1;
}

static int wait_for_completion(SATA_PORT* port, int slot) {
    while (1) {
        if ((port->ci & (1 << slot)) == 0)
            break;
        if (port->is & (1 << 30)) // Task file error
            return -1;
    }

    if (port->is & (1 << 30)) // Task file error
        return -1;

    return 0;
}

void sata_init(void) {
     hba = (SATA_HBA*)AHCI_BASE;
    
     // Enable AHCI mode
     hba->ghc |= 0x80000000;

     // Count implemented ports
     uint32_t pi = hba->pi;
     port_count = 0;
     for (int i = 0; i < 32; i++) {
         if (pi & 1) {
             port_count++;
         }
         pi >>= 1;
     }

     printf("SATA: Detected %d ports\n", port_count);

     // Initialize each implemented port
     for (int i = 0; i < port_count; i++) {
         SATA_PORT* port = &hba->ports[i];
        
         // Stop the port
         port->cmd &= ~HBA_PxCMD_ST;
         port->cmd &= ~HBA_PxCMD_FRE;

         // Clear any error bits
         port->serr = 0xFFFFFFFF;

         // Allocate and set up command list and received FIS structures
         // (This part would require memory allocation, which is not shown here)

         // Start the port
         port->cmd |= HBA_PxCMD_FRE;
         port->cmd |= HBA_PxCMD_ST;

         printf("SATA: Initialized port %d\n", i);
    }
}

int sata_read_sector(uint8_t port_num, uint64_t sector, uint32_t count, void* buffer) {
    if (port_num >= port_count) return -1;

    SATA_PORT* port = &hba->ports[port_num];
    int slot = find_free_cmd_slot(port);
    if (slot == -1) return -1;

    // Set up the command
    // (Command setup code here, similar to the previous example)

    // Issue the command
    port->ci = 1 << slot;

    // Wait for completion
    if (wait_for_completion(port, slot) != 0) {
        printf("SATA: Read error on port %d\n", port_num);
        return -1;
    }

    return 0;
}

int sata_write_sector(uint8_t port_num, uint64_t sector, uint32_t count, const void* buffer) {
    if (port_num >= port_count) return -1;

    SATA_PORT* port = &hba->ports[port_num];
    int slot = find_free_cmd_slot(port);
    if (slot == -1) return -1;

    // Set up the command
    // (Command setup code here, similar to the previous example)

    // Issue the command
    port->ci = 1 << slot;

    // Wait for completion
    if (wait_for_completion(port, slot) != 0) {
        printf("SATA: Write error on port %d\n", port_num);
        return -1;
    }

    return 0;
}

uint32_t sata_get_port_count(void) {
    return port_count;
}

int sata_detect(void) {
    // Читаем сигнатуру устройства
    uint32_t signature = port_dword_in(0x1F4);
    if (signature == 0xEB140101) {
        printf("SATA controller detected\n");
        return 1;
    }
    
    // Проверяем статус устройства
    uint8_t status = port_byte_in(0x1F7);
    if (status == 0xFF) {
        printf("No SATA device found\n");
        return 0;
    }
    
    // Проверяем идентификатор устройства
    port_byte_out(0x1F6, 0xE0);
    for(int i = 0; i < 4; i++) {
        port_byte_out(0x1F2 + i, 0);
    }
    
    // Отправляем команду IDENTIFY
    port_byte_out(0x1F7, 0xEC);
    
    // Ждем ответа
    status = port_byte_in(0x1F7);
    if (status == 0) {
        printf("SATA device not responding\n");
        return 0;
    }
    
    return 1;
}

// Проверяет присутствие устройства на конкретном порту
int sata_port_present(uint8_t port) {
    uint32_t port_base = 0x1F0 + (port * 0x80);
    
    // Читаем статус порта
    uint8_t status = port_byte_in(port_base + 7);
    if (status == 0xFF) {
        return 0;
    }
    
    // Проверяем сигнатуру устройства
    port_byte_out(port_base + 6, 0xE0 | ((port & 1) << 4));
    for(int i = 0; i < 4; i++) {
        port_byte_out(port_base + 2 + i, 0);
    }
    
    // Отправляем команду IDENTIFY
    port_byte_out(port_base + 7, 0xEC);
    
    // Ждем ответа
    status = port_byte_in(port_base + 7);
    if (status == 0) {
        return 0;
    }
    
    // Ждем готовности устройства
    while((status & 0x80) != 0) {
        status = port_byte_in(port_base + 7);
    }
    
    return 1;
}

// Получает модель устройства
void sata_get_model(uint8_t port, char* model) {
    uint32_t port_base = 0x1F0 + (port * 0x80);
    uint16_t data[256];
    
    // Отправляем команду IDENTIFY
    port_byte_out(port_base + 6, 0xE0 | ((port & 1) << 4));
    port_byte_out(port_base + 7, 0xEC);
    
    // Ждем готовности данных
    while((port_byte_in(port_base + 7) & 0x88) != 0x08);
    
    // Читаем данные идентификации
    for(int i = 0; i < 256; i++) {
        data[i] = port_word_in(port_base);
    }
    
    // Извлекаем название модели (слова 27-46)
    for(int i = 0; i < 20; i++) {
        model[i*2] = (data[27+i] >> 8) & 0xFF;
        model[i*2+1] = data[27+i] & 0xFF;
    }
    model[40] = '\0';
    
    // Убираем пробелы в конце
    for(int i = 39; i >= 0; i--) {
        if(model[i] == ' ') {
            model[i] = '\0';
        } else {
            break;
        }
    }
}

// Получает серийный номер устройства
void sata_get_serial(uint8_t port, char* serial) {
    uint32_t port_base = 0x1F0 + (port * 0x80);
    uint16_t data[256];
    
    // Отправляем команду IDENTIFY
    port_byte_out(port_base + 6, 0xE0 | ((port & 1) << 4));
    port_byte_out(port_base + 7, 0xEC);
    
    // Ждем готовности данных
    while((port_byte_in(port_base + 7) & 0x88) != 0x08);
    
    // Читаем данные идентификации
    for(int i = 0; i < 256; i++) {
        data[i] = port_word_in(port_base);
    }
    
    // Извлекаем серийный номер (слова 10-19)
    for(int i = 0; i < 10; i++) {
        serial[i*2] = (data[10+i] >> 8) & 0xFF;
        serial[i*2+1] = data[10+i] & 0xFF;
    }
    serial[20] = '\0';
    
    // Убираем пробелы в конце
    for(int i = 19; i >= 0; i--) {
        if(serial[i] == ' ') {
            serial[i] = '\0';
        } else {
            break;
        }
    }
}

// Получает размер устройства в секторах
uint64_t sata_get_size(uint8_t port) {
    uint32_t port_base = 0x1F0 + (port * 0x80);
    uint16_t data[256];
    
    // Отправляем команду IDENTIFY
    port_byte_out(port_base + 6, 0xE0 | ((port & 1) << 4));
    port_byte_out(port_base + 7, 0xEC);
    
    // Ждем готовности данных
    while((port_byte_in(port_base + 7) & 0x88) != 0x08);
    
    // Читаем данные идентификации
    for(int i = 0; i < 256; i++) {
        data[i] = port_word_in(port_base);
    }
    
    // Проверяем поддержку LBA48
    if(data[83] & (1 << 10)) {
        // Используем LBA48 (слова 100-103)
        uint64_t sectors = 0;
        sectors |= (uint64_t)data[100];
        sectors |= (uint64_t)data[101] << 16;
        sectors |= (uint64_t)data[102] << 32;
        sectors |= (uint64_t)data[103] << 48;
        return sectors;
    } else {
        // Используем LBA28 (слова 60-61)
        uint32_t sectors = 0;
        sectors |= data[60];
        sectors |= (uint32_t)data[61] << 16;
        return sectors;
    }
}

int sata_read_sectors(uint64_t lba, uint32_t count, void* buffer) {
    if (!buffer || count == 0) {
        printf("SATA: Invalid parameters\n");
        return -1;
    }

    // Проверяем готовность устройства
    uint8_t status = port_byte_in(SATA_PRIMARY_COMM_REGSTAT);
    if (status & 0x80) { // BSY bit
        printf("SATA: Device is busy\n");
        return -1;
    }

    // Отправляем параметры команды
    port_byte_out(SATA_PRIMARY_DRIVE_HEAD, 0x40); // Select drive (LBA mode)
    
    // Отправляем старшие биты LBA и count
    port_byte_out(SATA_PRIMARY_SECCOUNT, (count >> 8) & 0xFF);
    port_byte_out(SATA_PRIMARY_LBA_LO, (lba >> 24) & 0xFF);
    port_byte_out(SATA_PRIMARY_LBA_MID, (lba >> 32) & 0xFF);
    port_byte_out(SATA_PRIMARY_LBA_HI, (lba >> 40) & 0xFF);
    
    // Отправляем младшие биты LBA и count
    port_byte_out(SATA_PRIMARY_SECCOUNT, count & 0xFF);
    port_byte_out(SATA_PRIMARY_LBA_LO, lba & 0xFF);
    port_byte_out(SATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    port_byte_out(SATA_PRIMARY_LBA_HI, (lba >> 16) & 0xFF);

    // Отправляем команду чтения
    port_byte_out(SATA_PRIMARY_COMM_REGSTAT, SATA_CMD_READ_SECTORS);

    // Читаем данные
    uint8_t *buf = (uint8_t*)buffer;
    for (uint32_t i = 0; i < count; i++) {
        // Ждем готовности данных
        do {
            status = port_byte_in(SATA_PRIMARY_COMM_REGSTAT);
        } while ((status & 0x88) != 0x08); // Ждем DRQ и проверяем на ошибки

        // Читаем сектор
        insw(SATA_PRIMARY_DATA, buf + (i * 512), 256);
    }

    return 0;
}

/**
 * Записывает указанное количество секторов на SATA диск
 */
int sata_write_sectors(uint64_t lba, uint32_t count, const void* buffer) {
    if (!buffer || count == 0) {
        printf("SATA: Invalid parameters\n");
        return -1;
    }

    // Проверяем готовность устройства
    uint8_t status = port_byte_in(SATA_PRIMARY_COMM_REGSTAT);
    if (status & 0x80) {
        printf("SATA: Device is busy\n");
        return -1;
    }

    // Отправляем параметры команды
    port_byte_out(SATA_PRIMARY_DRIVE_HEAD, 0x40);
    
    // Отправляем старшие биты LBA и count
    port_byte_out(SATA_PRIMARY_SECCOUNT, (count >> 8) & 0xFF);
    port_byte_out(SATA_PRIMARY_LBA_LO, (lba >> 24) & 0xFF);
    port_byte_out(SATA_PRIMARY_LBA_MID, (lba >> 32) & 0xFF);
    port_byte_out(SATA_PRIMARY_LBA_HI, (lba >> 40) & 0xFF);
    
    // Отправляем младшие биты LBA и count
    port_byte_out(SATA_PRIMARY_SECCOUNT, count & 0xFF);
    port_byte_out(SATA_PRIMARY_LBA_LO, lba & 0xFF);
    port_byte_out(SATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    port_byte_out(SATA_PRIMARY_LBA_HI, (lba >> 16) & 0xFF);

    // Отправляем команду записи
    port_byte_out(SATA_PRIMARY_COMM_REGSTAT, SATA_CMD_WRITE_SECTORS);

    // Записываем данные
    const uint8_t *buf = (const uint8_t*)buffer;
    for (uint32_t i = 0; i < count; i++) {
        // Ждем готовности устройства принять данные
        do {
            status = port_byte_in(SATA_PRIMARY_COMM_REGSTAT);
        } while ((status & 0x88) != 0x08);

        // Записываем сектор
        outsw(SATA_PRIMARY_DATA, buf + (i * 512), 256);
    }

    // Ждем завершения записи
    do {
        status = port_byte_in(SATA_PRIMARY_COMM_REGSTAT);
    } while (status & 0x80);

    return 0;
}

/**
 * Сбрасывает кэш SATA диска
 */
int sata_flush(void) {
    // Отправляем команду FLUSH CACHE EXT
    port_byte_out(SATA_PRIMARY_COMM_REGSTAT, SATA_CMD_FLUSH);

    // Ждем завершения операции
    uint8_t status;
    do {
        status = port_byte_in(SATA_PRIMARY_COMM_REGSTAT);
    } while (status & 0x80); // Ждем пока BSY не очистится

    // Проверяем на ошибки
    if (status & 0x01) {
        printf("SATA: Error during flush operation\n");
        return -1;
    }

    return 0;
}