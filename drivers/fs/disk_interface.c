// DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//                    Version 2, December 2004
//  
// Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
// 
// Everyone is permitted to copy and distribute verbatim or modified
// copies of this license document, and changing it is allowed as long
// as the name is changed.
//  
//            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
// 
//  0. You just DO WHAT THE FUCK YOU WANT TO.

#include <disk_interface.h>
#include <stdint.h>
#include <string.h>
#include <drv/ata.h>
#include <sfat32.h>

uint8_t disk_get_status(disk_e disk) {
	
	return (uint8_t)get_status();
}

uint8_t disk_initialize(disk_e disk) {
	return 1;
}

uint8_t disk_read(disk_e disk, uint8_t* buffer, uint32_t lba, uint32_t count) {
    ata_pio_read48(lba, count, buffer);
	return buffer;
}

uint8_t disk_write(disk_e disk, const uint8_t* buffer, uint32_t lba, uint32_t count) {
	ata_pio_write48(lba, count, buffer);
    return buffer;
}