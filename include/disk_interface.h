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

#ifndef DISK_INTERFACE_H
#define DISK_INTERFACE_H

#include <stdint.h>

/// Add new physical disk here
typedef enum {
	DISK_SD_CARD
} disk_e;

/// Returns the status of the MSD (mass storage device)
uint8_t disk_get_status(disk_e disk);

/// Initializes at disk intrface
uint8_t disk_initialize(disk_e disk);

/// Read a number of sectors from the MSD
uint8_t disk_read(disk_e disk, uint8_t* buffer, uint32_t lba, uint32_t count);

/// Write a number of sectors to the MSD
uint8_t disk_write(disk_e disk, const uint8_t* buffer, uint32_t lba, uint32_t count);

#endif