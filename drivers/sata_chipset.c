#include <drv/sata_chipset.h>
#include <cpu/achi.h>
#include <stdio.h>
#include <io/iotools.h>
#include <drv/sata.h>
#include <cpu/pci_chipset.h>

sata_device_t device;
pci_device_t sata_controller;
bool controller_initialized = false;

pci_device_t* sata_chipset_find_controller(void) {
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint16_t dev = 0; dev < 32; dev++) {
            for (uint16_t func = 0; func < 8; func++) {
                uint32_t vendor_device = pci_config_read(bus, dev, func, 0);
                if ((vendor_device & 0xFFFF) != 0xFFFF) {
                    uint32_t class_subclass = pci_config_read(bus, dev, func, 8) >> 16;
                    uint8_t class_code = class_subclass >> 8;
                    uint8_t subclass = class_subclass & 0xFF;

                    if (class_code == SATA_PCI_CLASS && subclass == SATA_PCI_SUBCLASS) {
                        // Заполняем структуру контроллера
                        sata_controller.vendor_id = vendor_device & 0xFFFF;
                        sata_controller.device_id = vendor_device >> 16;
                        sata_controller.bus = bus;
                        sata_controller.device = dev;
                        sata_controller.function = func;
                        sata_controller.class_code = class_code;
                        sata_controller.subclass = subclass;
                        
                        controller_initialized = true;
                        return &sata_controller;
                    }
                }
            }
        }
    }
    return NULL;
}

bool sata_chipset_init(void) {
    printf("Initializing SATA chipset...\n");

    pci_device_t* controller = sata_chipset_find_controller();
    if (!controller) {
        printf("SATA controller not found in PCI devices.\n");
        return false;
    }

    printf("SATA controller found: Vendor ID: 0x%x, Device ID: 0x%x\n",
           controller->vendor_id, controller->device_id);

    // Включаем Bus Mastering для SATA контроллера
    uint32_t command = pci_config_read(controller->bus, 
                                     controller->device, 
                                     controller->function, 0x04);
    command |= 0x04; // Enable Bus Mastering
    pci_config_write(controller->bus, 
                    controller->device, 
                    controller->function, 0x04, command);

    // Получаем базовый адрес регистров SATA контроллера
    uint32_t bar5 = pci_config_read(controller->bus, 
                                   controller->device, 
                                   controller->function, 0x24); // BAR5
    if (bar5 == 0) {
        printf("Error: BAR5 is zero\n");
        return false;
    }

    uint32_t abar = bar5 & 0xFFFFFFF0; // Очищаем нижние 4 бита
    if (abar == 0) {
        printf("Error: ABAR is zero\n");
        return false;
    }

    printf("ABAR address: %x\n", abar);

    // Проверяем доступность адреса ABAR
    if (!abar || abar > 0xFFFFFFFF) {
        printf("Invalid ABAR address\n");
        return false;
    }

    // Инициализируем AHCI с проверкой указателя
    HBA_MEM* abar_struct = (HBA_MEM*)(uintptr_t)abar;
    if (!abar_struct) {
        printf("Failed to map ABAR address\n");
        return false;
    }

    // Безопасная инициализация AHCI
    init_ahci(abar_struct);

    // Проверяем доступность первого порта
    if (abar_struct->ports[0].clb == 0 || abar_struct->ports[0].fb == 0) {
        printf("Invalid port configuration\n");
        return false;
    }

    // Инициализация SATA устройства с проверками
    if (!sata_initialize(&device, 
                        abar_struct->ports[0].clb,
                        abar_struct->ports[0].fb, 
                        0, true)) {
        printf("Failed to initialize SATA device.\n");
        return false;
    }

    printf("SATA device initialized successfully.\n");
    return true;
}

bool sata_chipset_read(uint32_t lba, uint8_t sectors, void *buffer) {
    if (!controller_initialized) {
        printf("SATA controller not initialized.\n");
        return false;
    }

    return sata_read_sectors(&device, lba, sectors, buffer);
}

bool sata_chipset_write(uint32_t lba, uint8_t sectors, const void *buffer) {
    if (!controller_initialized) {
        printf("SATA controller not initialized.\n");
        return false;
    }

    return sata_write_sectors(&device, lba, sectors, buffer);
}

void sata_chipset_identify(void) {
    if (!controller_initialized) {
        printf("SATA controller not initialized.\n");
        return;
    }

    printf("SATA Device Information:\n");
    printf("  Controller: Vendor ID: %x, Device ID: %x\n",
           sata_controller.vendor_id, sata_controller.device_id);
    printf("  Class: %x, Subclass: %x\n",
           sata_controller.class_code, sata_controller.subclass);
    printf("  Bus: %d, Device: %d, Function: %d\n",
           sata_controller.bus, sata_controller.device, sata_controller.function);
    
    if (device.size > 0) {
        printf("  Device Size: %d sectors\n", device.size);
        printf("  Max LBA: %d\n", device.size - 1);
        printf("  Sector Size: 512 bytes\n");
    }
}