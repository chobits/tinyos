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
	struct block *block;
	int blk = minix_new_block_nr(sb);
	if (blk < 0)
		return NULL;
	if (data)
		*data = blk;
	block = minix_get_block(sb, blk);
	/*
	 * Clear the block, not setting dirty flag:
	 *  ( minix_new_block is used by minix_inode_mkdir() and bmap().
	 *    If dirty flag is set, minix_inode_dirty_block() cannot add
	 *    the dirty block to inode dirty block cache. )
	 *
	 * bmap() uses it to alloc 1/2-level indirect block, which must be
	 *  zeroed, otherwise the old block number in indirect block will be
	 *  used incorrectly.
	 *
	 * This change will fix the bug:
	 *  Running command `cp rm r; rm r; cp rm r; rm r`, we get a panic:
	 *  "Clear unexist-zone bitmap".
	 */
	if (block)
		memset(block->b_data, 0x0, block->b_size);
	return block;
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

static void bmap_put_block(struct super_block *sb, int blk)
{
	struct block *block;
	block = minix_get_block(sb, ZONE_MAP_BLK(minixsuper(sb), blk));
	if (!clearbit(block->b_data, ZONE_MAP_BLKOFF(minixsuper(sb), blk)))
		panic("Clear unexist-zone bitmap");
	block->b_dirty = 1;
	put_block(block);
}

static void bmap_put_one_level_blocks(struct inode *inode, int blk, int num)
{
	struct super_block *sb = inode->i_sb;
	struct block *block;
	unsigned short *br;
	int i;
	block = minix_get_block(sb, blk);
	if (!block)
		panic("Cannot get minix block %d", blk);
	br = (unsigned short *)block->b_data;
	for (i = 0; i < num; i++, br++) {
		if (*br)
			bmap_put_block(sb, *br);
	}
#ifdef MINIX_STRICT_CLEAR
	/* clear indirect block */
	memset(block->b_data, 0x0, block->b_size);
	minix_inode_dirty_block(inode, block);
#endif
	put_block(block);
	bmap_put_block(sb, blk);
}

void bmap_put_blocks(struct inode *inode)
{
	struct super_block *sb = inode->i_sb;
	struct minix_d_inode *mdi = i2mdi(inode);
	struct block *block;
	unsigned short *br;
	int blks, i, n;
	if (!mdi->i_size)
		return;
	blks = MINIX_BLOCKS_UP(mdi->i_size);
	if (blks > 7 + 512 + 512 * 512)
		panic("inode has too many blocks: %d", blks);
	/* direct blocks */
	n = min(blks, 7);
	blks -= n;
	for (i = 0; i < n; i++) {
		if (mdi->i_zone[i])
			bmap_put_block(sb, mdi->i_zone[i]);
	}
	/* one-level indirect blocks */
	n = min(blks, 512);
	blks -= n;
	if (!n)
		goto out;
	if (mdi->i_zone[7])
		bmap_put_one_level_blocks(inode, mdi->i_zone[7], n);
	/* two-level indirect blocks */
	if (!blks || !mdi->i_zone[8])
		goto out;
	block = minix_get_block(sb, mdi->i_zone[8]);
	if (!block)
		panic("Cannot get minix block:%d", mdi->i_zone[8]);
	br = (unsigned short *)block->b_data;
	for (i = 0; i < blks; i += n, br++) {
		n = min(512, blks - i);
		if (*br)
			bmap_put_one_level_blocks(inode, *br, n);
	}
#ifdef MINIX_STRICT_CLEAR
	/* clear indirect block */
	memset(block->b_data, 0x0, block->b_size);
	minix_inode_dirty_block(inode, block);
#endif
	put_block(block);
out:
	mdi->i_size = 0;
	for (i = 0; i < 9; i++)
		mdi->i_zone[i] = 0;
	minix_inode_dirty(inode);
}

static int imap_clear(struct super_block *sb, int ino)
{
	struct block *block;
	int r;
	block = minix_get_block(sb, INODE_MAP_BLK(ino));
	r = clearbit(block->b_data, ino & BITS_PER_BLOCK_MASK);
	if (!r)
		panic("Clear unexist-inode bitmap");
	block->b_dirty = 1;
	put_block(block);
	return r;
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
		block->b_dirty = 1;
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

struct minix_d_inode *imap_new_inode(struct super_block *sb, int *rino, struct block **b)
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
	mdi->i_mode = S_IFREG | 0755;	/* default type: regular file */
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

void imap_put_inode(struct inode *inode)
{
	/* free bitmap */
	imap_clear(inode->i_sb, inode->i_ino);
	/* free mdi and inode block */
	memset(i2mdi(inode), 0x0, MINIX_INODE_SIZE);
	minix_inode_dirty(inode);
	put_block(i2mi(inode)->m_iblock);
}
