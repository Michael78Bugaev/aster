#ifndef __VFS_DEVFS_H__
#define __VFS_DEVFS_H__

#define DEVFS_MAX_FILES        256

#define DEVFS_TYPE_DIRECTORY   0x02
#define DEVFS_TYPE_BLOCK_DEV   0x03
#define DEVFS_TYPE_CHAR_DEV    0x04

#include <stdint.h>
#include <fs/vfs.h>

typedef struct {
    uint8_t  inode_nr;          // Inode (or index number)
    uint8_t  parent_inode_nr;   // Parent inode (if in subdir, otherwise 0)
    char   name[128];         // Name of file
    uint8_t  type;              // Type
    uint8_t  major_num;         // Device major number (if applicable)
    uint8_t  minor_num;         // Device minor number (if applicable)
} devfs_file_t;

  devfs_file_t devfs_nodes[DEVFS_MAX_FILES-1];   // Max 256 files can be added to a CybFS system (inc subdirs)

  void devfs_init ();
  int devfs_readdir (vfs_node_t *node, uint32_t index, vfs_dirent_t *target_dirent);
  int devfs_finddir (vfs_node_t *node, const char *name, vfs_node_t *target_node);
  void devfs_mknod (vfs_node_t *node, const char *name, char device_type, uint8_t major_node, uint8_t minor_node);

  vfs_node_t *devfs_mount (struct vfs_mount *mount, device_t *dev, const char *path);
  void devfs_umount (struct vfs_mount *mount);

#endif // __VFS_DEVFS_H__