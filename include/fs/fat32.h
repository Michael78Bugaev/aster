#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stdbool.h>
#include <storage.h>
#include <drv/sata.h>
#include <cpu/achi.h>

// FAT32 Boot Sector structure
typedef struct {
    uint8_t  jump_code[3];
    uint8_t  oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_count;
    uint16_t root_dir_entries;
    uint16_t total_sectors_16;
    uint8_t  media_descriptor;
    uint16_t sectors_per_fat_16;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    
    // FAT32 specific
    uint32_t sectors_per_fat_32;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fsinfo_sector;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  fs_type[8];
} __attribute__((packed)) fat32_boot_sector_t;

// Directory entry structure
typedef struct {
    uint8_t  name[11];
    uint8_t  attributes;
    uint8_t  nt_reserved;
    uint8_t  creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat32_dir_entry_t;

// FAT32 filesystem context
typedef struct {
    storage_device_t* device;
    fat32_boot_sector_t boot_sector;
    uint32_t first_fat_sector;
    uint32_t first_data_sector;
    uint32_t sectors_per_cluster;
    uint32_t root_dir_cluster;
} fat32_context_t;

fat32_context_t FAT32;
char *current_directory = "/";

char *get_curdir();

// Function prototypes
bool fat32_init(sata_device_t* device, fat32_context_t* context);
bool fat32_read_cluster(fat32_context_t* context, uint32_t cluster, void* buffer);
bool fat32_write_cluster(fat32_context_t* context, uint32_t cluster, const void* buffer);
uint32_t fat32_get_next_cluster(fat32_context_t* context, uint32_t current_cluster);
uint32_t find_free_cluster(fat32_context_t* context);
bool fat32_create_file(fat32_context_t* context, const char* filename);
bool fat32_read_file(fat32_context_t* context, const char* filename, void* buffer, uint32_t* size);
bool fat32_write_file(fat32_context_t* context, const char* filename, const void* buffer, uint32_t size);
bool fat32_delete_file(fat32_context_t* context, const char* filename);
bool fat32_create_dir(fat32_context_t* context, const char* dirname);
bool fat32_read_dir(fat32_context_t* context, const char* dirname, fat32_dir_entry_t* entries, uint32_t* count);
bool fat32_delete_dir(fat32_context_t* context, const char* dirname);
bool list_directory(fat32_context_t* context, const char* path);
bool init_fat32_filesystem(sata_device_t* device);
uint32_t fat32_get_free_space(fat32_context_t* context);
bool fat32_update_fat(fat32_context_t* context, uint32_t cluster, uint32_t value);

#endif