#include <cpu/achi.h>
#include <cpu/pci.h>
#include <stdio.h>
#include <string.h>
#include <io/iotools.h>

#define AHCI_BASE 0x400000 // 4M

void init_ahci(HBA_MEM *abar)
{
    // Enable AHCI mode
    abar->ghc |= (1 << 31);
    
    // Enable interrupts
    abar->ghc |= (1 << 1);
    
    printf("AHCI initialized\n");
    probe_port(abar);
}

void probe_port(HBA_MEM *abar)
{
    uint32_t pi = abar->pi;
    int i = 0;
    while (i < 32)
    {
        if (pi & 1)
        {
            int dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                printf("SATA drive found at port %d\n", i);
                port_rebase(&abar->ports[i], i);
            }
            else if (dt == AHCI_DEV_SATAPI)
            {
                printf("SATAPI drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_SEMB) {
                printf("SEMB drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_PM) 
            {
                printf("PM drive found at port %d\n", i);
            }
            else
            {
                printf("No drive found at port %d\n", i);
            }
        }

        pi >>= 1;
        i++;
    }
}

int check_type(HBA_PORT *port)
{
    uint32_t ssts = port->ssts;

    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT) // Check drive status
        return AHCI_DEV_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE)
        return AHCI_DEV_NULL;

    switch (port->sig)
    {
    case SATA_SIG_ATAPI:
        return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
        return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
        return AHCI_DEV_PM;
    default:
        return AHCI_DEV_SATA;
    }
}

void port_rebase(HBA_PORT *port, int portno)
{
    stop_cmd(port); // Stop command engine

    // Command list offset: 1K*portno
    // Command list entry size = 32
    // Command list entry maxim count = 32
    // Command list maxim size = 32*32 = 1K per port
    port->clb = AHCI_BASE + (portno << 10);
    port->clbu = 0;
    memset((void *)(port->clb), 0, 1024);

    // FIS offset: 32K+256*portno
    // FIS entry size = 256 bytes per port
    port->fb = AHCI_BASE + (32 << 10) + (portno << 8);
    port->fbu = 0;
    memset((void *)(port->fb), 0, 256);

    // Command table offset: 40K + 8K*portno
    // Command table size = 256*32 = 8K per port
    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER *)(port->clb);
    for (int i = 0; i < 32; i++)
    {
        cmdheader[i].prdtl = 8; // 8 prdt entries per command table
        // Command table offset: 40K + 8K*portno + cmdheader_index*256
        cmdheader[i].ctba = AHCI_BASE + (40 << 10) + (portno << 13) + (i << 8);
        cmdheader[i].ctbau = 0;
        memset((void *)cmdheader[i].ctba, 0, 256);
    }

    start_cmd(port); // Start command engine
}

void start_cmd(HBA_PORT *port)
{
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR)
        ;

    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

void stop_cmd(HBA_PORT *port)
{
    // Clear ST (bit0)
    port->cmd &= ~HBA_PxCMD_ST;

    // Clear FRE (bit4)
    port->cmd &= ~HBA_PxCMD_FRE;

    // Wait until FR (bit14), CR (bit15) are cleared
    while (1)
    {
        if (port->cmd & HBA_PxCMD_FR)
            continue;
        if (port->cmd & HBA_PxCMD_CR)
            continue;
        break;
    }
}

int find_cmdslot(HBA_PORT *port)
{
    // If not set in SACT and CI, the slot is free
    uint32_t slots = (port->sact | port->ci);
    for (int i = 0; i < 32; i++)
    {
        if ((slots & 1) == 0)
            return i;
        slots >>= 1;
    }
    printf("Cannot find free command list entry\n");
    return -1;
}

bool read(HBA_PORT *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf)
{
    int i;
    port->is = (uint32_t)-1; // Clear pending interrupt bits
    int spin = 0; // Spin lock timeout counter
    int slot = find_cmdslot(port);
    if (slot == -1)
        return false;

    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER *)(port->clb);
    cmdheader += slot;
    cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t); // Command FIS size
    cmdheader->w = 0; // Read from device
    cmdheader->prdtl = (uint16_t)((count - 1) >> 4) + 1; // PRDT entries count

    HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL *)(cmdheader->ctba);
    memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) +
            (cmdheader->prdtl - 1) * sizeof(HBA_PRDT_ENTRY));

    // 8K bytes (16 sectors) per PRDT
    for (i = 0; i < cmdheader->prdtl - 1; i++)
    {
        cmdtbl->prdt_entry[i].dba = (uint32_t)buf;
        cmdtbl->prdt_entry[i].dbc = 8 * 1024 - 1; // 8K bytes (this value should always be set to 1 less than the actual value)
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4 * 1024; // 4K words
        count -= 16; // 16 sectors
    }
    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint32_t)buf;
    cmdtbl->prdt_entry[i].dbc = (count << 9) - 1; // 512 bytes per sector
    cmdtbl->prdt_entry[i].i = 1;

    // Setup command
    FIS_REG_H2D *cmdfis = (FIS_REG_H2D *)(&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1; // Command
    cmdfis->command = ATA_CMD_READ_DMA_EX;

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl >> 8);
    cmdfis->lba2 = (uint8_t)(startl >> 16);
    cmdfis->device = 1 << 6; // LBA mode

    cmdfis->lba3 = (uint8_t)(startl >> 24);
    cmdfis->lba4 = (uint8_t)starth;
    cmdfis->lba5 = (uint8_t)(starth >> 8);

    cmdfis->countl = count & 0xFF;
    cmdfis->counth = (count >> 8) & 0xFF;

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        printf("Port is hung\n");
        return false;
    }

    port->ci = 1 << slot; // Issue command

    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit 
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1 << slot)) == 0)
            break;
        if (port->is & HBA_PxIS_TFES) // Task file error
        {
            printf("Read disk error\n");
            return false;
        }
    }

    // Check again
    if (port->is & HBA_PxIS_TFES)
    {
        printf("Read disk error\n");
        return false;
    }

    return true;
}

int sata_flush(storage_device_t *device) {
    // Получаем указатель на HBA порт
    int spin;
    HBA_PORT *port = (HBA_PORT*)device->port;
    
    // Ждем пока порт не будет готов
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
        spin++;
    }
    if (spin == 1000000) {
        return -1; // Timeout
    }

    // Создаем командный заголовок
    HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(uint64_t)(port->clb);
    cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t); // Длина команды
    cmdheader->w = 0;  // Это команда чтения
    cmdheader->prdtl = 0; // Нет передачи данных

    // Создаем FIS структуру
    FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)((uint64_t)(port->fb));
    memset(cmdfis, 0, sizeof(FIS_REG_H2D));

    // Настраиваем команду
    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->command = 0xE7;
    cmdfis->device = 0;
    cmdfis->c = 1; // Command

    // Отправляем команду
    port->ci = 1;

    // Ждем завершения
    while (1) {
        if ((port->ci & 1) == 0) break;
        if (port->is & HBA_PxIS_TFES) {
            return -1; // Task File Error
        }
    }

    // Проверяем ошибки
    if (port->is & HBA_PxIS_TFES) {
        return -1;
    }

    return 0; // Успешное выполнение
}