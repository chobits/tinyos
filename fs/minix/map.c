#include <minix_fs.h>
#include <string.h>
#include <block.h>
#include <print.h>
#include <inode.h>
#include <fs.h>

int minix_new_block_nr(struct super_block *sb)
{
	struct minix_d_super_block *msb = minixsuper(sb);
	int i, r, blk = ZONE_MAP_START_BLK(msb);
	struct block *block;
	for (i = 0; i < msb->s_zmap_blocks; i++) {
		block = minix_get_block(sb, blk);
		if (!block)
			return -1;
		r = searchzerobit(block->b_data, BITS_PER_BLOCK, 1);
		/* bit-0 is reserved! */
		if (r > 0) {
			block->b_dirty = 1;
			put_block(block);
			return (msb->s_firstdatazone + i * BITS_PER_BLOCK + r - 1);
		}
		put_block(block);
		blk++;
	}
	return -1;
}

struct block *minix_new_block(struct super_block *sb, unsigned short *data)
{
	int blk = minix_new_block_nr(sb);
	if (blk < 0)
		return NULL;
	if (data)
		*data = blk;
	return minix_get_block(sb, blk);
}

int bmap(struct inode *inode, int blk, int create)
{
	struct minix_d_inode *mdi = i2mdi(inode);
	struct super_block *sb = inode->i_sb;
	struct block *block , *block2, *blockn;
	unsigned short *pblk;
	int rblk;
	/* max block number */
	if (blk >= 7 + 512 + 512 * 512)
		return -1;
	/* init parameters */
	block = block2 = blockn = NULL;
	rblk = -1;
	/* direct block */
	if (blk < 7) {
		rblk = mdi->i_zone[blk];
		if (!rblk) {
			if (!create)
				return 0;
			rblk = minix_new_block_nr(sb);
			if (rblk > 0) {
				mdi->i_zone[blk] = rblk;
				minix_inode_dirty(inode);
			}
		}
		return rblk;
	}
	/* one-level indirect block */
	blk -= 7;
	if (blk < 512) {
		pblk = &mdi->i_zone[7];
		if (!*pblk) {
			if (!create)
				return 0;
			block = minix_new_block(sb, pblk);
			if (!block)
				return -1;
			minix_inode_dirty(inode);
		} else if (!(block = minix_get_block(sb, *pblk)))
			return -1;

		rblk = MINIX_BDATA_BLK(block, blk);
		if (!rblk) {
			if (!create) {
				rblk = 0;
				goto out_block;
			}
			rblk = minix_new_block_nr(sb);
			if (rblk > 0) {
				MINIX_BDATA_BLK(block, blk) = rblk;
				minix_inode_dirty_block(inode, block);
			}
		}
		goto out_block;
	}
	/* two-level indirect block */
	blk -= 512;
	/* first level */
	pblk = &mdi->i_zone[8];
	if (!*pblk) {
		if (!create)
			return 0;
		block = minix_new_block(sb, pblk);
		if (!block)
			return -1;
		minix_inode_dirty(inode);
	} else if (!(block = minix_get_block(sb, *pblk)))
		return -1;
	/* second level */
	pblk = &MINIX_BDATA_BLK(block, blk / 512);
	if (!*pblk) {
		if (!create) {
			rblk = 0;
			goto out_block;
		}
		block2 = minix_new_block(sb, pblk);
		if (!block2)
			goto out_block;
		minix_inode_dirty_block(inode, block);
	} else if (!(block2 = minix_get_block(sb, *pblk)))
		goto out_block;

	rblk = MINIX_BDATA_BLK(block2, blk & 511);
	if (!rblk) {
		if (!create) {
			rblk = 0;
			goto out_block2;
		}
		rblk = minix_new_block_nr(sb);
		if (rblk > 0) {
			MINIX_BDATA_BLK(block2, blk & 511) = rblk;
			minix_inode_dirty_block(inode, block2);
		}
	}
out_block2:
	put_block(block2);
out_block:
	put_block(block);
	return rblk;
}

struct block *bmap_block(struct inode *inode, int blk, int create)
{
	if ((blk = bmap(inode, blk, create)) < 0)
		return NULL;
	else if (blk == 0)
		return MINIX_ZERO_BLOCK;
	return minix_get_block(inode->i_sb, blk);
}

static int imap_create(struct super_block *sb)
{
	struct block *block;
	int i, n, r;
	/* new inode */
	i = INODE_MAP_BLK(0);
	n = INODE_MAP_BLK(0) + minixsuper(sb)->s_imap_blocks;
	while (i < n) {
		block = minix_get_block(sb, i);
		r = searchzerobit(block->b_data, BITS_PER_BLOCK, 1);
		put_block(block);
		if (r >= 0) {
			r = (i - INODE_MAP_BLK(0)) * BITS_PER_BLOCK + r;
			break;
		}
		i++;
	}
	return r;
}

/* return orignal bit value */
static int imap_lookup(struct super_block *sb, unsigned int ino)
{
	struct block *block;
	int r;
	block = minix_get_block(sb, INODE_MAP_BLK(ino));
	r = testbit(block->b_data, ino & BITS_PER_BLOCK_MASK);
	put_block(block);
	return r;
}

struct block *imap_block(struct super_block *sb, unsigned int ino)
{
	/* 1 for used inode, 0 for free */
	if (imap_lookup(sb, ino) == 0)
		return NULL;
	return minix_get_block(sb, INODE_BLK(minixsuper(sb), ino));
}

/* If @ino equals, we create a new disk inode. */
struct minix_d_inode *imap_get_inode(struct super_block *sb, unsigned int ino,
		struct block **b)
{
	struct block *block = imap_block(sb, ino);
	if (!block)
		return NULL;
	/* Dont release block, minix_d_inode refers to it. */
	if (b)
		*b = block;
	return BLOCK2INODE(block, ino);
}

struct minix_d_inode *imap_new_inode(struct super_block *sb, int *rino,
		struct block **b)
{
	struct minix_d_inode *mdi;
	struct block *block;
	int ino = imap_create(sb);
	if (ino < 0)
		return NULL;
	block = minix_get_block(sb, INODE_BLK(minixsuper(sb), ino));
	if (!block)
		return NULL;
	/* init minix disk inode */
	mdi = BLOCK2INODE(block, ino);
	memset(mdi, 0x0, sizeof(*mdi));
	mdi->i_mode = 0755 | S_IFREG;	/* default type: regular file */
	mdi->i_nlinks = 1;
	/* minix disk inode is dirty */
	block->b_dirty = 1;
	/* save return value */
	if (rino)
		*rino = ino;
	if (b)
		*b = block;
	return mdi;
}
