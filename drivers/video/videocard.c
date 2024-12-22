#include <drv/videocard.h>
#include <drv/pci.h>
#include <stdio.h>
#include <string.h>

// Функция для получения имени устройства по его идентификатору
const char* get_device_name(uint16_t vendor_id, uint16_t device_id) {
    // Здесь можно добавить логику для получения имени устройства
    // Например, можно использовать таблицу соответствий
    if (vendor_id == INTEL_VENDOR_ID) {
        return "Intel Graphics";
    } else if (vendor_id == AMD_VENDOR_ID) {
        return "AMD Graphics";
    } else if (vendor_id == QEMU_VENDOR_ID) {
        return "QEMU Virtual GPU";
    } else if (vendor_id == VMWARE_VENDOR_ID) {
        return "VMware Virtual GPU";
    }
    return "Unknown Device";
}

// Функция для определения и вывода информации о видеокартах
void detect_and_print_video_cards() {
    pci_dev_t dev;
    uint16_t vendor_id, device_id;

    // Сканируем все шины, устройства и функции
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t device = 0; device < 32; device++) {
            for (uint32_t function = 0; function < 8; function++) {
                dev.bus_num = bus;
                dev.device_num = device;
                dev.function_num = function;

                // Считываем vendor ID
                vendor_id = pci_read(dev, PCI_VENDOR_ID);
                
                // Проверяем, существует ли устройство
                if (vendor_id == 0x00FF || vendor_id == 0xFFFF) {
                    continue; // Устройство не существует, переходим к следующему
                }

                // Считываем device ID
                device_id = pci_read(dev, PCI_DEVICE_ID);
                
                // Проверяем, действительно ли устройство активно
                if (device_id == 0x00FF || vendor_id == 0xFFFF) {
                    continue; // Если device ID невалиден, пропускаем
                }

                // Определяем тип видеокарты
                const char* device_name = get_device_name(vendor_id, device_id);
                if (vendor_id == INTEL_VENDOR_ID) {
                    printf("<(0f)>[INFO]:<(07)> Intel Video Card: %s [%4x:%4x]\n", device_name, vendor_id, device_id);
                } else if (vendor_id == AMD_VENDOR_ID) {
                    printf("<(0f)>[INFO]:<(07)> AMD Video Card: %s [%4x:%4x]\n", device_name, vendor_id, device_id);
                } else if (vendor_id == QEMU_VENDOR_ID) {
                    printf("<(0f)>[INFO]:<(07)> QEMU Video Card: %s [%4x:%4x]\n", device_name, vendor_id, device_id);
                } else if (vendor_id == VMWARE_VENDOR_ID) {
                    printf("<(0f)>[INFO]:<(07)> VMware Video Card: %s [%4x:%4x]\n", device_name, vendor_id, device_id);
                }
            }
        }
    }
}

// Функция инициализации драйвера видеокарт
void init_video_driver() {
    detect_and_print_video_cards();
}