#include <fs/disk.h>
#include <config.h>
#include <string.h>

static struct disk_t disks[FAT16_MAX_DISKS];
static int disks_total = 0;

extern int disk_reset(int drive_number);
extern int disk_read_sector(void *dest, int drive_number, int head, int sector, int cylinder, int total_sectors);

static void disk_locate(int bios_drive_id)
{
    if (disk_reset(bios_drive_id))
    {
        // Has to be static due to compiler bug issue with passing addresses to functions
        static int sectors_per_track = 0;
        static int total_heads = 0;

        // Ok this disk exists lets set it all up
        struct disk_t *disk_ = &disks[0];
        disk_->type = FAT16_DISK_TYPE_REAL;
        disk_->shared.r.bios_id = bios_drive_id;

        if (disk_get_details(bios_drive_id, &total_heads, &sectors_per_track))
        {
            disk_->shared.r.total_heads = total_heads;
            disk_->shared.r.sectors_per_track = sectors_per_track;
        }
        disk_->filesystem = fs_resolve(disk_);
    }
}

static void disk_locate_all()
{
    int i;
    for (i = 0; i < 4; i++)
    {
        disk_locate(0x80 + i);
    }
}

void disk_search_and_init()
{
    kprint("Searching for disks...\n");
    disks_total = 0;
    memset(disks, 0, sizeof(disks));
    disk_locate_all();
}

static int disk_registered(int index)
{
    struct disk_t empty_disk;
    memset(&empty_disk, 0, sizeof(empty_disk));
    return index < FAT16_MAX_DISKS &&
           memcmp(&disks[index], &empty_disk, sizeof(disks[index])) != 0;
}

struct disk_t *disk_get(int index)
{
    if (!disk_registered(index))
        return 0;

    return &disks[0];
}

int disk_read_block(struct disk_t *_disk, int lba, char *buf)
{
    struct real_disk *rdisk = &_disk->shared.r;
    int head = (lba % (rdisk->sectors_per_track * 2) / rdisk->sectors_per_track);
    int track = (lba / (rdisk->sectors_per_track * 2));
    int sector = (lba % rdisk->sectors_per_track + 1);

    return disk_read_sector(buf, rdisk->bios_id, head, sector, track, 1);
}