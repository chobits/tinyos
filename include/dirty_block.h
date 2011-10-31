#ifndef __DIRTY_BLOCK_H
#define __DIRTY_BLOCK_H

struct dirty_block_cache {
	int size;
	int free;			/* free block slot index */
	struct dirty_block_cache *next;
	struct block *blocks[0];	/* dynamic alloced slots based on size */
};

#define dbcsize(s) ((s) * sizeof(struct block) + sizeof(struct dirty_block_cache))
#define DBC_FIRST_SIZE	16
#define DBC_INC_REATE	2

#endif	/* dirty_block.h */
