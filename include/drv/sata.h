#ifndef SATA_H
#define SATA_H

#include <stdint.h>
#include <stdbool.h>
#include <cpu/achi.h>

// SATA registers
#define SATA_DATA        0x00
#define SATA_ERROR      0x01
#define SATA_FEATURES   0x01
#define SATA_SECCOUNT   0x02
#define SATA_SECNUM     0x03
#define SATA_CYLLOW     0x04
#define SATA_CYLHIGH    0x05
#define SATA_DEVHEAD    0x06
#define SATA_STATUS     0x07
#define SATA_COMMAND    0x07

// SATA commands
#define SATA_CMD_READ_DMA_EXT    0x25
#define SATA_CMD_WRITE_DMA_EXT   0x35
#define SATA_CMD_IDENTIFY        0xEC

// Status bits
#define SATA_STATUS_BSY  0x80
#define SATA_STATUS_DRDY 0x40
#define SATA_STATUS_DRQ  0x08
#define SATA_STATUS_ERR  0x01

#define ATA_DEV_BUSY  0x80
#define ATA_DEV_DRQ   0x08
#define ATA_DEV_ERR   0x01

// Статусы SATA
#define SATA_DEV_PRESENT     0x3
#define SATA_DEV_IPM_ACTIVE  0x1

// Команды ATA
#define ATA_CMD_FORMAT_TRACK    0x50    // Format track command

typedef struct {
    uint16_t base;          // Base I/O port
    uint16_t control;       // Control I/O port
    uint16_t bm_ide;        // Bus Master IDE I/O port
    bool     master;        // Is this a master device?
    uint32_t capabilities;  // Device capabilities
    uint32_t command_sets;  // Supported command sets
    uint32_t size;         // Size in sectors
    HBA_PORT port;
} sata_device_t;

sata_device_t  device;

// Function prototypes
bool sata_initialize(sata_device_t *device, uint16_t base, uint16_t control, uint16_t bm_ide, bool master);
bool sata_identify(sata_device_t *device);
bool sata_read_sectors(sata_device_t *device, uint32_t lba, uint8_t sectors, void *buffer);
bool sata_write_sectors(sata_device_t *device, uint32_t lba, uint8_t sectors, const void *buffer);
void sata_wait_ready(sata_device_t *device);
void sata_soft_reset(sata_device_t *device);
int sata_init_device(sata_device_t *device);
int sata_format(sata_device_t *device, uint32_t start_lba, uint32_t count);
bool format_drive(sata_device_t* device);

#endif // SATA_H