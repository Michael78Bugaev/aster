#include <config.h>
#include <cpu/mem.h>
#include <stdint.h>
#include <drv/device.h>
#include <fs/vfs.h>
#include <fs/asterfs.h>


vfs_mount_t vfs_mount_table[VFS_MAX_MOUNTS];    // Mount table with all mount points (@TODO: dynamically allocated or linkedlist)
vfs_system_t vfs_systems[VFS_MAX_FILESYSTEMS];  // There will be a maximum of 100 different filesystems that can be loaded (@TODO: linkedlist or dynamically allocation)


/**
 * Returns the mount from the path (path is formatted like MOUNT:PATH), the path
 * suffix is returned in **path or points to NULL when the mount is not found.
 */
vfs_mount_t *vsf_get_mount (const char *full_path, char **path) {
  char mount[11];
  int i;

  // Scan string for : or end of string
  int len=0;
  while (full_path[len] != ':' && full_path[len] != '\0') len++;

  // Did we find a ':'?
  if (full_path[len] == '\0') return NULL;   // No : found

  // Copy mount
  strncpy ((char *)&mount, full_path, len);

  // Set path to
  *path = (char *)(full_path+len+1);

  // Browse all mounts to see if it's available
  for (i=0; i!=VFS_MAX_MOUNTS; i++) {
    if (! vfs_mount_table[i].enabled) continue;
    if (strcmp (vfs_mount_table[i].mount, mount) == 0) return &vfs_mount_table[i];
  }

  // Mount not fond
  *path = NULL;
  return NULL;
}



/**
 *
 */
uint32_t vfs_read (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer) {
  // Check if it's a directory
  if ((node->flags & 0x7) != FS_FILE) return 0;

  if (! node->fileops || ! node->fileops->read) return NULL;
  return node->fileops->read (node, offset, size, buffer);
}

/**
 *
 */
uint32_t vfs_write (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer) {
  if (! node->fileops || ! node->fileops->write) return 0;
  return node->fileops->write (node, offset, size, buffer);
}

/**
 *
 */
void vfs_open(vfs_node_t *node) {
  if (! node->fileops || ! node->fileops->open) return;
  node->fileops->open (node);
}

/**
 *
 */
void vfs_close(vfs_node_t *node) {
  if (! node->fileops || ! node->fileops->close) return;
  node->fileops->close (node);
}

/**
 *
 */
int vfs_readdir(vfs_node_t *node, uint32_t index, struct dirent *target_dirent) {
//  printf ("vfs_readdir (%s, %d) : ", node->name, index);

  // Check if it's a directory
  if ((node->flags & 0x7) != FS_DIRECTORY) {
//    printf ("Not a dir\n");
    return NULL;
  }

  if (! node->fileops || ! node->fileops->readdir) {
//    printf ("Not fileops or fileops->readdir\n");
    return NULL;
  }
  return node->fileops->readdir (node, index, target_dirent);
}

/**
 *
 */
int vfs_finddir (vfs_node_t *node, const char *name, vfs_node_t *target_node) {
  // Check if it's a directory
  if ((node->flags & 0x7) != FS_DIRECTORY) return NULL;

  if (! node->fileops || ! node->fileops->finddir) return NULL;
  return node->fileops->finddir (node, name, target_node);
}

/**
 *
 */
void vfs_mknod (struct vfs_node *node, const char *name, char device_type, uint8_t major_node, uint8_t minor_node) {
//  printf ("vfs_mknod\n");
  // Check if it's a directory
  if ((node->flags & 0x7) != FS_DIRECTORY) return;

//  printf ("vfs_mknod: 1\n");

  if (! node->fileops || ! node->fileops->mknod) return;

//  printf ("vfs_mknod: 2\n");
  return node->fileops->mknod (node, name, device_type, major_node, minor_node);
}


/**
 * Return 1 when filesystem is registered (FAT12, CYBFS etc). 0 otherwise
 */
int vfs_is_registered (const char *tag) {
  int i;

  // Scan all fs slots
  for (i=0; i!=VFS_MAX_FILESYSTEMS; i++) {
    // Enabled and tag matched? we've got a match
    if (vfs_systems[i].enabled && strcmp (tag, vfs_systems[i].info.tag) == 0) return 1;
  }

  // Not found
  return 0;
}

/**
 * Registers a new filesystem to the VFS
 */
int vfs_register_filesystem (vfs_info_t *info) {
  int i;

  // Scan all fs slots
  for (i=0; i!=VFS_MAX_FILESYSTEMS; i++) {
    // Already enabled, try next slot
    if (vfs_systems[i].enabled) continue;

    vfs_systems[i].enabled = 1;
    strncpy (&vfs_systems[i].info, info, sizeof (vfs_info_t));
    vfs_systems[i].mount_count = 0;
    return 1;
  }
  printf("No more virtual filesystem slots available\n");
  // No more room :(
  return 0;
}

/**
 * Unregisters a filesystem by tagname
 */
int vfs_unregister_filesystem (const char *tag) {
  int i;

  /* @TODO: see if we have mountpoints present that uses this filesystem. If so, we cannot
   * disable this filesystem */

  for (i=0; i!=VFS_MAX_FILESYSTEMS; i++) {
    if (vfs_systems[i].enabled && strcmp (tag, vfs_systems[i].info.tag) == 0) {
      // System is still mounted at least 1 time
      if (vfs_systems[i].mount_count > 0) return 0;

      // disable filesystem. This is now a free slot again..
      vfs_systems[i].enabled = 0;
      return 1;
    }
  }

  // Nothing found
  return 0;
}

/**
 *
 */
vfs_system_t *vfs_get_vfs_system (const char *tag) {
  int i;

  // Browse all registered file systems
  for (i=0; i!=VFS_MAX_FILESYSTEMS; i++) {
    // Is it taken? (ie: tag is not empty)
    if (vfs_systems[i].enabled && strcmp (tag, vfs_systems[i].info.tag) == 0) {
      return &vfs_systems[i];
    }
  }

  // Not found
  return NULL;
}

/**
 *
 */
void vfs_init (void) {
  // Clear all fs slot data
  memset (vfs_systems, 0, sizeof (vfs_systems));

  // Clear all mount tables
  memset (vfs_mount_table, 0, sizeof (vfs_mount_table));

  //vfs_create_root_node ();
}


/**
 *
 */
vfs_mount_t *vfs_get_mount_from_path (const char *path) {
  char *path_suffix;
  vfs_mount_t *mount = vsf_get_mount (path, &path_suffix);
  if (mount == NULL) return NULL;

  return mount;
}

/**
 *
 */
int vfs_get_node_from_path (const char *path, vfs_node_t *node) {
  char component[255];
  int cs;

//  printf ("vfs_get_node_from_path('%s')\n", path);

  // Lookup correct node or return NULL when not found
  char *path_suffix;
  vfs_mount_t *mount = vsf_get_mount (path, &path_suffix);
  if (mount == NULL) return NULL;


//  printf ("VFS_GNFP Mount: '%s'\n", mount->mount);
//  printf ("PATH: '%s'\n", path_suffix);

  // This is a absolute path, start from root node (don't care
  char *c = path_suffix;
  if (*c != '/') return NULL;   // Must be absolute

  // Start with the supernode (the only in-memory node needed)
  memcpy (node, mount->supernode, sizeof(vfs_node_t));
  node->mount = mount;  // @TODO Is this needed? Not already present in supernode?

  c++;

  /* *c points to the first directory/file AFTER the '/' and node is the supernode.
   * Start browsing all directories and move to that node
   */

  // From this point, it's relative again
  while (*c) {
    if (*c == '/') c++;   // Skip directory separator if one is found (including root separator)

    /* node MUST be a directory, since the next thing we read is a directory component
     * and we still have something left on the path */
    if ((node->flags & 0x7) != FS_DIRECTORY) {
//      printf ("component is not a directory\n\n");
      return NULL;
    }

    // Find next directory component (or to end of string)
    for (cs = 0 ; *(c+cs) != '/' && *(c+cs) != 0; cs++);

    // Separate this directory component
    strncpy (component, c, cs);

    // Increase to next entry (starting at the directory separator)
    c+=cs;

//    printf ("Next component: '%s' (from inode %d)\n", component, node->inode_nr);

    // Dot directory, skip since we stay on the same node
    if (strcmp (component, ".") == 0) continue;   // We could vfs_finddir deal with it, or do it ourself, we are faster

    // DotDot directory, go back to the parent
    if (strcmp (component, "..") == 0) {
      // @TODO: make sure parent works
      printf ("parent wanted...\n");
      continue;
    }

    // Find the entry if it exists
    vfs_node_t new_node;
    if (! vfs_finddir (node, component, &new_node)) {
//      printf ("component not found\n\n");
      return NULL;   // Cannot find node... error :(
    }
    memcpy(node, &new_node, sizeof(vfs_node_t));
  }

//  printf ("All done.. returning vfs inode: %d\n\n", cur_node->inode_nr);
  return 1;
}




/**
 *
 */
int sys_umount (const char *mount_point) {
  printf ("Unmounting %s \n", mount_point);

  // if (vfs_mount_table[i].vfs_info.ref_count == 0) printf ("Something is wrong. We have a mounted system but the vfs_systems table says it's not mounted. halting.");
  // vfs_mount_table[i].vfs_info.ref_count--;

  return 0;
}


/**
 *
 */
int sys_mount (const char *device_path, const char *fs_type, const char *mount, const char *path, int mount_options) {
  device_t *dev_ptr = NULL;
  int i;

  // Find the filesystem itself (is it registered)
  vfs_system_t *fs_system = vfs_get_vfs_system (fs_type);
  if (! fs_system) { 
    printf ("Error: filesystem %s not found\n", fs_type);
    return 0; // Cannot find registered file system
  }

  // Check if mount is already mounted
  if (! (mount_options & MOUNTOPTION_REMOUNT)) {
    for (i=0; i!=VFS_MAX_MOUNTS; i++) {
      if (! vfs_mount_table[i].enabled) continue;
      if (strcmp(vfs_mount_table[i].mount, mount) == 0) return 0; // Mount already mounted
    }
  }

  // Some filesystems do not have a device (cybfs, devfs etc)
  if (device_path != NULL) {
    // Check device
    vfs_node_t dev_node;
    if (! vfs_get_node_from_path (device_path, &dev_node)) {  printf("Path not found\n"); return 0; } // Path not found

    // Cannot mount device if it's not a block device
    if ((dev_node.flags & 0x7) != FS_BLOCKDEVICE) {  printf("It is not a block device\n"); return 0; }

    dev_ptr = device_get_device (dev_node.major_num, dev_node.minor_num);
    if (! dev_ptr) {  printf("Can't find\n"); return 0; }    // Cannot find the device registered to this file
  }

  // Browse mount table, find first free slot
  for (i=0; i!=VFS_MAX_MOUNTS; i++) {

    if (! (mount_options & MOUNTOPTION_REMOUNT)) {
      // Continue when this slot already taken
      if (vfs_mount_table[i].enabled) continue;
    } else {
      // Continue when this slot not free and it's not the correct mount
      if (vfs_mount_table[i].enabled && strcmp (vfs_mount_table[i].mount, mount) != 0) continue;
    }

    // Populate mount_table info
    vfs_mount_table[i].dev = dev_ptr;
    vfs_mount_table[i].system = fs_system;
    vfs_mount_table[i].system->mount_count++;      // Increase mount count for this filesystem
    printf("Mounting...");
    strncpy (vfs_mount_table[i].mount, mount);
    printf("\nDone!\n");


    // Check if mount function is available
    //if (!vfs_mount_table[i].system->info.mountops || !vfs_mount_table[i].system->info.mountops->mount) {  printf("mount function isn't available\n"); return 0; } 

    // Do some filesystem specific mounting if needed
    vfs_mount_table[i].supernode = vfs_mount_table[i].system->info.mountops->mount (&vfs_mount_table[i], dev_ptr, path);

    // Error while doing fs specific mount init?
    if (! vfs_mount_table[i].supernode) {  printf("fs\n"); return 0; } 

    // @TODO: mutex this.. (@todo: why?)
    vfs_mount_table[i].enabled = 1;


    printf ("This system is currently mounted %d times\n", vfs_mount_table[i].system->mount_count);

    printf ("Mount table:\n");
    for (i=0; i!=VFS_MAX_MOUNTS; i++) {
      if (! vfs_mount_table[i].enabled) continue;
      printf ("%d  %-20s  %08s  %d\n", i, vfs_mount_table[i].mount, vfs_mount_table[i].system->info.tag, vfs_mount_table[i].system->mount_count);
    }


    // Return ok status
    return 1;
  }

  printf("Could not find a free slot\n");
  // Could not find a free slot
  return 0;
}



