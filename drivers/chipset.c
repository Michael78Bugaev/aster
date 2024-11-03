#include <drv/chipset.h>
#include <io/iotools.h>
#include <stdio.h>
#include <cpu/pci.h>

static chipset_info_t current_chipset;
static bool is_initialized = false;

bool chipset_init(void) {
    printf("Initializing chipset driver...\n");

    // Сканируем PCI шину для поиска чипсета
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint16_t slot = 0; slot < 32; slot++) {
            for (uint16_t func = 0; func < 8; func++) {
                uint32_t vendor_device = pci_config_read(bus, slot, func, 0);
                uint16_t vendor = vendor_device & 0xFFFF;
                
                // Проверяем известные ID производителей чипсетов
                if (vendor == INTEL_VENDOR_ID || vendor == AMD_VENDOR_ID) {
                    pci_device_t dev_info;
                    pci_get_device_info(bus, slot, func, &dev_info);
                    
                    // Заполняем информацию о чипсете
                    current_chipset.vendor_id = dev_info.vendor_id;
                    current_chipset.device_id = dev_info.device_id;
                    current_chipset.revision = dev_info.revision;
                    current_chipset.prog_if = dev_info.prog_if;
                    current_chipset.subclass = dev_info.subclass;
                    current_chipset.class_code = dev_info.class_code;
                    
                    printf("Chipset found: Vendor ID: %x, Device ID: %x\n",
                           current_chipset.vendor_id,
                           current_chipset.device_id);
                    
                    is_initialized = true;
                    return true;
                }
            }
        }
    }
    
    printf("No supported chipset found!\n");
    return false;
}

void chipset_get_info(chipset_info_t* info) {
    if (!is_initialized || !info) {
        return;
    }
    
    *info = current_chipset;
}

uint32_t chipset_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    if (!is_initialized) {
        return 0xFFFFFFFF;
    }
    
    return pci_config_read(bus, slot, func, offset);
}

void chipset_write_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    if (!is_initialized) {
        return;
    }
    
    pci_config_write(bus, slot, func, offset, value);
}

bool chipset_enable_feature(uint32_t feature) {
    if (!is_initialized) {
        return false;
    }
    
    // Здесь можно добавить логику включения конкретных функций чипсета
    // Например:
    uint32_t config = chipset_read_config(0, 0, 0, 0x40); // Пример регистра
    config |= feature;
    chipset_write_config(0, 0, 0, 0x40, config);
    
    return true;
}

bool chipset_disable_feature(uint32_t feature) {
    if (!is_initialized) {
        return false;
    }
    
    // Здесь можно добавить логику выключения конкретных функций чипсета
    uint32_t config = chipset_read_config(0, 0, 0, 0x40);
    config &= ~feature;
    chipset_write_config(0, 0, 0, 0x40, config);
    
    return true;
}