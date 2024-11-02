#include <fs/fat32.h>
#include <stdio.h>
#include <string.h>
#include <drv/sata.h>
#include <cpu/achi.h>
#include <cpu/mem.h>
#include <io/iotools.h>

#define FAT32_EOC_MIN 0x0FFFFFF8
#define FAT32_BAD_CLUSTER 0x0FFFFFF7
#define FAT32_CLUSTER_MASK 0x0FFFFFFF
#define SECTOR_SIZE 512
#define MAX_FILENAME_LENGTH 255
#define MAX_PATH_LENGTH 260

char *get_curdir()
{
    return current_directory;
}

bool fat32_init(sata_device_t* device, fat32_context_t* context) {
    if (!device || !context) {
        printf("FAT32: Invalid parameters\n");
        return false;
    }
    context->device = device;

    // Read boot sector
    uint8_t boot_sector_buffer[SECTOR_SIZE];
    printf("FAT32: Reading boot sector...\n");
    
    if (!sata_read_sectors(device, 0, 1, boot_sector_buffer)) {
        printf("FAT32: Failed to read boot sector\n");
        return false;
    }

    // Отладочный вывод содержимого загрузочного сектора
    printf("FAT32: Boot sector content:\n");
    printf("Jump code: %x %x %x\n", 
           boot_sector_buffer[0], 
           boot_sector_buffer[1], 
           boot_sector_buffer[2]);
    
    printf("OEM Name: %s\n", &boot_sector_buffer[3]);
    printf("Bytes per sector: %d\n", *(uint16_t*)&boot_sector_buffer[11]);
    printf("Sectors per cluster: %d\n", boot_sector_buffer[13]);
    printf("Reserved sectors: %d\n", *(uint16_t*)&boot_sector_buffer[14]);
    printf("Number of FATs: %d\n", boot_sector_buffer[16]);
    printf("Sectors per FAT: %d\n", *(uint32_t*)&boot_sector_buffer[36]);
    printf("Root cluster: %d\n", *(uint32_t*)&boot_sector_buffer[44]);
    printf("FS Type: %s\n", &boot_sector_buffer[82]);

    // Copy boot sector data
    memcpy(&context->boot_sector, boot_sector_buffer, sizeof(fat32_boot_sector_t));

    // Validate filesystem
    if (memcmp(context->boot_sector.fs_type, "FAT32   ", 8) != 0) {
        printf("FAT32: Invalid filesystem type: %s\n", context->boot_sector.fs_type);
        return false;
    }

    context->boot_sector.root_cluster = 2;
    // Calculate important values
    context->first_fat_sector = context->boot_sector.reserved_sectors;
    context->root_dir_cluster = context->boot_sector.root_cluster;
    context->first_data_sector = context->boot_sector.reserved_sectors +
        (context->boot_sector.fat_count * context->boot_sector.sectors_per_fat_32);
    context->sectors_per_cluster = context->boot_sector.sectors_per_cluster;

    printf("FAT32: Filesystem initialized successfully\n");
    printf("FAT32: First FAT sector: %d\n", context->first_fat_sector);
    printf("FAT32: First data sector: %d\n", context->first_data_sector);
    printf("FAT32: Root cluster: %d\n", context->root_dir_cluster);

    return true;
}

bool fat32_read_cluster(fat32_context_t* context, uint32_t cluster, void* buffer) {
    if (!context || !buffer || cluster < 2) {
        printf("FAT32: Invalid read cluster parameters\n");
        return false;
    }

    // Calculate first sector of cluster
    uint32_t first_sector = context->first_data_sector + 
        ((cluster - 2) * context->sectors_per_cluster);

    // Read entire cluster
    if (!sata_read_sectors(context->device, first_sector, 
                          context->sectors_per_cluster, buffer)) {
        printf("FAT32: Failed to read cluster %d (sector %d)\n", 
               cluster, first_sector);
        return false;
    }

    return true;
}

bool fat32_write_cluster(fat32_context_t* context, uint32_t cluster, const void* buffer) {
    if (!context || !buffer || cluster < 2) {
        printf("FAT32: Invalid write cluster parameters\n");
        return false;
    }

    // Calculate first sector of cluster
    uint32_t first_sector = context->first_data_sector + 
        ((cluster - 2) * context->sectors_per_cluster);

    // Write entire cluster
    if (!sata_write_sectors(context->device, first_sector, 
                           context->sectors_per_cluster, buffer)) {
        printf("FAT32: Failed to write cluster %d (sector %d)\n", 
               cluster, first_sector);
        return false;
    }

    return true;
}

uint32_t fat32_get_next_cluster(fat32_context_t* context, uint32_t current_cluster) {
    if (!context || !context->device) {
        printf("Error: Invalid FAT32 context or device in fat32_get_next_cluster\n");
        return 0x0FFFFFF7;  // Bad cluster
    }

    printf("DEBUG: Getting next cluster for %u (0x%x)\n", current_cluster, current_cluster);
    
    uint32_t fat_offset = current_cluster * 4;
    uint32_t fat_sector = context->first_fat_sector + (fat_offset / context->boot_sector.bytes_per_sector);
    uint32_t ent_offset = fat_offset % context->boot_sector.bytes_per_sector;

    printf("DEBUG: FAT sector: %u, entry offset: %u\n", fat_sector, ent_offset);

    uint8_t buffer[512];
    if (!sata_read_sectors(context->device, fat_sector, 1, buffer)) {
        printf("Error: Failed to read FAT sector %u\n", fat_sector);
        return 0x0FFFFFF7; // Bad cluster
    }

    // Безопасное чтение 4 байт без проблем выравнивания
    uint32_t next_cluster = buffer[ent_offset] |
                           ((uint32_t)buffer[ent_offset + 1] << 8) |
                           ((uint32_t)buffer[ent_offset + 2] << 16) |
                           ((uint32_t)buffer[ent_offset + 3] << 24);
    
    next_cluster &= 0x0FFFFFFF; // Маскируем верхние 4 бита

    printf("DEBUG: Next cluster value: %u (0x%x)\n", next_cluster, next_cluster);
    
    if (next_cluster >= 0x0FFFFFF8) {
        printf("DEBUG: End of cluster chain detected\n");
    }
    
    return next_cluster;
}

// Добавим функцию для форматирования в FAT32
bool fat32_format(sata_device_t* device) {
    printf("FAT32: Starting formating disk\n");

    // Создаем буфер для загрузочного сектора
    uint8_t boot_sector[SECTOR_SIZE];
    memset(boot_sector, 0, SECTOR_SIZE);
    
    fat32_boot_sector_t* bs = (fat32_boot_sector_t*)boot_sector;

    // Заполняем базовые поля загрузочного сектора
    bs->jump_code[0] = 0xEB;
    bs->jump_code[1] = 0x58;
    bs->jump_code[2] = 0x90;
    memcpy(bs->oem_name, "MSDOS5.0", 8);
    
    // Настраиваем параметры файловой системы
    bs->bytes_per_sector = SECTOR_SIZE;
    bs->sectors_per_cluster = 8; // типичное значение для FAT32
    bs->reserved_sectors = 32;
    bs->fat_count = 2;
    bs->media_descriptor = 0xF8; // фиксированный диск
    bs->sectors_per_track = 63;
    bs->head_count = 255;
    bs->hidden_sectors = 0;
    bs->total_sectors_32 = device->size; // используем размер диска

    // FAT32 специфичные поля
    bs->sectors_per_fat_32 = (device->size / SECTOR_SIZE) / 128; // примерный расчет
    bs->flags = 0;
    bs->fat_version = 0;
    bs->root_cluster = 2;
    bs->fsinfo_sector = 1;
    bs->backup_boot_sector = 6;
    bs->drive_number = 0x80;
    bs->boot_signature = 0x29;
    bs->volume_id = 0x12345678; // случайное значение
    memcpy(bs->volume_label, "ASTER KRNL ", 11);
    memcpy(bs->fs_type, "FAT32   ", 8);

    // Записываем загрузочный сектор
    if (!sata_write_sectors(device, 0, 1, boot_sector)) {
        printf("FAT32: Failed to write boot sector\n");
        return false;
    }

    // Создаем и записываем FSInfo сектор
    uint8_t fsinfo_sector[SECTOR_SIZE];
    memset(fsinfo_sector, 0, SECTOR_SIZE);
    *(uint32_t*)&fsinfo_sector[0] = 0x41615252; // сигнатура FSInfo
    *(uint32_t*)&fsinfo_sector[484] = 0x61417272; // сигнатура FSInfo
    if (!sata_write_sectors(device, 1, 1, fsinfo_sector)) {
        printf("FAT32: Failed to write FSInfo sector\n");
        return false;
    }

    // Очищаем FAT таблицы
    uint8_t fat_sector[SECTOR_SIZE];
    memset(fat_sector, 0, SECTOR_SIZE);
    for (uint32_t i = 0; i < bs->sectors_per_fat_32; i++) {
        if (!sata_write_sectors(device, bs->reserved_sectors + i, 1, fat_sector)) {
            printf("FAT32: Failed to write FAT sector %d\n", i);
            return false;
        }
    }

    // Очищаем корневой каталог
    uint8_t root_dir_sector[SECTOR_SIZE];
    memset(root_dir_sector, 0, SECTOR_SIZE);
    if (!sata_write_sectors(device, bs->reserved_sectors + bs->sectors_per_fat_32, 1, root_dir_sector)) {
        printf("FAT32: Failed to write root directory sector\n");
        return false;
    }

    printf("FAT32 format completed successfully!\n");
    return true;
}

// Вспомогательная функция для поиска свободного кластера
uint32_t find_free_cluster(fat32_context_t* context) {
    uint32_t fat_offset = 0;
    uint32_t fat_sector = context->first_fat_sector;
    uint32_t entries_per_sector = context->boot_sector.bytes_per_sector / 4;
    uint8_t buffer[SECTOR_SIZE];

    printf("FAT32: Searching for free cluster...\n");
    printf("FAT32: First FAT sector: %d\n", fat_sector);
    printf("FAT32: Entries per sector: %d\n", entries_per_sector);

    while (fat_sector < context->first_data_sector) {
        if (!sata_read_sectors(context->device, fat_sector, 1, buffer)) {
            printf("FAT32: Failed to read FAT sector %u\n", fat_sector);
            return 0;
        }

        for (uint32_t i = 0; i < entries_per_sector; i++) {
            uint32_t fat_entry = *(uint32_t*)&buffer[i * 4] & 0x0FFFFFFF;
            if (fat_entry == 0) {
                uint32_t free_cluster = fat_offset + i;
                printf("FAT32: Found free cluster: %u\n", free_cluster);
                return free_cluster;
            }
        }

        fat_offset += entries_per_sector;
        fat_sector++;
    }

    printf("FAT32: No free clusters found\n");
    return 0;
}

// Создание файла
bool fat32_create_file(fat32_context_t* context, const char* filename) {
    uint32_t parent_dir_cluster = context->root_dir_cluster;
    uint32_t free_cluster = find_free_cluster(context);
    if (free_cluster == 0) {
        printf("FAT32: No free space available\n");
        return false;
    }

    if (!fat32_update_fat(context, free_cluster, 0x0FFFFFFF)) {
        printf("FAT32: Failed to update FAT for new file\n");
        return false;
    }

    fat32_dir_entry_t new_entry;
    memset(&new_entry, 0, sizeof(fat32_dir_entry_t));
    strncpy(new_entry.name, filename, 11);
    new_entry.attributes = 0x20; // Обычный файл
    new_entry.first_cluster_low = free_cluster & 0xFFFF;
    new_entry.first_cluster_high = (free_cluster >> 16) & 0xFFFF;
    new_entry.file_size = 0;

    // Найдем свободное место в родительской директории и запишем новую запись
    uint8_t cluster_buffer[context->boot_sector.bytes_per_sector * context->sectors_per_cluster];
    if (!fat32_read_cluster(context, parent_dir_cluster, cluster_buffer)) {
        printf("FAT32: Failed to read parent directory cluster\n");
        return false;
    }

    fat32_dir_entry_t* entries = (fat32_dir_entry_t*)cluster_buffer;
    for (int i = 0; i < (context->boot_sector.bytes_per_sector * context->sectors_per_cluster) / sizeof(fat32_dir_entry_t); i++) {
        if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
            memcpy(&entries[i], &new_entry, sizeof(fat32_dir_entry_t));
            if (!fat32_write_cluster(context, parent_dir_cluster, cluster_buffer)) {
                printf("FAT32: Failed to write updated parent directory cluster\n");
                return false;
            }
            return true;
        }
    }

    printf("FAT32: No free directory entries found\n");
    return false;
}

// Чтение файла
bool fat32_read_file(fat32_context_t* context, const char* filename, void* buffer, uint32_t* size) {
    uint32_t current_cluster = context->root_dir_cluster;
    uint8_t cluster_buffer[context->boot_sector.bytes_per_sector * context->sectors_per_cluster];

    while (current_cluster != 0 && current_cluster < 0x0FFFFFF8) {
        if (!fat32_read_cluster(context, current_cluster, cluster_buffer)) {
            printf("FAT32: Failed to read cluster\n");
            return false;
        }

        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)cluster_buffer;
        for (int i = 0; i < (context->boot_sector.bytes_per_sector * context->sectors_per_cluster) / sizeof(fat32_dir_entry_t); i++) {
            if (strncmp(entries[i].name, filename, 11) == 0) {
                uint32_t file_cluster = (entries[i].first_cluster_high << 16) | entries[i].first_cluster_low;
                *size = entries[i].file_size;

                uint32_t bytes_read = 0;
                while (file_cluster != 0 && file_cluster < 0x0FFFFFF8 && bytes_read < *size) {
                    if (!fat32_read_cluster(context, file_cluster, (uint8_t*)buffer + bytes_read)) {
                        printf("FAT32: Failed to read file cluster\n");
                        return false;
                    }
                    bytes_read += context->boot_sector.bytes_per_sector * context->sectors_per_cluster;
                    file_cluster = fat32_get_next_cluster(context, file_cluster);
                }

                return true;
            }
        }

        current_cluster = fat32_get_next_cluster(context, current_cluster);
    }

    printf("File not found\n");
    return false;
}

// Запись в файл
bool fat32_write_file(fat32_context_t* context, const char* filename, const void* buffer, uint32_t size) {
    uint32_t current_cluster = context->root_dir_cluster;
    uint8_t cluster_buffer[context->boot_sector.bytes_per_sector * context->sectors_per_cluster];

    while (current_cluster != 0 && current_cluster < 0x0FFFFFF8) {
        if (!fat32_read_cluster(context, current_cluster, cluster_buffer)) {
            printf("FAT32: Failed to read cluster\n");
            return false;
        }

        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)cluster_buffer;
        for (int i = 0; i < (context->boot_sector.bytes_per_sector * context->sectors_per_cluster) / sizeof(fat32_dir_entry_t); i++) {
            if (strncmp(entries[i].name, filename, 11) == 0) {
                uint32_t file_cluster = (entries[i].first_cluster_high << 16) | entries[i].first_cluster_low;
                uint32_t bytes_written = 0;

                while (bytes_written < size) {
                    if (file_cluster == 0 || file_cluster >= 0x0FFFFFF8) {
                        uint32_t new_cluster = find_free_cluster(context);
                        if (new_cluster == 0) {
                            printf("FAT32: No free space available\n");
                            return false;
                        }
                        if (file_cluster != 0) {
                            // Update FAT
                            // This is a simplified version, you might need to implement proper FAT update
                            fat32_write_cluster(context, file_cluster, &new_cluster);
                        }
                        file_cluster = new_cluster;
                    }

                    uint32_t bytes_to_write = size - bytes_written;
                    if (bytes_to_write > context->boot_sector.bytes_per_sector * context->sectors_per_cluster) {
                        bytes_to_write = context->boot_sector.bytes_per_sector * context->sectors_per_cluster;
                    }

                    if (!fat32_write_cluster(context, file_cluster, (uint8_t*)buffer + bytes_written)) {
                        printf("FAT32: Failed to write file cluster\n");
                        return false;
                    }

                    bytes_written += bytes_to_write;
                    file_cluster = fat32_get_next_cluster(context, file_cluster);
                }

                // Update file size
                entries[i].file_size = size;
                if (!fat32_write_cluster(context, current_cluster, cluster_buffer)) {
                    printf("FAT32: Failed to write updated directory cluster\n");
                    return false;
                }

                return true;
            }
        }

        current_cluster = fat32_get_next_cluster(context, current_cluster);
    }

    printf("File not found\n");
    return false;
}

// Удаление файла
bool fat32_delete_file(fat32_context_t* context, const char* filename) {
    uint32_t current_cluster = context->root_dir_cluster;
    uint8_t cluster_buffer[context->boot_sector.bytes_per_sector * context->sectors_per_cluster];

    while (current_cluster != 0 && current_cluster < 0x0FFFFFF8) {
        if (!fat32_read_cluster(context, current_cluster, cluster_buffer)) {
            printf("FAT32: Failed to read cluster\n");
            return false;
        }

        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)cluster_buffer;
        for (int i = 0; i < (context->boot_sector.bytes_per_sector * context->sectors_per_cluster) / sizeof(fat32_dir_entry_t); i++) {
            if (strncmp(entries[i].name, filename, 11) == 0) {
                // Mark entry as deleted
                entries[i].name[0] = 0xE5;
                if (!fat32_write_cluster(context, current_cluster, cluster_buffer)) {
                    printf("FAT32: Failed to write updated directory cluster\n");
                    return false;
                }

                // Clear FAT entries
                uint32_t file_cluster = (entries[i].first_cluster_high << 16) | entries[i].first_cluster_low;
                while (file_cluster != 0 && file_cluster < 0x0FFFFFF8) {
                    uint32_t next_cluster = fat32_get_next_cluster(context, file_cluster);
                    fat32_write_cluster(context, file_cluster, &next_cluster);
                    file_cluster = next_cluster;
                }

                return true;
            }
        }

        current_cluster = fat32_get_next_cluster(context, current_cluster);
    }

    printf("File not found\n");
    return false;
}

// Создание директории
bool fat32_create_dir(fat32_context_t* context, const char* dirname) {
    uint32_t parent_dir_cluster = context->root_dir_cluster;
    uint32_t free_cluster = find_free_cluster(context);
    if (free_cluster == 0) {
        printf("FAT32: No free space available\n");
        return false;
    }

    fat32_dir_entry_t new_entry;
    memset(&new_entry, 0, sizeof(fat32_dir_entry_t));
    strncpy(new_entry.name, dirname, 11);
    new_entry.attributes = 0x10; // Директория
    new_entry.first_cluster_low = free_cluster & 0xFFFF;
    new_entry.first_cluster_high = (free_cluster >> 16) & 0xFFFF;
    new_entry.file_size = 0;

    // Найдем свободное место в родительской директории и запишем новую запись
    uint8_t cluster_buffer[context->boot_sector.bytes_per_sector * context->sectors_per_cluster];
    if (!fat32_read_cluster(context, parent_dir_cluster, cluster_buffer)) {
        printf("FAT32: Failed to read parent directory cluster\n");
        return false;
    }

    fat32_dir_entry_t* entries = (fat32_dir_entry_t*)cluster_buffer;
    for (int i = 0; i < (context->boot_sector.bytes_per_sector * context->sectors_per_cluster) / sizeof(fat32_dir_entry_t); i++) {
        if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
            memcpy(&entries[i], &new_entry, sizeof(fat32_dir_entry_t));
            if (!fat32_write_cluster(context, parent_dir_cluster, cluster_buffer)) {
                printf("FAT32: Failed to write updated parent directory cluster\n");
                return false;
            }
            return true;
        }
    }

    printf("No free directory entries found\n");
    return false;
}

// Чтение содержимого директории
bool fat32_read_dir(fat32_context_t* context, const char* dirname, fat32_dir_entry_t* entries, uint32_t* count) {
    uint32_t current_cluster = context->root_dir_cluster;
    uint8_t cluster_buffer[context->boot_sector.bytes_per_sector * context->sectors_per_cluster];

    while (current_cluster != 0 && current_cluster < 0x0FFFFFF8) {
        if (!fat32_read_cluster(context, current_cluster, cluster_buffer)) {
            printf("FAT32: Failed to read cluster\n");
            return false ;
        }

        fat32_dir_entry_t* dir_entries = (fat32_dir_entry_t*)cluster_buffer;
        for (int i = 0; i < (context->boot_sector.bytes_per_sector * context->sectors_per_cluster) / sizeof(fat32_dir_entry_t); i++) {
            if (strncmp(dir_entries[i].name, dirname, 11) == 0) {
                uint32_t dir_cluster = (dir_entries[i].first_cluster_high << 16) | dir_entries[i].first_cluster_low;
                uint32_t entry_count = 0;

                while (dir_cluster != 0 && dir_cluster < 0x0FFFFFF8) {
                    if (!fat32_read_cluster(context, dir_cluster, cluster_buffer)) {
                        printf("FAT32: Failed to read directory cluster\n");
                        return false;
                    }

                    fat32_dir_entry_t* dir_entries = (fat32_dir_entry_t*)cluster_buffer;
                    for (int j = 0; j < (context->boot_sector.bytes_per_sector * context->sectors_per_cluster) / sizeof(fat32_dir_entry_t); j++) {
                        if (dir_entries[j].name[0] != 0x00 && dir_entries[j].name[0] != 0xE5) {
                            memcpy(&entries[entry_count], &dir_entries[j], sizeof(fat32_dir_entry_t));
                            entry_count++;
                        }
                    }

                    dir_cluster = fat32_get_next_cluster(context, dir_cluster);
                }

                *count = entry_count;
                return true;
            }
        }

        current_cluster = fat32_get_next_cluster(context, current_cluster);
    }

    printf("Directory not found\n");
    return false;
}

// Удаление директории
bool fat32_delete_dir(fat32_context_t* context, const char* dirname) {
    uint32_t current_cluster = context->root_dir_cluster;
    uint8_t cluster_buffer[context->boot_sector.bytes_per_sector * context->sectors_per_cluster];

    while (current_cluster != 0 && current_cluster < 0x0FFFFFF8) {
        if (!fat32_read_cluster(context, current_cluster, cluster_buffer)) {
            printf("Failed to read cluster\n");
            return false;
        }

        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)cluster_buffer;
        for (int i = 0; i < (context->boot_sector.bytes_per_sector * context->sectors_per_cluster) / sizeof(fat32_dir_entry_t); i++) {
            if (strncmp(entries[i].name, dirname, 11) == 0) {
                // Mark entry as deleted
                entries[i].name[0] = 0xE5;
                if (!fat32_write_cluster(context, current_cluster, cluster_buffer)) {
                    printf("FAT32: Failed to write updated directory cluster\n");
                    return false;
                }

                // Clear FAT entries
                uint32_t dir_cluster = (entries[i].first_cluster_high << 16) | entries[i].first_cluster_low;
                while (dir_cluster != 0 && dir_cluster < 0x0FFFFFF8) {
                    uint32_t next_cluster = fat32_get_next_cluster(context, dir_cluster);
                    fat32_write_cluster(context, dir_cluster, &next_cluster);
                    dir_cluster = next_cluster;
                }

                return true;
            }
        }

        current_cluster = fat32_get_next_cluster(context, current_cluster);
    }

    printf("Directory not found\n");
    return false;
}

bool list_directory(fat32_context_t* context, const char* path) {
    if (!context || !context->device) {
        printf("Error: Invalid FAT32 context or device\n");
        return false;
    }

    printf("DEBUG: Starting directory listing for path: %s\n", path);
    printf("DEBUG: Root cluster: %u (0x%x)\n", context->root_dir_cluster, context->root_dir_cluster);

    uint32_t current_cluster = context->root_dir_cluster;
    if (current_cluster < 2) {
        printf("Error: Invalid root cluster number: %u\n", current_cluster);
        return false;
    }

    uint32_t bytes_per_sector = context->boot_sector.bytes_per_sector;
    uint32_t sectors_per_cluster = context->sectors_per_cluster;
    
    if (bytes_per_sector == 0 || sectors_per_cluster == 0) {
        printf("Error: Invalid bytes per sector (%u) or sectors per cluster (%u)\n", 
               bytes_per_sector, sectors_per_cluster);
        return false;
    }

    uint32_t bytes_per_cluster = bytes_per_sector * sectors_per_cluster;
    printf("DEBUG: Bytes per cluster: %u\n", bytes_per_cluster);

    uint8_t* cluster_buffer = (uint8_t*)malloc(bytes_per_cluster);
    if (!cluster_buffer) {
        printf("Error: Failed to allocate cluster buffer\n");
        return false;
    }

    printf("Directory listing for %s:\n", path);
    printf("Name          Size      Attributes\n");
    printf("------------------------------------\n");

    while (current_cluster >= 2 && current_cluster < 0x0FFFFFF8) {
        printf("DEBUG: Processing cluster %u (0x%x)\n", current_cluster, current_cluster);

        if (!fat32_read_cluster(context, current_cluster, cluster_buffer)) {
            printf("Error: Failed to read cluster %u\n", current_cluster);
            mfree(cluster_buffer);
            return false;
        }

        fat32_dir_entry_t* entries = (fat32_dir_entry_t*)cluster_buffer;
        uint32_t entries_per_cluster = bytes_per_cluster / sizeof(fat32_dir_entry_t);

        for (uint32_t i = 0; i < entries_per_cluster; i++) {
            if (entries[i].name[0] == 0x00) {
                // End of directory
                break;
            }
            if (entries[i].name[0] == 0xE5) {
                // Deleted entry
                continue;
            }

            char filename[13] = {0};
            memcpy(filename, entries[i].name, 11);
            filename[11] = '\0';

            // Convert filename from FAT format
            for (int j = 10; j >= 0 && filename[j] == ' '; j--) {
                filename[j] = '\0';
            }
            for (int j = 8; j < 11 && filename[j] != ' '; j++) {
                if (j == 8) strcat(filename, ".");
                strncat(filename, &filename[j]);
            }
            filename[12] = '\0';

            printf("%s ", filename);
            
            if (entries[i].attributes & 0x10) {
                printf("<DIR>      ");
            } else {
                printf("%d b ", entries[i].file_size);
            }

            printf("%c%c%c%c%c\n",
                   (entries[i].attributes & 0x01) ? 'R' : '-',
                   (entries[i].attributes & 0x02) ? 'H' : '-',
                   (entries[i].attributes & 0x04) ? 'S' : '-',
                   (entries[i].attributes & 0x10) ? 'D' : '-',
                   (entries[i].attributes & 0x20) ? 'A' : '-');
        }

        uint32_t next_cluster = fat32_get_next_cluster(context, current_cluster);
        printf("DEBUG: Next cluster: %u (0x%x)\n", next_cluster, next_cluster);
        
        if (next_cluster == current_cluster) {
            printf("Error: Circular cluster chain detected\n");
            mfree(cluster_buffer);
            return false;
        }
        if (next_cluster < 2 || next_cluster >= 0x0FFFFFF8) {
            break;  // End of cluster chain
        }
        current_cluster = next_cluster;
    }

    printf("------------------------------------\n");
    mfree(cluster_buffer);
    return true;
}

bool init_fat32_filesystem(sata_device_t* device) {
    printf("FAT32: Starting filesystem initialization\n");

    fat32_boot_sector_t bs;
    memset(&bs, 0, sizeof(fat32_boot_sector_t));

    // Устанавливаем базовые параметры
    bs.bytes_per_sector = 512;
    bs.sectors_per_cluster = 8;
    bs.reserved_sectors = 32;
    bs.fat_count = 2;
    bs.total_sectors_32 = device->size;

    // Устанавливаем сигнатуры и идентификаторы
    bs.jump_code[0] = 0xEB;
    bs.jump_code[1] = 0x58;
    bs.jump_code[2] = 0x90;
    memcpy(bs.oem_name, "ASTER-OS", 8);
    memcpy(bs.fs_type, "FAT32   ", 8);

    // Устанавливаем остальные параметры FAT32
    bs.media_descriptor = 0xF8;
    bs.root_cluster = 2;
    bs.fsinfo_sector = 1;
    bs.backup_boot_sector = 6;
    bs.drive_number = 0x80;
    bs.boot_signature = 0x29;
    
    // Вычисляем размер FAT
    uint32_t data_sectors = device->size - bs.reserved_sectors;
    uint32_t total_clusters = data_sectors / bs.sectors_per_cluster;
    uint32_t fat_size = ((total_clusters * 4) + 511) / 512;
    bs.sectors_per_fat_32 = fat_size;

    printf("FAT32: Writing boot sector...\n");
    printf("FAT32: FS Type being written: %s\n", bs.fs_type);
    
    // Записываем загрузочный сектор
    if (!sata_write_sectors(device, 0, 1, &bs)) {
        printf("FAT32: Failed to write boot sector\n");
        return false;
    }

    // Проверяем, что записалось правильно
    uint8_t verify_buffer[512];
    if (!sata_read_sectors(device, 0, 1, verify_buffer)) {
        printf("FAT32: Failed to verify boot sector\n");
        return false;
    }

    fat32_boot_sector_t* verify_bs = (fat32_boot_sector_t*)verify_buffer;
    printf("FAT32: Verifying written FS Type: %s\n", verify_bs->fs_type);

    // Инициализируем FAT
    memset(verify_buffer, 0, 512);
    *(uint32_t*)verify_buffer = 0x0FFFFFF8;  // Media descriptor
    *(uint32_t*)(verify_buffer + 4) = 0x0FFFFFFF;  // EOC marker
    *(uint32_t*)(verify_buffer + 8) = 0x0FFFFFFF;  // EOC marker for root dir

    // Записываем первый сектор FAT
    if (!sata_write_sectors(device, bs.reserved_sectors, 1, verify_buffer)) {
        printf("FAT32: Failed to write FAT\n");
        return false;
    }

    printf("FAT32: Filesystem initialized successfully\n");
    return true;
}

uint32_t fat32_get_free_space(fat32_context_t* context) {
    uint32_t free_clusters = 0;
    uint32_t fat_offset = 0;
    uint32_t fat_sector = context->first_fat_sector;
    uint32_t entries_per_sector = context->boot_sector.bytes_per_sector / 4;
    uint8_t buffer[SECTOR_SIZE];

    while (fat_sector < context->first_data_sector) {
        if (!sata_read_sectors(context->device, fat_sector, 1, buffer)) {
            printf("FAT32: Failed to read FAT sector %x\n", fat_sector);
            return 0;
        }

        for (uint32_t i = 0; i < entries_per_sector; i++) {
            uint32_t fat_entry = *(uint32_t*)&buffer[i * 4] & 0x0FFFFFFF;
            if (fat_entry == 0) {
                free_clusters++;
            }
        }

        fat_offset += entries_per_sector;
        fat_sector++;
    }

    printf("FAT32: Free clusters: %d\n", free_clusters);
    printf("FAT32: Free space: %d bytes\n", free_clusters * context->boot_sector.sectors_per_cluster * context->boot_sector.bytes_per_sector);
    return free_clusters;
}

bool fat32_update_fat(fat32_context_t* context, uint32_t cluster, uint32_t value) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = context->first_fat_sector + (fat_offset / context->boot_sector.bytes_per_sector);
    uint32_t ent_offset = fat_offset % context->boot_sector.bytes_per_sector;

    uint8_t buffer[SECTOR_SIZE];
    if (!sata_read_sectors(context->device, fat_sector, 1, buffer)) {
        printf("FAT32: Failed to read FAT sector %x\n", fat_sector);
        return false;
    }

    *(uint32_t*)&buffer[ent_offset] = value & 0x0FFFFFFF;

    if (!sata_write_sectors(context->device, fat_sector, 1, buffer)) {
        printf("FAT32: Failed to write FAT sector %x\n", fat_sector);
        return false;
    }

    return true;
}