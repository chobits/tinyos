/*
 * high-level APIs associated pages
 */
#include <print.h>
#include <page.h>
#include <mm.h>

void *kva_page(struct page *page)
{
	/* If page is highmem page, we map it to kernel space address */
	/* Not implemented now! */
	return (void *)PG2VADDR(page);
}

void hold_page(struct page *page)
{
	page->pg_refcnt++;
}

void free_page(struct page *p)
{
	p->pg_refcnt--;
	if (p->pg_refcnt == 0) {
		if (p->pg_cowshare)
			panic("alloc page with cow share %d", p->pg_cowshare);
		free_pages(p, 0);
	} else if (p->pg_refcnt < 0)
		panic("Free free page(%d)\n", p->pg_refcnt);
}

void put_free_page(void *p)
{
	free_page(VADDR2PG(p));
}

/* using reference count in 0-order page */
void *get_free_page(void)
{
	struct page *p = alloc_pages(PG_ZONE_NOR, 0);
	if (!p)
		return NULL;
	if (p->pg_cowshare)
		panic("alloc page with cow share %d", p->pg_cowshare);
	p->pg_refcnt = 1;
	return (void *)PG2VADDR(p);
}

void *get_free_page_pa(void)
{
	struct page *p = alloc_pages(PG_ZONE_NOR, 0);
	if (!p)
		return NULL;
	if (p->pg_cowshare)
		panic("alloc page with cow share %d", p->pg_cowshare);
	p->pg_refcnt = 1;
	return (void *)PG2ADDR(p);
}

