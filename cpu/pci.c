// pci.c
#include <cpu/pci.h>
#include <io/iotools.h>
#include <stdio.h>

uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    port_dword_out(PCI_CONFIG_ADDRESS, address);
    return port_dword_in(PCI_CONFIG_DATA);
}

void pci_config_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    port_dword_out(PCI_CONFIG_ADDRESS, address);
    port_dword_out(PCI_CONFIG_DATA, value);
}

int pci_find_device(uint16_t vendor_id, uint16_t device_id, uint16_t *bus, uint16_t *device, uint16_t *function) {
    for (uint16_t b = 0; b < 256; b++) {
        for (uint16_t d = 0; d < 32; d++) {
            for (uint16_t f = 0; f < 8; f++) {
                uint32_t vendor_device = pci_config_read(b, d, f, 0);
                if ((vendor_device & 0xFFFF) == vendor_id && 
                    ((vendor_device >> 16) & 0xFFFF) == device_id) {
                    *bus = b;
                    *device = d;
                    *function = f;
                    return 1;
                }
            }
        }
    }
    return 0;
}

uint32_t pci_read_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar) {
    uint32_t bar_value = pci_config_read(bus, slot, func, PCI_BAR0 + (bar * 4));
    
    // Если это 64-битный BAR, нужно прочитать и верхнюю часть
    if ((bar_value & 0x6) == 0x4) {
        uint32_t bar_value_high = pci_config_read(bus, slot, func, PCI_BAR0 + (bar * 4) + 4);
        return (bar_value_high << 32) | (bar_value & ~0xF);
    }
    
    return bar_value & ~0xF;  // Маскируем нижние 4 бита (флаги)
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
                    
                    printf("PCI Device: Bus %d, Device %d, Function %d\ Vendor ID: 0x%X, Device ID: 0x%X\n",
                           dev_info.bus, dev_info.device, dev_info.function, 
                           dev_info.vendor_id, dev_info.device_id);
                }
            }
        }
    }
}

// Функция для чтения PCI конфигурации
uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (device << 11) |
                                 (func << 8) | (offset & 0xfc) | 0x80000000);
    port_dword_out(PCI_CONFIG_ADDRESS, address);
    return port_dword_in(PCI_CONFIG_DATA);
}

// Поиск AHCI контроллера
HBA_MEM* find_ahci_controller() {
    for(uint8_t bus = 0; bus < 256; bus++) {
        for(uint8_t device = 0; device < 32; device++) {
            uint32_t id = pci_read(bus, device, 0, 0);
            if(id != 0xFFFFFFFF) {  // Устройство существует
                uint32_t class = pci_read(bus, device, 0, 0x08);
                uint8_t prog_if = (class >> 8) & 0xFF;
                uint8_t subclass = (class >> 16) & 0xFF;
                uint8_t classcode = (class >> 24) & 0xFF;

                if(classcode == AHCI_CLASS && 
                   subclass == AHCI_SUBCLASS && 
                   prog_if == AHCI_PROG_IF) {
                    // Нашли AHCI контроллер
                    uint32_t bar5 = pci_read(bus, device, 0, 0x24); // ABAR
                    printf("AHCI controller found at bus %d device %d\n", bus, device);
                    printf("ABAR: 0x%x\n", bar5);
                    return (HBA_MEM*)(bar5 & ~0xF); // Очищаем нижние биты
                }
            }
        }
    }
    return NULL;
}