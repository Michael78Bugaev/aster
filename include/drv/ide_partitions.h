/******************************************************************************
 *
 *  File        : ide_paritions.h
 *  Description : ide partioning function defines.
 *
 *****************************************************************************/

#ifndef __DRIVERS_IDE_PARTITIONS_H__
#define __DRIVERS_IDE_PARTITIONS_H__

  #include <drv/ide.h>

 #pragma pack(1)
  struct ide_mbr_part {
    uint8_t   boot;
    uint8_t   first_head;
    uint8_t   first_sector;
    uint8_t   first_cylinder;
    uint8_t   system_id;
    uint8_t   last_head;
    uint8_t   last_sector;
    uint8_t   last_cylinder;
    uint32_t  first_lba_sector;
    uint32_t  size;
  };

  #pragma pack(1)
  struct ide_mbr {
    uint8_t code[446];
    struct ide_mbr_part partition[4];
    uint16_t signature;
  };
  
  typedef struct {
      ide_drive_t  *drive;        // IDE drive structure for this partition
      uint32_t       lba_start;     // LBA start
      uint32_t       lba_end;       // LBA end
      uint32_t       lba_size;      // start - end
      uint8_t        bootable;      // 0 = not bootable
      uint8_t        system_id;     // 83 = linux
  } ide_partition_t;

  void ide_read_partition_table (ide_drive_t *drive, uint32_t lba_sector);
  
  uint32_t ide_partition_block_read (uint8_t major, uint8_t minor, uint32_t offset, uint32_t size, char *buffer);
  uint32_t ide_partition_block_write (uint8_t major, uint8_t minor, uint32_t offset, uint32_t size, char *buffer);
  void ide_partition_block_open(uint8_t major, uint8_t minor);
  void ide_partition_block_close(uint8_t major, uint8_t minor);
  void ide_partition_block_seek(uint8_t major, uint8_t minor, uint32_t offset, uint8_t direction);

#endif //__DRIVERS_IDE_PARTITIONS_H__