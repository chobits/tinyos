/*
 * This file has definitions for some important minix filesystem 
 * structures etc.
 */
#ifndef __MINIX_FS_H
#define __MINIX_FS_H

#include <list.h>
#include <inode.h>
#include <fs.h>

struct block;

#define MINIX_BLOCK_SHIFT	10
#define MINIX_BLOCK_SIZE	(1 << MINIX_BLOCK_SHIFT)
#define MINIX_BLOCK_MASK	(MINIX_BLOCK_SIZE - 1)
#define MINIX_BLOCKS(x)		((x) >> MINIX_BLOCK_SHIFT)
#define MINIX_BLOCKS_UP(x)	MINIX_BLOCKS((x) + MINIX_BLOCK_MASK)
#define MINIX_BLOCK_OFF(x)	((x) & MINIX_BLOCK_MASK)

#define MINIX_ZERO_BLOCK	((struct block *)0xffffdead)
#define MINIX_BDATA_BLK(b, blk)	(((unsigned short *)(b)->b_data)[blk])

#define NAME_LEN		14
#define MINIX_ROOT_INO		1
#define MINIX_SUPER_BLK		1

#define I_MAP_SLOTS		8
#define Z_MAP_SLOTS		8
#define MINIX_SUPER_MAGIC	0x137F
#define MINIX_MAX_SIZE		((7 + 512 + 512 * 512) * MINIX_BLOCK_SIZE)

#define MINIX_INODE_SIZE	(sizeof(struct minix_d_inode))
#define MINIX_DENTRY_SIZE	(sizeof(struct minix_dentry))

#define INODES_PER_BLOCK	((MINIX_BLOCK_SIZE) / MINIX_INODE_SIZE)
#define MINIX_DENTRIES_PER_BLOCK	((MINIX_BLOCK_SIZE) / MINIX_DENTRY_SIZE)
#define BITS_PER_BLOCK		(8 * MINIX_BLOCK_SIZE)
#define BITS_PER_BLOCK_MASK	(BITS_PER_BLOCK - 1)

/* number of blocks containing inodes */
#define INODES(sb)		DIV_UP((sb)->s_ninodes, INODES_PER_BLOCK)
/* inode-block starting block nr */
#define INODE_START_BLK(sb)	(2 + (sb)->s_imap_blocks + (sb)->s_zmap_blocks)
#define INODE_BLK(sb, ino)	(INODE_START_BLK(sb) + (ino) / INODES_PER_BLOCK)
#define INODE_MAP_BLOCKS(sb)	DIV_UP((sb)->s_ninodes, BITS_PER_BLOCK)
#define ZONE_MAP_BLOCKS(sb)	DIV_UP((sb)->s_nzones - (sb)->s_firstdatazone, BITS_PER_BLOCK)
#define ZONE_MAP_START_BLK(sb)	((sb)->s_imap_blocks + 2)
#define INODE_MAP_BLK(ino)	(2 + (ino) / BITS_PER_BLOCK)
#define BLOCK2INODE(b, i)	((struct minix_d_inode *)(b)->b_data + ((i) - 1) % INODES_PER_BLOCK)

/* first logic data block nr */
#define FIRST_ZONE_BLK(sb) \
	(2 + INODES(sb) + (sb)->s_imap_blocks + (sb)->s_zmap_blocks)

#define minixsuper(s)		((struct minix_d_super_block *)((s)->s_block->b_data))
#define i2mi(i)			((struct minix_inode *)(i))
#define i2mdi(i)		(i2mi(i)->m_dinode)
#define minix_inode_dirty(i)	i2mi(i)->m_iblock->b_dirty = 1

/* minix inode in device */
struct minix_d_inode {
	unsigned short i_mode;
	unsigned short i_uid;
	unsigned long i_size;
	unsigned long i_time;
	unsigned char i_gid;
	unsigned char i_nlinks;
	unsigned short i_zone[9];
} __attribute__((packed));

struct minix_inode {
	struct inode m_inode;
	struct minix_d_inode *m_dinode;
	struct hlist_node m_hnode;
	struct block *m_iblock;		/* block containing d_inode */
} __attribute__((packed));

/*
struct inode {
	unsigned long i_atime;
	unsigned long i_ctime;
	unsigned short i_dev;
	unsigned short i_num;
	unsigned short i_count;
	unsigned char i_lock;
	unsigned char i_dirt;
	unsigned char i_pipe;
	unsigned char i_mount;
	unsigned char i_seek;
	unsigned char i_update;
} __attribute__((packed));
*/
/* super_block in device(disk) */
struct minix_d_super_block {
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
} __attribute__((packed));

struct minix_dentry {
	unsigned short d_ino;
	char d_name[NAME_LEN];
} __attribute__((packed));

extern struct block *imap_block(struct super_block *, unsigned int);
extern struct inode *minix_get_inode(struct super_block *, unsigned int);
extern struct minix_d_inode *imap_get_inode(struct super_block *, unsigned int, struct block **);
extern struct block *bmap_block(struct inode *, int, int);
extern struct block *minix_get_block(struct super_block *, int);
extern int bmap(struct inode *, int, int);
extern void minix_inode_dirty_block(struct inode *inode, struct block *block);

#endif	/* minix_fs.h */
