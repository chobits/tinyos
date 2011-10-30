#ifndef __BLOCK_H
#define __BLOCK_H

#include <list.h>
#include <compile.h>

#define BLOCK_SHIFT	10
#define BLOCK_SIZE	(1 << BLOCK_SHIFT)
#define BLOCK_MASK	(BLOCK_SIZE - 1)

/* block buffer structure */
struct block {
	struct hlist_node b_hnode;	/* listed in block hash table */
	struct list_head b_list;	/* listed in block device */
	int b_nr;			/* block number: start from 0 */
	int b_sects;			/* sectors of block: hd drive uses it */
	int b_size;			/* block size */
	int b_refcnt;			/* reference count */
	void *b_data;			/* data point */
	struct block_device *b_dev;
	unsigned int 	b_dirty:1,	/* block is writen ,not synhcronized */
			b_read:1,	/* set if disk data has been read */
			b_buffer:1;	/* in the block buffer */
	unsigned int b_private;		/* associated inode or some other structure */
};

struct block_device;
struct bdev_ops {
	int (*read)(struct block *);
	int (*write)(struct block *);
	int (*set_block_size)(struct block_device *, int);
};

struct block_device {
	struct list_head block_list;	/* all in-memory blocks of device */
	int size;			/* block size */
	int blocks;			/* number of total blocks */
	int no;
	struct bdev_ops *ops;		/* virtual functions table */
};

extern int set_block_size(struct block_device *bdev, int size);
extern void flush_block_buffer(void);
extern struct block *get_block(struct block_device *bdev, int blk);
extern void put_block(struct block *block);
extern int write_block(struct block *block);
extern void sync_blocks(void);

static _inline void flush_block(struct block *block)
{
	if (block->b_dirty)
		write_block(block);
}

#endif	/* block.h */
