#include <drv/disk.h>
#include <config.h>
#include <cpu/mem.h>
#include <fs/file.h>
#include <io/iotools.h>

struct disk disk;

int disk_read_sector(int lba, int total, void* buffer)
{
  port_byte_out(0x1F6, (lba >> 24) | 0xE0);
  port_byte_out(0x1F2, total);
  port_byte_out(0x1F3, (unsigned char)(lba & 0xFF));
  port_byte_out(0x1F4, (unsigned char)(lba >> 8));
  port_byte_out(0x1F5, (unsigned char)(lba >> 16));
  port_byte_out(0x1F7, 0x20);

  unsigned short* ptr = (unsigned short*) buffer;

  for (int i = 0; i < total; i++)
  {
    // wait for the buffer to be ready
    char c = _insb(0x1F7);
    
    while(!(c & 0x08))
    {
      c = _insb(0x1F7);
    }

    // copy from hard disk to memory 
    for (int j = 0; j < 256; j++)
    {
      _insw(0x1F0);
      ptr++;
    }
  }

  return 0;
}

void disk_search_and_init()
{
  memset(&disk, 0, sizeof(disk));
  disk.type = DISK_TYPE_REAL;
  disk.sector_size = FAT16_SECTOR_SIZE;
  disk.filesystem = fs_resolve(&disk);
  disk.id = 0;
}

struct disk* disk_get(int index)
{
  if (index != 0)
  {
    return 0;
  }

  return &disk;
}

int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buffer)
{
  if (idisk != &disk)
  {
    return -INVALID_ARGUMENT_ERROR;
  }

  return disk_read_sector(lba, total, buffer);
}