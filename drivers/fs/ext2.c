
/******************************************************************************
 *
 *  File        : ext2.c
 *  Description : Ext2 filesystem
 *
 *****************************************************************************/

#include <stdint.h>
#include <cpu/mem.h>
#include <fs/vfs.h>
#include <fs/ext2.h>
// #include "drivers/floppy.h"
#include <drv/ide.h>

// File operations
static struct vfs_fileops ext2_fileops = {
    .read = ext2_read, .write = ext2_write,
    .open = ext2_open, .close = ext2_close,
    .readdir = ext2_readdir, .finddir = ext2_finddir
};

// Mount operations
static struct vfs_mountops ext2_mountops = {
    .mount = ext2_mount, .umount = ext2_umount
};

// Global structure
static vfs_info_t ext2_vfs_info = { .tag = "ext2",
                                    .name = "EXT2 File System",
                                    .mountops = &ext2_mountops
                                  };

// Root supernode (will be overwritten on mounting)
static vfs_node_t ext2_supernode = {
  .inode_nr = EXT2_ROOT_INO,
  .name = "/",
  .owner = 0,
  .length = 0,
  .flags = FS_DIRECTORY,
  .major_num = 0,
  .minor_num = 0,
  .fileops = &ext2_fileops,
  .mount = NULL,
};



/**
 * Converts a block number to LBA sector
 *
 * @param mount
 * @param inode_block
 * @return
 */
uint32_t ext2_block2diskoffset(struct vfs_mount *mount, uint32_t ext2_block) {
  ext2_info_t *ext2_info = mount->fs_data;
//  printf("b2d: %d\n", ext2_block);
  // @TODO: Do we really want to fixate sector size here?
  return ext2_block * ext2_info->sectors_per_block * IDE_SECTOR_SIZE;
}

/**
 * Reads one or more ext2 blocks into memory
 *
 * @param mount
 * @param block_num
 * @param block_count
 * @param buffer
 * @return 0 when something is wrong
 */
int ext2_read_block(struct vfs_mount *mount, int block_num, int block_count, char *buffer) {
  ext2_info_t *ext2_info = mount->fs_data;

  // Convert block inti
  uint32_t offset = ext2_block2diskoffset(mount, block_num);
  uint32_t size = block_count * ext2_info->block_size;
  return (mount->dev->read (mount->dev->major_num, mount->dev->minor_num, offset, size, buffer) == size);
}

/**
 * Allocates and loads a block.
 *
 * NOTE, you need to free this block yourself
 *
 * @param mount
 * @param offset
 * @param size
 * @return
 */
void *ext2_allocate_and_read_block(struct vfs_mount *mount, int block_num, int block_count) {
  ext2_info_t *ext2_info = mount->fs_data;

  // Allocate buffer
  void *buffer = malloc(block_count * ext2_info->block_size);
  if (!buffer) return NULL;
  memset(buffer, 0, block_count * ext2_info->block_size);

  // Read from block
  if (! ext2_read_block(mount, block_num, block_count, buffer)) {
      // Error while reading
      mfree(buffer);
      return NULL;
  }

  return buffer;
}

/**
 * NOTE, you need to free this block yourself
 *
 * @param mount
 * @param offset
 * @param size
 * @return
 */
void *ext2_allocate_and_read_offset(struct vfs_mount *mount, int offset, int size) {
  // Allocate buffer
  void *buffer = malloc(size);
  if (!buffer) return NULL;
  memset(buffer, 0, size);

  // Read direct from offset/size
  if (mount->dev->read (mount->dev->major_num, mount->dev->minor_num, offset, size, buffer) != size) {
      // Error while reading
      mfree(buffer);
      return NULL;
  }

  return buffer;
}

/**
 * Read inode from disk
 *
 * NOTE, you need to free this inode!
 *
 * @param mount
 * @param inode_nr
 * @return
 */
ext2_inode_t *ext2_read_inode(struct vfs_mount *mount, uint32_t inode_nr) {
  ext2_info_t *ext2_info = mount->fs_data;

  // Check if inode number is correct
  if (! inode_nr || inode_nr > ext2_info->superblock->inodeCount) {
    printf ("Incorrect inode number");
    return NULL;
  }

  // Find the blockgroup in which this inode resides
  uint32_t block_group = (inode_nr - 1) / ext2_info->superblock->inodesInGroupCount;
  if (block_group > ext2_info->group_descriptor_count) return NULL;

  // @TODO: Inode size must be checked from superblock (version 1+)

  // We will only read 1 block: the block that has actually got our inode data
  uint32_t inodes_per_block = ext2_info->block_size / 128;
  uint32_t block_offset = ((inode_nr - 1) % ext2_info->superblock->inodesInGroupCount) / inodes_per_block;

  // Find the actual start of inodes in this group
  ext2_blockdescriptor_t *group_descriptor = &ext2_info->block_descriptor[block_group];
  uint32_t inode_block = group_descriptor->inodeTableStart + block_offset;

  // Find the offset of the inode inside the block
  uint32_t inode_index = (((inode_nr - 1) % ext2_info->superblock->inodesInGroupCount) % inodes_per_block) * 128;


  // Read the block that holds our inode
  char *data_block;
  if (! (data_block = ext2_allocate_and_read_block(mount, inode_block, 1))) {
    printf ("Error: could not read inode from device %d:%d", mount->dev->major_num, mount->dev->minor_num);
    return NULL;
  }

  // Read inode entry from block
  ext2_inode_t *inode = (ext2_inode_t *)malloc(sizeof(ext2_inode_t));
  memcpy(inode, (char *)data_block+inode_index, sizeof(ext2_inode_t));

  // Free data block
  mfree(data_block);

  // And return
  return inode;
}










/**
 * Called when a device that holds a EXT2 image gets mounted onto a mount_point
 */
vfs_node_t *ext2_mount (struct vfs_mount *mount, device_t *dev, const char *path) {
  //  printf ("fat12_mount\n");

  // Create infoblock for this mount
  mount->fs_data = malloc (sizeof (ext2_info_t));
  if (!mount->fs_data) goto cleanup;
  ext2_info_t *ext2_info = mount->fs_data;

  // Load superblock
  if (! (ext2_info->superblock = ext2_allocate_and_read_offset(mount, 1024, 1024))) goto cleanup;


  // Check filesystem
  if (ext2_info->superblock->filesystemState == 2) {
    switch(ext2_info->superblock->errorHandling) {
      case 1 :
                printf("Warning: EXT2 filesystem has errors.\n");
                break;
      case 2 :
                printf("Remounting EXT2 filesystem in RO mode (but this is not yet implemented\n");
                break;
      case 3 :
      default :
                printf("EXT2 filesystem has errors\n");
                break;
    }

  }

  // Precalc some values
  ext2_info->block_size = 1024 << ext2_info->superblock->blockSizeLog2;
  ext2_info->group_descriptor_count = ext2_info->superblock->firstDataBlock + 1;
  ext2_info->group_count = ext2_info->superblock->inodeCount / ext2_info->superblock->inodesInGroupCount;
  ext2_info->sectors_per_block = ext2_info->block_size / IDE_SECTOR_SIZE ;
  ext2_info->first_group_start = ext2_info->group_descriptor_count + ext2_info->sectors_per_block;

  // Load other blocks that are needed a lot
  if (! (ext2_info->block_descriptor = ext2_allocate_and_read_offset(mount, EXT2_BLOCKGROUPDESCRIPTOR_BLOCK * ext2_info->block_size, ext2_info->block_size))) goto cleanup;

  // Create and return root node
  ext2_inode_t *inode = ext2_read_inode(mount, EXT2_ROOT_INO);
  ext2_supernode.length = inode->sizeLow;
  ext2_supernode.mount = mount;
  mfree(inode);
  return &ext2_supernode;

cleanup:
  // Things went wrong when we are here. Do a cleanup
  if (ext2_info->block_descriptor) mfree (ext2_info->block_descriptor);
  if (ext2_info->superblock) mfree (ext2_info->superblock);
  if (ext2_info) mfree (ext2_info);
  return NULL;
}


/**
 * Called when a mount_point gets unmounted
 */
void ext2_umount (struct vfs_mount *mount) {
  ext2_info_t *ext2_info = (ext2_info_t *)mount->fs_data;

  // Free up info
  if (ext2_info->superblock) mfree (ext2_info->superblock);
  if (ext2_info->block_descriptor) mfree (ext2_info->block_descriptor);
  mfree (ext2_info);
}



/**
 *
 */
uint32_t ext2_read (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer) {
  return 0;
/*
  ext2_superblock_t *superblock = node->mount->fs_data;

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


  // Calculate disk offset
  uint32_t disk_offset = (fat12_info->dataOffset+cluster) * fat12_info->bpb->SectorsPerCluster * fat12_info->bpb->BytesPerSector;


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

  return size;
*/
}

/**
 *
 */
uint32_t ext2_write (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer) {
  return 0;
//  return node->block->write (node->block->major, node->block->minor, offset, size, buffer);
}

/**
 *
 */
void ext2_open (vfs_node_t *node) {
  // node->block->open (node->block->major, node->block->minor);
}

/**
 *
 */
void ext2_close (vfs_node_t *node) {
//  node->block->close (node->block->major, node->block->minor);
}


/**
 * Initialises the ext2 on current drive
 */
void ext2_init (void) {
  // Register file system to the VFS
  vfs_register_filesystem (&ext2_vfs_info);
}



int ext2_finddir (vfs_node_t *node, const char *name, vfs_node_t *target_node) {
  ext2_info_t *ext2_info = node->mount->fs_data;
  uint32_t i;

  // Check if it's a directory
  if ((node->flags & 0x7) != FS_DIRECTORY) return NULL;

  // Hmz,.. looks like we need to start from the first entry and read until
  // we find the correct entry. @TODO: needs more caching.
  ext2_inode_t *inode = ext2_read_inode(node->mount, node->inode_nr);

  // So what we do is read the *COMPLETE* directory structure into memory,
  // and afterwards we iterate through it. I can't even begin to describe how
  // bad this method is.

  // Allocate buffer
  char *buffer = (char *)malloc(inode->sizeLow);
  char *buf_ptr = buffer;
  ext2_dir_t *ext2_dir = (ext2_dir_t *)buffer;


  // Read the directory blocks
  for (i=0; i!=inode->sizeLow / ext2_info->block_size; i++) {
    // @TODO: We don't do indirect blocks now
    if (i > 11) {
      printf ("Ext2: we can only read direct blocks\n");
      mfree(buffer);
      mfree(inode);
      return NULL;
    }

    // Block 0 encountered, reached end of blocks
    if (inode->directPointerBlock[i] == 0) break;

    // Read blocks from disk into buffer
    if (! ext2_read_block(node->mount, inode->directPointerBlock[i], 1, buf_ptr)) {
      printf("Ext2: cannot read complete block\n");
      mfree(buffer);
      mfree(inode);
      return NULL;
    }

    // Store next data here
    buf_ptr += ext2_info->block_size;
  }



  // Iterate until we find the correct name
  uint8_t namelen = strlen(name);
  while (ext2_dir->inode_nr != 0 &&
         ext2_dir->name_len != namelen &&
         strncmp(name, (char *)&ext2_dir->name, ext2_dir->name_len) != 0) {

    ext2_dir = (ext2_dir_t *)( (void *)ext2_dir + ext2_dir->rec_len );
  }

  // Could not find entry
  if (ext2_dir->inode_nr == 0) {
    mfree(buffer);
    mfree(inode);
    return NULL;
  }

  // Found the node

  // Read file inode
  ext2_inode_t *file_inode = ext2_read_inode(node->mount, ext2_dir->inode_nr);

  // Copy node info into new node
  memcpy (target_node, node, sizeof (vfs_node_t));

  // Set correct values for new inode
  target_node->inode_nr = ext2_dir->inode_nr;
  strncpy((char *)target_node->name, (char *)&ext2_dir->name, ext2_dir->name_len);
  target_node->owner = file_inode->uid;
  target_node->length = file_inode->sizeLow; // @TODO: 32bit.
  target_node->flags = ((file_inode->typeAndPermissions & EXT2_S_IFDIR) == EXT2_S_IFDIR) ? FS_DIRECTORY : FS_FILE;

  mfree(buffer);
  mfree(inode);
  mfree(file_inode);

  return 1;
}


int ext2_readdir (vfs_node_t *node, uint32_t index, vfs_dirent_t *target_dirent) {
  ext2_info_t *ext2_info = node->mount->fs_data;
  uint32_t i;

  // Check if it's a directory
  if ((node->flags & 0x7) != FS_DIRECTORY) return NULL;

  // Hmz,.. looks like we need to start from the first entry and read until
  // we find the correct entry. @TODO: needs more caching.
  ext2_inode_t *inode = ext2_read_inode(node->mount, node->inode_nr);

  // So what we do is read the *COMPLETE* directory structure into memory,
  // and afterwards we iterate through it. I can't even begin to describe how
  // bad this method is.

  // But since we don't have a fixed length, we always must start at the
  // first block. But let's merge the whole system: load a block, iterate
  // through the block, if found, we're done. Not found, continue...

  // Allocate buffer
  char *buffer = (char *)malloc(inode->sizeLow);
  char *buf_ptr = buffer;
  ext2_dir_t *ext2_dir = (ext2_dir_t *)buffer;

  // Read the directory blocks
  for (i=0; i!=inode->sizeLow / ext2_info->block_size; i++) {
    // @TODO: We don't do indirect blocks now, sorry
    if (i > 11) {
      printf ("Ext2: we can only read direct blocks\n");
      mfree(buffer);
      mfree(inode);
      return NULL;
    }

    // Block 0 encountered, reached end of blocks
    if (inode->directPointerBlock[i] == 0) break;

//    printf("IDPB[%d]: %d\n", i, inode->directPointerBlock[i]);

    // Read blocks from disk into buffer
    if (! ext2_read_block(node->mount, inode->directPointerBlock[i], 1, buf_ptr)) {
      printf("Ext2: cannot read complete block\n");
      mfree(buffer);
      mfree(inode);
      return NULL;
    }

    // Store next data here
    buf_ptr += ext2_info->block_size;
  }

  // Iterate through N directory entries
  while (1) {
    if (ext2_dir->inode_nr == 0) {
      // No more inodes found (index too large probably)
      mfree(buffer);
      mfree(inode);
      return NULL;
    }

    // Break when we have reached our index
    if (index <= 0) break;

    // Decrease index
    index--;

    // Goto next entry
//    printf ("[%d] Rec_len: %d (Inode: %d)\n", index, ext2_dir->rec_len, ext2_dir->inode_nr);
    ext2_dir = (ext2_dir_t *)( (void *)ext2_dir + ext2_dir->rec_len );
  }

  // ext2_dir points to correct directory
  strncpy((char *)target_dirent->name, (char *)&ext2_dir->name, ext2_dir->name_len);
  target_dirent->inode_nr = ext2_dir->inode_nr;

  mfree(buffer);
  mfree(inode);
  return 1;
}