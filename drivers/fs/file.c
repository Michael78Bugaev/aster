#include <fs/file.h>
#include <config.h>
#include <cpu/mem.h>
#include <fs/fat16.h>
#include <fs/disk.h>
#include <string.h>
#include <stdint.h>

struct filesystem *filesystems[FAT16_MAX_FILESYSTEMS];

static struct filesystem **fs_get_free_filesystem()
{
    int i = 0;
    for (i = 0; i < FAT16_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] == 0)
        {
            return &filesystems[i];
        }
    }

    return 0;
}

void fs_insert_filesystem(struct filesystem *filesystem)
{
    struct filesystem **fs;
    if (filesystem == 0)
    {
        kprintc("panic: NULL filesystem provided", 0x0C);
    }

    fs = fs_get_free_filesystem();
    if (!fs)
    {
        kprintc("panic: No more filesystem slots available, failed to register filesystem", 0x0C);
    }

    *fs = filesystem;
    kprint("filesystem ");
    kprint(filesystem->name);
    kprint(" initialized!\n");
}

/**
 * Loads statically compiled filesystems
 */
static void fs_static_load()
{
    fs_insert_filesystem(fat16_init());
}

void fs_load()
{
    kprint("Initiliazing FAT16...\n");
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

/**
 * 
 * Tests the given filename to see if the path is a valid format
 * \warning This function does not test if the path exists or not
 * Valid paths
 * 0:/
 * 0:/testing/abc
 * 1:/abc/testing
 * 
 * Invalid paths
 * A:/abc
 * B:/
 */
static int fs_valid_path_format(char *filename)
{
    int len = strnlen(filename, FAT16_MAX_PATH);
    return len >= 3 && isdigit(filename[0]) && memcmp(&filename[1], ":/", 2) == 0;
}

static int fs_get_drive_by_path(char *filename)
{
    int len = strnlen(filename, FAT16_MAX_PATH);
    if (!fs_valid_path_format(filename))
    {
        return -FAT16_BAD_PATH;
    }

    return to_integer(filename[0]);
}

int fopen(char *filename, char mode)
{
    kprint("opening file '");
    kprint(filename);
    kprint("'...\n");
    int i = 0;
    int drive_no = fs_get_drive_by_path(filename);
    if (drive_no < 0)
    {
        return drive_no;
    }


    char* start_of_relative_path = &filename[2];
    struct disk_t *disk = disk_get(drive_no);
        
    if (!disk)
    {
        return -FAT16_INVALID_DRIVE;
    }
    kprint("testing");
    return filesystems[0]->open(disk, "abc", mode);
}

struct filesystem* fs_resolve(struct disk_t* disk)
{
    struct filesystem *fs = 0;
    int i;
    for (i = 0; i < FAT16_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0)
        {
            fs = filesystems[i];
            break;
        }
    }

    return fs;
}