#ifndef PCI_CHIPSET_H
#define PCI_CHIPSET_H

#include <stdint.h>
#include <stdbool.h>
#include <cpu/pci.h>
#include <drv/chipset.h>

#define PCI_MAX_DEVICES 32

typedef struct {
    pci_device_t devices[PCI_MAX_DEVICES];
    uint32_t device_count;
} pci_driver_t;

bool pci_driver_init(void);
void pci_driver_scan_bus(void);
pci_device_t* pci_driver_find_device(uint16_t vendor_id, uint16_t device_id);
bool pci_driver_enable_bus_mastering(pci_device_t* device);
uint32_t pci_driver_read_config(pci_device_t* device, uint8_t offset);
void pci_driver_write_config(pci_device_t* device, uint8_t offset, uint32_t value);

#endif // PCI_DRIVER_H