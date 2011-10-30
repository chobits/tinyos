#include <disk.h>
#include <block.h>
#include <list.h>

#define HD_DEVNO 3

static int hd_read_block(struct block *block);
static int hd_write_block(struct block *block);
static int hd_set_block_size(struct block_device *bdev, int size);

struct block_device hd_bdev;
static struct bdev_ops hd_bdev_ops = {
	.read = hd_read_block,
	.write = hd_write_block,
	.set_block_size = hd_set_block_size,
};

static int hd_set_block_size(struct block_device *bdev, int size)
{
	if (size < 0 || size & BLOCK_SIZE)
		return -1;
	bdev->size = size;
	return 0;
}

static int hd_read_block(struct block *block)
{
	int r;
	r = ide_read_sect(block->b_sects, block->b_data,
		block->b_nr * block->b_sects);
	return (r == block->b_sects) ? 0 : -1;
}

static int hd_write_block(struct block *block)
{
	int r;
	r = ide_write_sect(block->b_sects, block->b_data,
		block->b_nr * block->b_sects);
	return (r == block->b_sects) ? 0 : -1;
}

void hd_init(void)
{
	hd_bdev.size = BLOCK_SIZE;
	hd_bdev.blocks = ide_blocks();
	hd_bdev.ops = &hd_bdev_ops;
	hd_bdev.no = HD_DEVNO;
	list_init(&hd_bdev.block_list);
}
