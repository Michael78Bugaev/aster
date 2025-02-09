/******************************************************************************
 *
 *  File        : device.h
 *  Description : Device driver handling
 *
 *****************************************************************************/
#ifndef __DEVICE_H__
#define __DEVICE_H__

    #include <stdint.h>

    /* All major node numbers for devices. By accessing the file, the kernel
     * knows which driver is responsible for handling */
    #define DEV_MAJOR_MISC          0   // Misc devices (null, zero, rand etc)
    #define DEV_MAJOR_FDC           1   // Floppy disks
    #define DEV_MAJOR_IDE           2   // Ide controllers (ATA or ATAPI)
    #define DEV_MAJOR_HDC           3   // Hard disks (partitioned block device)
    #define DEV_MAJOR_CONSOLES     10   // Consoles (3,0 = kconsole)    (@TODO: not used)

    typedef struct {
      uint8_t  major_num;            // Major device node
      uint8_t  minor_num;            // Minor device node
      void   *data;                // Some data that might accompany the device
      struct device_t *next;       // Pointer to next device

      // Block device functions
      uint32_t(*read)(uint8_t major, uint8_t minor, uint32_t offset, uint32_t size, char *buffer);
      uint32_t(*write)(uint8_t major, uint8_t minor, uint32_t offset, uint32_t size, char *buffer);
      void (*open)(uint8_t major, uint8_t minor);
      void (*close)(uint8_t major, uint8_t minor);
      void (*seek)(uint8_t major, uint8_t minor, uint32_t offset, uint8_t direction);
    } device_t;


  int device_register (device_t *dev, const char *filename);
  int device_unregister (device_t *dev);
  void device_init (void);
  device_t *device_get_device (int major_num, int minor_num);

#endif // __DEVICE_H__