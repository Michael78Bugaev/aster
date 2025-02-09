/******************************************************************************
 *
 *  File        : ide.c
 *  Description : IDE device
 *
 *  Most code here taken from http://wiki.osdev.org/IDE
 *
 *****************************************************************************/

#include <drv/ide.h>
#include <io/iotools.h>
#include <drv/ide_ata.h>
#include <drv/ide_atapi.h>
#include <drv/ide_partitions.h>
#include <stdint.h>
#include <stdio.h>
#include <drv/device.h>
#include <config.h>
#include <cpu/mem.h>
#include <drv/pci.h>

char ide_irq_invoked = 0;

// These are the standard IO ports for the controllers (only 1 controller currently supported)
// @TODO: more controllers could be supported by boot-param: IDE2=x,x,x,x,x,x,x,x
uint16_t ide_controller_ioports[][8] = {
                                      { 0x1F0, 0x3F0, 0x170, 0x370, 0x1E8, 0x3E0, 0x168, 0x360 }
                                     };

/**
 * Write to port
 */
void ide_port_write (ide_channel_t *channel, uint8_t reg, uint8_t data) {
  if (reg > 0x07 && reg < 0x0C) ide_port_write (channel, IDE_REG_CONTROL, 0x80 | channel->no_int);

  if (reg < 0x08) port_byte_out (channel->base + reg - 0x00, data);
  else if (reg < 0x0C) port_byte_out (channel->base + reg - 0x06, data);
  else if (reg < 0x0E) port_byte_out (channel->dev_ctl + reg - 0x0A, data);
  else if (reg < 0x16) port_byte_out (channel->bm_ide + reg - 0x0E, data);

  if (reg > 0x07 && reg < 0x0C) ide_port_write (channel, IDE_REG_CONTROL, channel->no_int);
}


/**
 * Read from port
 */
uint8_t ide_port_read (ide_channel_t *channel, uint8_t reg) {
  uint8_t result;

  if (reg > 0x07 && reg < 0x0C) ide_port_write (channel, IDE_REG_CONTROL, 0x80 | channel->no_int);

  if (reg < 0x08) result = port_byte_in (channel->base + reg - 0x00);
  else if (reg < 0x0C) result = port_byte_in (channel->base + reg - 0x06);
  else if (reg < 0x0E) result = port_byte_in (channel->dev_ctl + reg - 0x0A);
  else if (reg < 0x16) result = port_byte_in (channel->bm_ide + reg - 0x0E);

  if (reg > 0x07 && reg < 0x0C) ide_port_write(channel, IDE_REG_CONTROL, channel->no_int);
  return result;
}


/**
 *
 */
int ide_polling (ide_channel_t *channel, char advanced_check) {
  int i;

//  printf("ide_polling()\n");

  // 400 uSecond delay by reading the altstatus port 4 times
  for (i=0; i<4; i++) ide_port_read (channel, IDE_REG_ALTSTATUS);

  // Wait for BSY to be zero.
  while (ide_port_read (channel, IDE_REG_STATUS) & IDE_SR_BSY) ;

  if (advanced_check) {
    char state = ide_port_read (channel, IDE_REG_STATUS); // Read Status Register.

    if (state & IDE_SR_DF) return 1;  // Device Fault.
    if (state & IDE_SR_ERR) return 2; // Error.
    if ((state & IDE_SR_DRQ) == 0) return 3; // DRQ should be set
  }

  return 0;
}


/**
 *
 */
void ide_port_read_buffer (ide_channel_t *channel, uint8_t reg, uint32_t buffer, uint16_t quads) {
  if (reg > 0x07 && reg < 0x0C) ide_port_write(channel, IDE_REG_CONTROL, 0x80 | channel->no_int);

  if (reg < 0x08) insl (channel->base  + reg - 0x00, buffer, quads);
  else if (reg < 0x0C) insl (channel->base + reg - 0x06, buffer, quads);
  else if (reg < 0x0E) insl (channel->dev_ctl + reg - 0x0A, buffer, quads);
  else if (reg < 0x16) insl (channel->bm_ide + reg - 0x0E, buffer, quads);

  if (reg > 0x07 && reg < 0x0C) ide_port_write (channel, IDE_REG_CONTROL, channel->no_int);
}





/**
 * Read specified number of sectors from drive into buffer,
 * returns number of sectors read
 */
uint32_t ide_sector_read (ide_drive_t *drive, uint32_t lba_sector, uint32_t sector_count, char *buffer) {
  int ret;

//  printf ("\nide_sector_read (drive, %d, %d, %08X)\n", lba_sector, sector_count, buffer);

  // Not enabled drive
  if (! drive->enabled) return 0;

  // Incorrect sector
  if (lba_sector > drive->size) return 0;

  if (drive->type == IDE_DRIVE_TYPE_ATA) {
    ret = ide_ata_access (IDE_DIRECTION_READ, drive, lba_sector, sector_count, buffer);
  } else if (drive->type == IDE_DRIVE_TYPE_ATAPI) {
    ret = ide_atapi_access (IDE_DIRECTION_READ, drive, lba_sector, sector_count, buffer);
  } else {
    printf ("Unknown type (neither ATA nor ATAPI)\n");
  }

  return ret;
}


/**
 * Write specified number of sectors from buffer to drive
 */
void ide_write_sectors(ide_drive_t *drive, uint32_t lba_sector, uint32_t count, char *buffer) {
  int ret;

  // Not enabled drive
  if (! drive->enabled) return;

  // Incorrect sector
  if (lba_sector > drive->size) return;

  if (drive->type == IDE_DRIVE_TYPE_ATA) {
    ret = ide_ata_access (IDE_DIRECTION_WRITE, drive, lba_sector, count, buffer);
  } else if (drive->type == IDE_DRIVE_TYPE_ATAPI) {
    ret = ide_atapi_access (IDE_DIRECTION_WRITE, drive, lba_sector, count, buffer);
  } else {
    printf ("Unknown type (neither ATA nor ATAPI)\n");
  }

  return;
}


/**
 * Sleep for X milliseconds. Based on the fact that reading the ALT_STATUS register takes 100 uSeconds
 */
void ide_sleep (ide_channel_t *channel, int ms) {
  int i;
  for (i=0; i<ms*10; i++) ide_port_read (channel, IDE_REG_ALTSTATUS);
}


/**
 *
 */
void ide_init_drive (ide_drive_t *drive) {
  char type = IDE_DRIVE_TYPE_ATA;
  char err = 0;
  char status;
  int i;

//  printf ("    ide_init_drive() : %d \n", drive->channel->controller->controller_nr);

  // Default, drive is not enabled
  drive->enabled = 0;

  // Select drive
  ide_port_write (drive->channel, IDE_REG_HDDEVSEL, 0xA0 | (drive->drive_nr << 4));
  ide_sleep (drive->channel, 1);

  ide_port_write (drive->channel, IDE_REG_COMMAND, IDE_CMD_IDENTIFY);
  ide_sleep (drive->channel, 1);

  // Polling:
  if (ide_port_read (drive->channel, IDE_REG_STATUS) == 0) return; // No drive found

  while (1) {
    status = ide_port_read (drive->channel, IDE_REG_STATUS);
    if ((status & IDE_SR_ERR)) { err = 1; break; } // If Err, Device is not ATA.
    if (!(status & IDE_SR_BSY) && (status & IDE_SR_DRQ)) break; // Everything is right.
  }

  // Check if it's an ATAPI drive (if not ATA detected)
  if (err != 0) {
    uint8_t cl = ide_port_read (drive->channel, IDE_REG_LBA1);
    uint8_t ch = ide_port_read (drive->channel, IDE_REG_LBA2);

    if (cl == 0x14 && ch ==0xEB) type = IDE_DRIVE_TYPE_ATAPI;
    else if (cl == 0x69 && ch == 0x96) type = IDE_DRIVE_TYPE_ATAPI;
    else return; // Unknown Type (may not be a device).

    ide_port_write (drive->channel, IDE_REG_COMMAND, IDE_CMD_IDENTIFY_PACKET);
    ide_sleep (drive->channel, 1);
  }

  // Read device identification
  char *ide_info = (char *)malloc(512);
  ide_port_read_buffer(drive->channel, IDE_REG_DATA, (uint32_t)ide_info, 128);

  // Read device parameters
  drive->enabled      = 1;
  drive->type         = type;
  drive->signature    = *(uint16_t *)(ide_info + IDE_IDENT_DEVICETYPE);
  drive->capabilities = *(uint16_t *)(ide_info + IDE_IDENT_CAPABILITIES);
  drive->command_sets = *(uint32_t *)(ide_info + IDE_IDENT_COMMANDSETS);

  // Get drive size
  if (drive->command_sets & (1 << 26)) {
    // Device uses 48-Bit Addressing:
    drive->size  = *(uint32_t *)(ide_info + IDE_IDENT_MAX_LBA);
    drive->lba48 = 1;
  } else {
    // Device uses CHS or 28-bit Addressing
    drive->size  = *(uint32_t *)(ide_info + IDE_IDENT_MAX_LBA);
    drive->lba48 = 0;
  }

//  printf ("size %08X sectors\n", drive->size);
//  printf ("size %d MB\n", drive->size / 2 / 1024);

  // Get identification string
  for (i=0; i<40; i+=2) {
    drive->model[i] = ide_info[IDE_IDENT_MODEL + i + 1];
    drive->model[i+1] = ide_info[IDE_IDENT_MODEL + i];
  }
  drive->model[40] = 0; // terminate string

  mfree(ide_info);



  // Register device so we can access it
  device_t *device = (device_t *)malloc (sizeof (device_t));
  device->major_num = DEV_MAJOR_IDE;
  device->minor_num = (drive->channel->controller->controller_nr << 3) + (drive->channel->channel_nr << 1) + drive->drive_nr;
  device->data = (ide_drive_t *)drive;

  device->read = ide_block_read;
  device->write = ide_block_write;
  device->open = ide_block_open;
  device->close = ide_block_close;
  device->seek = ide_block_seek;

//   // Create device name
//   char filename[20];
//   memset(filename, 0, sizeof (filename));
//   printf(filename, "IDE%dC%dD%d", drive->channel->controller->controller_nr, drive->channel->channel_nr, drive->drive_nr);

//   // Register device
// //  printf ("\n*** Registering device DEVICE:/%s\n", filename);
//   //device_register(device, filename);

  // Initialise partitions from the MBR
  // @TODO: Read partitions from ATAPI drives
  //if (drive->type == IDE_DRIVE_TYPE_ATA) ide_read_partition_table (drive, 0);

//  printf ("    ide_init_drive() done \n");
}


/**
 *
 */
void ide_init_channel (ide_channel_t *channel) {
//  printf ("  ide_init_channel()\n");

  // Disable IRQ
  ide_port_write (channel, IDE_REG_CONTROL, 2);

  // Init both drives (if any)
  channel->drive[0].channel = channel;
  channel->drive[0].drive_nr = 0;
  ide_init_drive (&channel->drive[0]);

  channel->drive[1].channel = channel;
  channel->drive[1].drive_nr = 1;
  ide_init_drive (&channel->drive[1]);

//  printf ("  ide_init_channel() done\n");
}


/**
 *
 */
void ide_init_controller (ide_controller_t *ctrl, pci_device_t *pci_dev, uint16_t io_port[8]) {
//  printf ("ide_init_controller(%04X %04X)\n", io_port[0],io_port[1]);

  // Read PCI information for port settings
  uint32_t bar[5];
  bar[0] = pci_config_get_dword (pci_dev, 0x10) & 0xFFFFFFFC;
  bar[1] = pci_config_get_dword (pci_dev, 0x14) & 0xFFFFFFFC;
  bar[2] = pci_config_get_dword (pci_dev, 0x18) & 0xFFFFFFFC;
  bar[3] = pci_config_get_dword (pci_dev, 0x1C) & 0xFFFFFFFC;
  bar[4] = pci_config_get_dword (pci_dev, 0x20) & 0xFFFFFFFC;

  // Set standard info for channel 0 (master)
  int channel_nr;
  for (channel_nr=0; channel_nr != IDE_CONTROLLER_MAX_CHANNELS; channel_nr++) {
//    printf ("Checking controller channel %d\n", channel_nr);
    ctrl->channel[channel_nr].channel_nr = channel_nr;
    ctrl->channel[channel_nr].base = io_port[channel_nr*2+0];
    ctrl->channel[channel_nr].dev_ctl = io_port[channel_nr*2+1] + 4;
    ctrl->channel[channel_nr].bm_ide = bar[4] + 0;
    ctrl->channel[channel_nr].pci = pci_dev;
    ctrl->channel[channel_nr].controller = ctrl;   // Link back to controller from the channel
    ide_init_channel (&ctrl->channel[channel_nr]);
  }

  // Enable this controller
  ctrl->enabled = 1;

//  printf ("ide_init_controller() done\n");
}


/**
 *
 */
void ide_init (void) {
  int controller_num;

//  printf ("ide_init() start\n");

  // Clear ide_controllers info
  memset (ide_controllers, 0, sizeof(ide_controllers));

  // Detect mass controller PCI devices
  pci_device_t *pci_dev = NULL;
  controller_num = 0;
  while (pci_dev = pci_find_next_class (pci_dev, 0x01, 0x01), pci_dev != NULL) {
//    printf ("Found controller %04X:%04X [class: %02X:%02X] on [%02x:%02x:%02x]\n", pci_dev->vendor_id, pci_dev->device_id, pci_dev->class, pci_dev->subclass, pci_dev->bus, pci_dev->slot, pci_dev->func);

    // Set controller number (for easy searching)
    ide_controllers[controller_num].controller_nr = controller_num;

    // Initialise controller (and drives)
    ide_init_controller (&ide_controllers[controller_num], pci_dev, ide_controller_ioports[controller_num]);

    // Increase controller number
    controller_num++;
  }

//  printf ("ide_init() done\n");

  // print controller information
  int i,j,k;
  for (i=0; i!=MAX_IDE_CONTROLLERS; i++) {
    if (! ide_controllers[i].enabled) continue;

    printf("IDE controller %d\n", i);
    for (j=0; j!=IDE_CONTROLLER_MAX_CHANNELS; j++) {
      for (k=0; k!=IDE_CONTROLLER_MAX_DRIVES; k++) {
        if (! ide_controllers[i].channel[j].drive[k].enabled) continue;
        printf ("%02d/%02d: [%s] (%4dMB) '%s'\n",
                 j, k,
                 (ide_controllers[i].channel[j].drive[k].type==0?"ATA  ":"ATAPI"),
                 ide_controllers[i].channel[j].drive[k].size / 1024 / 2,
                 ide_controllers[i].channel[j].drive[k].model
                );
      }
    }
  }


}



/**
 * Can read larger blocks
 *
 * @param major
 * @param minor
 * @param offset
 * @param size
 * @param buffer
 * @return
 */
uint32_t ide_block_read (uint8_t major, uint8_t minor, uint32_t offset, uint32_t size, char *buffer) {
//  printf("ide_block_read(%08X (%04X))\n", offset, size);
  uint32_t lba_sector = offset / IDE_SECTOR_SIZE;
  uint32_t read_size = 0;

  if (major != DEV_MAJOR_IDE) return 0;

  // Fetch drive info from device
  device_t *device;
  ide_drive_t *drive = (ide_drive_t *)device->data;
  if (! drive || ! drive->enabled) {
    printf("Incorrect drive specified\n");
    return 0;
  }

  // Read pre misaligned sector data
  if (offset % IDE_SECTOR_SIZE > 0) {
    uint8_t restcount = offset % IDE_SECTOR_SIZE;
//    printf("ide preread(%d)\n", restcount);
    ide_sector_read(drive, lba_sector, 1, (char *)drive->databuf);
    memcpy(buffer, &drive->databuf[restcount], 512-restcount);

    read_size += restcount;
    lba_sector++;
    size -= restcount;
    buffer += restcount;
  }

  // Read as many full sectors as possible
  while (size >= IDE_SECTOR_SIZE) {
    // Read full sectors
    ide_sector_read (drive, lba_sector, 1, buffer);

//    printf ("Size: %d\n", size);

    read_size += IDE_SECTOR_SIZE;
    lba_sector++;
    size -= IDE_SECTOR_SIZE;
    buffer += IDE_SECTOR_SIZE;
  }

  // Read post misaligned sector data
  if (size > 0) {
//    printf ("ide postread(%d)", size);
    ide_sector_read(drive, lba_sector, 1, (char *)drive->databuf);
    memcpy(buffer, &drive->databuf[0], size);

    read_size += size;
  }

  return read_size;
}

uint32_t ide_block_write (uint8_t major, uint8_t minor, uint32_t offset, uint32_t size, char *buffer) {
  if (major != DEV_MAJOR_IDE) return 0;
  printf("ide_block_write(%d, %d, %d, %d, %08X)\n", major, minor, offset, size, buffer);
  printf("write to IDE not supported yet\n");
  return 0;
}
void ide_block_open(uint8_t major, uint8_t minor) {
  if (major != DEV_MAJOR_IDE) return;
  printf ("ide_block_open(%d, %d)\n", major, minor);
  // Doesn't do anything. Device already open?
}
void ide_block_close(uint8_t major, uint8_t minor) {
  if (major != DEV_MAJOR_IDE) return;
  printf ("ide_block_close(%d, %d)\n", major, minor);
  // Doesn't do anything. Device never closes?
}
void ide_block_seek(uint8_t major, uint8_t minor, uint32_t offset, uint8_t direction) {
  if (major != DEV_MAJOR_IDE) return;
  printf ("ide_block_seek(%d, %d, %d, %d)\n", major, minor, offset, direction);
  // Doesn't do anything.
}