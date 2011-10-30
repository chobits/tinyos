#ifndef __PAGE_H
#define __PAGE_H

#include <types.h>
#include <compile.h>
#include <list.h>

#define LOW_MEM_START	0x00000000	/* low memory:    [0, 16M) */
#define NOR_MEM_START	0x01000000	/* normal memory: [16M, 768M) */
#define HIGH_MEM_START	0x30000000	/* high memory:   [768M, ~) */

#define PGSHIFT		12
#define PGMASK		(PGSIZE - 1)
#define PGOFFSET(v)	(((unsigned int)(v)) & PGMASK)
#define PN(addr)	(((unsigned int )addr) >> PGSHIFT)
#define VPN(vaddr)	PN(PADDR(vaddr))
#define PNADDR(pn)	((pn) << PGSHIFT)
#define PNVADDR(pn)	VADDR(PNADDR(pn))

#define PGS_UP(size)	(((size) + PGMASK) >> PGSHIFT)
#define PG_SIZE(n)	PNADDR(n)

#define LOW_MEM_PN	PN(LOW_MEM_START)
#define NOR_MEM_PN	PN(NOR_MEM_START)
#define HIGH_MEM_PN	PN(HIGH_MEM_START)

#define PG_MAX_ORDER	10
#define PG_ORDERS	(PG_MAX_ORDER + 1)
#define PG_NIL_ORDER	-1

#define PG_ZONE_LOW	0
#define PG_ZONE_NOR	1
#define PG_ZONE_HIGH	2
#define PG_ZONES	3

#define PG2PN(p)	(((struct page *)p) - page_table)
#define PG2ADDR(p)	PNADDR(PG2PN(p))
#define PG2VADDR(p)	PNVADDR(PG2PN(p))
#define PN2PG(pn)	(&page_table[pn])
#define ADDR2PG(pa)	PN2PG(PN(pa))
#define VADDR2PG(va)	PN2PG(VPN(va))

struct page_area {
	struct list_head free_list;
	int free_num;
};

struct page_zone {
	struct page_area area[PG_ORDERS];
	int start;	/* start page number */
	int total;	/* total page number */
	int free;	/* free page number */
};

struct page {
	int pg_refcnt;
	struct list_head pg_list;
	unsigned int	pg_zone:2,
			pg_reserved:1,	/* page is reserved */
			pg_buddy:1;	/* page is in area::free_list */
	int pg_order;			/* area order */
	int pg_cowshare;		/* copy-on-page share numbers */
	void *pg_private;
};

static _inline void setpagezone(struct page *p, int type)
{
	p->pg_zone = type;
}

/* name: capital letter */
#define __PAGE_FLAG_CLEAR(name)\
	static _inline void unsetpage##name(struct page *page)\
	{ page->pg_##name = 0; }

#define __PAGE_FLAG_SET(name)\
	static _inline void setpage##name(struct page *page)\
	{ page->pg_##name = 1; }

#define __PAGE_FLAG_DETECT(name)\
	static _inline int page_##name(struct page *page)\
	{ return !!page->pg_##name; }

#define PAGE_FLAGS(name)\
	__PAGE_FLAG_CLEAR(name)\
	__PAGE_FLAG_SET(name)\
	__PAGE_FLAG_DETECT(name)

PAGE_FLAGS(reserved);
PAGE_FLAGS(buddy);

extern struct page *page_table;
extern unsigned int npage;

#endif	/* page.h */
