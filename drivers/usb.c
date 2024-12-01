#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>
#include <io/iotools.h>
#include <drv/usb.h>

void init_usb() {
    // Инициализация USB контроллера
    INFO("initiliasing usb devices");
    detect_usb_devices();
}

void detect_usb_devices() {
    // Здесь мы должны взаимодействовать с USB контроллером, чтобы обнаружить устройства
    for (uint8_t bus = 0; bus < 255; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            // Чтение данных о устройстве
            uint32_t device_id = 0x8086; // Используем PCI для обнаружения
            uint16_t vendor_id = device_id & 0xFFFF;
            uint16_t product_id = (device_id >> 16) & 0xFFFF;

            // Проверяем, является ли устройство действительным
            if (vendor_id != 0xFFFF && product_id != 0xFFFF) {
                usb_device_t *dev = &usb_devices[usb_device_count++];
                dev->vendor_id = vendor_id;
                dev->product_id = product_id;

                // Форматируем имя устройства без использования sprintf
                char temp_name[64];
                char vendor_str[10];
                char product_str[10];

                // Преобразуем vendor_id и product_id в строки
                itoa(vendor_id, vendor_str, 16); // Преобразуем в шестнадцатеричную строку
                itoa(product_id, product_str, 16);

                // Формируем имя устройства
                strncpy(temp_name, "usb: ", sizeof("usb: "));
                strncat(temp_name, vendor_str);
                strncat(temp_name, product_str);

                // Копируем имя в структуру устройства
                strncpy(dev->device_name, temp_name, sizeof(dev->device_name) - 1);
                dev->device_name[sizeof(dev->device_name) - 1] = '\0'; // Обеспечиваем нулевое завершение

                printf("[INFO]: usb device dev_id 0x%x name: %s\n", device_id, temp_name);
            }
        }
    }
}

void list_usb_devices() {
    printf("usb devices:\n");
    for (uint8_t i = 0; i < usb_device_count; i++) {
        printf("  %s\n", usb_devices[i].device_name);
    }
}

usb_device_t* get_usb_device(uint8_t index) {
    if (index < usb_device_count) {
        return &usb_devices[index];
    }
    return NULL;
}