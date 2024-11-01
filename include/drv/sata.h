// sata.h
#ifndef SATA_H
#define SATA_H

#include <stdint.h>

#define MAX_SATA_PORTS 32

#define SATA_PRIMARY_DATA         0x1F0
#define SATA_PRIMARY_ERR          0x1F1
#define SATA_PRIMARY_SECCOUNT     0x1F2
#define SATA_PRIMARY_LBA_LO       0x1F3
#define SATA_PRIMARY_LBA_MID      0x1F4
#define SATA_PRIMARY_LBA_HI       0x1F5
#define SATA_PRIMARY_DRIVE_HEAD   0x1F6
#define SATA_PRIMARY_COMM_REGSTAT 0x1F7

// Команды SATA
#define SATA_CMD_READ_SECTORS  0x24    // READ SECTORS EXT
#define SATA_CMD_WRITE_SECTORS 0x34    // WRITE SECTORS EXT
#define SATA_CMD_FLUSH        0xEA    // FLUSH CACHE EXT

// Определения битов для port->cmd
#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

// SATA controller registers
typedef struct {
    volatile uint32_t clb;       // Command List Base Address
    volatile uint32_t clbu;      // Command List Base Address Upper 32-Bits
    volatile uint32_t fb;        // FIS Base Address
    volatile uint32_t fbu;       // FIS Base Address Upper 32-Bits
    volatile uint32_t is;        // Interrupt Status
    volatile uint32_t ie;        // Interrupt Enable
    volatile uint32_t cmd;       // Command and Status
    volatile uint32_t reserved0; // Reserved
    volatile uint32_t tfd;       // Task File Data
    volatile uint32_t sig;       // Signature
    volatile uint32_t ssts;      // Serial ATA Status (SCR0:SStatus)
    volatile uint32_t sctl;      // Serial ATA Control (SCR2:SControl)
    volatile uint32_t serr;      // Serial ATA Error (SCR1:SError)
    volatile uint32_t sact;      // Serial ATA Active (SCR3:SActive)
    volatile uint32_t ci;        // Command Issue
    volatile uint32_t sntf;      // Serial ATA Notification (SCR4:SNotification)
    volatile uint32_t fbs;       // FIS-based Switching Control
} SATA_PORT;

typedef struct {
    uint32_t cap;
    uint32_t ghc;
    uint32_t is;
    uint32_t pi;
    uint32_t vs;
    uint32_t ccc_ctl;
    uint32_t ccc_ports;
    uint32_t em_loc;
    uint32_t em_ctl;
    uint32_t cap2;
    uint32_t bohc;
    uint8_t  reserved[0xA0-0x2C];
    SATA_PORT ports[MAX_SATA_PORTS];
} SATA_HBA;

typedef volatile struct {
    uint32_t clb;        // Command List Base Address
    uint32_t clbu;       // Command List Base Address Upper 32-Bits
    uint32_t fb;         // FIS Base Address
    uint32_t fbu;        // FIS Base Address Upper 32-Bits
    uint32_t is;         // Interrupt Status
    uint32_t ie;         // Interrupt Enable
    uint32_t cmd;        // Command and Status
    uint32_t rsv0;       // Reserved
    uint32_t tfd;        // Task File Data
    uint32_t sig;        // Signature
    uint32_t ssts;       // SATA Status
    uint32_t sctl;       // SATA Control
    uint32_t serr;       // SATA Error
    uint32_t sact;       // SATA Active
    uint32_t ci;         // Command Issue
    uint32_t sntf;       // SATA Notification
    uint32_t fbs;        // FIS-based Switching Control
} HBA_PORT;

typedef volatile struct {
    uint32_t cap;        // Host Capabilities
    uint32_t ghc;        // Global Host Control
    uint32_t is;         // Interrupt Status
    uint32_t pi;         // Ports Implemented
    uint32_t vs;         // Version
    uint32_t ccc_ctl;    // Command Completion Coalescing Control
    uint32_t ccc_pts;    // Command Completion Coalescing Ports
    uint32_t em_loc;     // Enclosure Management Location
    uint32_t em_ctl;     // Enclosure Management Control
    uint32_t cap2;       // Host Capabilities Extended
    uint32_t bohc;       // BIOS/OS Handoff Control and Status
    uint8_t  rsv[0xA0-0x2C]; // Reserved
    uint8_t  vendor[0x100-0xA0]; // Vendor Specific registers
    HBA_PORT ports[32];  // Port Control Registers
} HBA_MEM;

// Function prototypes
void sata_init(void);
int sata_read_sector(uint8_t port, uint64_t sector, uint32_t count, void* buffer);
int sata_write_sector(uint8_t port, uint64_t sector, uint32_t count, const void* buffer);
uint32_t sata_get_port_count(void);

int sata_read_sectors(uint64_t lba, uint32_t count, void* buffer);
int sata_write_sectors(uint64_t lba, uint32_t count, const void* buffer);
int sata_flush(void);

#endif // SATA_H