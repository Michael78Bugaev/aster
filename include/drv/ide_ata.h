#ifndef __DRIVERS_IDE_ATA_H__
#define __DRIVERS_IDE_ATA_H__

#include <drv/ide.h>

uint32_t ide_ata_access(char direction, ide_drive_t *drive, uint32_t lba_sector, uint32_t sector_count, char *buf);

#endif //__DRIVERS_IDE_ATA_H__