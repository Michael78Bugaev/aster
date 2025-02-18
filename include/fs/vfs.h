

/******************************************************************************
 *
 *  File        : vfs.h
 *  Description : Virtual File System
 *
 *****************************************************************************/
#ifndef __VFS_H__
#define __VFS_H__

    #include <stdint.h>
    #include <drv/device.h>


    // (sys_)mount() options
    #define MOUNTOPTION_REMOUNT         1         // Overwrite an existing mount



    #define VFS_MAX_FILESYSTEMS     100     // Maximum 100 different filesystem
    #define VFS_MAX_MOUNTS          255     // Maximum 255 mounts can be made

    // Defines for filesystem flags
    #define FS_FILE          0x01
    #define FS_DIRECTORY     0x02
    #define FS_CHARDEVICE    0x03   // Major and minor must be set
    #define FS_BLOCKDEVICE   0x04   // Major and minor must be set
    #define FS_PIPE          0x05
    #define FS_SYMLINK       0x06
    #define FS_MOUNTPOINT    0x08

    // Simple defines
    typedef uint32_t inode_t;     // Unique identified on the current mounted filesystem

    // Forward declaration for next structures
    struct vfs_node;
    struct vfs_mount;
    struct dirent;


    // Holds all possible file operations on files inside a FS
    struct vfs_fileops {
      uint32_t(*read)(struct vfs_node *, uint32_t, uint32_t, char *);
      uint32_t(*write)(struct vfs_node *, uint32_t, uint32_t, char *);
      void (*open)(struct vfs_node *);
      void (*close)(struct vfs_node *);
      int (*readdir)(struct vfs_node *, uint32_t, struct dirent *);
      int (*finddir)(struct vfs_node *, const char *, struct vfs_node *);
      void (*mknod)(struct vfs_node *, const char *, char, uint8_t, uint8_t);
    };


    // Mount / system operations for file system
    struct vfs_mountops {
      struct vfs_node *(*mount)(struct vfs_mount *, device_t *, const char *path);   // Called when a device is mounted to a specific node
      void (*umount)(struct vfs_mount *);               // Called when a device is unmounted
    };



    // Info block that is needed to register the filesystem by vfs_register_filesystem()
    typedef struct {
      char                 tag[15];           // VFS tag (FAT12, EXT3, REISERFS, JFS etc)
      char                 name[255];         // More detailed name
      struct vfs_mountops  *mountops;         // Mount operations
    } vfs_info_t;

    // Internal structure that holds all registered file systems
    typedef struct {
      int        enabled;        // Enabled or not
      vfs_info_t info;           // File system information (as given by the filesystem)
      int        mount_count;    // Reference count on how many times this is system is mounted
    } vfs_system_t;


    // Internal structure that holds an active mount point
    typedef struct vfs_mount {
      int              enabled;           // Enabled or not
      char             mount[10];         // Path this is mounted on
      vfs_system_t     *system;           // Mount information
      device_t         *dev;              // Device that is used by this mount
      struct vfs_node  *supernode;        // Supernode for this mount (node for root for this block)
      void             *fs_data;          // Pointer to additional data (could be anything)
    } vfs_mount_t;

    // Abstract representation for a directory entry
    typedef struct dirent {
//        struct dirent *parent;      // Points to the parent directory (..) if any
//        struct dirent *next;        // Points to next entry in this directory

        char    name[255];          // Name of current object
        inode_t inode_nr;           // Inode nr of the current object
    } vfs_dirent_t;

    // Abstract representation for a file entry
    typedef struct vfs_node {
        inode_t              inode_nr;        // Inode number
        char                 name[255];       // Filename
        uint32_t               owner;           // Owner ID
        uint32_t               length;          // File length
        uint32_t               flags;           // Node type
        uint8_t                major_num;       // Major number (only for devices)
        uint8_t                minor_num;       // Minor number (only for devices)

        struct vfs_fileops   *fileops;        // File operations
        struct vfs_mount     *mount;          // Mount point
    } vfs_node_t;





    // Root of the filesystem (defined in vfs.c)
    extern vfs_node_t *vfs_root;

    // Exported file system functions
    uint32_t vfs_read (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
    uint32_t vfs_write (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
    void vfs_create (vfs_node_t *node, const char *name);
    void vfs_open (vfs_node_t *node);
    void vfs_close (vfs_node_t *node);
    int vfs_readdir (vfs_node_t *node, uint32_t index, vfs_dirent_t *target_dirent);
    int vfs_finddir (vfs_node_t *node, const char *name, vfs_node_t *target_node);
    void vfs_mknod (struct vfs_node *node, const char *name, char device_type, uint8_t major_node, uint8_t minor_node);

    // Filesystem registration functionality
    int vfs_register_filesystem (vfs_info_t *info);
    int vfs_unregister_filesystem (const char *tag);

    // Mount/Unmount service calls
    int sys_mount (const char *device_path, const char *fs_type, const char *mount, const char *path, int mount_options);
    int sys_umount (const char *mount_point);

    // @TODO: remove
    int vfs_get_node_from_path (const char *path, vfs_node_t *node);
    vfs_mount_t *vfs_get_mount_from_path (const char *path);

    // VFS init function
    void vfs_init (void);


#endif // __VFS_H__