#include <stdint.h>
#include <config.h>
#include <string.h>
#include <cpu/mem.h>
#include <fs/vfs.h>
#include <fs/asterfs.h>
#include <stdio.h>


// File operations
static struct vfs_fileops ASTERFS_fileops = {
    .read = ASTERFS_read, .write = ASTERFS_write,
    .open = ASTERFS_open, .close = ASTERFS_close,
    .readdir = ASTERFS_readdir, .finddir = ASTERFS_finddir
};

// Mount operations
static struct vfs_mountops ASTERFS_mountops = {
    .mount = ASTERFS_mount, .umount = ASTERFS_umount
};

// Global structure
static vfs_info_t ASTERFS_vfs_info = { .tag = "ASTERFS",
                              .name = "AsterOS File System",
                              .mountops = &ASTERFS_mountops
                            };

// Root supernode
static vfs_node_t ASTERFS_supernode = {
  .inode_nr = 0,
  .name = "/",
  .owner = 0,
  .length = 0,
  .flags = FS_DIRECTORY,
  .major_num = 0,
  .minor_num = 0,
  .fileops = &ASTERFS_fileops,
};

char helloworlddata[] = "Hello world!\nThis file is the first readable file from Aster OS!\n\nHave fun!\n\0";


// @TODO: Remove most of these items.. only /DEVICES should be present so we can mount our root system
const ASTERFS_file_t ASTERFS_default_layout[] = {
                                        // Dummy root
                                        { 0,  0,  "ASTERFS-root",      0,                    0, 0,  0, NULL },
                                        // Mandatory FS Structure
                                        { 1,  0,  "SYSTEM",          ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        { 2,  1,  ".",               ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        { 3,  1,  "..",              ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        { 4,  0,  "PROGRAM",         ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        { 5,  4,  ".",               ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        { 6,  4,  "..",              ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        { 7,  0,  "DEVICE",          ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        { 8,  7,  ".",               ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        { 9,  7,  "..",              ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        {10,  0,  "INFO",            ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        {11, 10,  ".",               ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        {12, 10,  "..",              ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        {13,  0,  "USER",            ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        {14, 13,  ".",               ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        {15, 13,  "..",              ASTERFS_TYPE_DIRECTORY, 0, 0,  0, NULL },
                                        // Files in /SYSTEM
                                        {16,  1,  "kernel.bin",      ASTERFS_TYPE_FILE,      0, 0,  0, NULL },
                                        {17,  1,  "bootlvl1.bin",    ASTERFS_TYPE_FILE,      0, 0,  0, NULL },
                                        {18,  1,  "bootlvl2.bin",    ASTERFS_TYPE_FILE,      0, 0,  0, NULL },
                                        {19,  1,  "fat12.drv",       ASTERFS_TYPE_FILE,      0, 0,  0, NULL },
                                        {20,  1,  "ASTERFS.drv",      ASTERFS_TYPE_FILE,      0, 0,  0, NULL },
                                        // Files in /INFO
                                        {34, 10,  "cpu",             ASTERFS_TYPE_FILE,      0, 0,  0, NULL },
                                        {35, 10,  "memory",          ASTERFS_TYPE_FILE,      0, 0,  0, NULL },
                                        {36, 10,  "fdc",             ASTERFS_TYPE_FILE,      0, 0,  0, NULL },
                                        {37, 10,  "hdc",             ASTERFS_TYPE_FILE,      0, 0,  0, NULL },
                                        // Files in /DEVICE
                                        {38,  7,  "FLOPPY0",         ASTERFS_TYPE_BLOCK_DEV, 1, 0,  0, NULL },
                                        {39,  7,  "FLOPPY1",         ASTERFS_TYPE_BLOCK_DEV, 1, 1,  0, NULL },
                                        {40,  7,  "FLOPPY2",         ASTERFS_TYPE_BLOCK_DEV, 1, 2,  0, NULL },
                                        {41,  7,  "FLOPPY3",         ASTERFS_TYPE_BLOCK_DEV, 1, 3,  0, NULL }
                                       };

vfs_dirent_t ASTERFS_rd_dirent;
vfs_node_t ASTERFS_fd_node;


/**
 * Intializes ASTERFS filesystem
 */
void ASTERFS_init () {
  int i;

  // Clear out all files
  for (i=0; i!=ASTERFS_MAX_FILES; i++) {
    //printf("%d\n", i);
    ASTERFS_nodes[i].inode_nr = i;
    ASTERFS_nodes[i].type = 0;
  }
//   printf("a\n");
//   int* alloc = malloc(sizeof(ASTERFS_default_layout));
//   if (alloc == NULL) { printf("error\n"); }
//   else { printf("okey\n"); }
//   // Copy default file hierachy
  strncpy(&ASTERFS_vfs_info, &ASTERFS_default_layout, sizeof (ASTERFS_default_layout));

  // Register filesystem to the VFS
  vfs_register_filesystem (&ASTERFS_vfs_info);
}


/**
 * Read file data
 */
uint32_t ASTERFS_read (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer) {
  // Offset is outside file
  if (offset > ASTERFS_nodes[node->inode_nr].length) return 0;

  // Don't read more than we are capable off
  if (offset + size > ASTERFS_nodes[node->inode_nr].length) size = ASTERFS_nodes[node->inode_nr].length - offset;

  // Copy file data to buffer
  memcpy (buffer, ASTERFS_nodes[node->inode_nr].data+offset, size);

  // Return number of bytes read
  return size;
}

/**
 * Writes data to file
 */
uint32_t ASTERFS_write (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer) {
  printf ("ASTERFS_write()\n");
  // Cannot write at the moment
  return 0;
}

/**
 * Opens a file
 */
void ASTERFS_open (vfs_node_t *node) {
  printf ("ASTERFS_open()\n");
  // Cannot open files
}

/**
 * Closes a file
 */
void ASTERFS_close (vfs_node_t *node) {
  printf ("ASTERFS_close()\n");
  // Cannot close files
}

/**
 * Read directory entry X (numerical dir seek)
 */
vfs_dirent_t *ASTERFS_readdir (vfs_node_t *dirnode, uint32_t index) {
  int i, found;

//  printf ("ASTERFS_readdir(%s, %d)\n", dirnode->name, index);

  // We can check which 'subdirectory' we are in, by checking the parent_inode_nr of the
  // file inside the ASTERFS system. The inode_nr of the file is the same as the index in
  // the system.
  index++;
  found = 0;
  for (i=1; i!=ASTERFS_MAX_FILES; i++) {
    // This entry is empty
    if (ASTERFS_nodes[i].type == 0) continue;

    // This this entry in the correct directory?
    if (ASTERFS_nodes[i].parent_inode_nr == dirnode->inode_nr) {
      // An entry is found. Decrease index counter
      index--;
//      printf ("Found file in dir dec index to %d\n", index);
      if (index == 0) {
        found = i;
        break;
      }
    }
  }

  // No file found
  if (! found) return NULL;

  printf ("Found index: %d\n", found);

  ASTERFS_rd_dirent.inode_nr = index;
  strcpy (ASTERFS_rd_dirent.name, ASTERFS_nodes[found].name);

  return &ASTERFS_rd_dirent;
}


/**
 * Return directory entry 'name' (basically associative dir seek)
 */
vfs_node_t *ASTERFS_finddir (vfs_node_t *dirnode, const char *name) {
  uint32_t index, i;

//  printf ("ASTERFS_finddir (%s, %s);\n", dirnode->name, name);

  // Get the directory ID where this file resides in
  index = 0;
  uint32_t parent_inode = ASTERFS_nodes[dirnode->inode_nr].inode_nr;
  for (i=1; i!=ASTERFS_MAX_FILES; i++) {
    // This entry is empty
    if (ASTERFS_nodes[i].type == 0) continue;

    // If not in the same directory, skip
    if (ASTERFS_nodes[i].parent_inode_nr != parent_inode) continue;

    // Is this the correct file?
    if (strcmp (ASTERFS_nodes[i].name, name) == 0) {
      index = i;    // Save this index
      break;
    }
  }

  // Nothing found ?
  if (index == 0) return NULL;

  // Fill VFS structure with ASTERFS information
  ASTERFS_fd_node.inode_nr = index;
  strcpy (ASTERFS_fd_node.name, ASTERFS_nodes[index].name);
  ASTERFS_fd_node.owner = 0;
  ASTERFS_fd_node.length = ASTERFS_nodes[index].length;
  ASTERFS_fd_node.major_num = ASTERFS_fd_node.minor_num = 0;
  ASTERFS_fd_node.major_num = ASTERFS_nodes[index].device_major_num;
  ASTERFS_fd_node.minor_num = ASTERFS_nodes[index].device_minor_num;
  switch (ASTERFS_nodes[index].type) {
    case ASTERFS_TYPE_FILE :
                            ASTERFS_fd_node.flags = FS_FILE;
                            break;
    case ASTERFS_TYPE_DIRECTORY :
                            ASTERFS_fd_node.flags = FS_DIRECTORY;
                            break;
    case ASTERFS_TYPE_BLOCK_DEV :
                            ASTERFS_fd_node.flags = FS_BLOCKDEVICE;
                            break;
    case ASTERFS_TYPE_CHAR_DEV :
                            ASTERFS_fd_node.flags = FS_CHARDEVICE;
                            break;
  }

  // Copy the mount information from the parent
  ASTERFS_fd_node.fileops = &ASTERFS_fileops;   // Everything here is handled by ASTERFS of course

  // Return structure
  return &ASTERFS_fd_node;
}

/**
 *
 */
vfs_node_t *ASTERFS_mount (struct vfs_mount *mount, device_t *dev, const char *path) {
  printf ("\n*** Mounting ASTERFS_mount on %s (chroot from '%s') \n", mount->mount, path);

  // Return root node for this system
  if (strcmp (path, "/") == 0) {
    printf ("ASTERFS: Returning root node.\n");
    return &ASTERFS_supernode;
  }

  // Return directory node
  // @TODO: Return another node when we are not mounted to the root
  printf ("ASTERFS: Returning directory node.\n");
  return &ASTERFS_supernode;
}

/**
 *
 */
void ASTERFS_umount (struct vfs_mount *mount) {
  printf ("\n*** Unmounting ASTERFS_mount from %s\n", mount->mount);
}

