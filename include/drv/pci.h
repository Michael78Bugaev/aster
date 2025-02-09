/******************************************************************************
 *
 *  File        : pci.c
 *  Description :
 *
 *****************************************************************************/
#ifndef __PCI_H__
#define __PCI_H__

  #include <stdint.h>

  #define PCI_MAX_DEVICES     255

  typedef struct {
    uint16_t  enabled;
    uint16_t  index;      // Needed because we want to lookup index when only the device is known occasionally
    uint16_t  class;
    uint16_t  subclass;
    uint16_t  vendor_id;
    uint16_t  device_id;
    uint16_t  bus;        // Where on the PCI is this device
    uint16_t  slot;
    uint16_t  func;
    char    config_space[256];
  } pci_device_t;


  #define PCI_CONFIG_ADDRESS    0xCF8
  #define PCI_CONFIG_DATA       0xCFC

  #define PCI_MAX_BUS           256
  #define PCI_MAX_SLOT          32
  #define PCI_MAX_FUNC          8

  void pci_init (void);
  pci_device_t *pci_find_next_class (pci_device_t *idx, int class, int subclass);
  pci_device_t *pci_find_next_position (pci_device_t *dev, int bus, int slot, int func);

  uint8_t pci_config_get_byte (pci_device_t *dev, int offset);
  uint16_t pci_config_get_word (pci_device_t *dev, int offset);
  uint32_t pci_config_get_dword (pci_device_t *dev, int offset);

#endif //__PCI_H__