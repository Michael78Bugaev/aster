#include <stdio.h>
#include <string.h>
#include <io/iotools.h>
#include <drv/pci.h>
#include <cpu/pit.h>

struct pci_device pci_devices[MAX_PCI_DEVICES];
int pci_device_count = 0;

// Функция для чтения данных из PCI
uint32_t pci_read_dword(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset) {
    uint32_t address = (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000;
    port_dword_out(PCI_CONFIG_ADDRESS, address);
    return port_dword_in(PCI_CONFIG_DATA);
}

// Функция для извлечения информации о модели устройства
void fetch_pci_device_model(struct pci_device *device, uint16_t bus, uint16_t slot, uint16_t func) {
    uint32_t dword = pci_read_dword(bus, slot, func, PCI_STRING_OFFSET);
}

// Функция для обнаружения всех PCI-устройств
void detect_pci_devices() {
    printf("Detected PCI Devices:\n");
    printf("Vendor ID   Device ID   Class Code  Subclass Code Base Address  Model\n");
    printf("-----------------------------------------------------------------------\n");

    for (uint16_t bus = 0; bus < 20; bus++) {
        for (uint16_t slot = 0; slot < 4; slot++) {
            for (uint16_t func = 0; func < 8; func++) {
                uint32_t device_id = pci_read_dword(bus, slot, func, PCI_VENDOR_ID_OFFSET);
                uint16_t vendor_id = device_id & 0xFFFF;
                uint16_t device_id_value = (device_id >> 16) & 0xFFFF;

                // Проверяем, является ли устройство действительным
                if ((device_id_value != 0xFFFF) && (vendor_id != 0xFFFF)) {
                    uint8_t class_code = (pci_read_dword(bus, slot, func, PCI_CLASS_CODE_OFFSET) >> 24) & 0xFF;
                    uint8_t subclass_code = (pci_read_dword(bus, slot, func, PCI_CLASS_CODE_OFFSET) >> 16) & 0xFF;

                    if (pci_device_count < MAX_PCI_DEVICES) {
                        pci_devices[pci_device_count].vendor_id = vendor_id;
                        pci_devices[pci_device_count].device_id = device_id_value;
                        pci_devices[pci_device_count].class_code = class_code;
                        pci_devices[pci_device_count].subclass_code = subclass_code;
                        pci_devices[pci_device_count].base_address = pci_read_dword(bus, slot, func, PCI_BASE_ADDRESS_OFFSET); // Пример адреса

                        // Получаем информацию о модели устройства
                        fetch_pci_device_model(&pci_devices[pci_device_count], bus, slot, func);

                        // Печатаем информацию о найденном устройстве в виде таблицы
                        printf(" %x      %x      %x         %x           %x           \"%s\"\n", 
                               vendor_id, 
                               device_id_value, 
                               class_code, 
                               subclass_code, 
                               pci_devices[pci_device_count].base_address, 
                               pci_devices[pci_device_count].model);

                        pci_device_count++;
                    }
                }
            }
        }
    }
}

// Функция для печати информации о PCI-устройстве в виде таблицы
void print_pci_device_info(struct pci_device *device) {
    printf("%x %x %x %x %x %s\n", 
           device->vendor_id, 
           device->device_id, 
           device->class_code, 
           device->subclass_code, 
           device->base_address, 
           device->model);
}

// Инициализация PCI
void init_pci() {
    // Инициализация других компонентов, если нужно
    detect_pci_devices();
}