#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "fs.h"

#if DEBUG
#define warning(fmt, ...) fprintf(stderr, "WARNING: "fmt"\n", __VA_ARGS__)
#define DEBUG_SB(sb) \
	do {								\
		fprintf(stderr, "----------super block:-------------\n" \
		"s_inodes: %d\ns_nzones: %d\ns_imap_blocks: %d\n"	\
		"s_zmap_blocks: %d\ns_firstdatazone: %d\n"		\
		"s_log_zone_size: %u\ns_max_size: %u\n"			\
		"s_magic: 0x%x\n-----------------------------------\n",	\
		(sb)->s_ninodes, (sb)->s_nzones, (sb)->s_imap_blocks,	\
		(sb)->s_zmap_blocks, (sb)->s_firstdatazone,		\
		(sb)->s_log_zone_size, (sb)->s_max_size, (sb)->s_magic);\
	} while (0)
#else
#define DEBUG_SB(sb)
#define warning(fmt, ...)
#endif

#define CLEAN_BLOCK_BUFFER() memset(block_buffer, 0x0, BLOCK_SIZE)
#define CLEAN_SUPER_BLOCK() memset(super_block, 0x0, BLOCK_SIZE)

#define WRITE_BLOCK(blknr, blk, str)					\
do {									\
	if (lseek(devfd, (blknr) * BLOCK_SIZE, SEEK_SET) == -1)		\
		per_exit("lseek");					\
	if (BLOCK_SIZE != write(devfd, blk, BLOCK_SIZE)) { 		\
		per_exit("write: "str);					\
	}								\
} while (0)

#define WRITE_SUPER_BLOCK(blknr, str) WRITE_BLOCK((blknr) + blkoff, super_block, str)
#define WRITE_BLOCK_BUFFER(blknr, str) WRITE_BLOCK((blknr) + blkoff, block_buffer, str)

static int devfd = -1;
static char *program = NULL;
static char *devfile = NULL;
static unsigned int blocks = 0;
static int blkoff = 0;

/*
 * used by boot/sb/imap/zmap/inodeblk/datablk repeatedly
 * Although it is ugly to do it, it is simply enough:P
 */
static unsigned char block_buffer[BLOCK_SIZE];
static unsigned char super_block[BLOCK_SIZE];
static struct d_super_block *sb = (struct d_super_block *)super_block;
static unsigned char *map_blk = block_buffer;
static unsigned char *inode_blk = block_buffer;

void per_exit(char *str)
{
	if (errno)
		perror(str);
	else
		fprintf(stderr, "ERROR: %s\n", str);
	exit(EXIT_FAILURE);
}


void open_set_param(void)
{
	struct stat sbuf;
	unsigned long size;

	/* get total blocks number of disk image */
	devfd = open(devfile, O_RDWR);
	if (devfd == -1)
		per_exit("open devfile");
	if (fstat(devfd, &sbuf) == -1)
		per_exit("fstat");
	if (!S_ISBLK(sbuf.st_mode)) {
		warning("file:%s is not block device", devfile);
		blocks = sbuf.st_size / BLOCK_SIZE;
	} else {
		if (ioctl(devfd, BLKGETSIZE, &size) == -1)
			per_exit("ioctl");
		blocks = size / (BLOCK_SIZE / 512);
	}
	/* get filesystem block offset */
	if (lseek(devfd, 506, SEEK_SET) != 506)
		per_exit("lseek");
	if (read(devfd, &blkoff, 2) != 2)
		per_exit("read");
	blocks -= blkoff;
	printf(" <Minix fs starts from %dth block with %d blocks>\n",
		blkoff, blocks);
	/* check */
	if (blocks < 10 || blocks > 65535)
		per_exit("invalid block number");
}

void clean_boot(void)
{
	CLEAN_BLOCK_BUFFER();
	WRITE_BLOCK_BUFFER(0, "boot block");
}

void write_super_block(void)
{

	CLEAN_SUPER_BLOCK();
	/* init 'static' data */
	sb->s_log_zone_size = 0;
	sb->s_max_size = (7 + 512 + 512 * 512) * BLOCK_SIZE;
	sb->s_magic = MINIX_SUPER_MAGIC;

	/* init 'dynamic' data, which need to be caculated */
	sb->s_ninodes = blocks / 3;
	sb->s_nzones = blocks;
	sb->s_imap_blocks = ALIGN_UP(sb->s_ninodes - 1, 8 * BLOCK_SIZE);

	/* up and down algrithm */
	sb->s_zmap_blocks = 0;	/* bits: all blocks - boot/sb/imap/zmap/inodeblk */
	while (sb->s_zmap_blocks != ALIGN_UP(blocks - FIRST_ZONE_BLK(sb), 8 * BLOCK_SIZE))
		sb->s_zmap_blocks = ALIGN_UP(blocks - FIRST_ZONE_BLK(sb), 8 * BLOCK_SIZE);

	/* start from 0, which is boot block number */
	sb->s_firstdatazone = FIRST_ZONE_BLK(sb);

	WRITE_SUPER_BLOCK(1, "super block");

	warning("write super block: blocks %d\n first data block %d\n",
		blocks, sb->s_firstdatazone);

	DEBUG_SB(sb);
}

void write_maps(void)
{
	int i;

	CLEAN_BLOCK_BUFFER();

	if (lseek(devfd, (2 + blkoff) * BLOCK_SIZE, SEEK_SET) == -1)
		per_exit("lseek");
	/* zone map bits only stand for [data block](|boot|sb|imap|zmap|inodeblk|[data block]|)*/
	for (i = 0; i < sb->s_imap_blocks + sb->s_zmap_blocks; i++)
		if (BLOCK_SIZE != write(devfd, map_blk, BLOCK_SIZE))
			per_exit("write map_blks");
}

void write_root(void)
{
	int i;
	struct d_inode *root;

	inode_blk = (unsigned char *)block_buffer;
	CLEAN_BLOCK_BUFFER();
	/* mark inode map */
	map_blk[0] = 0x03;	/* 0-reserved 1-ROOT_INO */
	WRITE_BLOCK_BUFFER(2, "root inode map");

	/*
	 * mark zone map
	 * 0-reserved
 	 * 1-first data block containing dentry:"." and ".."
	 */
	map_blk[0] = 0x03;
	WRITE_BLOCK_BUFFER(2 + sb->s_imap_blocks, "root inode map");

#define dentry2 "\x1\x0.\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"\
		"\x1\x0..\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
#define d2len (2 * sizeof(struct dir_entry))
	/* write inode */
	root = (struct d_inode *)inode_blk;
	root->i_mode = S_IFDIR | 0755;
	root->i_size = d2len;
	root->i_time = time(NULL);
	root->i_zone[0] = sb->s_firstdatazone;
	root->i_nlinks = 2;
	WRITE_BLOCK_BUFFER(INODE_BLK(sb), "root inode");

	/* write block(dentry:"." and "..") */
	CLEAN_BLOCK_BUFFER();
	memcpy(block_buffer, dentry2, d2len);
	WRITE_BLOCK_BUFFER(sb->s_firstdatazone, "root data block(dentry)");
}

void usage(void)
{
	fprintf(stderr, "Usage %s devfile\n", program);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	program = argv[0];
	if (argc != 2)
		usage();
	devfile = argv[1];
	open_set_param();
	/*
	 * It is *not* necessary to clean boot block.
	 * But I do it to avoid extra troublesome ;-)
	 */
	clean_boot();

	write_super_block();
	write_maps();
	write_root();
	return 0;
}
