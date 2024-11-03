#include <cpu/pci.h>
#include <io/iotools.h>
#include <stdio.h>

uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    port_dword_out(PCI_CONFIG_ADDRESS, address);
    tmp = port_dword_in(PCI_CONFIG_DATA);
    return tmp;
}

void pci_config_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    port_dword_out(PCI_CONFIG_ADDRESS, address);
    port_dword_out(PCI_CONFIG_DATA, value);
}

void pci_get_device_info(uint8_t bus, uint8_t device, uint8_t function, pci_device_t *dev_info) {
    uint32_t reg = pci_config_read(bus, device, function, 0);
    dev_info->vendor_id = reg & 0xFFFF;
    dev_info->device_id = (reg >> 16) & 0xFFFF;

    reg = pci_config_read(bus, device, function, 8);
    dev_info->revision = reg & 0xFF;
    dev_info->prog_if = (reg >> 8) & 0xFF;
    dev_info->subclass = (reg >> 16) & 0xFF;
    dev_info->class_code = (reg >> 24) & 0xFF;

    dev_info->bus = bus;
    dev_info->device = device;
    dev_info->function = function;
}

void pci_scan_bus(void) {
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint16_t device = 0; device < 32; device++) {
            for (uint16_t function = 0; function < 8; function++) {
                uint32_t vendor_device = pci_config_read(bus, device, function, 0);
                if ((vendor_device & 0xFFFF) != 0xFFFF) {
                    pci_device_t dev_info;
                    pci_get_device_info(bus, device, function, &dev_info);
                    printf("Found PCI device at %x:%x\n", dev_info.vendor_id, dev_info.device_id);
                }
            }
        }
    }
}