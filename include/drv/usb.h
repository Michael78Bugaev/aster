#ifndef USB_H
#define USB_H

#include <stdint.h>

#define MAX_USB_DEVICES 128

typedef struct {
    uint16_t vendor_id;
    uint16_t product_id;
    uint8_t class_code;
    uint8_t subclass_code;
    uint8_t protocol_code;
    char device_name[64];
} usb_device_t;

usb_device_t usb_devices[MAX_USB_DEVICES];
uint8_t usb_device_count = 0;

void init_usb();
void detect_usb_devices();
void list_usb_devices();
usb_device_t* get_usb_device(uint8_t index);

#endif // USB_H