#include <fs/fat32.h>
#include <drv/ata.h>
#include <string.h>
#include <cpu/mem.h>
#include <stdio.h>

BPB_FAT32 bpb;

// Внутренние функции
static uint32_t fat32_cluster_to_sector(uint32_t cluster);
static int fat32_read_cluster(uint8_t drive, uint32_t cluster, uint32_t buffer);
static int fat32_write_cluster(uint8_t drive, uint32_t cluster, uint32_t buffer);
static uint32_t fat32_find_free_cluster(uint8_t drive);
static int fat32_alloc_cluster(uint8_t drive, uint32_t cluster);
static int fat32_free_cluster(uint8_t drive, uint32_t cluster);
static int fat32_find_entry(uint8_t drive, const char *path, DIR_ENTRY_FAT32 *entry, uint32_t *parent_cluster);
static int fat32_add_entry(uint8_t drive, uint32_t parent_cluster, const char *name, uint8_t attr);
static int fat32_remove_entry(uint8_t drive, uint32_t parent_cluster, const char *name);
static int fat32_write_entry(uint8_t drive, uint32_t parent_cluster, DIR_ENTRY_FAT32 *entry);
static uint32_t fat32_find_next_cluster(uint8_t drive, uint32_t cluster);
static int fat32_chain_clusters(uint8_t drive, uint32_t current_cluster, uint32_t next_cluster);

// Инициализация FAT32 на указанном диске
int fat32_init(uint8_t drive) {
    // Чтение заголовка раздела BPB
    if (ide_read_sectors(drive, 1, 0, (uint32_t)&bpb) != 0) {
        return FAT32_ERROR; // Ошибка чтения
    }

    // Проверка сигнатуры
    if (bpb.BS_BootSig != 0x29) {
        return FAT32_ERROR; // Неверная сигнатура
    }

    return FAT32_SUCCESS;
}

// Чтение файла по пути
int fat32_read_file(uint8_t drive, const char *path, uint32_t buffer, uint32_t max_size) {
    DIR_ENTRY_FAT32 entry;
    uint32_t parent_cluster;
    if (fat32_find_entry(drive, path, &entry, &parent_cluster) != FAT32_SUCCESS) {
        return FAT32_ERROR; // Файл не найден
    }

    if (entry.DIR_Attr & 0x10) {
        return FAT32_ERROR; // Это директория, а не файл
    }

    uint32_t cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
    uint32_t bytes_read = 0;
    uint32_t sector = fat32_cluster_to_sector(cluster);

    while (bytes_read < entry.DIR_FileSize && bytes_read < max_size) {
        if (fat32_read_cluster(drive, cluster, buffer + bytes_read) != FAT32_SUCCESS) {
            return FAT32_ERROR; // Ошибка чтения
        }

        bytes_read += bpb.BPB_BytsPerSec * bpb.BPB_SecPerClus;
        cluster = fat32_find_next_cluster(drive, cluster);
        sector = fat32_cluster_to_sector(cluster);
    }

    return bytes_read; // Возвращаем количество прочитанных байт
}

// Запись файла по пути
int fat32_write_file(uint8_t drive, const char *path, uint32_t buffer, uint32_t size) {
    DIR_ENTRY_FAT32 entry;
    uint32_t parent_cluster;
    if (fat32_find_entry(drive, path, &entry, &parent_cluster) == FAT32_SUCCESS) {
        if (entry.DIR_Attr & 0x10) {
            return FAT32_ERROR; // Это директория, а не файл
        }
    } else {
        // Файл не найден, создаем новый
        if (fat32_add_entry(drive, parent_cluster, path, 0) != FAT32_SUCCESS) {
            return FAT32_ERROR; // Ошибка создания файла
        }

        if (fat32_find_entry(drive, path, &entry, &parent_cluster) != FAT32_SUCCESS) {
            return FAT32_ERROR; // Файл не найден после создания
        }
    }

    uint32_t cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
    if (cluster == 0) {
        cluster = fat32_find_free_cluster(drive);
        if (cluster == 0) {
            return FAT32_ERROR; // Нет свободного кластера
        }

        fat32_alloc_cluster(drive, cluster);
        entry.DIR_FstClusHI = (cluster >> 16) & 0xFFFF;
        entry.DIR_FstClusLO = cluster & 0xFFFF;
    }

    uint32_t bytes_written = 0;
    uint32_t sector = fat32_cluster_to_sector(cluster);

    while (bytes_written < size) {
        if (fat32_write_cluster(drive, cluster, buffer + bytes_written) != FAT32_SUCCESS) {
            return FAT32_ERROR; // Ошибка записи
        }

        bytes_written += bpb.BPB_BytsPerSec * bpb.BPB_SecPerClus;
        if (bytes_written < size) {
            uint32_t next_cluster = fat32_find_next_cluster(drive, cluster);
            if (next_cluster >= 0x0FFFFFF8) {
                next_cluster = fat32_find_free_cluster(drive);
                if (next_cluster == 0) {
                    return FAT32_ERROR; // Нет свободного кластера
                }

                fat32_alloc_cluster(drive, next_cluster);
                fat32_chain_clusters(drive, cluster, next_cluster);
            }

            cluster = next_cluster;
            sector = fat32_cluster_to_sector(cluster);
        }
    }

    entry.DIR_FileSize = size;
    fat32_write_entry(drive, parent_cluster, &entry);
    return bytes_written; // Возвращаем количество записанных байт
}

// Создание файла по пути
int fat32_create_file(uint8_t drive, const char *path) {
    uint32_t parent_cluster;
    DIR_ENTRY_FAT32 entry;
    if (fat32_find_entry(drive, path, &entry, &parent_cluster) == FAT32_SUCCESS) {
        return FAT32_ERROR; // Файл уже существует
    }

    return fat32_add_entry(drive, parent_cluster, path, 0);
}

// Удаление файла по пути
int fat32_delete_file(uint8_t drive, const char *path) {
    uint32_t parent_cluster;
    DIR_ENTRY_FAT32 entry;
    if (fat32_find_entry(drive, path, &entry, &parent_cluster) != FAT32_SUCCESS) {
        return FAT32_ERROR; // Файл не найден
    }

    if (entry.DIR_Attr & 0x10) {
        return FAT32_ERROR; // Это директория, а не файл
    }

    uint32_t cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
    while (cluster < 0x0FFFFFF8) {
        uint32_t next_cluster = fat32_find_next_cluster(drive, cluster);
        fat32_free_cluster(drive, cluster);
        cluster = next_cluster;
    }

    return fat32_remove_entry(drive, parent_cluster, path);
}

// Создание директории по пути
int fat32_create_dir(uint8_t drive, const char *path) {
    uint32_t parent_cluster;
    DIR_ENTRY_FAT32 entry;
    if (fat32_find_entry(drive, path, &entry, &parent_cluster) == FAT32_SUCCESS) {
        return FAT32_ERROR; // Директория уже существует
    }

    uint32_t cluster = fat32_find_free_cluster(drive);
    if (cluster == 0) {
        return FAT32_ERROR; // Нет свободного кластера
    }

    fat32_alloc_cluster(drive, cluster);
    fat32_add_entry(drive, parent_cluster, path, 0x10);

    if (fat32_find_entry(drive, path, &entry, &parent_cluster) != FAT32_SUCCESS) {
        return FAT32_ERROR; // Директория не найдена после создания
    }

    entry.DIR_FstClusHI = (cluster >> 16) & 0xFFFF;
    entry.DIR_FstClusLO = cluster & 0xFFFF;
    fat32_write_entry(drive, parent_cluster, &entry);

    uint32_t sector = fat32_cluster_to_sector(cluster);
    DIR_ENTRY_FAT32 dot_entry = {0};
    strcpy((char*)dot_entry.DIR_Name, ".          ");
    dot_entry.DIR_Attr = 0x10;
    fat32_write_cluster(drive, cluster, (uint32_t)&dot_entry);

    DIR_ENTRY_FAT32 dotdot_entry = {0};
    strcpy((char*)dotdot_entry.DIR_Name, "..         ");
    dotdot_entry.DIR_Attr = 0x10;
    fat32_write_cluster(drive, cluster, (uint32_t)&dotdot_entry);

    return FAT32_SUCCESS;
}

// Удаление директории по пути
int fat32_delete_dir(uint8_t drive, const char *path) {
    uint32_t parent_cluster;
    DIR_ENTRY_FAT32 entry;
    if (fat32_find_entry(drive, path, &entry, &parent_cluster) != FAT32_SUCCESS) {
    return FAT32_ERROR; // Директория не найдена
    }

    if (!(entry.DIR_Attr & 0x10)) {
        return FAT32_ERROR; // Это файл, а не директория
    }

    uint32_t cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
    if (cluster == 0) {
        return FAT32_ERROR; // Неверный кластер директории
    }

    // Проверка, пуста ли директория
    uint32_t sector = fat32_cluster_to_sector(cluster);
    DIR_ENTRY_FAT32 *dir_entry;
    if (fat32_read_cluster(drive, cluster, (uint32_t)&dir_entry) != FAT32_SUCCESS) {
        return FAT32_ERROR; // Ошибка чтения кластера директории
    }

    int is_empty = 1;
    for (int i = 2; i < bpb.BPB_BytsPerSec * bpb.BPB_SecPerClus / sizeof(DIR_ENTRY_FAT32); i++) {
        if (dir_entry->DIR_Name[0] != 0xE5 && dir_entry->DIR_Name[0] != 0x00) {
            is_empty = 0;
            break;
        }
        dir_entry++;
    }

    if (!is_empty) {
        return FAT32_ERROR; // Директория не пуста
    }

    // Освобождение кластеров директории
    while (cluster < 0x0FFFFFF8) {
        uint32_t next_cluster = fat32_find_next_cluster(drive, cluster);
        fat32_free_cluster(drive, cluster);
        cluster = next_cluster;
    }

    return fat32_remove_entry(drive, parent_cluster, path);
}

// Список содержимого директории по пути
int fat32_list_dir(uint8_t drive, const char *path, DIR_ENTRY_FAT32 *buffer, uint32_t max_entries) {
    uint32_t parent_cluster;
    DIR_ENTRY_FAT32 entry;
    if (fat32_find_entry(drive, path, &entry, &parent_cluster) != FAT32_SUCCESS) {
        return FAT32_ERROR; // Директория не найдена
    }

    if (!(entry.DIR_Attr & 0x10)) {
        return FAT32_ERROR; // Это файл, а не директория
    }

    uint32_t cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
    if (cluster == 0) {
        return FAT32_ERROR; // Неверный кластер директории
    }

    uint32_t sector = fat32_cluster_to_sector(cluster);
    DIR_ENTRY_FAT32 *dir_entry;
    uint32_t entries_read = 0;

    while (cluster < 0x0FFFFFF8 && entries_read < max_entries) {
        if (fat32_read_cluster(drive, cluster, (uint32_t)&dir_entry) != FAT32_SUCCESS) {
            return FAT32_ERROR; // Ошибка чтения кластера директории
        }

        for (int i = 0; i < bpb.BPB_BytsPerSec * bpb.BPB_SecPerClus / sizeof(DIR_ENTRY_FAT32); i++) {
            if (dir_entry->DIR_Name[0] != 0xE5 && dir_entry->DIR_Name[0] != 0x00) {
                buffer[entries_read] = *dir_entry;
                entries_read++;
                if (entries_read >= max_entries) {
                    break;
                }
            }
            dir_entry++;
        }

        cluster = fat32_find_next_cluster(drive, cluster);
        sector = fat32_cluster_to_sector(cluster);
    }

    return entries_read; // Возвращаем количество прочитанных записей
}

// Внутренняя функция: конвертация кластера в сектор
static uint32_t fat32_cluster_to_sector(uint32_t cluster) {
    return bpb.BPB_RsvdSecCnt + (bpb.BPB_NumFATs * bpb.BPB_FATSz32) + ((cluster - 2) * bpb.BPB_SecPerClus);
}

// Внутренняя функция: чтение кластера
static int fat32_read_cluster(uint8_t drive, uint32_t cluster, uint32_t buffer) {
    uint32_t sector = fat32_cluster_to_sector(cluster);
    return ide_read_sectors(drive, bpb.BPB_SecPerClus, sector, buffer);
}

// Внутренняя функция: запись кластера
static int fat32_write_cluster(uint8_t drive, uint32_t cluster, uint32_t buffer) {
    uint32_t sector = fat32_cluster_to_sector(cluster);
    return ide_write_sectors(drive, bpb.BPB_SecPerClus, sector, buffer);
}

// Внутренняя функция: поиск свободного кластера
static uint32_t fat32_find_free_cluster(uint8_t drive) {
    uint32_t cluster = 2;
    uint32_t fat_sector = bpb.BPB_RsvdSecCnt + ((cluster * 4) / bpb.BPB_BytsPerSec);
    uint32_t fat_offset = (cluster * 4) % bpb.BPB_BytsPerSec;
    uint32_t next_cluster;

    while (cluster < bpb.BPB_TotSec32 / bpb.BPB_SecPerClus) {
        if (ide_read_sectors(drive, 1, fat_sector, (uint32_t)&next_cluster) != 0) {
            return 0; // Ошибка чтения FAT
        }

        if ((next_cluster & 0x0FFFFFFF) == 0) {
            return cluster;
        }

        cluster++;
        fat_sector = bpb.BPB_RsvdSecCnt + ((cluster * 4) / bpb.BPB_BytsPerSec);
        fat_offset = (cluster * 4) % bpb.BPB_BytsPerSec;
    }

    return 0; // Нет свободного кластера
}

// Внутренняя функция: выделение кластера
static int fat32_alloc_cluster(uint8_t drive, uint32_t cluster) {
    uint32_t fat_sector = bpb.BPB_RsvdSecCnt + ((cluster * 4) / bpb.BPB_BytsPerSec);
    uint32_t fat_offset = (cluster * 4) % bpb.BPB_BytsPerSec;
    uint32_t next_cluster = 0x0FFFFFFF;

    if (ide_write_sectors(drive, 1, fat_sector, (uint32_t)&next_cluster) != 0) {
        return FAT32_ERROR; // Ошибка записи FAT
    }

    return FAT32_SUCCESS;
}

// Внутренняя функция: освобождение кластера
static int fat32_free_cluster(uint8_t drive, uint32_t cluster) {
    uint32_t fat_sector = bpb.BPB_RsvdSecCnt + ((cluster * 4) / bpb.BPB_BytsPerSec);
    uint32_t fat_offset = (cluster * 4) % bpb.BPB_BytsPerSec;
    uint32_t next_cluster = 0;

    if (ide_write_sectors(drive, 1, fat_sector, (uint32_t)&next_cluster) != 0) {
        return FAT32_ERROR; // Ошибка записи FAT
    }

    return FAT32_SUCCESS;
}

// Внутренняя функция: поиск записи по пути
static int fat32_find_entry(uint8_t drive, const char *path, DIR_ENTRY_FAT32 *entry, uint32_t *parent_cluster) {
    char *path_copy = strdup(path);
    char *token = strtok(path_copy, "/");
    uint32_t current_cluster = bpb.BPB_RootClus;

    while (token != NULL) {
        uint32_t sector = fat32_cluster_to_sector(current_cluster);
        DIR_ENTRY_FAT32 *dir_entry;
        int found = 0;

        while (current_cluster < 0x0FFFFFF8 && !found) {
            if (fat32_read_cluster(drive, current_cluster, (uint32_t)&dir_entry) != FAT32_SUCCESS) {
                mfree(path_copy);
                return FAT32_ERROR; // Ошибка чтения кластера
            }

            for (int i = 0; i < bpb.BPB_BytsPerSec * bpb.BPB_SecPerClus / sizeof(DIR_ENTRY_FAT32); i++) {
                if (dir_entry->DIR_Name[0] != 0xE5 && dir_entry->DIR_Name[0] != 0x00) {
                    char name[12];
                    memcpy(name, dir_entry->DIR_Name, 11);
                    name[11] = '\0';

                    if (strcmp(name, token) == 0) {
                        *entry = *dir_entry;
                        *parent_cluster = current_cluster;
                        found = 1;
                        break;
                    }
                }
                dir_entry++;
            }

            current_cluster = fat32_find_next_cluster(drive, current_cluster);
            sector = fat32_cluster_to_sector(current_cluster);
        }

        if (!found) {
            mfree(path_copy);
            return FAT32_ERROR; // Запись не найдена
        }

        if (!(entry->DIR_Attr & 0x10)) {
            mfree(path_copy);
            return FAT32_ERROR; // Это файл, а не директория
        }

        current_cluster = (entry->DIR_FstClusHI << 16) | entry->DIR_FstClusLO;
        token = strtok(NULL, "/");
    }

    mfree(path_copy);
    return FAT32_SUCCESS;
}

// Внутренняя функция: добавление записи
static int fat32_add_entry(uint8_t drive, uint32_t parent_cluster, const char *name, uint8_t attr) {
    uint32_t sector = fat32_cluster_to_sector(parent_cluster);
    DIR_ENTRY_FAT32 *dir_entry;
    int found = 0;

    while (parent_cluster < 0x0FFFFFF8 && !found) {
        if (fat32_read_cluster(drive, parent_cluster, (uint32_t)&dir_entry) != FAT32_SUCCESS) {
            return FAT32_ERROR; // Ошибка чтения кластера
        }

        for (int i = 0; i < bpb.BPB_BytsPerSec * bpb.BPB_SecPerClus / sizeof(DIR_ENTRY_FAT32); i++) {
            if (dir_entry->DIR_Name[0] == 0xE5 || dir_entry->DIR_Name[0] == 0x00) {
                memset(&dir_entry, 0, sizeof(DIR_ENTRY_FAT32));
                memcpy(dir_entry->DIR_Name, name, strlen(name));
                dir_entry->DIR_Name[strlen(name)] = 0x20;
                dir_entry->DIR_Attr = attr;
                dir_entry->DIR_FstClusLO = 0;
                dir_entry->DIR_FstClusHI = 0;
                dir_entry->DIR_FileSize = 0;

                if (fat32_write_cluster(drive, parent_cluster, (uint32_t)&dir_entry) != FAT32_SUCCESS) {
                    return FAT32_ERROR; // Ошибка записи кластера
                }

                found = 1;
                break;
            }
            dir_entry++;
        }

        parent_cluster = fat32_find_next_cluster(drive, parent_cluster);
        sector = fat32_cluster_to_sector(parent_cluster);
    }

    if (!found) {
        return FAT32_ERROR; // Нет свободной записи
    }

    return FAT32_SUCCESS;
}

// Внутренняя функция: удаление записи
static int fat32_remove_entry(uint8_t drive, uint32_t parent_cluster, const char *name) {
    uint32_t sector = fat32_cluster_to_sector(parent_cluster);
    DIR_ENTRY_FAT32 *dir_entry;
    int found = 0;

    while (parent_cluster < 0x0FFFFFF8 && !found) {
        if (fat32_read_cluster(drive, parent_cluster, (uint32_t)&dir_entry) != FAT32_SUCCESS) {
            return FAT32_ERROR; // Ошибка чтения кластера
        }

        for (int i = 0; i < bpb.BPB_BytsPerSec * bpb.BPB_SecPerClus / sizeof(DIR_ENTRY_FAT32); i++) {
            if (dir_entry->DIR_Name[0] != 0xE5 && dir_entry->DIR_Name[0] != 0x00) {
                char entry_name[12];
                memcpy(entry_name, dir_entry->DIR_Name, 11);
                entry_name[11] = '\0';

                if (strcmp(entry_name, name) == 0) {
                    dir_entry->DIR_Name[0] = 0xE5; // Помечаем запись как удаленную

                    if (fat32_write_cluster(drive, parent_cluster, (uint32_t)&dir_entry) != FAT32_SUCCESS) {
                        return FAT32_ERROR; // Ошибка записи кластера
                    }

                    found = 1;
                    break;
                }
            }
            dir_entry++;
        }

        parent_cluster = fat32_find_next_cluster(drive, parent_cluster);
        sector = fat32_cluster_to_sector(parent_cluster);
    }

    if (!found) {
        return FAT32_ERROR; // Запись не найдена
    }

    return FAT32_SUCCESS;
}

// Внутренняя функция: запись записи
static int fat32_write_entry(uint8_t drive, uint32_t parent_cluster, DIR_ENTRY_FAT32 *entry) {
    uint32_t sector = fat32_cluster_to_sector(parent_cluster);
    DIR_ENTRY_FAT32 *dir_entry;
    int found = 0;

    while (parent_cluster < 0x0FFFFFF8 && !found) {
        if (fat32_read_cluster(drive, parent_cluster, (uint32_t)&dir_entry) != FAT32_SUCCESS) {
            return FAT32_ERROR; // Ошибка чтения кластера
        }

        for (int i = 0; i < bpb.BPB_BytsPerSec * bpb.BPB_SecPerClus / sizeof(DIR_ENTRY_FAT32); i++) {
            if (dir_entry->DIR_Name[0] != 0xE5 && dir_entry->DIR_Name[0] != 0x00) {
                char entry_name[12];
                memcpy(entry_name, dir_entry->DIR_Name, 11);
                entry_name[11] = '\0';

                if (strcmp(entry_name, (char*)entry->DIR_Name) == 0) {
                    // Найдена запись, обновляем ее
                    if (fat32_write_cluster(drive, parent_cluster, (uint32_t)entry) != FAT32_SUCCESS) {
                        return FAT32_ERROR; // Ошибка записи кластера
                    }

                    found = 1;
                    break;
                }
            }
            dir_entry++;
        }

        parent_cluster = fat32_find_next_cluster(drive, parent_cluster);
        sector = fat32_cluster_to_sector(parent_cluster);
    }

    if (!found) {
        return FAT32_ERROR; // Запись не найдена
    }

    return FAT32_SUCCESS;
}

// Внутренняя функция: поиск следующего кластера
static uint32_t fat32_find_next_cluster(uint8_t drive, uint32_t cluster) {
    uint32_t fat_sector = bpb.BPB_RsvdSecCnt + ((cluster * 4) / bpb.BPB_BytsPerSec);
    uint32_t fat_offset = (cluster * 4) % bpb.BPB_BytsPerSec;
    uint32_t next_cluster;

    if (ide_read_sectors(drive, 1, fat_sector, (uint32_t)&next_cluster) != 0) {
        return 0; // Ошибка чтения FAT
    }

    return next_cluster & 0x0FFFFFFF;
}

// Внутренняя функция: связывание кластеров
static int fat32_chain_clusters(uint8_t drive, uint32_t current_cluster, uint32_t next_cluster) {
    uint32_t fat_sector = bpb.BPB_RsvdSecCnt + ((current_cluster * 4) / bpb.BPB_BytsPerSec);
    uint32_t fat_offset = (current_cluster * 4) % bpb.BPB_BytsPerSec;
    uint32_t fat_entry;

    if (ide_read_sectors(drive, 1, fat_sector, (uint32_t)&fat_entry) != 0) {
        return FAT32_ERROR; // Ошибка чтения FAT
    }

    fat_entry = (fat_entry & 0xF0000000) | next_cluster;
    if (ide_write_sectors(drive, 1, fat_sector, (uint32_t)&fat_entry) != 0) {
        return FAT32_ERROR; // Ошибка записи FAT
    }

    return FAT32_SUCCESS;
}

int fat32_format(uint8_t drive, uint32_t sectors_per_cluster, uint32_t sectors_per_fat, uint32_t total_sectors) {
    // Инициализация BPB
    memset(&bpb, 0, sizeof(BPB_FAT32));

    // Заголовок загрузочного сектора
    bpb.BS_jmpBoot[0] = 0xEB;
    bpb.BS_jmpBoot[1] = 0x3C;
    bpb.BS_jmpBoot[2] = 0x90;
    memcpy(bpb.BS_OEMName, "FAT32   ", 8);
    bpb.BPB_BytsPerSec = 512;
    bpb.BPB_SecPerClus = sectors_per_cluster;
    bpb.BPB_RsvdSecCnt = 32;
    bpb.BPB_NumFATs = 2;
    bpb.BPB_RootEntCnt = 0;
    bpb.BPB_TotSec16 = 0;
    bpb.BPB_Media = 0xF8;
    bpb.BPB_FATSz16 = 0;
    bpb.BPB_SecPerTrk = 63;
    bpb.BPB_NumHeads = 255;
    bpb.BPB_HiddSec = 0;
    bpb.BPB_TotSec32 = total_sectors;
    bpb.BPB_FATSz32 = sectors_per_fat;
    bpb.BPB_ExtFlags = 0;
    bpb.BPB_FSVer = 0;
    bpb.BPB_RootClus = 2;
    bpb.BPB_FSInfo = 1;
    bpb.BPB_BkBootSec = 6;
    memset(bpb.BPB_Reserved, 0, 12);
    bpb.BS_DrvNum = 0x80;
    bpb.BS_Reserved1 = 0;
    bpb.BS_BootSig = 0x29;
    bpb.BS_VolID = 0x12345678;
    memcpy(bpb.BS_VolLab, "NO NAME    ", 11);
    memcpy(bpb.BS_FilSysType, "FAT32   ", 8);

    // Запись BPB в загрузочный сектор
    if (ide_write_sectors(drive, 1, 0, (uint32_t)&bpb) != 0) {
        return FAT32_ERROR; // Ошибка записи BPB
    }

    // Инициализация FSInfo сектора
    uint8_t fsinfo_sector[512];
    memset(fsinfo_sector, 0, 512);
    fsinfo_sector[0] = 0x52;
    fsinfo_sector[1] = 0x52;
    fsinfo_sector[2] = 0x61;
    fsinfo_sector[3] = 0x41;
    fsinfo_sector[484] = 0x72;
    fsinfo_sector[485] = 0x72;
    fsinfo_sector[486] = 0x41;
    fsinfo_sector[487] = 0x61;
    fsinfo_sector[492] = 0x00;
    fsinfo_sector[493] = 0x00;
    fsinfo_sector[494] = 0x00;
    fsinfo_sector[495] = 0x00;
    fsinfo_sector[496] = 0x00;
    fsinfo_sector[497] = 0x00;
    fsinfo_sector[498] = 0x00;
    fsinfo_sector[499] = 0x00;
    fsinfo_sector[500] = 0x00;
    fsinfo_sector[501] = 0x00;
    fsinfo_sector[502] = 0x00;
    fsinfo_sector[503] = 0x00;
    fsinfo_sector[504] = 0x00;
    fsinfo_sector[505] = 0x00;
    fsinfo_sector[506] = 0x00;
    fsinfo_sector[507] = 0x00;
    fsinfo_sector[508] = 0x00;
    fsinfo_sector[509] = 0x00;
    fsinfo_sector[510] = 0x55;
    fsinfo_sector[511] = 0xAA;

    if (ide_write_sectors(drive, 1, bpb.BPB_FSInfo, (uint32_t)fsinfo_sector) != 0) {
        return FAT32_ERROR; // Ошибка записи FSInfo сектора
    }

    // Инициализация FAT
    uint8_t fat_sector[512];
    memset(fat_sector, 0, 512);
    fat_sector[0] = 0xF8;
    fat_sector[1] = 0xFF;
    fat_sector[2] = 0xFF;
    fat_sector[3] = 0xFF;
    fat_sector[4] = 0xFF;
    fat_sector[5] = 0xFF;
    fat_sector[6] = 0xFF;
    fat_sector[7] = 0x0F;

    for (uint32_t i = 0; i < bpb.BPB_NumFATs; i++) {
        if (ide_write_sectors(drive, bpb.BPB_FATSz32, bpb.BPB_RsvdSecCnt + i * bpb.BPB_FATSz32, (uint32_t)fat_sector) != 0) {
            return FAT32_ERROR; // Ошибка записи FAT
        }
    }

    // Создание корневой директории
    uint8_t root_sector[512];
    memset(root_sector, 0, 512);

    if (ide_write_sectors(drive, bpb.BPB_SecPerClus, bpb.BPB_RsvdSecCnt + bpb.BPB_NumFATs * bpb.BPB_FATSz32, (uint32_t)root_sector) != 0) {
        return FAT32_ERROR; // Ошибка записи корневой директории
    }

    return FAT32_SUCCESS;
}