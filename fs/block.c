#include <slab.h>
#include <disk.h>
#include <block.h>
#include <print.h>

static struct slab *block_slab;

#define BLOCK_HASH_SHIFT	4
#define BLOCK_HASH_SIZE		(1 << BLOCK_HASH_SHIFT)
#define BLOCK_HASH_MASK		(BLOCK_HASH_SIZE - 1)
static struct hlist_head block_htable[BLOCK_HASH_SIZE];
static LIST_HEAD(block_buffer_list);

static _inline struct block *get_block_ref(struct block *block)
{
	block->b_refcnt++;
	return block;
}

static int bdev_hash(int bdevno, int blk)
{
	return ((blk >> bdevno) + (blk ^ bdevno));
}

static struct block *lookup_hash_block(struct block_device *bdev, int blk)
{
	struct block *block;
	struct hlist_node *node;
	struct hlist_head *head;
	int hash;

	hash = bdev_hash(bdev->no, blk) & BLOCK_HASH_MASK;
	head = &block_htable[hash];
	hlist_for_each_entry(block, node, head, b_hnode) {
		if (block->b_dev == bdev && block->b_nr == blk)
			goto found;
	}
	return NULL;
found:
	/* Lucky! We get a cached one. */
	if (block->b_buffer) {
		block->b_refcnt = 0;
		block->b_buffer = 0;
		list_move(&block->b_list, &bdev->block_list);
	}
	return get_block_ref(block);
}

static void block_unhash(struct block *block)
{
	if (!hlist_unhashed(&block->b_hnode))
		hlist_del(&block->b_hnode);
}

static void block_hash(struct block_device *bdev, struct block *block)
{
	struct hlist_head *head;
	int hash;
	hash = bdev_hash(bdev->no, block->b_nr) & BLOCK_HASH_MASK;
	head = &block_htable[hash];
	hlist_add_head(&block->b_hnode, head);
}

void free_block(struct block *block)
{
	list_del(&block->b_list);
	block_unhash(block);
	kfree(block->b_data);
	slab_free_object(block_slab, block);
}

struct block *alloc_block(struct block_device *bdev, int blk)
{
	struct block *block = slab_alloc_object(block_slab);
	if (!block)
		return NULL;
	block->b_data = kmalloc(bdev->size);
	if (!block->b_data) {
		slab_free_object(block_slab, block);
		return NULL;
	}
	block->b_nr = blk;
	block->b_size = bdev->size;
	block->b_sects = bdev->size / SECT_SIZE;	/* Ramdisk will not use. */
	block->b_refcnt = 1;
	block->b_dev = bdev;
	block->b_dirty = 0;
	block->b_read = 0;
	block->b_buffer = 0;
	block_hash(bdev, block);
	list_add(&block->b_list, &bdev->block_list);
	return block;
}

int write_block(struct block *block)
{
	struct block_device *bdev = block->b_dev;
	int r = -1;
	if (bdev && bdev->ops)
		r = bdev->ops->write(block);
	if (r == 0)
		block->b_dirty = 0;
	return r;
}

void put_block(struct block *block)
{
	block->b_refcnt--;
	if (block->b_refcnt == 0) {
		/* We dont free block memory or sync block, just caching it. */
		list_move(&block->b_list, &block_buffer_list);
		block->b_buffer = 1;
	} else if (block->b_refcnt < 0) {
		printk("Free block too many times(%d)\n", block->b_nr);
	}
}

int read_block(struct block *block)
{
	struct block_device *bdev = block->b_dev;
	int r = -1;
	if (bdev && bdev->ops)
		r = bdev->ops->read(block);
	if (r == 0)
		block->b_read = 1;
	return r;
}

struct block *get_block(struct block_device *bdev, int blk)
{
	struct block *block;
	if (blk >= bdev->blocks)
		return NULL;
	/* lookup it in hash table */
	block = lookup_hash_block(bdev, blk);
	/* We dont cache block, kmalloc(BLOCK_SIZE) will cache it! */
	/* if it isnt in hash table, we alloc a new one. */
	if (!block) {
		block = alloc_block(bdev, blk);
		if (!block)
			return NULL;
	}
	if (!block->b_read)
		read_block(block);
	return block;
}

/* Called when there is no enough free memory. */
void flush_block_buffer(void)
{
	struct block *block;
	while (!list_empty(&block_buffer_list)) {
		block = list_first_entry(&block_buffer_list, struct block, b_list);
		flush_block(block);
		free_block(block);
	}
}

void sync_blocks(void)
{
	struct hlist_head *head = block_htable;
	int i = BLOCK_HASH_SIZE;
	struct hlist_node *node;
	struct block *block;
	while (i > 0) {
		hlist_for_each_entry(block, node, head, b_hnode)
			flush_block(block);
		head++;
		i--;
	}
}

int set_block_size(struct block_device *bdev, int size)
{
	int r = -1;
	if (bdev->ops && bdev->ops->set_block_size)
		r = bdev->ops->set_block_size(bdev, size);
	return r;
}

void sys_sync(void)
{
	sync_blocks();
}

#ifdef DEBUG_BLOCK
extern struct block_device hd_bdev;
void block_test(void)
{
	struct block *block;
	unsigned char *p;
	int i;
	block = get_block(&hd_bdev, 0);
	if (!block)
		panic("Cannot get block");
	if (block != get_block(&hd_bdev, 0))
		panic("Cannot get hash block");
	put_block(block);
	p = (unsigned char *)block->b_data;
	if (p[510] != 0x55 || p[511] != 0xaa)
		panic("ERROR 0x55aa");
	p[0] = 'h';
	p[1] = 'k';
	block->b_dirty = 1;
	put_block(block);
	block = get_block(&hd_bdev, 0);
	p = (unsigned char *)block->b_data;
	if (p[0] != 'h' || p[1] != 'k')
		panic("block WRITE or READ error");
	for (i = 0; i < 512; i++)
		p[i] = 0xff;
	block->b_dirty = 1;
	put_block(block);
	printk("flush buffer\n");
	flush_block_buffer();
	block = get_block(&hd_bdev, 0);
	printk("test block ok\n");
}
#else
#define block_test()
#endif	/* !DEBUG_BLOCK */

extern void hd_init(void);
void block_init(void)
{
	int i;
	/* init block buffer allocator */
	block_slab = alloc_slab(sizeof(struct block));
	if (!block_slab)
		panic("No memory for block slab");
	/* init block buffer hash table */
	for (i = 0; i < BLOCK_HASH_SIZE; i++)
		hlist_head_init(&block_htable[i]);
	hd_init();
	block_test();
}
