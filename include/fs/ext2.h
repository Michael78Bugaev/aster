#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>

#define EXT2_SUPERBLOCK_OFFSET 1024
#define EXT2_BLOCK_SIZE 1024
#define EXT2_INODE_SIZE 128

// Типы файлов
#define EXT2_S_IFDIR  0x4000
#define EXT2_S_IFREG  0x8000

// Структура суперблока ext2
typedef struct {
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t reserved_blocks;
    uint32_t free_blocks;
    uint32_t free_inodes;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_fragment_size;
    uint32_t blocks_per_group;
    uint32_t fragments_per_group;
    uint32_t inodes_per_group;
    uint32_t mount_time;
    uint32_t write_time;
    uint16_t mount_count;
    uint16_t max_mount_count;
    uint16_t magic_signature;
    uint16_t filesystem_state;
    uint16_t error_handling;
    uint16_t minor_revision_level;
    uint32_t last_check_time;
    uint32_t check_interval;
    uint32_t creator_os;
    uint32_t revision_level;
    uint16_t default_uid;
    uint16_t default_gid;
    uint32_t first_inode;
    uint16_t inode_size;
    uint16_t block_group_number;
    uint32_t optional_features;
    uint32_t required_features;
    uint32_t readonly_features;
    uint8_t  filesystem_id[16];
    uint8_t  volume_name[16];
    uint8_t  last_mounted[64];
    uint32_t algorithm_usage_bitmap;
    uint8_t  preallocated_blocks[80];
    uint8_t  journal_uuid[16];
    uint32_t journal_inode;
    uint32_t journal_device;
    uint32_t orphan_inode_list_head;
} __attribute__((packed)) ext2_superblock_t;

// Структура группы блоков ext2
typedef struct {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint16_t pad;
    uint8_t  reserved[12];
} __attribute__((packed)) ext2_block_group_descriptor_t;

// Структура inode ext2
typedef struct {
    uint16_t mode;
    uint16_t uid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t osd1;
    uint32_t block[15];
    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint8_t  osd2[12];
} __attribute__((packed)) ext2_inode_t;

#define EXT2_SUPERBLOCK_OFFSET 1024
#define EXT2_BLOCK_SIZE 1024
#define EXT2_INODE_SIZE 128
#define EXT2_ROOT_INODE 2

// Функции для работы с ext2
void ext2_init(uint8_t drive);
int ext2_read_inode(uint8_t drive, uint32_t inode_number, ext2_inode_t *inode);
int ext2_write_inode(uint8_t drive, uint32_t inode_number, ext2_inode_t *inode);
int ext2_read_block(uint8_t drive, uint32_t block_number, void *buffer);
int ext2_write_block(uint8_t drive, uint32_t block_number, void *buffer);
void ext2_format(uint8_t drive, uint32_t total_blocks, uint32_t total_inodes);

#endif // EXT2_H