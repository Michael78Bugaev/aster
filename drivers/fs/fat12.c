#include <config.h>
#include <cpu/mem.h>
#include <fs/vfs.h>
#include <fs/fat12.h>
// #include "drivers/floppy.h"
#include <drv/device.h>

// File operations
static struct vfs_fileops fat12_fileops = {
    .read = fat12_read, .write = fat12_write,
    .open = fat12_open, .close = fat12_close,
    .readdir = fat12_readdir, .finddir = fat12_finddir
};

// Mount operations
static struct vfs_mountops fat12_mountops = {
    .mount = fat12_mount, .umount = fat12_umount
};

// Global structure
static vfs_info_t fat12_vfs_info = { .tag = "fat12",
                                     .name = "FAT12 File System",
                                     .mountops = &fat12_mountops
                                   };

// Root supernode
static vfs_node_t fat12_supernode = {
  .inode_nr = 0,
  .name = "/",
  .owner = 0,
  .length = 0,
  .flags = FS_DIRECTORY,
  .major_num = 0,
  .minor_num = 0,
  .fileops = &fat12_fileops,
  .mount = NULL,
};

// @TODO: We run into trouble when we mount separate ext2 systems???
vfs_dirent_t dirent;   // Entry that gets returned by fat12_readdir
vfs_node_t filenode;   // Entry that gets returned by fat12_finddir

// Forward defines
void fat12_convert_dos_to_c_filename (char *longName, const char *dosName);
void fat12_convert_c_to_dos_filename (const char *longName, char *dosName);
uint16_t fat12_get_next_cluster (fat12_fat_t *fat, uint16_t cluster);



/**
 * Called when a device that holds a FAT12 image gets mounted onto a mount_point
 */
vfs_node_t *fat12_mount (struct vfs_mount *mount, device_t *dev, const char *path) {
//  printf ("fat12_mount\n");

  // Allocate fat12_info for this mount
  mount->fs_data = malloc (sizeof (fat12_fatinfo_t));
  fat12_fatinfo_t *fat12_info = mount->fs_data; // Alias for easier usage
  if (!fat12_info) goto cleanup;
  memset (fat12_info, 0, sizeof (fat12_fatinfo_t));

  // Allocate and read BPB
  fat12_info->bpb = (fat12_bpb_t *)malloc (sizeof (fat12_bpb_t));
  if (!fat12_info->bpb) goto cleanup;
  memset (fat12_info->bpb, 0, sizeof (fat12_bpb_t));

  // Read boot sector from disk
  int rb = mount->dev->read (mount->dev->major_num, mount->dev->minor_num, 0, 512, (char *)fat12_info->bpb);
  if (rb != 512) {
    printf ("Cannot read enough bytes from boot sector (%d read, %d needed)\n", rb, fat12_info->bpb->BytesPerSector);
    goto cleanup;
  }

/*
  printf ("OEMName           %c%c%c%c%c%c%c%c\n", fat12_info->bpb->OEMName[0],fat12_info->bpb->OEMName[1],fat12_info->bpb->OEMName[2],fat12_info->bpb->OEMName[3],fat12_info->bpb->OEMName[4],fat12_info->bpb->OEMName[5],fat12_info->bpb->OEMName[6],fat12_info->bpb->OEMName[7]);
  printf ("BytesPerSector    %04x\n", fat12_info->bpb->BytesPerSector);
  printf ("SectorsPerCluster %02x\n", fat12_info->bpb->SectorsPerCluster);
  printf ("ReservedSectors   %04x\n", fat12_info->bpb->ReservedSectors);
  printf ("NumberOfFats      %02x\n", fat12_info->bpb->NumberOfFats);
  printf ("NumDirEntries     %04x\n", fat12_info->bpb->NumDirEntries);
  printf ("NumSectors        %04x\n", fat12_info->bpb->NumSectors);
  printf ("Media             %02x\n", fat12_info->bpb->Media);
  printf ("SectorsPerFat     %04x\n", fat12_info->bpb->SectorsPerFat);
  printf ("SectorsPerTrack   %04x\n", fat12_info->bpb->SectorsPerTrack);
  printf ("HeadsPerCyl       %04x\n", fat12_info->bpb->HeadsPerCyl);
  printf ("HiddenSectors     %08x\n", fat12_info->bpb->HiddenSectors);
  printf ("LongSectors       %08x\n", fat12_info->bpb->LongSectors);
*/

  // Set global FAT information as read and calculated from the bpb
  fat12_info->numSectors              = fat12_info->bpb->NumSectors;
  fat12_info->fatOffset               = fat12_info->bpb->ReservedSectors + fat12_info->bpb->HiddenSectors;
  fat12_info->fatSizeBytes            = fat12_info->bpb->BytesPerSector * fat12_info->bpb->SectorsPerFat;
  fat12_info->fatSizeSectors          = fat12_info->bpb->SectorsPerFat;
  fat12_info->fatEntrySizeBits        = 8;
  fat12_info->numRootEntries          = fat12_info->bpb->NumDirEntries;
  fat12_info->numRootEntriesPerSector = fat12_info->bpb->BytesPerSector / 32;
  fat12_info->rootOffset              = (fat12_info->bpb->NumberOfFats * fat12_info->bpb->SectorsPerFat) + 1;
  fat12_info->rootSizeSectors         = (fat12_info->bpb->NumDirEntries * 32 ) / fat12_info->bpb->BytesPerSector;
  fat12_info->dataOffset              = fat12_info->rootOffset + fat12_info->rootSizeSectors - 2;

/*
  printf ("FAT12INFO\n");
  printf ("numSectors        %04X\n", fat12_info->numSectors);
  printf ("fatOffset         %04X\n", fat12_info->fatOffset);
  printf ("fatSizeBytes      %04X\n", fat12_info->fatSizeBytes);
  printf ("fatSizeSectors    %04X\n", fat12_info->fatSizeSectors);
  printf ("fatEntrySizeBits  %04X\n", fat12_info->fatEntrySizeBits);
  printf ("numRootEntries    %04X\n", fat12_info->numRootEntries);
  printf ("numRootEntriesPS  %04X\n", fat12_info->numRootEntriesPerSector);
  printf ("rootOffset        %04X\n", fat12_info->rootOffset);
  printf ("rootSizeSectors   %04X\n", fat12_info->rootSizeSectors);
  printf ("dataOffset        %04X\n", fat12_info->dataOffset);
  printf ("----------------\n");
*/

  // Read (primary) FAT table
  int i;
  fat12_info->fat = (fat12_fat_t *)malloc (fat12_info->fatSizeBytes);
  if (!fat12_info->fat) goto cleanup;

  memset (fat12_info->fat, 0, fat12_info->fatSizeBytes);

  for (i=0; i!=fat12_info->fatSizeSectors; i++) {
    uint32_t offset = (fat12_info->fatOffset+i) * fat12_info->bpb->BytesPerSector;
    int rb = mount->dev->read (mount->dev->major_num, mount->dev->minor_num, offset, fat12_info->bpb->BytesPerSector, (char *)fat12_info->fat+(i*512));
    if (rb != fat12_info->bpb->BytesPerSector) {
      printf ("Cannot read fat sector %d\n", i);
      goto cleanup;
    }
  }

/*
  int k,j;
  for (k=0, i=0; i!=20; i++) {
    printf ("  Entry %04d  : ", i*10);
    for (j=0; j!=10; j++,k++) printf ("%04X ", fat12_get_next_cluster (fat12_info->fat, k));
    printf ("\n");
  }
  printf ("\n");
*/

/*
  fat12_dirent_t *direntbuf = (fat12_dirent_t *)malloc (_bpb->BytesPerSector);
  fat12_dirent_t *dirent;
  for (i=0; i!=fat12_info->rootSizeSectors; i++) {
    uint32_t offset = (fat12_info->rootOffset+i) * fat12_info->bpb->BytesPerSector;
//@TODO    node->block->read (node->block->major, node->block->minor, offset, fat12_info->bpb->BytesPerSector, direntbuf);

    for (dirent = direntbuf, j=0; j!=fat12_info->numRootEntriesPerSector; j++,dirent++) {
      printf ("-------------\n");
      printf ("Filename     : %c%c%c%c%c%c%c%c\n", dirent->Filename[0], dirent->Filename[1], dirent->Filename[2], dirent->Filename[3], dirent->Filename[4], dirent->Filename[5], dirent->Filename[6], dirent->Filename[7]);
      printf ("Ext          : %c%c%c\n", dirent->Ext[0], dirent->Ext[1], dirent->Ext[2]);
      printf ("Attrib       : %02X\n", dirent->Attrib);
      printf ("FirstCluster : %04X\n", dirent->FirstCluster);
      printf ("FileSize     : %02X\n", dirent->FileSize);
    }
  }
*/

//  printf ("Returning FAT12's supernode\n");
  return &fat12_supernode;


  // Jumped to when error during mount
cleanup:
  if (fat12_info && fat12_info->fat) mfree (fat12_info->fat);
  if (fat12_info && fat12_info->bpb) mfree (fat12_info->bpb);
  if (fat12_info) mfree (fat12_info);
  return NULL;
}

/**
 * Called when a mount_point gets unmounted
 */
void fat12_umount (struct vfs_mount *mount) {
//  printf ("Unmounting fat12_info structure");
  // Free our fs_data
  fat12_fatinfo_t *fat12_info = (fat12_fatinfo_t *)mount->fs_data;

  mfree (fat12_info->fat);
  mfree (fat12_info->bpb);
  mfree (fat12_info);
}



/**
 *
 */
uint32_t fat12_read (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer) {
  fat12_fatinfo_t *fat12_info = node->mount->fs_data; // Alias for easier usage
  char *buf_ptr = buffer;
  int count = size;
  int tmp;

//  printf ("Reading inode: %d  Offset: %d  Size: %d\n", node->inode_nr, offset, size);

  // We do need nothing to read
  if (size == 0) return 0;

  // Cannot read behind file length
  if (offset > node->length) return 0;

  // We can only read X amount of bytes, so adjust maximum size
  if (offset + size > node->length) {
    printf ("Truncated size from %d\n", size);
    size = node->length - offset;
    printf ("to %d\n", size);
  }

  // Find the starting cluster we need to read from
  int cluster_size = fat12_info->bpb->SectorsPerCluster * fat12_info->bpb->BytesPerSector;
  if (cluster_size != 512) printf ("cluster size needs to be 512!");

  int skip_cluster_count = offset / cluster_size;
  int cluster_offset = offset % cluster_size;
//  printf ("We need to skip the first %d clusters\n", skip_cluster_count);

  /* Find start cluster and skip X amount of clusters to find the correct position in the
   * file */
  uint16_t cluster = node->inode_nr;
  while (skip_cluster_count > 0) {
//    printf ("Pre skip: %02X\n", cluster);
    cluster = fat12_get_next_cluster (fat12_info->fat, cluster);
//    printf ("Post skip: %02X\n", cluster);
    skip_cluster_count--;
  }


  uint32_t disk_offset = (fat12_info->dataOffset+cluster) * fat12_info->bpb->SectorsPerCluster * fat12_info->bpb->BytesPerSector;
//  printf ("Start cluster  : %08X\n", cluster);
//  printf ("Disk offset    : %08X\n", disk_offset);
//  printf ("Cluster offset : %08X\n", cluster_offset);


  // Read partial cluster
  if (size + cluster_offset < 512) {
//    printf ("Reading partial cluster\n");
    // We do not cross a sector, only 1 sector is needed
    node->mount->dev->read (node->mount->dev->major_num, node->mount->dev->minor_num, disk_offset + cluster_offset, size, buffer);
//    printf ("Returing %d bytes read\n", size);
    return size;
  }


  // read intial half block
  if (cluster_offset > 0) {
//    printf ("Reading intial half cluster\n");
    tmp = (cluster_size - cluster_offset);
    disk_offset += 512-tmp;
    node->mount->dev->read (node->mount->dev->major_num, node->mount->dev->minor_num, disk_offset, tmp, buf_ptr);

    disk_offset += tmp;
    buf_ptr += tmp;
    count -= tmp;
  }

  // read whole blocks
  while (count > 512) {
//    printf ("Reading whole cluster\n");
    node->mount->dev->read (node->mount->dev->major_num, node->mount->dev->minor_num, disk_offset, 512, buf_ptr);

    disk_offset += 512;
    buf_ptr += 512;
    count -= 512;
  }

  // read last half block
  if (count > 0) {
//    printf ("Reading final partial cluster\n");
    tmp = count;

    node->mount->dev->read (node->mount->dev->major_num, node->mount->dev->minor_num, disk_offset, tmp, buf_ptr);

    disk_offset += tmp;
    buf_ptr += tmp;
    count -= tmp;
  }


//  printf ("Returing %d bytes read\n", size);
  return size;
}

/**
 *
 */
uint32_t fat12_write (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer) {
  return 0;
//  return node->block->write (node->block->major, node->block->minor, offset, size, buffer);
}

/**
 *
 */
void fat12_open (vfs_node_t *node) {
//  node->block->open (node->block->major, node->block->minor);
}

/**
 *
 */
void fat12_close (vfs_node_t *node) {
//  node->block->close (node->block->major, node->block->minor);
}


/**
 *
 */
vfs_dirent_t *fat12_readdir (vfs_node_t *node, uint32_t index) {
  int i;

  fat12_fatinfo_t *fat12_info = node->mount->fs_data; // Alias for easier usage

//  printf ("fat12_readdir\n");

  // Check if it's a directory
  if ((node->flags & 0x7) != FS_DIRECTORY) return NULL;

  // Every sector holds this many directories
  int dirsPerSector = fat12_info->bpb->BytesPerSector / 32;
  int sectorNeeded = index / dirsPerSector;   // Sector we need (@TODO: problem when sector != cluster)
  int indexNeeded = index % dirsPerSector;    // N'th entry in this sector needed

  // Create entry that holds 1 sector
  fat12_dirent_t *direntbuf = (fat12_dirent_t *)malloc (fat12_info->bpb->BytesPerSector);
//  printf ("INODE_NR: %d %d\n", node->inode_nr, index);

  // Do we need to read the root directory?
  if (node->inode_nr == 0) {
//    printf ("Root read\n");
    uint32_t offset = (fat12_info->rootOffset+sectorNeeded) * fat12_info->bpb->BytesPerSector;
//    printf ("Offset: %08X\n", offset);
    node->mount->dev->read (node->mount->dev->major_num, node->mount->dev->minor_num, offset, fat12_info->bpb->BytesPerSector, (char *)direntbuf);
  } else {
//    printf ("Cluster read read\n");
    // First start cluster (which is the INODE number :P), and seek N'th cluster
    uint16_t cluster = node->inode_nr;
    for (i=0; i!=sectorNeeded; i++) {
      cluster = fat12_get_next_cluster (fat12_info->fat, cluster);
    }

    // Read this cluster
    uint32_t offset = (fat12_info->dataOffset+cluster) * fat12_info->bpb->BytesPerSector;
//    printf ("Offset: %08X\n", offset);
    node->mount->dev->read (node->mount->dev->major_num, node->mount->dev->minor_num, offset, fat12_info->bpb->BytesPerSector, (char *)direntbuf);
  }

  // Seek correcy entry
  fat12_dirent_t *direntbufptr = direntbuf;
  for (i=0; i!=indexNeeded; i++) {
//    printf ("i: %d\n", i);
    direntbufptr++;
    //direntbufptr += sizeof (fat12_dirent_t);
//    printf ("DBP: %08X\n", direntbufptr);
  }

//  printf ("%c %c %c %c\n", direntbufptr->Filename[0], direntbufptr->Filename[1], direntbufptr->Filename[2], direntbufptr->Filename[3]);

  // No more entries when first char of filename is 0
  if (direntbufptr->Filename[0] == 0) {
    mfree (direntbuf);
    return NULL;
  }

  // Create returning value
  fat12_convert_dos_to_c_filename (dirent.name, (char *)direntbufptr->Filename);
  dirent.inode_nr = direntbufptr->FirstCluster;

//printf ("dirent.name: '%s'\n", dirent.name);
  mfree (direntbuf);
  return &dirent;
}


/**
 * Returns:
 *   0 = end of buffer, need more data
 *   1 = found item. filenode is filled
 *   2 = no more entries
 */
int fat12_parse_directory_sector (fat12_dirent_t *direntbuf, vfs_node_t *node, const char *dosName) {
  fat12_fatinfo_t *fat12_info = node->mount->fs_data; // Alias for easier usage
//  printf ("fat12_parse_directory_sector\n");
  fat12_dirent_t *direntbufptr = direntbuf;
  int j;

  // Browse all entries in the sector
  for (j=0; j!=fat12_info->numRootEntriesPerSector; j++, direntbufptr++) {
    // No more entries when first char of filename is 0
    if (direntbufptr->Filename[0] == 0) return 2;

//    printf ("Equal: '%s' and '%s' \n", dosName, direntbufptr->Filename);

    // Is this the correct file?
    if (strncmp (dosName, (char *)direntbufptr->Filename, 11) == 0) {
      // Copy parent data (so we don't have to set functions etc)
      memcpy (&filenode, node, sizeof (vfs_node_t));

      // Create returning value
      filenode.inode_nr = direntbufptr->FirstCluster;
      fat12_convert_dos_to_c_filename (filenode.name, (char *)direntbufptr->Filename);
      filenode.owner = 0;
      filenode.length = direntbufptr->FileSize;
      filenode.flags = (direntbufptr->Attrib == 0x10) ? FS_DIRECTORY : FS_FILE;

//      printf ("INODE : %d\n", filenode.inode_nr);

      return 1;
    }
  } // for entries

  // All entries parsed, more entries probably available and not yet found
  return 0;
}


/**
 *
 */
vfs_node_t *fat12_finddir (vfs_node_t *node, const char *name) {
  fat12_fatinfo_t *fat12_info = node->mount->fs_data; // Alias for easier usage
  char dosName[12];
  int i;

  // Check if it's a directory
  if ((node->flags & 0x7) != FS_DIRECTORY) return NULL;

  // Convert filename to FAT name
  fat12_convert_c_to_dos_filename (name, dosName);

  // Allocate buffer memory for directory entry
  fat12_dirent_t *direntbuf = (fat12_dirent_t *)malloc (fat12_info->bpb->BytesPerSector);

  // Do we need to read the root directory?
  if (node->inode_nr == 0) {
    // Read all root directory sectors until we reach the correct one or end of sectors

    for (i=0; i!=fat12_info->rootSizeSectors; i++) {
      // Read directory sector
      uint32_t offset = (fat12_info->rootOffset+i) * fat12_info->bpb->BytesPerSector;
      node->mount->dev->read (node->mount->dev->major_num, node->mount->dev->minor_num, offset, fat12_info->bpb->BytesPerSector, (char *)direntbuf);

      int ret = fat12_parse_directory_sector (direntbuf, node, dosName);
      switch (ret) {
        case 0 :
                  // More directories needed, read next sector
                  break;
        case 1 :
                  // Directory found
                  mfree (direntbuf);
                  return &filenode;
        case 2 :
                  // No directory found and end of buffer
                  mfree (direntbuf);
                  return NULL;
      }
    } // for sectors

  } else {
    // Read 'normal' subdirectory
    uint16_t cluster = node->inode_nr;

    // Do as long as we have file entries (always padded on sector which is always divved by 512)
    do {
      // read 1 sector at a time
      uint32_t offset = (fat12_info->dataOffset+cluster) * fat12_info->bpb->BytesPerSector;
      node->mount->dev->read (node->mount->dev->major_num, node->mount->dev->minor_num, offset, fat12_info->bpb->BytesPerSector, (char *)direntbuf);

      int ret = fat12_parse_directory_sector (direntbuf, node, dosName);
      switch (ret) {
        case 0 :
                  // More directories needed, read next sector
                  break;
        case 1 :
                  // Directory found
                  mfree (direntbuf);
                  return &filenode;
        case 2 :
                  // No directory found and end of buffer
                  mfree (direntbuf);
                  return NULL;
      }

      // Fetch next cluster from file
      cluster = fat12_get_next_cluster (fat12_info->fat, cluster);
      // Repeat until we hit end of cluster list
    } while (cluster > 0x002 && cluster <= 0xFF7);
  }

  mfree (direntbuf);
  return NULL;
}



/**
 * Converts dosname '123456  .123' to longname (123456.123)
 */
void fat12_convert_dos_to_c_filename (char *longName, const char *dosName) {
  int i = 0;
  int j = 0;

  // Add chars from filename
  while (i < 8 && dosName[i] != ' ') longName[j++] = dosName[i++];

  // Add extension if any is available
  if (dosName[8] != ' ') {
    longName[j++] = '.';

    i=8;
    while (i < 11 && dosName[i] != ' ') longName[j++] = dosName[i++];
  }

  // Add termination string
  longName[j] = 0;
}


/**
 * Convert a long filename to a FAT-compatible name (8.3 chars)
 * Does:
 *  - get the first 8 chars (max) from filename
 *  - get the first 3 chars (max) from extension (if any)
 *  - support dot-files
 *  - return \0-terminated string and space-padding
 * Does not:
 *  - uppercase files
 *  - change long filenames into '~n' format
 */
void fat12_convert_c_to_dos_filename (const char *longName, char *dosName) {
  int i;

  // C-terminated string with 11 spaces
  for (i=0; i!=11; i++) dosName[i] = ' ';
  dosName[11] = 0;

  if (strcmp (longName, ".") == 0) {
    dosName[0] = '.';
    return;
  }

  if (strcmp (longName, "..") == 0) {
    dosName[0] = '.';
    dosName[1] = '.';
    return;
  }

  // Find offset of the extension
  i=0;
  char *extension = NULL;
  while (longName[i] && longName[i] != '.') i++;
  if (longName[i] == '.') extension = (char *)(longName+i);

  // Split filename and extension
  if (extension) {
    *extension = 0; // Split string,  filename and extension are now 2 separated c-strings
    extension++;

    /* The extension starts from the beginning. This means this file is a dot-file so there
     * is no extension, only a filename */
    if (extension == longName+1) extension = 0;
  }

  // Copy filename (max 8 chars)
  i=0;
  while (longName[i] && i < 8) {
    dosName[0+i] = longName[i];
    i++;
  }

  // Copy extension to correct spot if any (max 3 chars)
  if (extension) {
    i=0;
    while (extension[i] && i < 3) {
      dosName[8+i] = extension[i];
      i++;
    }
  }
}


/**
 * Initialises the FAT12 on current drive
 */
void fat12_init (void) {
  // Register filesystem to the VFS
  vfs_register_filesystem (&fat12_vfs_info);
}


/**
 * Finds the next cluster inside the FAT table
 */
uint16_t fat12_get_next_cluster (fat12_fat_t *fat, uint16_t cluster) {
  uint32_t offset = cluster + (cluster / 2);

  uint16_t *tmp = (uint16_t *)((char *)fat + offset);
  uint16_t next_cluster = *tmp;

  if ( (cluster & 1) == 0) {
    next_cluster &= 0x0FFF;
  } else {
    next_cluster >>= 4;
  }

  return next_cluster;
}