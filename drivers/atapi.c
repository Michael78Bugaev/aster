#include <drv/ide.h>
#include <drv/ide_atapi.h>
#include <config.h>
#include <cpu/mem.h>
#include <drv/pci.h>


uint32_t ide_atapi_access(char direction, ide_drive_t *drive, uint32_t lba_sector, uint32_t sector_count, char *buf) {
  // @TODO: create atapi
  printf("IDE_ATAPI_ACCESS not supported\n");
  return 0;
}