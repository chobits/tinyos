/*
 * A simple slab memory allocator referred to wikipedia.
 */
#include <mm.h>
#include <slab.h>
#include <types.h>
#include <page.h>
#include <list.h>
#include <print.h>
#include <string.h>

static struct list_head slab_list;		/* all slabs */
static struct slab slab_cube;			/* for allocing slab structure */
/* 
 * All flushable caches:
 *  This parameter is designed to accelerate slab flush methods.
 *  We stop flushing when @slab_flush_caches slab caches have been flushed,
 *  not flushing every cache of all slabs.
 */
static int slab_flush_caches;

static struct slab *cubes[KMALLOC_CUBES];	/* kmalloc for size:4/8/.../2048 */

void slab_flush_cache(struct slab *slab, struct slab_cache *cache)
{
	int order = log2(slab->pages);
	struct page *p;
	int i;
	list_del(&cache->list);
	p = VADDR2PG(cache);
	for (i = 0; i < slab->pages; i++)
		list_init(&p[i].pg_list);
	/* free cache */
	free_pages(p, order);
}

/* flush all free caches of @slab */
void flush_slab(struct slab *slab)
{
	struct slab_cache *cache, *current;
	struct list_head *__safe;
	int try = 0;
	/* If objects of current cache are free, we try to free current cache */
	current = slab->current;
	if (current->free == current->total)
		try = 1;

	list_for_each_entry_safe(cache, &slab->free_cache, list, __safe) {
		if (slab_flush_caches <= 0 && try != 1)
			break;
		if (cache->free < cache->total) {
			/*
			 * We dont try to lookup the best alternative cache,
			 *  jut selecting the first one!
			 */
			if (try && cache->free < current->free) {
				current = cache;
				try = 2;
			}
			continue;
		}
		slab_flush_cache(slab, cache);
		slab_flush_caches--;
	}
	/* If getting new cache, we free current cache */
	if (try && current != slab->current) {
		slab_flush_cache(slab, slab->current);
		slab_flush_caches--;
		list_move(&current->list, &slab->cache);
		slab->current = current;
	}
}

/* flush caches of all slabs in slab list */
void flush_slabs(void)
{
	struct slab *slab;
	list_for_each_entry(slab, &slab_list, list) {
		if (slab_flush_caches <= 0)
			break;
		flush_slab(slab);
	}
}

static struct slab_cache *slab_alloc_cache(struct slab *slab)
{
	struct slab_cache *cache;
	struct slab_object *object;
	struct page *page, *p;
	int n;
	page = alloc_pages(PG_ZONE_NOR, log2(slab->pages));
	if (!page) {
		/* TODO: Using flush_pages() instead! */
		flush_slabs();
		page = alloc_pages(PG_ZONE_NOR, log2(slab->pages));
		if (!page)
			return NULL;
	}
	/*
	 * How is cache object aligned?
	 *  Cache is at the bottom of the cache buffer, in which cacse
	 *  the first object is always aligned to memory cache line! 
	 */
	n = slab->pages;
	cache = PG2CACHE(page, n);
	/* init pages for fast objects free */
	p = page;
	while (n > 0) {
		/* list-use trick */
		p->pg_list.prev = (struct list_head *)slab;
		p->pg_list.next = (struct list_head *)cache;
		p++;
		n--;
	}
	/* init cache */
	n = (PG_SIZE(slab->pages) - SLAB_CACHE_SIZE) / slab->size;
	cache->total = n;
	cache->free = n;
	list_add(&cache->list, &slab->cache);
	/* add objects to head of free object list */
	object = (struct slab_object *)PG2VADDR(page);
	cache->object = object;
	while (n > 1) {
		object->next = slab_next_object(slab, object);
		object = object->next;
		n--;
	}
	object->next = NULL;
	return cache;
}

static void slab_free_cache(struct slab *slab)
{
	struct slab_cache *cache;
	/* free cache */
	while (!list_empty(&slab->cache)) {
		cache = list_first_entry(&slab->cache, struct slab_cache, list);
		slab_flush_cache(slab, cache);
	}
	/* free free-cache */
	while (!list_empty(&slab->free_cache)) {
		cache = list_first_entry(&slab->free_cache, struct slab_cache, list);
		slab_flush_cache(slab, cache);
	}
}

struct slab_cache *slab_lookup_cache(struct slab *slab)
{
	struct slab_cache *cache;
	/*
	 * We dont try to find a free-object cache,
	 * directly allocing a new cache from free cache or
	 * buddy page allocator.
	 */
	/* look up a free cache from free cache list */
	if (!list_empty(&slab->free_cache)) {
		cache = list_first_entry(&slab->free_cache, struct slab_cache, list);
		list_move(&cache->list, &slab->cache);
		if (cache->total == cache->free)
			slab_flush_caches--;
		return cache;
	}
	/* alloc a new cache */
	return slab_alloc_cache(slab);
}

void *slab_alloc_object(struct slab *slab)
{
	struct slab_object *object;
	struct slab_cache *cache = slab->current;
	/* prealloc new objects */
	if (!cache || cache->free <= 0) {
		cache = slab_lookup_cache(slab);
		if (!cache)
			return NULL;
		slab->current = cache;
	}
	/* delete object from free object list */
	object = cache->object;
	cache->object = object->next;
	cache->free--;
	return (void *)object;
}

/* No safe check, caller should be careful. */
void slab_free_object(struct slab *slab, void *addr)
{
	struct slab_object *object = (struct slab_object *)addr;
	struct page *p = PN2PG(VPN(addr));
	struct slab_cache *cache = PGSLABCACHE(p);
	/* Free object(list it into cache) */
	object->next = cache->object;
	cache->object = object;
	/*
	 * Update free cache list:
	 *  When free objects number of cache exceeds quota,
	 *  we put the cache into free cache list, in which case
	 *  slab_lookup_cache() can get cache from it.
	 */
	cache->free++;
	if (cache != slab->current) {
		if (cache->free == SLAB_CACHE_FREE_QUOTA(cache))
			list_move(&cache->list, &slab->free_cache);
		if (cache->free == cache->total)
			slab_flush_caches++;
	}
}

static void init_slab(struct slab *slab, int size)
{
	slab->size = size;
	slab->pages = 1 << log2up(SLAB_PGS(size));
	slab->current = NULL;
	list_init(&slab->free_cache);
	list_init(&slab->cache);
}

struct slab *alloc_slab(int size)
{
	struct slab *slab;
	if (size > SLAB_OBJECT_MAX_SIZE)
		return NULL;
	slab = slab_alloc_object(&slab_cube);
	if (!slab)
		return NULL;
	init_slab(slab, size);
	/* prealloc a cache */
	slab->current = slab_alloc_cache(slab);
	if (!slab->current) {
		slab_free_object(&slab_cube, slab);
		return NULL;
	}
	/* list to global slab list */
	list_add(&slab->list, &slab_list);
	return slab;
}

void free_slab(struct slab *slab)
{
	/* kill slab cache */
	slab_free_cache(slab);
	/* free slab to slab cache */
	list_del(&slab->list);
	slab_free_object(&slab_cube, slab);
}

void *kmalloc(int size)
{
	int i;
	if (size < 0 || size > KMALLOC_MAX_SIZE)
		return NULL;
	size = (size - 1) >> 2;
	for (i = 0; i < KMALLOC_CUBES; i++) {
		if (!size)
			return slab_alloc_object(cubes[i]);
		size >>= 1;
	}
	return NULL;
}

void kfree(void *addr)
{
	struct page *p = PN2PG(VPN(addr));
	slab_free_object(PGSLAB(p), addr);
}

//#define TEST_SLAB_ALLOC_OBJECTS 10
//#define TEST_SLAB_ALLOC
#ifdef TEST_SLAB_ALLOC_OBJECTS
void test_kmalloc_pages(int i)
{
	struct slab *slab = cubes[i];
	struct slab_cache *cache;
	int n = 0;
	list_for_each_entry(cache, &slab->cache, list)
		n += slab->pages;	
	list_for_each_entry(cache, &slab->free_cache, list)
		n += slab->pages;	
	printk("kmalloc(%d) %d pages(%d/%d)\n",
		4 << i, n, n - slab->pages, slab->pages);
}

void test_last_page(void)
{
	struct page *p;
	int i = 0;
	while ((p = get_free_page()) != NULL)
		i++;
	printk("last pages %d\n", i);
}

int test_kmalloc(void)
{
	void *prev;
	void *first;
	void *next;
	void *p;
	int size = 2048;
	int i = 0;
	/* test */
	printk("kmalloc(%4d) ", size);
	prev = kmalloc(size);
	first = prev;
	/* alloc */
	while (1) {
		p = kmalloc(size);
		if (!p) {
			size /= 2;
			if (size == 0)
				break;
			continue;
		}
		*(unsigned int *)prev = (unsigned int)p;
		prev = p;
		i += size;
	}
	printk("%8xbytes %8dkb %8d mb ", i, i >>10, i >>20);
	/* free */
	p = first;
	while (p) {
		next = (void *)*(unsigned int *)p;
		kfree(p);
		p = next;
	}
	test_last_page();
	return i;
}

void test_slab(void)
{
	int prev;
	int i = 0;
	prev = test_kmalloc();
	while (i < TEST_SLAB_ALLOC_OBJECTS) {
		if (prev != test_kmalloc())
			panic("!=");
		i++;
	}
}
#elif defined(TEST_SLAB_ALLOC)

int test_objects(struct slab *slab)
{
	struct slab_cache *cache;
	int n = 0;
	int i;
	list_for_each_entry(cache, &slab->cache, list)
		n += cache->total - cache->free;	
	i = n;
	printk("[co %d", i);
	list_for_each_entry(cache, &slab->free_cache, list)
		n += cache->total - cache->free;	
	printk(" fo %d]", n - i);
	return n;
}

int test_pages(struct slab *slab)
{
	struct slab_cache *cache;
	int n = 0;
	int i;
	list_for_each_entry(cache, &slab->cache, list)
		n += slab->pages;	
	i = n;
	printk("[cp %d", i);
	list_for_each_entry(cache, &slab->free_cache, list)
		n += slab->pages;	
	printk(" fp %d]", n - i);
	return n;
}

int test_slab_alloc(void)
{
	struct slab *slab;
	int n, nn;
	void *first, *p, *prev;
	slab = alloc_slab(234);
	n = 1;
	first = slab_alloc_object(slab);
	prev = first;
	while (1) {
		p = slab_alloc_object(slab);
		if (!p)
			break;
		n++;
		*(unsigned int *)prev = (unsigned int)p;
		prev = p;
	}
	nn = n;
	prev = first;
	while (1) {
		p = (void *)*(unsigned int *)prev;
		slab_free_object(slab, prev);
		n--;
		if (n == 0)
			break;
		prev = p;
	}
	printk("alloc %d\n", nn);
	/* nn */
	n = 1;
	first = slab_alloc_object(slab);
	prev = first;
	while (1) {
		p = slab_alloc_object(slab);
		if (!p)
			break;
		n++;
		*(unsigned int *)prev = (unsigned int)p;
		prev = p;
	}
	if (nn != n)
		panic("%d != %d", nn, n);
	free_slab(slab);
	return nn;
}

int test_slab_alloc2(void)
{
	struct list_head list;
	struct slab *slab;
	int i = 0;
	int k = 0;
	list_init(&list);
	while (1) {
		slab = alloc_slab(234);
		if (!slab)
			break;
		list_move(&slab->list, &list);
		i++;
	}
	printk("alloc %d slabs\n", i);
	while (!list_empty(&list)) {
		slab = list_first_entry(&list, struct slab, list);
		free_slab(slab);
		k++;
	}
	if (i != k)
		panic("Alloced slabs number doesnt equal with freed slabs number\n");
	return i;
}

extern struct page_zone zones[];
void test_debug(char *str)
{
	if (str)
		printk("%s ", str);
	printk("fpages %d spages %d slab objects %d\n",
		zones[1].free, test_pages(&slab_cube), test_objects(&slab_cube));
}

void test_slab(void)
{
	int k, kk;
	test_debug(NULL);
	k = test_slab_alloc();
	if (k != test_slab_alloc())
		panic("test slab alloc error");
	test_debug(NULL);
	kk = test_slab_alloc2();
	if (kk != test_slab_alloc2())
		panic("test slab alloc 2 error");
	test_debug("[-]");
	flush_slab(&slab_cube);
	test_debug("[+]");
	if (k != test_slab_alloc())
		panic("test slab alloc error");
}
#else
#define test_slab()
#endif

void slab_init(void)
{
	int i;

	/* We cannt flush slab cache during initialization. */
	slab_flush_caches = 0;
	list_init(&slab_list);

	/* Create slab-structure slab manually. */
	init_slab(&slab_cube, sizeof(struct slab));
	slab_cube.current = slab_alloc_cache(&slab_cube);
	if (!slab_cube.current)
		panic("alloc cache for slab cube");
	list_add(&slab_cube.list, &slab_list);
	printk("init slab cube\n");
	/* Init kmalloc_slab */
	for (i = 0; i < KMALLOC_CUBES; i++) {
		cubes[i] = alloc_slab(4 << i);
		if (!cubes[i])
			panic("alloc cache for kmalloc(%d) cube", 4 << i);
	}
	printk("init kmalloc cubes\n");
	test_slab();
}

