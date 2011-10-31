/*
 * Dirty block cache for fast file data synchronization
 *  (Currentry dirty blocks cache is associated with global inode.)
 */
#include <dirty_block.h>
#include <string.h>
#include <print.h>
#include <inode.h>
#include <block.h>
#include <slab.h>

static struct dirty_block_cache *alloc_dbc(int size)
{
	struct dirty_block_cache *dbc;
	dbc = kmalloc(dbcsize(size));
	if (dbc) {
		memset(dbc, 0x0, dbcsize(size));
		dbc->size = size;
	}
	return dbc;
}

static _inline void free_dbc(struct dirty_block_cache *dbc)
{
	kfree(dbc);
}

static _inline void add_dbc(struct dirty_block_cache *dbc, struct block *block)
{
	dbc->blocks[dbc->free++] = block;
}

static struct dirty_block_cache *inode_lookup_free_dbc(struct inode *inode)
{
	struct dirty_block_cache **prev, *dbc;
	prev = &inode->i_dbc;
	dbc = inode->i_dbc;
	while (dbc) {
		/* find the free dbc */
		if (dbc->free < dbc->size) {
			if (dbc != inode->i_dbc) {
				/* add free dbc to head of inode dbc list */
				*prev = dbc->next;
				dbc->next = inode->i_dbc;
				inode->i_dbc = dbc;
			}
			break;
		}
		prev = &dbc->next;
		dbc = dbc->next;
	}
	return dbc;
}

static struct dirty_block_cache *inode_alloc_dbc(struct inode *inode)
{
	struct dirty_block_cache *dbc = alloc_dbc(DBC_FIRST_SIZE);
	if (!dbc)
		panic("No memory for dirty block cache");
	if (inode->i_dbc)
		dbc->next = inode->i_dbc;
	inode->i_dbc = dbc;
	return dbc;
}

/* Caller should check whether block is in dbc. */
void inode_add_dbc(struct inode *inode, struct block *block)
{
	struct dirty_block_cache *dbc = inode->i_dbc;
	/* lookup free dbc */
	dbc = inode_lookup_free_dbc(inode);
	/* if no free dbc, alloc new one */
	if (!dbc)
		dbc = inode_alloc_dbc(inode);
	add_dbc(dbc, block);
}

void inode_free_dbc(struct inode *inode)
{
	struct dirty_block_cache *dbc, *next;
	for (dbc = inode->i_dbc; dbc; dbc = next) {
		next = dbc->next;
		free_dbc(dbc);
	}
	inode->i_dbc = NULL;
}

void inode_sync_dbc(struct inode *inode)
{
	struct dirty_block_cache *dbc;
	int i;
	for (dbc = inode->i_dbc; dbc; dbc = dbc->next) {
		for (i = dbc->free - 1; i >= 0; i--)
			flush_block(dbc->blocks[i]);
		dbc->free = 0;
	}
}

