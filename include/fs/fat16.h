#include <fs/file.h>

extern struct disk_t* disk;
#define FAT16_SIGNATURE 0x29
struct filesystem* fat16_init();