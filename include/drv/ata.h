#ifndef ATA_H
#define ATA_H

#include <stdint.h>

#define ATA_PRIMARY 0x1F0
#define ATA_SECONDARY 0x170
#define ATA_CMD_READ 0x20
#define ATA_CMD_WRITE 0x30
#define ATA_STATUS_REG 0x07
#define ATA_DATA_REG 0x00
#define ATA_SECTOR_COUNT_REG 0x02
#define ATA_LBA_LOW_REG 0x03
#define ATA_LBA_MID_REG 0x04
#define ATA_LBA_HIGH_REG 0x05
#define ATA_DEV_REG 0x06

typedef struct {
    uint16_t base;
    uint16_t control;
    uint16_t bus_master;
} ata_device_t;

void ata_init();
void ata_read(uint8_t drive, uint32_t lba, uint8_t sector_count, void *buffer);
void ata_write(uint8_t drive, uint32_t lba, uint8_t sector_count, const void *buffer);

#endif // ATA_H