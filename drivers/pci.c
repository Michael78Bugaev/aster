/******************************************************************************
 *
 *  File        : pci.c
 *  Description :
 *
 *
 *****************************************************************************/
#include <stdint.h>
#include <cpu/mem.h>
#include <io/iotools.h>
#include <stdio.h>
#include <config.h>
#include <drv/pci.h>

pci_device_t pci_list[PCI_MAX_DEVICES];     // TODo: Should be a Linked list.


uint16_t pci_config_get_word (pci_device_t *dev, int offset) {
  uint16_t *cs = (uint16_t *)&dev->config_space[offset];
  return *cs;
}
uint8_t pci_config_get_byte (pci_device_t *dev, int offset) {
  return (uint8_t)dev->config_space[offset];
}
uint32_t pci_config_get_dword (pci_device_t *dev, int offset) {
  uint32_t *cs = (uint32_t *)&dev->config_space[offset];
  return *cs;
}


/**
 *
 */
uint16_t pci_readword (uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset) {
  uint32_t address = 0x80000000; // Bit 31 set
  address |= (uint32_t)bus << 16;
  address |= (uint32_t)slot << 11;
  address |= (uint32_t)func << 8;
  address |= (uint32_t)offset & 0xfc;

  outportl(PCI_CONFIG_ADDRESS, address);
  uint32_t ret = inportl (PCI_CONFIG_DATA);

  ret = ret >> ( (offset & 2) * 8) & 0xffff;
  return ret;
}

/**
 *
 */
int pci_get_next_free_slot () {
  int i;

  for (i=0; i!=PCI_MAX_DEVICES; i++) {
    if (! pci_list[i].enabled) return i;
  }
  return -1;
}

/**
 * Returns next PCI-device. On first call, *dev is NULL. Feed next device into dev to iterate
 * the list. Returns NULL on end (or nothing found). subclass is optional and can be -1
 */
pci_device_t *pci_find_next_class (pci_device_t *dev, int class, int subclass) {
  // Temp device is beginning of list or the next device when one is given in *dev
  pci_device_t *tmp = (dev == NULL) ? &pci_list[0] : &pci_list[dev->index+1];

  while (1) { // Scary, should be linked lists!
//    kprintf ("FNC: %d\n", tmp->index);
    if (class == -1 || tmp->class == class) {
      // Subclass is optional
      if (subclass == -1 || tmp->subclass == subclass) {
        // Only return when this slot is enabled
        if (tmp->enabled) return tmp;
      }
    }

    // Found end of line
    if (tmp->index == PCI_MAX_DEVICES-1) return NULL;

    // Get next entry
    tmp = &pci_list[tmp->index+1];
  }

  // Should not come here
  return NULL;
}

/**
 * Returns next PCI-device. On first call, *dev is NULL. Feed next device into dev to iterate
 * the list. Returns NULL on end (or nothing found). subclass is optional and can be -1
 */
pci_device_t *pci_find_next_position (pci_device_t *dev, int bus, int slot, int func) {
  // Temp device is beginning of list or the next device when one is given in *dev
  pci_device_t *tmp = (dev == NULL) ? &pci_list[0] : &pci_list[dev->index+1];

  while (1) { // Scary, should be linked lists!
    if (bus == -1 || tmp->bus == bus) {
      if (slot == -1 || tmp->slot == slot) {
        if (func == -1 || tmp->func == func) {
          // Only return when this slot is enabled
          if (tmp->enabled) return tmp;
        }
      }
    }

    // Found end of line
    if (tmp->index == PCI_MAX_DEVICES-1) return NULL;

    // Get next entry
    tmp = &pci_list[tmp->index+1];
  }

  // Should not come here
  return NULL;
}

/**
 *
 */
void pci_init (void) {
  int bus,slot,func;

  // Clear list
  memset (&pci_list, 0, sizeof (pci_list));

  // Indices MUST be set correctly, even for non-enabled entries
  int i;
  for (i=0; i!=PCI_MAX_DEVICES; i++) pci_list[i].index = i;

  // Scan all buses, slots and functions
  for (bus=0; bus != PCI_MAX_BUS; bus++) {
    for (slot=0; slot != PCI_MAX_SLOT; slot++) {
      uint16_t vendor = pci_readword (bus, slot, 0, 0);
      if (vendor == 0xFFFF) continue;  // Nothing on this slot

      for (func=0; func != PCI_MAX_FUNC; func++) {

        // Read additional info
        uint16_t device = pci_readword (bus, slot, func, 2);
        if (device == 0xFFFF) continue;  // Nothing on this slot

        // Find free slot
        int idx = pci_get_next_free_slot ();
        if (idx == -1) printf("PCI: no more free slots!\n");

        // Fill config space
        int i;
        for (i=0; i!=128; i++) {
          uint16_t tmp = pci_readword (bus, slot, func, i*2);
          pci_list[idx].config_space[i*2+0] = LO8(tmp);
          pci_list[idx].config_space[i*2+1] = HI8(tmp);
        }

        pci_device_t *dev = &pci_list[idx];

        pci_list[idx].class = pci_config_get_byte (dev, 0x0B);
        pci_list[idx].subclass = pci_config_get_byte (dev, 0x0A);
        pci_list[idx].vendor_id = pci_config_get_word (dev, 0x00);
        pci_list[idx].device_id = pci_config_get_word (dev, 0x02);
        pci_list[idx].bus = bus;
        pci_list[idx].slot = slot;
        pci_list[idx].func = func;

        printf("Added slot %d with %4x:%4x [%2x:%2x]\n", idx, pci_list[idx].vendor_id, pci_list[idx].device_id, pci_list[idx].class, pci_list[idx].subclass);

        // Enable this slot
        pci_list[idx].enabled = 1;

        // Don't check next function if this device is single-function device
        uint8_t header = pci_config_get_byte (dev, 0x0e);
        if ( func == 0 && (header & 0x80) != 0x80) break;
      } // func
    } // slot
  } // bus

}