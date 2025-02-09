#ifndef __VFS_FAT12_H__
#define __VFS_FAT12_H__

#include <stdint.h>

// Fat structure is array of chars
typedef char * fat12_fat_t;

#pragma pack(1)
typedef struct {
    uint8_t   jmp_command[3];

    uint8_t   OEMName[8];
    uint16_t  BytesPerSector;
    uint8_t   SectorsPerCluster;
    uint16_t  ReservedSectors;
    uint8_t   NumberOfFats;
    uint16_t  NumDirEntries;
    uint16_t  NumSectors;
    uint8_t   Media;
    uint16_t  SectorsPerFat;
    uint16_t  SectorsPerTrack;
    uint16_t  HeadsPerCyl;
    uint32_t  HiddenSectors;
    uint32_t  LongSectors;

    uint32_t  SectorsPerFat32;   //sectors per FAT
    uint16_t  Flags;             //flags
    uint16_t  Version;           //version
    uint32_t  RootCluster;       //starting root directory
    uint16_t  InfoCluster;
    uint16_t  BackupBoot;        //location of bootsector copy
    uint16_t  Reserved[6];
} fat12_bpb_t;

#pragma pack(1)
typedef struct {
  uint8_t   Filename[8];           //filename
  uint8_t   Ext[3];                //extension (8.3 filename format)
  uint8_t   Attrib;                //file attributes
  uint8_t   Reserved;
  uint8_t   TimeCreatedMs;         //creation time
  uint16_t  TimeCreated;
  uint16_t  DateCreated;           //creation date
  uint16_t  DateLastAccessed;
  uint16_t  FirstClusterHiBytes;
  uint16_t  LastModTime;           //last modification date/time
  uint16_t  LastModDate;
  uint16_t  FirstCluster;          //first cluster of file data
  uint32_t  FileSize;              //size in bytes
} fat12_dirent_t;


typedef struct {
  char        name[255];              // name of the file
  uint8_t       eof;                    // 0 = more data, 1 = no more data left
  uint32_t      length;                 // length of the file
  uint32_t      offset;                 // Offset in the file (0= start)
  uint32_t      flags;                  // File flags
  uint32_t      currentCluster;         // Current cluser
  uint16_t      currentClusterOffset;   // Offset in the cluster
} fat12_file_t;

typedef struct {
	uint8_t  numSectors;
	uint32_t fatOffset;
	uint8_t  fatSizeBytes;
	uint8_t  fatSizeSectors;
	uint8_t  fatEntrySizeBits;
	uint32_t numRootEntries;
	uint32_t numRootEntriesPerSector;
	uint32_t rootEntrySectors;
	uint32_t rootOffset;
	uint32_t rootSizeSectors;
	uint32_t dataOffset;

  fat12_bpb_t *bpb;            // Pointer to Bios Parameter Block
  fat12_fat_t *fat;            // Pointer to (primary) FAT table
} fat12_fatinfo_t;


  void fat12_init (void);
  uint32_t fat12_read (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
  uint32_t fat12_write (vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
  void fat12_open (vfs_node_t *node);
  void fat12_close (vfs_node_t *node);
  vfs_dirent_t *fat12_readdir (vfs_node_t *node, uint32_t index);
  vfs_node_t *fat12_finddir (vfs_node_t *node, const char *name);

  vfs_node_t *fat12_mount (struct vfs_mount *mount, device_t *dev, const char *path);
  void fat12_umount (struct vfs_mount *mount);

#endif // __VFS_FAT12_H__