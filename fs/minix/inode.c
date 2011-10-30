#include <minix_fs.h>
#include <string.h>
#include <types.h>
#include <print.h>
#include <inode.h>
#include <slab.h>
#include <list.h>
#include <block.h>

int minix_inode_read(struct inode *inode, char *buf, size_t size, off_t off);
int minix_inode_write(struct inode *inode, char *buf, size_t size, off_t off);
struct inode *minix_inode_sub_lookup(struct inode *dir, char *base, int len);
void minix_inode_update_size(struct inode *inode, size_t size);
void minix_put_inode(struct inode *inode);

static struct inode_operations minix_iops = {
	.read = minix_inode_read,
	.write = minix_inode_write,
	.sub_lookup = minix_inode_sub_lookup,
	.update_size = minix_inode_update_size,
	.close = minix_put_inode,
};

static struct slab *minix_inode_slab;
#define MINIX_INODE_HASH_SHIFT	4
#define MINIX_INODE_HASH_SIZE	(1 << MINIX_INODE_HASH_SHIFT)
#define MINIX_INODE_HASH_MASK	(MINIX_INODE_HASH_SIZE - 1)
static struct hlist_head minix_inode_htable[MINIX_INODE_HASH_SIZE];

static void minix_inode_unhash(struct minix_inode *mi)
{
	if (!hlist_unhashed(&mi->m_hnode))
		hlist_del(&mi->m_hnode);
}

static void minix_inode_hash(struct minix_inode *mi)
{
	struct hlist_head *head;
	head = &minix_inode_htable[mi->m_inode.i_ino & MINIX_INODE_HASH_MASK];
	hlist_add_head(&mi->m_hnode, head);
}

struct minix_inode *minix_alloc_inode(struct super_block *sb,
	struct minix_d_inode *mdi, unsigned int ino, struct block *block)
{
	struct inode *inode;
	struct minix_inode *mi = slab_alloc_object(minix_inode_slab);
	if (mi) {
		/* init commond inode */
		inode = &mi->m_inode;
		inode->i_size = mdi->i_size;
		inode->i_ino = ino;
		inode->i_sb = sb;
		inode->i_mode = mdi->i_mode;
		inode->i_refcnt = 1;
		inode->i_ops = &minix_iops;
		/* init minix inode */
		mi->m_dinode = mdi;
		mi->m_block = block;
		minix_inode_hash(mi);
		list_init(&mi->m_dirty);
	}
	return mi;
}

struct inode *minix_lookup_dev_inode(struct super_block *sb, unsigned int ino)
{
	struct minix_d_super_block *msb = minixsuper(sb);
	struct minix_d_inode *mdi;
	struct minix_inode *mi;
	struct block *block;
	if (ino > msb->s_ninodes)
		return NULL;
	mdi = imap_get_inode(sb, ino, &block);
	if (!mdi)
		return NULL;
	mi = minix_alloc_inode(sb, mdi, ino, block);
	if (!mi) {
		put_block(block);	/* release d_inode */
		return NULL;
	}
	return &mi->m_inode;
}

struct inode *minix_lookup_hash_inode(struct super_block *sb, unsigned int ino)
{
	struct hlist_head *head = &minix_inode_htable[ino & MINIX_INODE_HASH_MASK];
	struct hlist_node *node;
	struct minix_inode *mi;
	hlist_for_each_entry(mi, node, head, m_hnode)
		if (mi->m_inode.i_ino == ino)
			goto found;
	return NULL;
found:
	return get_inode_ref(&mi->m_inode);
}

struct inode *minix_get_inode(struct super_block *sb, unsigned int ino)
{
	struct inode *inode;
	inode = minix_lookup_hash_inode(sb, ino);
	if (!inode)
		inode = minix_lookup_dev_inode(sb, ino);
	return inode;
}

void minix_free_inode(struct inode *inode)
{
	minix_inode_unhash(i2mi(inode));
	/* free d_inode */
	put_block(i2mi(inode)->m_block);
}

void minix_put_inode(struct inode *inode)
{
	inode->i_refcnt--;	
	if (inode->i_refcnt == 0) {
		/* synchronize inode block, but caching the inode */
		flush_block(i2mi(inode)->m_block);
	} else if (inode->i_refcnt < 0) {
		printk("Free minix inode too many times(%d)\n", inode->i_refcnt);
	}
}

int minix_inode_write(struct inode *inode, char *buf, size_t size, off_t off)
{
	struct block *block;
	int onesize;
	int wsize = 0;
	void *src = (void *)buf;
	int blk = MINIX_BLOCKS(off);
	if (off >= MINIX_MAX_SIZE)
		return 0;
	if (off + size > MINIX_MAX_SIZE)
		size = MINIX_MAX_SIZE - off;
	off = MINIX_BLOCK_OFF(off);
	while (wsize < size) {
		block = bmap_block(inode, blk, 1);
		if (!block)
			break;
		onesize = min(block->b_size - off, size - wsize);
		memcpy(block->b_data + off, src, onesize);
		block->b_dirty = 1;
		put_block(block);
		/* for next loop */
		src += onesize;
		wsize += onesize;
		blk++;
		off = 0;
	}
	return wsize;
}

/* inode_read has done the sanity checking of @size and @off */
int minix_inode_read(struct inode *inode, char *buf, size_t size, off_t off)
{
	struct block *block;
	int onesize;
	int rsize = 0;
	void *dst = (void *)buf;
	int blk = MINIX_BLOCKS(off);
	off = MINIX_BLOCK_OFF(off);

	while (rsize < size) {
		block = bmap_block(inode, blk, 0);
		if (!block)
			break;
		onesize = min(MINIX_BLOCK_SIZE - off, size - rsize);
		if (block == MINIX_ZERO_BLOCK) {
			/* read lazy written block */
			memset(dst, 0x0, onesize);
		} else {
			memcpy(dst, block->b_data + off, onesize);
			put_block(block);
		}
		/* for next loop */
		dst += onesize;
		rsize += onesize;
		blk++;
		off = 0;
	}
	return rsize;
}

struct inode *minix_inode_sub_lookup(struct inode *dir, char *base, int len)
{
	struct inode *inode;
	struct block *block;
	struct minix_dentry *de;
	int entries, i, k, n;

	if (len > 14)
		return NULL;
	entries = dir->i_size / MINIX_DENTRY_SIZE;
	for (i = 0; i < entries; i += n) {
		block = bmap_block(dir, i / MINIX_DENTRIES_PER_BLOCK, 1);
		de = (struct minix_dentry *)block->b_data;
		n = min(entries - i, MINIX_DENTRIES_PER_BLOCK);
		for (k = 0; k < n; k++, de++) {
			/* strncmp skips deleted entry automatically. */
			if (!strncmp(de->d_name, base, len))
				goto found;
		}
		put_block(block);
	}
	return NULL;
found:
	inode = minix_get_inode(dir->i_sb, de->d_ino);
	put_block(block);
	return inode;
}

void minix_inode_update_size(struct inode *inode, size_t size)
{
	struct minix_inode *mi = i2mi(inode);
	mi->m_dinode->i_size = size;
	mi->m_block->b_dirty = 1;
}

void minix_inode_init(void)
{
	int i;
	/* init inode slab */
	minix_inode_slab = alloc_slab(sizeof(struct minix_inode));
	if (!minix_inode_slab)
		panic("No enough memory for minix inode slab");
	/* init hash table */
	for (i = 0; i < MINIX_INODE_HASH_SIZE; i++)
		hlist_head_init(&minix_inode_htable[i]);
}
