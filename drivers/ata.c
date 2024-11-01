#include <io/iotools.h>
#include <vga.h>
#include <stdint.h>
#include <cpu/mem.h>
#include <drv/ata.h>
#include <drv/sata.h>

int is_drive_exist = 0;


/**
 * To use the IDENTIFY command, select a target drive by sending 0xA0 for the master drive,
 * or 0xB0 for the slave, to the "drive select" IO port. On the Primary bus, this would be port 0x1F6.
 * Then set the Sectorcount, LBAlo, LBAmid, and LBAhi IO ports to 0 (port 0x1F2 to 0x1F5). Then send
 * the IDENTIFY command (0xEC) to the Command IO port (0x1F7).
 *
 * Then read the Status port (0x1F7) again.
 * If the value read is 0, the drive does not exist.
 * For any other value: poll the Status port (0x1F7) until bit 7 (BSY, value = 0x80) clears.
 * Because of some ATAPI drives that do not follow spec, at this point you need to check the
 * LBAmid and LBAhi ports (0x1F4 and 0x1F5) to see if they are non-zero. If so, the drive is not ATA,
 * and you should stop polling. Otherwise, continue polling one of the Status
 * ports until bit 3 (DRQ, value = 8) sets, or until bit 0 (ERR, value = 1) sets.
 *
 * At that point, if ERR is clear, the data is ready to read from the Data port (0x1F0).
 * Read 256 16-bit values, and store them.
 */
int is_ex()
{
    return is_drive_exist;
}
uint8_t identify() {
    printf("Starting indentify IDE (ATA) disks...\n");
    port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
    port_byte_out(ATA_PRIMARY_DRIVE_HEAD, 0xA0);
    port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
    port_byte_out(ATA_PRIMARY_SECCOUNT, 0);
    port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
    port_byte_out(ATA_PRIMARY_LBA_LO, 0);
    port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
    port_byte_out(ATA_PRIMARY_LBA_MID, 0);
    port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
    port_byte_out(ATA_PRIMARY_LBA_HI, 0);
    port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
    port_byte_out(ATA_PRIMARY_COMM_REGSTAT, 0xEC);
    port_byte_out(ATA_PRIMARY_COMM_REGSTAT, 0xE7);

    // Read the status port. If it's zero, the drive does not exist.
    uint8_t status = port_byte_in(ATA_PRIMARY_COMM_REGSTAT);

    printf("Waiting for status...");
    while(status & STAT_BSY) {
        status = port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
    }
    
    if(status == 0)
    {
        printf("fail.\n");
        printf("panic: IDE drive does not exist.\n");
        printf("warning: IDE disk will not be avaliable\n");
        return 0;
    }
    printf("done.\n");
    printf("polling while STAT_BSY...");
    while(status & STAT_BSY) {
      status = port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
    }
    printf("done.\n");

    uint8_t mid = port_byte_in(ATA_PRIMARY_LBA_MID);
    uint8_t hi = port_byte_in(ATA_PRIMARY_LBA_HI);
    if(mid || hi) {
        // The drive is not ATA. (Who knows what it is.)
        printf("error: the drive is not ATA\n");
        printf("warning: IDE disk will not be avaliable\n");
        return 0;
    }

    printf("waiting for ERR or DRQ.\n");
    // Wait for ERR or DRQ
    while(!(status & (STAT_ERR | STAT_DRQ))) {
        status = port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
    }

    if(status & STAT_ERR) {
        // There was an error on the drive. Forget about it.
        printf("error: drive error\n");
        printf("warning: IDE disk will not be avaliable\n");
        return 0;
    }

    printf("reading IDENTIFY structure\n");
    //uint8_t *buff = kmalloc(40960, 0, NULL);
    malloc(256 * 2);
    uint8_t buff[256 * 2];
    insw(ATA_PRIMARY_DATA, buff, 256);
    printf("success. disk is ready to go.\n");
    mfree(&buff);
    // We read it!
    is_drive_exist = 1;
    return 1;
}

/**
 * An example of a 28 bit LBA PIO mode read on the Primary bus:
 *
 *     Send 0xE0 for the "master" or 0xF0 for the "slave", ORed with the highest 4 bits of the LBA to port 0x1F6: port_byte_out(0x1F6, 0xE0 | (slavebit << 4) | ((LBA >> 24) & 0x0F))
 *     Send a NULL byte to port 0x1F1, if you like (it is ignored and wastes lots of CPU time): port_byte_out(0x1F1, 0x00)
 *     Send the sectorcount to port 0x1F2: port_byte_out(0x1F2, (unsigned char) count)
 *     Send the low 8 bits of the LBA to port 0x1F3: port_byte_out(0x1F3, (unsigned char) LBA))
 *     Send the next 8 bits of the LBA to port 0x1F4: port_byte_out(0x1F4, (unsigned char)(LBA >> 8))
 *     Send the next 8 bits of the LBA to port 0x1F5: port_byte_out(0x1F5, (unsigned char)(LBA >> 16))
 *     Send the "READ SECTORS" command (0x20) to port 0x1F7: port_byte_out(0x1F7, 0x20)
 *     Wait for an IRQ or poll.
 *     Transfer 256 16-bit values, a uint16_t at a time, into your buffer from I/O port 0x1F0. (In assembler, REP INSW works well for this.)
 *     Then loop back to waiting for the next IRQ (or poll again -- see next note) for each successive sector.
 */


void ata_pio_read28(uint32_t LBA, uint8_t sectorcount, uint8_t *target) {
    // HARD CODE MASTER (for now)
    port_byte_out(ATA_PRIMARY_DRIVE_HEAD, 0xE0 | ((LBA >> 24) & 0x0F));
    port_byte_out(ATA_PRIMARY_ERR, 0x00);
    port_byte_out(ATA_PRIMARY_SECCOUNT, sectorcount);
    port_byte_out(ATA_PRIMARY_LBA_LO, LBA & 0xFF);
    port_byte_out(ATA_PRIMARY_LBA_MID, (LBA >> 8) & 0xFF);
    port_byte_out(ATA_PRIMARY_LBA_HI, (LBA >> 16) & 0xFF);
    port_byte_out(ATA_PRIMARY_COMM_REGSTAT, 0x20);

    uint8_t i;
    for(i = 0; i < sectorcount; i++) {
        // POLL!
        while(1) {
            uint8_t status = port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
            if(status & STAT_DRQ) {
                // Drive is ready to transfer data!
                break;
            }
        }
        // Transfer the data!
        insw(ATA_PRIMARY_DATA, (void *)target, 256);
        target += 256;
    }

}

/**
 * 48-bit LBA read
 *
 * Send 0x40 for the "master" or 0x50 for the "slave" to port 0x1F6: port_byte_out(0x1F6, 0x40 | (slavebit << 4))
 * port_byte_out (0x1F2, sectorcount high byte)
 * port_byte_out (0x1F3, LBA4)
 * port_byte_out (0x1F4, LBA5)
 * port_byte_out (0x1F5, LBA6)
 * port_byte_out (0x1F2, sectorcount low byte)
 * port_byte_out (0x1F3, LBA1)
 * port_byte_out (0x1F4, LBA2)
 * port_byte_out (0x1F5, LBA3)
 * Send the "READ SECTORS EXT" command (0x24) to port 0x1F7: port_byte_out(0x1F7, 0x24)
 */

void ata_pio_read48(uint64_t LBA, uint16_t sectorcount, uint8_t *target) {
    // HARD CODE MASTER (for now)
    port_byte_out(ATA_PRIMARY_DRIVE_HEAD, 0x40);                     // Select master
    port_byte_out(ATA_PRIMARY_SECCOUNT, (sectorcount >> 8) & 0xFF ); // sectorcount high
    port_byte_out(ATA_PRIMARY_LBA_LO, (LBA >> 24) & 0xFF);           // LBA4
    port_byte_out(ATA_PRIMARY_LBA_MID, (LBA >> 32) & 0xFF);          // LBA5
    port_byte_out(ATA_PRIMARY_LBA_HI, (LBA >> 40) & 0xFF);           // LBA6
    port_byte_out(ATA_PRIMARY_SECCOUNT, sectorcount & 0xFF);         // sectorcount low
    port_byte_out(ATA_PRIMARY_LBA_LO, LBA & 0xFF);                   // LBA1
    port_byte_out(ATA_PRIMARY_LBA_MID, (LBA >> 8) & 0xFF);           // LBA2
    port_byte_out(ATA_PRIMARY_LBA_HI, (LBA >> 16) & 0xFF);           // LBA3
    port_byte_out(ATA_PRIMARY_COMM_REGSTAT, 0x24);                   // READ SECTORS EXT


    uint8_t i;
    for(i = 0; i < sectorcount; i++) {
        // POLL!
        uint8_t status = port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
        if(status & STAT_DRQ) {
             printf("drive is ready to transfer data\n");
             return;
        }
        // Transfer the data!
        insw(ATA_PRIMARY_DATA, (void *)target, 256);
        target += 256;
    }

}

/**
 * To write sectors in 48 bit PIO mode, send command "WRITE SECTORS EXT" (0x34), instead.
 * (As before, do not use REP OUTSW when writing.) And remember to do a Cache Flush after
 * each write command completes.
 */
void ata_pio_write48(uint64_t LBA, uint16_t sectorcount, uint8_t *target) {

    // HARD CODE MASTER (for now)
    port_byte_out(ATA_PRIMARY_DRIVE_HEAD, 0x40);                     // Select master
    port_byte_out(ATA_PRIMARY_SECCOUNT, (sectorcount >> 8) & 0xFF ); // sectorcount high
    port_byte_out(ATA_PRIMARY_LBA_LO, (LBA >> 24) & 0xFF);           // LBA4
    port_byte_out(ATA_PRIMARY_LBA_MID, (LBA >> 32) & 0xFF);          // LBA5
    port_byte_out(ATA_PRIMARY_LBA_HI, (LBA >> 40) & 0xFF);           // LBA6
    port_byte_out(ATA_PRIMARY_SECCOUNT, sectorcount & 0xFF);         // sectorcount low
    port_byte_out(ATA_PRIMARY_LBA_LO, LBA & 0xFF);                   // LBA1
    port_byte_out(ATA_PRIMARY_LBA_MID, (LBA >> 8) & 0xFF);           // LBA2
    port_byte_out(ATA_PRIMARY_LBA_HI, (LBA >> 16) & 0xFF);           // LBA3
    port_byte_out(ATA_PRIMARY_COMM_REGSTAT, 0x34);                   // READ SECTORS EXT

    uint8_t i;
    for(i = 0; i < sectorcount; i++) {
        // POLL!
        while(1) {
            uint8_t status = port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
            if(status & STAT_DRQ) {
                // Drive is ready to transfer data!
                break;
            }
            else if(status & STAT_ERR) {
                 return;
            }
        }
        // Transfer the data!
        outsw(ATA_PRIMARY_DATA, (void *)target, 256);
        target += 256;
    }

    // Flush the cache.
    port_byte_out(ATA_PRIMARY_COMM_REGSTAT, 0xE7);
    // Poll for BSY.
    while(port_byte_in(ATA_PRIMARY_COMM_REGSTAT) & STAT_BSY) {}
}

uint16_t get_status()
{
    uint16_t status = port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
    while (status & STAT_BSY)
    {
        status = port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
    }
    return status;
}

int ata_detect(void) {
    // Проверяем наличие ATA устройств
    uint16_t signature = port_word_in(0x1F2);
    return (signature == 0xEB14 || signature == 0x9669);
}

int ata_identify(uint8_t channel) {
    uint16_t io_base = (channel == ATA_PRIMARY) ? 0x1F0 : 0x170;
    
    // Отправляем команду IDENTIFY
    port_byte_out(io_base + 6, 0xA0);  // Select master drive
    port_byte_out(io_base + 7, 0xEC);  // IDENTIFY command
    
    // Ждем ответа
    if (port_byte_in(io_base + 7) == 0) return 0;
    
    while (port_byte_in(io_base + 7) & 0x80);  // Wait until BSY clears
    
    if (port_byte_in(io_base + 4) != 0 && port_byte_in(io_base + 5) != 0) {
        // Device exists
        while ((port_byte_in(io_base + 7) & 0x08) == 0);  // Wait for DRQ
        
        // Read identify data
        for (int i = 0; i < 256; i++) {
            uint16_t data = port_word_in(io_base);
            // Store data as needed
        }
        return 1;
    }
    
    return 0;
}

void ata_get_model(uint8_t channel, char* model) {
    uint16_t io_base = (channel == ATA_PRIMARY) ? 0x1F0 : 0x170;
    
    // Model name starts at word 27 and is 40 bytes long
    for (int i = 0; i < 20; i++) {
        uint16_t data = port_word_in(io_base);
        model[i*2] = (data >> 8) & 0xFF;
        model[i*2 + 1] = data & 0xFF;
    }
    model[40] = '\0';
    
    // Trim trailing spaces
    for (int i = 39; i >= 0; i--) {
        if (model[i] == ' ')
            model[i] = '\0';
        else
            break;
    }
}

void ata_get_serial(uint8_t channel, char* serial) {
    uint16_t io_base = (channel == ATA_PRIMARY) ? 0x1F0 : 0x170;
    
    // Serial number starts at word 10 and is 20 bytes long
    for (int i = 0; i < 10; i++) {
        uint16_t data = port_word_in(io_base);
        serial[i*2] = (data >> 8) & 0xFF;
        serial[i*2 + 1] = data & 0xFF;
    }
    serial[20] = '\0';
    
    // Trim trailing spaces
    for (int i = 19; i >= 0; i--) {
        if (serial[i] == ' ')
            serial[i] = '\0';
        else
            break;
    }
}

uint64_t ata_get_size(uint8_t channel) {
    uint16_t io_base = (channel == ATA_PRIMARY) ? 0x1F0 : 0x170;
    
    // LBA48 sector count is at words 100-103
    uint64_t sectors = 0;
    for (int i = 3; i >= 0; i--) {
        uint16_t data = port_word_in(io_base);
        sectors = (sectors << 16) | data;
    }
    
    return sectors;
}

/**
 * Читает указанное количество секторов с диска
 * @param lba Начальный адрес сектора
 * @param count Количество секторов для чтения
 * @param buffer Буфер для данных
 * @return 0 при успехе, -1 при ошибке
 */
int ata_read_sectors(uint64_t lba, uint32_t count, void* buffer) {
    // Проверяем, существует ли диск
    if (!is_ex()) {
        printf("ATA: No disk present\n");
        return -1;
    }

    // Проверяем параметры
    if (buffer == NULL || count == 0) {
        printf("ATA: Invalid parameters\n");
        return -1;
    }

    // Получаем статус диска
    uint8_t status = get_status();
    if (status & STAT_ERR) {
        printf("ATA: Device error before read\n");
        return -1;
    }

    // Читаем сектора используя LBA48 режим
    ata_pio_read48(lba, count, buffer);

    // Проверяем статус после операции
    status = get_status();
    if (status & STAT_ERR) {
        printf("ATA: Error during read operation\n");
        return -1;
    }

    return 0;
}

/**
 * Записывает указанное количество секторов на диск
 * @param lba Начальный адрес сектора
 * @param count Количество секторов для записи
 * @param buffer Буфер с данными
 * @return 0 при успехе, -1 при ошибке
 */
int ata_write_sectors(uint64_t lba, uint32_t count, const void* buffer) {
    // Проверяем, существует ли диск
    if (!is_ex()) {
        printf("ATA: No disk present\n");
        return -1;
    }

    // Проверяем параметры
    if (buffer == NULL || count == 0) {
        printf("ATA: Invalid parameters\n");
        return -1;
    }

    // Получаем статус диска
    uint8_t status = get_status();
    if (status & STAT_ERR) {
        printf("ATA: Device error before write\n");
        return -1;
    }

    // Записываем сектора используя LBA48 режим
    ata_pio_write48(lba, count, (uint8_t*)buffer);

    // Проверяем статус после операции
    status = get_status();
    if (status & STAT_ERR) {
        printf("ATA: Error during write operation\n");
        return -1;
    }

    return 0;
}

/**
 * Сбрасывает кэш диска на физический носитель
 * @return 0 при успехе, -1 при ошибке
 */
int ata_flush(void) {
    // Проверяем, существует ли диск
    if (!is_ex()) {
        printf("ATA: No disk present\n");
        return -1;
    }

    // Отправляем команду FLUSH CACHE
    port_byte_out(ATA_PRIMARY_COMM_REGSTAT, 0xE7);

    // Ждем, пока устройство не будет готово
    uint8_t status;
    do {
        status = port_byte_in(ATA_PRIMARY_COMM_REGSTAT);
    } while (status & STAT_BSY);

    // Проверяем на ошибки
    if (status & STAT_ERR) {
        printf("ATA: Error during flush operation\n");
        return -1;
    }

    return 0;
}

void init_ahci() {
    HBA_MEM* abar = find_ahci_controller();
    if(!abar) {
        printf("SATA: No AHCI controller found\n");
        return;
    }

    // Включаем AHCI режим
    abar->ghc |= (1 << 31); // Enable AHCI
    
    // Проверяем порты
    uint32_t pi = abar->pi; // Ports implemented
    int ports_count = 0;
    
    printf("SATA: Scanning ports...\n");
    
    for(int i = 0; i < 32; i++) {
        if(pi & (1 << i)) { // Порт реализован
            HBA_PORT* port = &abar->ports[i];
            uint32_t ssts = port->ssts;
            uint8_t ipm = (ssts >> 8) & 0x0F;
            uint8_t det = ssts & 0x0F;
            
            if(det == 3 && ipm == 1) { // Устройство присутствует и активно
                ports_count++;
                printf("SATA: Device found on port %d\n", i);
                
                // Инициализация порта
                port->cmd &= ~HBA_PxCMD_ST; // Остановить команды
                port->cmd &= ~HBA_PxCMD_FRE; // Остановить FIS receive
                
                // Ждем остановки
                while(port->cmd & (HBA_PxCMD_CR | HBA_PxCMD_FR));
                
                // Настраиваем Command List и FIS
                // ... (настройка DMA и других структур)
                
                // Запускаем порт
                port->cmd |= HBA_PxCMD_FRE;
                port->cmd |= HBA_PxCMD_ST;
            }
        }
    }
    
    printf("SATA: Detected %d ports\n", ports_count);
}