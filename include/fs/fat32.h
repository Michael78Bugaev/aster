#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>

#define FAT32_SUCCESS 0
#define FAT32_ERROR -1

// Определение структуры заголовка раздела FAT32
typedef struct {
    uint8_t BS_jmpBoot[3];
    uint8_t BS_OEMName[8];
    uint16_t BPB_BytsPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    uint8_t BPB_Reserved[12];
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    uint8_t BS_VolLab[11];
    uint8_t BS_FilSysType[8];
} __attribute__((packed)) BPB_FAT32;

// Определение структуры записи каталога FAT32
typedef struct {
    uint8_t DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
} __attribute__((packed)) DIR_ENTRY_FAT32;

// Функции для работы с FAT32
int fat32_init(uint8_t drive);
int fat32_read_file(uint8_t drive, const char *path, uint32_t buffer, uint32_t max_size);
int fat32_write_file(uint8_t drive, const char *path, uint32_t buffer, uint32_t size);
int fat32_create_file(uint8_t drive, const char *path);
int fat32_delete_file(uint8_t drive, const char *path);
int fat32_create_dir(uint8_t drive, const char *path);
int fat32_delete_dir(uint8_t drive, const char *path);
int fat32_list_dir(uint8_t drive, const char *path, DIR_ENTRY_FAT32 *buffer, uint32_t max_entries);
int fat32_format(uint8_t drive, uint32_t sectors_per_cluster, uint32_t sectors_per_fat, uint32_t total_sectors);

#endif // FAT32_H