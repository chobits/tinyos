#ifndef __SLAB_H
#define __SLAB_H

#include <list.h>

struct slab_object {
	struct slab_object *next;
};

/*
 * Slab cache and objects are in the same page.
 * Why?
 *  If we alloc a slab for slab cache, then slab cache will
 *  recurse infinitely when slab cache allocs slab cache!
 */
struct slab_cache {
	int total;			/* total objects */
	int free;			/* free objects */
	struct slab_object *object;	/* free object list */
	struct list_head list;		/* listed in its slab */
};

struct slab {
	int size;			/* object size */
	int pages;			/* cache contaning max pages number */
	struct slab_cache *current;	/* one free cache of all cache */
	struct list_head cache;		/* pages containing in-use objects */
	struct list_head free_cache;	/* pages containing free objects */
	struct list_head list;		/* listed in global slab list */
};

#define slab_next_object(s, o) ((struct slab_object *)((void *)(o) + (s)->size))

#define KMALLOC_CUBES			10
#define KMALLOC_MAX_SIZE		(4 << KMALLOC_CUBES)

#define SLAB_CACHE_SIZE			sizeof(struct slab_cache)
#define SLAB_CACHE_MIN_OBJECTS		16
#define SLAB_CACHE_FREE_QUOTA(c)	((cache)->total / 2)
#define SLAB_OBJECT_MAX_SIZE		2048

#define PGSLAB(p)	((struct slab *)(p)->pg_list.prev)
#define PGSLABCACHE(p)	((struct slab_cache *)(p)->pg_list.next)
#define SLAB_PGS(size)	PGS_UP(SLAB_CACHE_MIN_OBJECTS * (size) + SLAB_CACHE_SIZE)
/* cache is at the bottom of cache buffer */
#define PG2CACHE(p, n)	(((struct slab_cache *)PG2VADDR((p) + (n))) - 1)

extern void kfree(void *);
extern void *kmalloc(int);
extern void free_slab(struct slab *);
extern struct slab *alloc_slab(int);
extern void slab_free_object(struct slab *, void *);
extern void *slab_alloc_object(struct slab *);

#endif	/* slab.h */
