#ifndef __VFS_EXT2_H__
#define __VFS_EXT2_H__

#include <stdint.h>
#include <fs/asterfs.h>
#include <fs/devfs.h>

//// Ext structure is array of chars
//typedef char * ext2_ext2_t;



#define EXT2_BLOCKGROUPDESCRIPTOR_BLOCK       2
#define EXT2_BLOCKBITMAP_BLOCK                3
#define EXT2_INODEBITMAP_BLOCK                4
#define EXT2_INODETABLE_BLOCK                 5

// Reserved inode numbers
#define EXT2_BAD_INO                       1
#define EXT2_ROOT_INO                      2
#define EXT2_ACL_IDX_INO                   3
#define EXT2_ACL_DATA_INO                  4
#define EXT2_BOOT_LOADER_INO               5
#define EXT2_UNDEL_DIR_INO                 6


// -- file format --
#define EXT2_S_IFSOCK	0xC000	// socket
#define EXT2_S_IFLNK	0xA000	// symbolic link
#define EXT2_S_IFREG	0x8000	// regular file
#define EXT2_S_IFBLK	0x6000	// block device
#define EXT2_S_IFDIR	0x4000	// directory
#define EXT2_S_IFCHR	0x2000	// character device
#define EXT2_S_IFIFO	0x1000	// fifo

// -- process execution user/group override --
#define EXT2_S_ISUID	0x0800	// Set process User ID
#define EXT2_S_ISGID	0x0400	// Set process Group ID
#define EXT2_S_ISVTX	0x0200	// sticky bit

// -- access rights --
#define EXT2_S_IRUSR	0x0100	// user read
#define EXT2_S_IWUSR	0x0080	// user write
#define EXT2_S_IXUSR	0x0040	// user execute
#define EXT2_S_IRGRP	0x0020	// group read
#define EXT2_S_IWGRP	0x0010	// group write
#define EXT2_S_IXGRP	0x0008	// group execute
#define EXT2_S_IROTH	0x0004	// others read
#define EXT2_S_IWOTH	0x0002	// others write
#define EXT2_S_IXOTH	0x0001	// others execute

#pragma pack(1)
typedef struct {
    uint32_t      inodeCount;
    uint32_t      blockCount;
    uint32_t      reservedBlockCount;
    uint32_t      unallocatedBlockCount;
    uint32_t      unalloactedInodeCount;
    uint32_t      firstDataBlock;
    uint32_t      blockSizeLog2;
    uint32_t      fragmentSizeLog2;
    uint32_t      blocksInGroupCount;
    uint32_t      fragmentsInGroupCount;
    uint32_t      inodesInGroupCount;
    uint32_t      lastMountTime;
    uint32_t      lastWriteTime;
    uint16_t      mountCountSinceCheck;
    uint16_t      maxMountCountBeforeCheck;
    uint16_t      ext2Signature;
    uint16_t      filesystemState;
    uint16_t      errorHandling;
    uint16_t      versionMinor;
    uint32_t      consistencyCheckTime;
    uint32_t      intervalConsistencyCheckTime;
    uint32_t      operatingSystemId;
    uint32_t      versionMajor;
    uint16_t      reservedBlocksUid;
    uint16_t      reservedBlocksGid;
} ext2_superblock_t;

#pragma pack(1)
typedef struct {
    uint32_t      firstNonReservedInode;
    uint16_t      inodeSize;
    uint16_t      superblockBlockgroup;
    uint32_t      optionalFeatures;
    uint32_t      requiredFeatures;
    uint32_t      readonlyFeatures;
    char        fileSystemId[16];
    char        volumeName[16];
    char        lastMountPath[64];
    uint32_t      compressionUsed;
    uint8_t       preallocateFileBlockCount;
    uint8_t       preallocateDirectoryBlockCount;
    uint16_t      _reserved;
    char        journalId[16];
    uint32_t      journalInode;
    uint32_t      journalDevice;
    uint32_t      orphanInodeListHead;
} ext2_superblock_extended_t;

#pragma pack(1)
typedef struct {
    uint32_t      blockUsageBitmapAddress;
    uint32_t      inodeUsageBitmapAddress;
    uint32_t      inodeTableStart;
    uint16_t      unallocatedBlockCount;
    uint16_t      unallocatedInodeCount;
    uint16_t      directoryCount;
    char        _reserved[14];
} ext2_blockdescriptor_t;

#pragma pack(1)
typedef struct {
    uint16_t      typeAndPermissions;
    uint16_t      uid;
    uint32_t      sizeLow;
    uint32_t      atime;
    uint32_t      ctime;
    uint32_t      mtime;
    uint32_t      dtime;
    uint16_t      gid;
    uint16_t      linkCount;
    uint32_t      sectorCount;
    uint32_t      flags;
    uint32_t      ossValue;
    uint32_t      directPointerBlock[12];
    uint32_t      singleIndirectPointerBlock;
    uint32_t      doubleIndirectPointerBlock;
    uint32_t      tripleIndirectPointerBlock;
    uint32_t      generatorNumber;
    uint32_t      fileACL;
    union {
        uint32_t      sizeHigh;
        uint32_t      directoryACL;
    };
} ext2_inode_t;


#pragma pack(1)
typedef struct {
    uint32_t      inode_nr;
    uint16_t      rec_len;
    uint8_t       name_len;
    uint8_t       file_type;
    char        name[];
} ext2_dir_t;


typedef struct {
    // Pointers to pre-read items
    ext2_superblock_t       *superblock;
    ext2_blockdescriptor_t  *block_descriptor;
    void                    *block_bitmap;
    void                    *inode_bitmap;

    // Precalced values
    uint32_t                  block_size;
    uint32_t                  group_count;
    uint32_t                  sectors_per_block;
    uint32_t                  first_group_start;
    uint32_t                  group_descriptor_count;
} ext2_info_t;



  void ext2_init (void);
  uint32_t ext2_read (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
  uint32_t ext2_write (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
  void ext2_open (vfs_node_t *node);
  void ext2_close (vfs_node_t *node);
  int ext2_readdir (vfs_node_t *node, uint32_t index, vfs_dirent_t *target_dirent);
  int ext2_finddir (vfs_node_t *node, const char *name, vfs_node_t *target_node);

  vfs_node_t *ext2_mount (struct vfs_mount *mount, device_t *dev, const char *path);
  void ext2_umount (struct vfs_mount *mount);

#endif // __VFS_EXT2_H__
