#include <string.h>
#include <print.h>
#include <list.h>
#include <page.h>
#include <mm.h>

struct page_zone zones[PG_ZONES];

/* all page descriptors */
struct page *page_table;
unsigned int npage;

void zone_debug(int type)
{
	struct page_zone *zone = &zones[type];
	struct page_area *area = zone->area;
	int i;
	printk("zone %2d start %8d free %8x total %8x\n",
		type, zone->start, zone->free, zone->total);
	printk(" area free list: ");
	for (i = 0; i < PG_ORDERS; i++) {
		printk("%5d", area->free_num);
		area++;
	}
	printk("\n");
}

static void _inline area_del_page(struct page_area *area, struct page *p)
{
	area->free_num--;
	unsetpagebuddy(p);
	list_del_init(&p->pg_list);
}

static void _inline area_add_page(struct page_area *area, struct page *p)
{
	area->free_num++;
	setpagebuddy(p);
	list_add(&p->pg_list, &area->free_list);
}

#define PG_ORDER_MASK(order) ((1 << (order)) - 1)
static void free_zone_pages(struct page_zone *zone, struct page *p, int order)
{
	struct page_area *area = &zone->area[order];
	struct page *buddy;
	unsigned int idx;

	idx = PG2PN(p);
	if (idx & PG_ORDER_MASK(order))
		return;
	zone->free += (1 << order);
	while (order < PG_MAX_ORDER) {
		buddy = &page_table[idx ^ (1 << order)];
		/* detect whether @buddy is our real buddy page */
		if (!page_buddy(buddy) || buddy->pg_order != order)
			break;
		/* pop buddy */
		area_del_page(area, buddy);
		buddy->pg_order = PG_NIL_ORDER;
		if (area->free_num < 0) {
			panic("area %d page %d %s",
			area - zone->area, PG2PN(buddy),
				page_buddy(buddy)?"buddy":"Unbuddy");
		}
		/* for next loop */
		idx &= ~(1 << order);
		order++;
		area++;
	}
	/* push buddy */
	p = &page_table[idx];
	area_add_page(area, p);
	p->pg_order = order;
}

void free_pages(struct page *page, int order)
{
	if (order < 0 || order > PG_MAX_ORDER || page_reserved(page) ||
		page_buddy(page) || page->pg_order != PG_NIL_ORDER) {
		printk("Free pages error\n");
		return;
	}
	if (page->pg_refcnt || page->pg_cowshare)
		panic("Free in-use pages");
	free_zone_pages(&zones[page->pg_zone], page, order);
}

static struct page *alloc_zone_pages(struct page_zone *zone, int order)
{
	struct page_area *area = &zone->area[order];
	struct page *p;
	int alloc_order = order;
	/* get minium alloc_order, which is larger than order */
	while (alloc_order < PG_ORDERS) {
		if (area->free_num > 0)
			break;
		area++;
		alloc_order++;
	}
	if (alloc_order >= PG_ORDERS)
		return NULL;
	p = list_first_entry(&area->free_list, struct page, pg_list);
	if (!page_buddy(p))
		panic("Unbuddy?");

	area_del_page(area, p);
	p->pg_order = PG_NIL_ORDER;
	/* We always alloc last pages! */
	while (alloc_order > order) {
		alloc_order--;
		area--;
		area_add_page(area, p);
		p->pg_order = alloc_order;
		p += (1 << alloc_order);
	}
	zone->free -= 1 << order;
	if (page_buddy(p))
		panic("Buddy?");
	return p;
}

struct page *alloc_pages(int zone, int order)
{
	if (order < 0 || order > PG_MAX_ORDER ||
		zone > PG_ZONE_HIGH || zone < PG_ZONE_LOW) {
		printk("Alloc pages error: zone %d order %d\n", zone, order);
		return NULL;
	}
	return alloc_zone_pages(&zones[zone], order);
}

static void zone_init(int type, int start, int end)
{
	struct page_zone *zone = &zones[type];
	struct page_area *area = zone->area;
	struct page *p = &page_table[start];
	int i;

	zone->free = 0;
	zone->total = max(end - start + 1, 0);
	zone->start = zone->total > 0 ? start : -1;
	/* init area */
	for (i = 0; i < PG_ORDERS; i++) {
		list_init(&area->free_list);
		area->free_num = 0;
		area++;
	}
	/* init zone pages */
	for (i = start; i <= end; i++) {
		setpagezone(p, type);
		p->pg_order = PG_NIL_ORDER;
		if (!page_reserved(p))
			free_pages(p, 0);
		p++;
	}
}

extern void *boot_memory_exit(void);
extern void e820_memory_reserved(void);
void pages_init(void)
{
	int i, n;
	page_table = boot_alloc(sizeof(struct page), npage);
	memset(page_table, 0x0, npage * sizeof(struct page));
	/* reserve e820 map reserved memory */
	e820_memory_reserved();
	/* reserve alloced boot memory */
	n = PN(boot_memory_exit());
	for (i = NOR_MEM_PN; i < n; i++)
		setpagereserved(&page_table[i]);
	printk("Normal free memory start from %dth page\n", n);
	/* init low memory */
	zone_init(PG_ZONE_LOW, LOW_MEM_PN, NOR_MEM_PN - 1);
	zone_init(PG_ZONE_NOR, NOR_MEM_PN, min(npage, HIGH_MEM_PN) - 1);
	zone_init(PG_ZONE_HIGH, HIGH_MEM_PN, npage - 1);

	zone_debug(PG_ZONE_LOW);
	zone_debug(PG_ZONE_NOR);
	zone_debug(PG_ZONE_HIGH);
#ifdef DEBUG_LOW_MEM
	while (alloc_pages(0, 0) != NULL);
	zone_debug(PG_ZONE_LOW);
	for (i = 0; i < NOR_MEM_PN; i++)
		free_pages(page_table+i, 0);
	zone_debug(PG_ZONE_LOW);
#endif
}
