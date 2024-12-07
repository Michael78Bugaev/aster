#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>

// Структура суперблока
struct ext2_super_block {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count;
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime;
    uint32_t s_wtime;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck;
    uint32_t s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuid;
    uint16_t s_def_resgid;
    // Другие поля...
};

// Структура группы блоков
struct ext2_group_desc {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    uint32_t bg_reserved[3];
};

// Структура индексного дескриптора
struct ext2_inode {
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size;
    uint32_t i_atime;
    uint32_t i_ctime;
    uint32_t i_mtime;
    uint32_t i_dtime;
    uint16_t i_gid;
    uint16_t i_links_count;
    uint32_t i_blocks;
    uint32_t i_flags;
    uint32_t i_osd1;
    uint32_t i_block[15];
    uint32_t i_generation;
    uint32_t i_file_acl;
    uint32_t i_dir_acl;
    uint32_t i_faddr;
    uint32_t i_osd2[3];
};

// Функции для работы с ext2
void read_super_block(uint8_t drive, struct ext2_super_block *super_block);
void read_group_desc(uint8_t drive, uint32_t group_num, struct ext2_group_desc *group_desc);
void read_inode(uint8_t drive, uint32_t inode_num, struct ext2_inode *inode);
void read_file_data(uint8_t drive, struct ext2_inode *inode, uint8_t *data);
void mount_ext2(uint8_t drive);
uint32_t find_free_inode(uint8_t drive, struct ext2_super_block *super_block, struct ext2_group_desc *group_desc);

// Функция для нахождения свободного блока
uint32_t find_free_block(uint8_t drive, struct ext2_super_block *super_block, struct ext2_group_desc *group_desc);

// Функция для создания файла
void ext2_create_file(uint8_t drive, const char *filename, const char *data);

#endif // EXT2_H
