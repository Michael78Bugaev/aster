#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#define MAX_PCI_DEVICES 256
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC
#define PCI_VENDOR_ID_OFFSET 0x00
#define PCI_DEVICE_ID_OFFSET 0x02
#define PCI_CLASS_CODE_OFFSET 0x08
#define PCI_BASE_ADDRESS_OFFSET 0x10
#define PCI_HEADER_TYPE_OFFSET 0x0E
#define PCI_STRING_OFFSET 0x20 // Смещение для строки с моделью (обычно 0x20 для стандартных устройств)

#define MAX_MODEL_LENGTH 64

// Структура для хранения информации о PCI-устройстве
struct pci_device {
    uint16_t vendor_id;       // Идентификатор производителя
    uint16_t device_id;       // Идентификатор устройства
    uint8_t class_code;       // Код класса устройства
    uint8_t subclass_code;    // Код подкласса устройства
    uint8_t interface;         // Интерфейс устройства
    uint32_t base_address;     // Базовый адрес устройства
    char model[MAX_MODEL_LENGTH]; // Модель устройства (если доступно)
};

// Функции для работы с PCI
uint32_t pci_read_dword(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset);
void detect_pci_devices();
void init_pci();
void print_pci_device_info(struct pci_device *device);
void fetch_pci_device_model(struct pci_device *device, uint16_t bus, uint16_t slot, uint16_t func);

#endif // PCI_H