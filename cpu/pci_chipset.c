#include <cpu/pci_chipset.h>
#include <stdio.h>
#include <string.h>

static pci_driver_t pci_driver;

bool pci_driver_init(void) {
    printf("Initializing PCI driver...\n");
    memset(&pci_driver, 0, sizeof(pci_driver_t));
    
    // Проверяем, инициализирован ли чипсет
    chipset_info_t chipset_info;
    chipset_get_info(&chipset_info);
    if (chipset_info.vendor_id == 0 || chipset_info.device_id == 0) {
        printf("Error: Chipset not initialized. Please initialize chipset first.\n");
        return false;
    }

    pci_driver_scan_bus();
    printf("PCI driver initialized. Found %d devices.\n", pci_driver.device_count);
    return true;
}

void pci_driver_scan_bus(void) {
    for (uint16_t bus = 0; bus < 256 && pci_driver.device_count < PCI_MAX_DEVICES; bus++) {
        for (uint16_t device = 0; device < 32 && pci_driver.device_count < PCI_MAX_DEVICES; device++) {
            for (uint16_t function = 0; function < 8 && pci_driver.device_count < PCI_MAX_DEVICES; function++) {
                uint32_t vendor_device = pci_config_read(bus, device, function, 0);
                if ((vendor_device & 0xFFFF) != 0xFFFF) {
                    pci_get_device_info(bus, device, function, &pci_driver.devices[pci_driver.device_count]);
                    printf("Found PCI device: %x:%x (bus %d, device %d, function %d)\n",
                           pci_driver.devices[pci_driver.device_count].vendor_id,
                           pci_driver.devices[pci_driver.device_count].device_id,
                           bus, device, function);
                    pci_driver.device_count++;
                }
            }
        }
    }
}

pci_device_t* pci_driver_find_device(uint16_t vendor_id, uint16_t device_id) {
    for (uint32_t i = 0; i < pci_driver.device_count; i++) {
        if (pci_driver.devices[i].vendor_id == vendor_id && pci_driver.devices[i].device_id == device_id) {
            return &pci_driver.devices[i];
        }
    }
    return NULL;
}

bool pci_driver_enable_bus_mastering(pci_device_t* device) {
    if (!device) return false;

    uint32_t command = pci_config_read(device->bus, device->device, device->function, 0x04);
    command |= 0x04; // Enable Bus Mastering
    pci_config_write(device->bus, device->device, device->function, 0x04, command);
    
    printf("Enabled Bus Mastering for device %x:%x\n", device->vendor_id, device->device_id);
    return true;
}

uint32_t pci_driver_read_config(pci_device_t* device, uint8_t offset) {
    if (!device) return 0xFFFFFFFF;
    return pci_config_read(device->bus, device->device, device->function, offset);
}

void pci_driver_write_config(pci_device_t* device, uint8_t offset, uint32_t value) {
    if (!device) return;
    pci_config_write(device->bus, device->device, device->function, offset, value);
}