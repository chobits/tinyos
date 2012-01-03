#include <minix_fs.h>
#include <string.h>
#include <types.h>
#include <print.h>
#include <inode.h>
#include <slab.h>
#include <list.h>
#include <block.h>
#include <fs.h>

static struct inode_operations minix_reg_iops;
static struct inode_operations minix_dir_iops;

static struct slab *minix_inode_slab;
#define MINIX_INODE_HASH_SHIFT	4
#define MINIX_INODE_HASH_SIZE	(1 << MINIX_INODE_HASH_SHIFT)
#define MINIX_INODE_HASH_MASK	(MINIX_INODE_HASH_SIZE - 1)
static struct hlist_head minix_inode_htable[MINIX_INODE_HASH_SIZE];

static _inline void minix_dec_link(struct inode *inode)
{
	i2mdi(inode)->i_nlinks--;
	minix_inode_dirty(inode);
}

static _inline void minix_inc_link(struct inode *inode)
{
	i2mdi(inode)->i_nlinks++;
	minix_inode_dirty(inode);
}

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
		inode->i_dbc = NULL;
		if (S_ISDIR(inode->i_mode))
			inode->i_ops = &minix_dir_iops;
		else	/* Other types use regular ops defaultly. */
			inode->i_ops = &minix_reg_iops;
		/* init minix inode */
		mi->m_dinode = mdi;
		mi->m_iblock = block;
		minix_inode_hash(mi);
	}
	return mi;
}

struct inode *minix_new_inode(struct super_block *sb)
{
	struct minix_d_inode *mdi;
	struct inode *inode;
	struct block *block;
	int ino;
	mdi = imap_new_inode(sb, &ino, &block);
	if (!mdi)
		return NULL;
	inode = (struct inode *)minix_alloc_inode(sb, mdi, ino, block);
	if (!inode)
		panic("No memory for inode");
	return inode;
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

void minix_free_inode(struct minix_inode *mi)
{
	minix_inode_unhash(mi);
	slab_free_object(minix_inode_slab, mi);
}

void minix_put_inode(struct inode *inode)
{
	/* free block bitmap and data blocks */
	bmap_put_blocks(inode);
	/* free inode bitmap and inode block */
	imap_put_inode(inode);
	/* free mi(i) */
	minix_free_inode(i2mi(inode));
}

void minix_inode_release(struct inode *inode)
{
	/* Dont synchronize inode or datas, only cache the inode. */
}

/* Mark inode data block(including indirect map block) dirty */
void minix_inode_dirty_block(struct inode *inode, struct block *block)
{
	if (!block->b_dirty) {
		block->b_dirty = 1;
		inode_add_dbc(inode, block);
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
		onesize = min(MINIX_BLOCK_SIZE - off, size - wsize);
		memcpy(block->b_data + off, src, onesize);
		minix_inode_dirty_block(inode, block);
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

struct minix_dentry *minix_lookup_dentry(struct inode *dir, char *base, int len, struct block **b)
{
	struct minix_dentry *de;
	struct block *block;
	int entries, i, k, n;

	if (len > MINIX_NAME_LEN)
		return NULL;

	entries = dir->i_size / MINIX_DENTRY_SIZE;
	for (i = 0; i < entries; i += n) {
		block = bmap_block(dir, i / MINIX_DENTRIES_PER_BLOCK, 0);
		de = (struct minix_dentry *)block->b_data;
		n = min(entries - i, MINIX_DENTRIES_PER_BLOCK);
		for (k = 0; k < n; k++, de++) {
			if (de->d_ino == 0)
				continue;
			/* strncmp skips deleted entry automatically. */
			if (strlen(de->d_name) == len &&
				!strncmp(de->d_name, base, len))
				goto found;
		}
		put_block(block);
	}
	return NULL;
found:
	if (b)
		*b = block;
	else
		put_block(block);
	return de;
}

struct inode *minix_inode_sub_lookup(struct inode *dir, char *base, int len)
{
	struct inode *inode;
	struct block *block;
	struct minix_dentry *de;

	if (len > MINIX_NAME_LEN)
		return NULL;
	de = minix_lookup_dentry(dir, base, len, &block);
	if (!de)
		return NULL;
	inode = minix_get_inode(dir->i_sb, de->d_ino);
	put_block(block);
	return inode;
}

struct inode *minix_inode_create(struct inode *dir, char *base, int len)
{
	struct minix_dentry *de;
	struct inode *inode;
	struct block *block;
	int entries, i, k, n;

	if (len > MINIX_NAME_LEN)
		return NULL;
	/* exist? */
	de = minix_lookup_dentry(dir, base, len, NULL);
	if (de)
		return NULL;
	/* find an empty dir entry */
	entries = dir->i_size / MINIX_DENTRY_SIZE;
	for (i = 0; i < entries; i += n) {
		block = bmap_block(dir, i / MINIX_DENTRIES_PER_BLOCK, 0);
		de = (struct minix_dentry *)block->b_data;
		n = min(entries - i, MINIX_DENTRIES_PER_BLOCK);
		for (k = 0; k < n; k++, de++) {
			if (de->d_ino == 0)
				goto found;
		}
		if (i + n < entries || !(entries % MINIX_DENTRIES_PER_BLOCK)) {
			put_block(block);
			block = NULL;
		}
	}
	/* create a new data block and get dir entry from it */
	if (!block) {
		block = bmap_block(dir, entries / MINIX_DENTRIES_PER_BLOCK, 1);
		if (!block)
			return NULL;
		if (block->b_refcnt != 1)
			panic("Not new block(ref:%d)", block->b_refcnt);
		de = (struct minix_dentry *)block->b_data;
	}
	inode_update_size(dir, dir->i_size + MINIX_DENTRY_SIZE);
found:
	inode = minix_new_inode(dir->i_sb);
	if (!inode)
		goto out;
	/* fill dir entry */
	strncpy(de->d_name, base, len);
	de->d_name[len] = '\0';
	de->d_ino = inode->i_ino;
	minix_inode_dirty_block(dir, block);
out:
	put_block(block);
	return inode;
}

struct inode *minix_inode_mkdir(struct inode *dir, char *base, int len)
{
	struct minix_dentry *de;
	struct inode *inode;
	struct block *block;
	if (!(inode = minix_inode_create(dir, base, len)))
		return NULL;
	/* update dir inode information */
	minix_inc_link(dir);
	/* update inode information */
	i2mdi(inode)->i_mode = 0755 | S_IFDIR;
	i2mdi(inode)->i_nlinks = 2;
	inode->i_mode = 0755 | S_IFDIR;
	inode->i_ops = &minix_dir_iops;
	inode_update_size(inode, 2 * MINIX_DENTRY_SIZE);
	/* create dir entry "." and ".." */
	block = minix_new_block(dir->i_sb, &i2mdi(inode)->i_zone[0]);
	if (!block)
		panic("Cannot alloc new block for minixfs");
	/* "." */
	de = (struct minix_dentry *)block->b_data;
	strcpy(de->d_name, ".");
	de->d_ino = inode->i_ino;
	/* ".." */
	de++;
	strcpy(de->d_name, "..");
	de->d_ino = dir->i_ino;
	minix_inode_dirty_block(inode, block);
	put_block(block);
	return inode;
}

/* unlink can delete the (disk or memory)inode directly. */
int minix_inode_unlink(struct inode *dir, char *base, int len,
			int (*can_delete)(struct inode *))
{
	struct block *block;		/* dir entry block */
	struct minix_dentry *de;
	struct inode *inode;
	int r = -1;

	de = minix_lookup_dentry(dir, base, len, &block);
	if (!de)
		return -1;
	inode = minix_get_inode(dir->i_sb, de->d_ino);
	if (!inode)
		goto out_put_block;
	/* permission check */
	if (inode->i_refcnt > 1)
		goto out_put_inode;
	if (can_delete && !can_delete(inode))
		goto out_put_inode;

	/* delete it from the data block of dir */
	memset(de, 0x0, MINIX_DENTRY_SIZE);
	minix_inode_dirty_block(dir, block);
	/* delete inode */
	minix_dec_link(inode);
	minix_put_inode(inode);
	r = 0;
out_put_inode:
	put_inode(inode);
out_put_block:
	put_block(block);
	return r;
}

static int minix_dir_empty(struct inode *dir)
{
	struct block *block;
	struct minix_dentry *de;
	int entries, i, k, n;
	/* Assert that @dir is directory inode. */
	entries = dir->i_size / MINIX_DENTRY_SIZE;
	for (i = 0; i < entries; i += n) {
		n = min(entries - i, MINIX_DENTRIES_PER_BLOCK);
		block = bmap_block(dir, i / MINIX_DENTRIES_PER_BLOCK, 0);
		if (!block)
			continue;
		de = (struct minix_dentry *)block->b_data;
		for (k = 0; k < n; k++, de++) {
			if (de->d_ino == 0)
				continue;
			if (de->d_name[0] != '.')
				goto not_empty;
			if (de->d_name[1] == '\0') {
				if (de->d_ino != dir->i_ino)
					panic("The inode number of . corrupts");
				continue;
			}
			if (de->d_name[1] != '.' || de->d_name[2] != '\0')
				goto not_empty;
		}
		put_block(block);
	}
	return 1;
not_empty:
	put_block(block);
	return 0;
}

static int can_rmdir(struct inode *inode)
{
	if ((i2mdi(inode)->i_nlinks != 2) || !S_ISDIR(inode->i_mode))
		return 0;
	return minix_dir_empty(inode);
}

int minix_inode_rmdir(struct inode *dir, char *base, int len)
{
	int r = minix_inode_unlink(dir, base, len, can_rmdir);
	if (r == 0)
		minix_dec_link(dir);
	return r;
}

static int can_rm(struct inode *inode)
{
	return ((i2mdi(inode)->i_nlinks == 1) && !S_ISDIR(inode->i_mode));
}

int minix_inode_rm(struct inode *dir, char *base, int len)
{
	return minix_inode_unlink(dir, base, len, can_rm);
}

/*
 * get dir entries from @dir and write them into @ds
 * Note: it may get no @ds because of empty minix dentry(d_ino == 0),
 *       usermode program should take care of it!
 */
int minix_inode_getdir(struct inode *dir, int start, int num, struct dir_stat *ds)
{
	struct block *block;
	struct minix_dentry *de;
	int r, i, k, n, off;

	num = min(dir->i_size / MINIX_DENTRY_SIZE, start + num);
	r = 0;
	for (i = start; i < num; i += n) {
		off = i % MINIX_DENTRIES_PER_BLOCK;
		n = min(num - i, MINIX_DENTRIES_PER_BLOCK - off);
		block = bmap_block(dir, i / MINIX_DENTRIES_PER_BLOCK, 0);
		if (!block)
			continue;
		de = (struct minix_dentry *)block->b_data + off;
		for (k = 0; k < n; k++, de++) {
			/* no dir */
			if (de->d_ino == 0)
				continue;
			/* real copy */
			ds->len = strcpy(ds->name, de->d_name);
			ds->inode = de->d_ino;
			ds++;
			r++;
		}
		put_block(block);
	}
	return r;
}

void minix_inode_stat(struct inode *inode, struct file_stat *stat)
{
	stat->link = i2mdi(inode)->i_nlinks;
}

int minix_get_pathname(struct inode *inode, char *pathname, size_t len)
{
	struct minix_dentry *de;

	if (len < MINIX_NAME_LEN + 1)
		return -1;

	de = minix_lookup_dentry(inode, ".", 1, NULL);
	if (!de)
		return -1;

	strncpy(pathname, de->d_name, MINIX_NAME_LEN);

	pathname[MINIX_NAME_LEN] = '\0';

	return 0;
}

void minix_inode_truncate(struct inode *inode)
{
	bmap_put_blocks(inode);
	inode_update_size(inode, 0);
}

void minix_inode_update_size(struct inode *inode, size_t size)
{
	struct minix_inode *mi = i2mi(inode);
	mi->m_dinode->i_size = size;
	mi->m_iblock->b_dirty = 1;
}

void minix_sync_inode(struct inode *inode)
{
	/* synchronize inode */
	flush_block(i2mi(inode)->m_iblock);
	/* synchronize dirty blocks */
	inode_sync_dbc(inode);
	/* free dirty blocks */
	inode_free_dbc(inode);
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

/* regular directory operations */
static struct inode_operations minix_dir_iops = {
	.sub_lookup = minix_inode_sub_lookup,
	.update_size = minix_inode_update_size,
	.release = minix_inode_release,
	.sync = minix_sync_inode,
	.getdir = minix_inode_getdir,
	.mkdir = minix_inode_mkdir,
	.rmdir = minix_inode_rmdir,
	.create = minix_inode_create,
	.rm = minix_inode_rm,
	.stat = minix_inode_stat,
	.get_pathname = minix_get_pathname,
};

/* regular file operations */
static struct inode_operations minix_reg_iops = {
	.read = minix_inode_read,
	.write = minix_inode_write,
	.update_size = minix_inode_update_size,
	.release = minix_inode_release,
	.sync = minix_sync_inode,
	.stat = minix_inode_stat,
	.truncate = minix_inode_truncate,
};

