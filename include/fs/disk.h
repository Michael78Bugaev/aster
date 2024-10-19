#ifndef DISK_H
#define DISK_H
#include <fs/file.h>
typedef unsigned int FAT16_DISK_TYPE;

extern struct disk_t* diskt;
// States this is a physical piece of hardware
#define FAT16_DISK_TYPE_REAL 0

struct real_disk
{
    // The BIOS ID of this disk I.e 0x80 (1st hard disk)
    int bios_id;
    int total_heads;
    int sectors_per_track;
};

struct disk_t
{
    FAT16_DISK_TYPE type;
    struct filesystem *filesystem;
    union disks_u {
        struct real_disk r;
    } shared;

    // Private data for filesystem to manage, the filesystem can do what he wants with this pointer everyone else leave it alone
    void* fs_private;
};

/**
 * Finds all disks and initializes them
 */
void disk_search_and_init();
struct disk_t* disk_get(int index);
int disk_read_block(struct disk_t *_disk, int sector, char* buf);
int disk_get_details(int bios_disk_id, int *total_heads, int *sectors_per_track);
#endif