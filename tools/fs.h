/*
 * This file has definitions for some important file table
 *  structures etc, which is used _only_ for tools/mkfs.minix.c
 */
#ifndef __FS_H
#define __FS_H

#include <sys/types.h>

#define BLKGETSIZE 0x00001260
#define NAME_LEN 14
#define ROOT_INO 1

#define I_MAP_SLOTS 8
#define Z_MAP_SLOTS 8
#define MINIX_SUPER_MAGIC 0x137F

#define INODE_SIZE (sizeof(struct d_inode))
#define BLOCK_SIZE 1024
#define BLOCK_SIZE_BITS 10
#ifndef NULL
#define NULL ((void *) 0)
#endif

#define INODES_PER_BLOCK ((BLOCK_SIZE)/ INODE_SIZE)
#define DIR_ENTRIES_PER_BLOCK ((BLOCK_SIZE)/(sizeof (struct dir_entry)))

#define ALIGN_UP(n, align) ((n + ((align) - 1)) / (align))

/* number of blocks containing inodes */
#define INODES(sb) ALIGN_UP((sb)->s_ninodes , INODES_PER_BLOCK)
/* inode-block starting block nr */
#define INODE_BLK(sb) (2 + (sb)->s_imap_blocks + (sb)->s_zmap_blocks)

/* first logic data block nr */
#define FIRST_ZONE_BLK(sb) \
	(2 + INODES(sb) + (sb)->s_imap_blocks + (sb)->s_zmap_blocks)

/* inode in device(disk) */
struct d_inode {
	unsigned short i_mode;
	unsigned short i_uid;
	unsigned long i_size;
	unsigned long i_time;
	unsigned char i_gid;
	unsigned char i_nlinks;
	unsigned short i_zone[9];
};

/* super_block in device(disk) */
struct d_super_block {
	unsigned short s_ninodes;
	unsigned short s_nzones;	/* max disk size is 64MB */
	unsigned short s_imap_blocks;
	/* 
	 * zone map stands for logic data block 
	 * *excluding* boot|sb|imap|zmap|inodeblk 
	 */
	unsigned short s_zmap_blocks;	
	unsigned short s_firstdatazone;
	unsigned short s_log_zone_size;
	unsigned long s_max_size;
	unsigned short s_magic;
};

struct dir_entry {
	unsigned short inode;
	char name[NAME_LEN];
};

#endif	/* fs.h */
