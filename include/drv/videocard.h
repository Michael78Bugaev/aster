#ifndef VIDEOCARD_H
#define VIDEOCARD_H

#include <stdint.h>

// Определяем идентификаторы для видеокарт
#define INTEL_VENDOR_ID 0x8086
#define AMD_VENDOR_ID 0x1002
#define QEMU_VENDOR_ID 0x1AF4
#define VMWARE_VENDOR_ID 0x15AD

// Структура для хранения информации о видеокарте
typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    const char* name;
} video_card_t;

// Функция для инициализации драйвера видеокарт
void init_video_driver();

// Функция для определения и вывода информации о видеокартах
void detect_and_print_video_cards();

#endif