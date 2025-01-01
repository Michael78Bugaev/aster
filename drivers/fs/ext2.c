#include <fs/ext2.h>

static ext2_superblock_t superblock;
static ext2_block_group_descriptor_t *bgd_table;

int ide_read_sectors(uint8_t drive, uint8_t num_sectors, uint32_t lba, uint32_t buffer);
int ide_write_sectors(uint8_t drive, uint8_t num_sectors, uint32_t lba, uint32_t buffer);

void ext2_init(uint8_t drive) {
    // Читаем суперблок
    ide_read_sectors(drive, 2, EXT2_SUPERBLOCK_OFFSET / 512, (uint32_t)&superblock);

    // Проверяем сигнатуру суперблока
    if (superblock.magic_signature != 0xEF53) {
        printf("error\n");
        return;
    }

    // Читаем таблицу дескрипторов групп блоков
    bgd_table = malloc(superblock.blocks_per_group * sizeof(ext2_block_group_descriptor_t));
    ide_read_sectors(drive, (superblock.blocks_per_group * sizeof(ext2_block_group_descriptor_t) + 511) / 512,
                     (EXT2_SUPERBLOCK_OFFSET + EXT2_BLOCK_SIZE) / 512, (uint32_t)bgd_table);
}

int ext2_read_inode(uint8_t drive, uint32_t inode_number, ext2_inode_t *inode) {
    if (inode_number < 1 || inode_number > superblock.total_inodes) {
        return -1; // Неверный номер i-нода
    }

    uint32_t block_group = (inode_number - 1) / superblock.inodes_per_group;
    uint32_t inode_index = (inode_number - 1) % superblock.inodes_per_group;

    // Читаем блок с i-нодами
    uint32_t block = bgd_table[block_group].inode_table + (inode_index * superblock.inode_size) / EXT2_BLOCK_SIZE;
    uint8_t buffer[EXT2_BLOCK_SIZE];
    ide_read_sectors(drive, 1, block, (uint32_t)buffer);

    // Копируем нужный i-нод
    memcpy(inode, &buffer[(inode_index * superblock.inode_size) % EXT2_BLOCK_SIZE], sizeof(ext2_inode_t));

    return 0;
}

int ext2_write_inode(uint8_t drive, uint32_t inode_number, ext2_inode_t *inode) {
    if (inode_number < 1 || inode_number > superblock.total_inodes) {
        return -1; // Неверный номер i-нода
    }

    uint32_t block_group = (inode_number - 1) / superblock.inodes_per_group;
    uint32_t inode_index = (inode_number - 1) % superblock.inodes_per_group;

    // Читаем блок с i-нодами
    uint32_t block = bgd_table[block_group].inode_table + (inode_index * superblock.inode_size) / EXT2_BLOCK_SIZE;
    uint8_t buffer[EXT2_BLOCK_SIZE];
    ide_read_sectors(drive, 1, block, (uint32_t)buffer);

    // Записываем i-нод в буфер
    memcpy(&buffer[(inode_index * superblock.inode_size) % EXT2_BLOCK_SIZE], inode, sizeof(ext2_inode_t));

    // Записываем блок обратно на диск
    ide_write_sectors(drive, 1, block, (uint32_t)buffer);

    return 0;
}

int ext2_read_block(uint8_t drive, uint32_t block_number, void *buffer) {
    return ide_read_sectors(drive, 1, block_number, (uint32_t)buffer);
}

int ext2_write_block(uint8_t drive, uint32_t block_number, void *buffer) {
    return ide_write_sectors(drive, 1, block_number, (uint32_t)buffer);
}

void ext2_format(uint8_t drive, uint32_t total_blocks, uint32_t total_inodes) {
    ext2_superblock_t superblock;
    ext2_block_group_descriptor_t bgd;

    // Очистка диска
    uint8_t zero_block[EXT2_BLOCK_SIZE] = {0};
    for (uint32_t i = 0; i < total_blocks; i++) {
        ide_write_sectors(drive, 1, i, (uint32_t)zero_block);
    }

    // Инициализация суперблока
    memset(&superblock, 0, sizeof(ext2_superblock_t));
    superblock.total_inodes = total_inodes;
    superblock.total_blocks = total_blocks;
    superblock.reserved_blocks = total_blocks / 100; // 1% зарезервировано
    superblock.free_blocks = total_blocks - superblock.reserved_blocks - 2; // 2 блока для суперблока и таблицы дескрипторов
    superblock.free_inodes = total_inodes - 1;
    superblock.first_data_block = 1;
    superblock.log_block_size = 0; // 2^(10+0) = 1024
    superblock.log_fragment_size = 0;
    superblock.blocks_per_group = 8192; // Примерное значение
    superblock.fragments_per_group = 8192;
    superblock.inodes_per_group = total_inodes;
    superblock.mount_time = 0;
    superblock.write_time = 0;
    superblock.mount_count = 0;
    superblock.max_mount_count = 20;
    superblock.magic_signature = 0xEF53;
    superblock.filesystem_state = 1; // Очистка, нет ошибок
    superblock.error_handling = 1; // Игнорировать ошибки
    superblock.minor_revision_level = 0;
    superblock.last_check_time = 0;
    superblock.check_interval = 0;
    superblock.creator_os = 0; // Linux
    superblock.revision_level = 1;
    superblock.default_uid = 0;
    superblock.default_gid = 0;
    superblock.first_inode = EXT2_ROOT_INODE;
    superblock.inode_size = EXT2_INODE_SIZE;
    superblock.block_group_number = 0;
    superblock.optional_features = 0;
    superblock.required_features = 0;
    superblock.readonly_features = 0;
    memset(superblock.filesystem_id, 0, sizeof(superblock.filesystem_id));
    strcpy((char *)superblock.volume_name, "EXT2_FS");
    memset(superblock.last_mounted, 0, sizeof(superblock.last_mounted));
    superblock.algorithm_usage_bitmap = 0;
    memset(superblock.preallocated_blocks, 0, sizeof(superblock.preallocated_blocks));
    memset(superblock.journal_uuid, 0, sizeof(superblock.journal_uuid));
    superblock.journal_inode = 0;
    superblock.journal_device = 0;
    superblock.orphan_inode_list_head = 0;

    // Запись суперблока
    ide_write_sectors(drive, 2, EXT2_SUPERBLOCK_OFFSET / 512, (uint32_t)&superblock);

    // Инициализация дескриптора группы блоков
    memset(&bgd, 0, sizeof(ext2_block_group_descriptor_t));
    bgd.block_bitmap = 2; // Битовая карта блоков
    bgd.inode_bitmap = 3; // Битовая карта i-нодов
    bgd.inode_table = 4; // Таблица i-нодов
    bgd.free_blocks_count = superblock.free_blocks;
    bgd.free_inodes_count = superblock.free_inodes;
    bgd.used_dirs_count = 0;

    // Запись дескриптора группы блоков
    ide_write_sectors(drive, 1, (EXT2_SUPERBLOCK_OFFSET + EXT2_BLOCK_SIZE) / 512, (uint32_t)&bgd);

    // Инициализация битовой карты блоков
    uint8_t block_bitmap[EXT2_BLOCK_SIZE] = {0};
    block_bitmap[0] = 0xFC; // Блоки 0, 1, 2, 3 зарезервированы
    ide_write_sectors(drive, 1, bgd.block_bitmap, (uint32_t)block_bitmap);

    // Инициализация битовой карты i-нодов
    uint8_t inode_bitmap[EXT2_BLOCK_SIZE] = {0};
    inode_bitmap[0] = 0x02; // Только i-нод 2 (корневой каталог) используется
    ide_write_sectors(drive, 1, bgd.inode_bitmap, (uint32_t)inode_bitmap);

    // Инициализация корневого каталога
    ext2_inode_t root_inode;
    memset(&root_inode, 0, sizeof(ext2_inode_t));
    root_inode.mode = EXT2_S_IFDIR | 0755;
    root_inode.uid = 0;
    root_inode.size = EXT2_BLOCK_SIZE;
    root_inode.atime = 0;
    root_inode.ctime = 0;
    root_inode.mtime = 0;
    root_inode.dtime = 0;
    root_inode.gid = 0;
    root_inode.links_count = 2; // Точка и две ссылки на .
    root_inode.blocks = 1;
    root_inode.block[0] = 5; // Первый блок данных для корневого каталога

    // Запись корневого i-нода
    ide_write_sectors(drive, 1, bgd.inode_table, (uint32_t)&root_inode);

    // Инициализация данных корневого каталога
    uint8_t root_dir_block[EXT2_BLOCK_SIZE] = {0};
    struct {
        uint32_t inode;
        uint16_t rec_len;
        uint8_t  name_len;
        uint8_t  file_type;
        char     name[2];
    } __attribute__((packed)) dot_entry = { EXT2_ROOT_INODE, 12, 1, 2, "." };
    struct {
        uint32_t inode;
        uint16_t rec_len;
        uint8_t  name_len;
        uint8_t  file_type;
        char     name[2];
    } __attribute__((packed)) dotdot_entry = { EXT2_ROOT_INODE, EXT2_BLOCK_SIZE - 12, 2, 2, ".." };

    memcpy(root_dir_block, &dot_entry, sizeof(dot_entry));
    memcpy(root_dir_block + sizeof(dot_entry), &dotdot_entry, sizeof(dotdot_entry));

    // Запись данных корневого каталога
    ide_write_sectors(drive, 1, root_inode.block[0], (uint32_t)root_dir_block);
    printf("Success!\n");
}