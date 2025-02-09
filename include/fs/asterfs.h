#ifndef __VFS_ASTERFS_H__
#define __VFS_ASTERFS_H__

#define ASTERFS_MAX_FILES        256

#define ASTERFS_TYPE_FILE        0x01
#define ASTERFS_TYPE_DIRECTORY   0x02
#define ASTERFS_TYPE_BLOCK_DEV   0x03
#define ASTERFS_TYPE_CHAR_DEV    0x04

#include <stdint.h>
#include <fs/devfs.h>

typedef struct {
    uint8_t  inode_nr;          // Inode (or index number)
    uint8_t  parent_inode_nr;   // Parent inode (if in subdir, otherwise 0)
    char   name[128];         // Name of file
    uint8_t  type;              // Type
    uint8_t  device_major_num;  // Device major number (if applicable)
    uint8_t  device_minor_num;  // Device minor number (if applicable)
    uint32_t length;            // Length of data
    char   *data;             // Pointer to data
} ASTERFS_file_t;

ASTERFS_file_t ASTERFS_nodes[ASTERFS_MAX_FILES-1];   // Max 256 files can be added to a ASTERFS system (inc subdirs)

void ASTERFS_init ();
uint32_t ASTERFS_read (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
uint32_t ASTERFS_write (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
void ASTERFS_open (vfs_node_t *node);
void ASTERFS_close (vfs_node_t *node);
vfs_dirent_t *ASTERFS_readdir (vfs_node_t *node, uint32_t index);
vfs_node_t *ASTERFS_finddir (vfs_node_t *node, const char *name);

vfs_node_t *ASTERFS_mount (struct vfs_mount *mount, device_t *dev, const char *path);
void ASTERFS_umount (struct vfs_mount *mount);

#endif // __VFS_ASTERFS_H__