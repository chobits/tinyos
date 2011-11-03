#include <minix_fs.h>
#include <block.h>
#include <print.h>
#include <types.h>
#include <task.h>
#include <boot.h>
#include <fs.h>
#include <mm.h>

static struct super_block minix_sb;

static void minix_check_super(struct minix_d_super_block *msb)
{
	if (msb->s_magic != MINIX_SUPER_MAGIC)
		panic("Unknown fs type, not minix");
	if (msb->s_nzones > minix_sb.s_bdev->blocks - minix_sb.s_start)
		panic("zones %d is larger then %d usable blocks",
			msb->s_nzones, minix_sb.s_bdev->blocks - minix_sb.s_start);
	if (msb->s_max_size != MINIX_MAX_SIZE)
		panic("Invalid max size %d", msb->s_max_size);
	if (msb->s_log_zone_size != 0)
		panic("Invalid log zone size %d", msb->s_log_zone_size);
	if (msb->s_firstdatazone != FIRST_ZONE_BLK(msb))
		panic("Invalid first data zone %d", msb->s_firstdatazone);
	if (msb->s_imap_blocks != INODE_MAP_BLOCKS(msb))
		panic("Invalid imap blocks");
	if (msb->s_zmap_blocks != ZONE_MAP_BLOCKS(msb))
		panic("Invalid zmap blocks");
	printk("The super block of minix filesystem is ok!\n");
}

extern struct block_device hd_bdev;

struct block *minix_get_block(struct super_block *s, int blk)
{
	return get_block(s->s_bdev, s->s_start + blk);
}

extern void minix_fs_test(void);
extern void minix_inode_init(void);
void minix_fs_init(void)
{
	/* get super block */
	minix_sb.s_bdev = &hd_bdev;
	set_block_size(minix_sb.s_bdev, MINIX_BLOCK_SIZE);
	minix_sb.s_start = MINIXFS_START;
	if (minix_sb.s_start >= minix_sb.s_bdev->blocks)
		panic("minixfs starts from %d block over %d blocks of device",
			minix_sb.s_start, minix_sb.s_bdev->blocks);
	minix_sb.s_block = minix_get_block(&minix_sb, MINIX_SUPER_BLK);
	if (!minix_sb.s_block)
		panic("Cannot get minixfs super block");
	/* check super block */
	minix_check_super(minixsuper(&minix_sb));

	minix_inode_init();

	/* init root dir */
	ctask->fs.root_dir = minix_get_inode(&minix_sb, MINIX_ROOT_INO);
	if (!ctask->fs.root_dir)
		panic("Cannot get minix root dir");
	ctask->fs.current_dir = get_inode_ref(ctask->fs.root_dir);
	/* debug inode */
//	minix_fs_test();
}
