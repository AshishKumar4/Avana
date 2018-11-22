#ifndef VFS_H
#define VFS_H

#include "stdio.h"
#include "Arch/x86.h"
#include "Arch/x86/custom_defs.h"
#include "string.h"
#include "AqFS/Aqfs.h"

typedef struct FSnode_Desc
{
  uint32_t type;        // File or Folder or Hardlink etc.
  uint32_t fs_type;     // AqFS or Ext2 or FAT etc
  uint32_t desc_sz;
  uint32_t desc_type;
  uint32_t sDesc[];          // Specific Descriptor Structure
}FSnode_Desc_t;

typedef struct VDirectory
{
  char name[64];
  uintptr_t dir_desc;
}VDirectory_t;

VDirectory_t current_dir;

typedef struct FS_Desc
{
  char name[8];

  uint32_t f_type;
  VDirectory_t root;

  int  (*read)();
  int  (*write)();
  uintptr_t  (*load)();
  int  (*close)();
  int  (*getSize)();
  int  (*listnodes)(char*);
  int  (*makenode)(char*, int);
  int  (*delnode)(char*);
/*
  func_t mkfl;
  func_t mkdir;
  func_t ls;
  func_t cd;
  func_t cp;
  func_t editfl;
  func_t cat;
  func_t mv;
  func_t rm;*/
}FS_Desc_t;

typedef struct Partition_Desc
{
  char name[8];
  FS_Desc_t FS;
}Partition_Desc_t;

Partition_Desc_t* Current_Partition;
/*
typedef struct FILE_desc
{
  uintptr_t handle;
  uint32_t fstream_ptr;
  uint32_t fsize;
  char mode[8];
}FILE;
*/
typedef struct Partition_struct
{
  uint8_t boot_indicator;
  uint8_t starting_head; //255
  uint8_t starting_sector; //63
  uint8_t starting_cylinder; //1023
  uint8_t sys_id; //Use C2 or ED
  uint8_t ending_end;
  uint8_t ending_sector;
  uint8_t ending_cylinder;
  uint32_t relative_sector;
  uint32_t total_sectors;
}Partition_struct_t;

typedef struct Identity_Sectors
{
	char name[8];  //Storage media Name
	uint32_t active_partition;
	Partition_struct_t	partitions[4];
	//uint8_t partitions; //partitions i.e, number of root directories.
	//uint32_t Number_of_sectors;
	//uint32_t bytes_per_sector;
	//uint64_t partition_locations[32]; //locations of the partitions. 32 partitions on 1 disk supported
	//uint64_t reserved;
}Identity_Sectors_t;

VDirectory_t curr_dir;
uintptr_t VFS_ptr_currentDir;

void vfs_init();
void vfs_setup_aqfs();
void vfs_setup_aqfs2();
void vfs_setup_ext2();

#endif
