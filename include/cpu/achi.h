#ifndef AHCI_H
#define AHCI_H

#include <stdint.h>
#include <storage.h>

#define SATA_SIG_ATA    0x00000101  // SATA drive
#define SATA_SIG_ATAPI  0xEB140101  // SATAPI drive
#define SATA_SIG_SEMB   0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM     0x96690101  // Port multiplier

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

// Port x Interrupt Status Register Bits
#define HBA_PxIS_TFES  (1 << 30)   // Task File Error Status
#define HBA_PxIS_HBFS  (1 << 29)   // Host Bus Fatal Error Status
#define HBA_PxIS_HBDS  (1 << 28)   // Host Bus Data Error Status
#define HBA_PxIS_IFS   (1 << 27)   // Interface Fatal Error Status
#define HBA_PxIS_PRCS  (1 << 22)   // PhyRdy Change Status
#define HBA_PxIS_IPMS  (1 << 21)   // Incorrect Port Multiplier Status
#define HBA_PxIS_OFS   (1 << 24)   // Overflow Status
#define HBA_PxIS_INFS  (1 << 26)   // Interface Non-fatal Error Status
#define HBA_PxIS_UFS   (1 << 20)   // Unknown FIS Status
#define HBA_PxIS_SDBS  (1 << 3)    // Set Device Bits FIS Status
#define HBA_PxIS_DSS   (1 << 2)    // DMA Setup FIS Status
#define HBA_PxIS_PSS   (1 << 1)    // PIO Setup FIS Status
#define HBA_PxIS_DHRS  (1 << 0)    // Device to Host Register FIS Status

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

#define ATA_CMD_READ_DMA_EX 0x25

typedef enum
{
    FIS_TYPE_REG_H2D    = 0x27,
    FIS_TYPE_REG_D2H    = 0x34,
    FIS_TYPE_DMA_ACT    = 0x39,
    FIS_TYPE_DMA_SETUP  = 0x41,
    FIS_TYPE_DATA       = 0x46,
    FIS_TYPE_BIST       = 0x58,
    FIS_TYPE_PIO_SETUP  = 0x5F,
    FIS_TYPE_DEV_BITS   = 0xA1,
} FIS_TYPE;

typedef struct FIS_REG_H2D
{
    uint8_t  fis_type;
    uint8_t  pmport:4;
    uint8_t  rsv0:3;
    uint8_t  c:1;
    uint8_t  command;
    uint8_t  featurel;
    uint8_t  lba0;
    uint8_t  lba1;
    uint8_t  lba2;
    uint8_t  device;
    uint8_t  lba3;
    uint8_t  lba4;
    uint8_t  lba5;
    uint8_t  featureh;
    uint8_t  countl;
    uint8_t  counth;
    uint8_t  icc;
    uint8_t  control;
    uint8_t  rsv1[4];
} FIS_REG_H2D;

typedef volatile struct HBA_PORT
{
    uint32_t clb;
    uint32_t clbu;
    uint32_t fb;
    uint32_t fbu;
    uint32_t is;
    uint32_t ie;
    uint32_t cmd;
    uint32_t rsv0;
    uint32_t tfd;
    uint32_t sig;
    uint32_t ssts;
    uint32_t sctl;
    uint32_t serr;
    uint32_t sact;
    uint32_t ci;
    uint32_t sntf;
    uint32_t fbs;
    uint32_t rsv1[11];
    uint32_t vendor[4];
} HBA_PORT;

typedef volatile struct HBA_MEM
{
    uint32_t cap;
    uint32_t ghc;
    uint32_t is;
    uint32_t pi;
    uint32_t vs;
    uint32_t ccc_ctl;
    uint32_t ccc_pts;
    uint32_t em_loc;
    uint32_t em_ctl;
    uint32_t cap2;
    uint32_t bohc;
    uint8_t  rsv[0xA0-0x2C];
    uint8_t  vendor[0x100-0xA0];
    HBA_PORT ports[32];
} HBA_MEM;

typedef struct HBA_CMD_HEADER
{
    uint8_t  cfl:5;
    uint8_t  a:1;
    uint8_t  w:1;
    uint8_t  p:1;
    uint8_t  r:1;
    uint8_t  b:1;
    uint8_t  c:1;
    uint8_t  rsv0:1;
    uint8_t  pmp:4;
    uint16_t prdtl;
    volatile uint32_t prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t rsv1[4];
} HBA_CMD_HEADER;

typedef struct HBA_PRDT_ENTRY
{
    uint32_t dba;
    uint32_t dbau;
    uint32_t rsv0;
    uint32_t dbc:22;
    uint32_t rsv1:9;
    uint32_t i:1;
} HBA_PRDT_ENTRY;

typedef struct HBA_CMD_TBL
{
    uint8_t  cfis[64];
    uint8_t  acmd[16];
    uint8_t  rsv[48];
    HBA_PRDT_ENTRY prdt_entry[1];
} HBA_CMD_TBL;


// Port Command Bits
#define HBA_PxCMD_ST    0x0001  // Start
#define HBA_PxCMD_SUD   0x0002  // Spin-Up Device
#define HBA_PxCMD_POD   0x0004  // Power On Device
#define HBA_PxCMD_CLO   0x0008  // Command List Override
#define HBA_PxCMD_FRE   0x0010  // FIS Receive Enable
#define HBA_PxCMD_FR    0x4000  // FIS Receive Running
#define HBA_PxCMD_CR    0x8000  // Command List Running
#define HBA_PxCMD_ICC   0xF0000 // Interface Communication Control
#define HBA_PxCMD_ACTIVE 0x1    // Device Detection

// FIS Types
#define FIS_TYPE_REG_H2D    0x27
#define FIS_TYPE_REG_D2H    0x34
#define FIS_TYPE_DMA_ACT    0x39
#define FIS_TYPE_DMA_SETUP  0x41
#define FIS_TYPE_DATA       0x46
#define FIS_TYPE_BIST       0x58
#define FIS_TYPE_PIO_SETUP  0x5F
#define FIS_TYPE_DEV_BITS   0xA1


void init_ahci(HBA_MEM *abar);
int check_type(HBA_PORT *port);
void probe_port(HBA_MEM *abar);
void port_rebase(HBA_PORT *port, int portno);
void start_cmd(HBA_PORT *port);
void stop_cmd(HBA_PORT *port);
int find_cmdslot(HBA_PORT *port);
bool read(HBA_PORT *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf);
int sata_flush(storage_device_t *device);

#endif