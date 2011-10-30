#ifndef __PAGING_H
#define __PAGING_H

typedef unsigned int pte_t;
typedef unsigned int pde_t;

#define PTE_PADDR(pte)	(((unsigned int)(pte)) & 0xfffff000)
#define PDE_PADDR(pde)	PTE_PADDR(pde)
#define PTE(pa, flags)	(PTE_PADDR(pa) | (flags))
#define PDE(pa, flags)	PTE(pa, flags)
#define PAGE_PDES	1024
#define PAGE_PTES	PAGE_PDES
#define PGD_SHIFT	22
#define PTE_SHIFT	12
#define VA_NPDE(va)	(((unsigned int)(va)) >> 22)
#define VA_NPTE(va)	((((unsigned int)(va)) >> 12) & 0x3ff)
#define PDE_FLAGS(pde)	((pde) & 0xfff)

/*
 * Page is writable in these situations(intel manual volume 3A 4-6):
 *  1. CR0.WP = 0
 *  2. CR0.WP = 1(write protect)
 *      1) CPL < 3, PTE_W = 1
 *      2) CPL = 3, PTE_W = 1, PTE_U = 1
 */
/* PDE/PTE flags(intel manual volume 3A 4-11) */
#define PDE_P		0x001	/* present */
#define PDE_W		0x002	/* writable */
#define PDE_U		0x004	/* if 0, usermode cannot access the page */
#define PDE_PWT		0x008	/* page-level write-through */
#define PDE_PCD		0x010	/* page-level cache disable */
#define PDE_A		0x020	/* accessed */
#define PDE_BIT6	0x040	/* bit6 is ignored */
#define PDE_PS		0x080	/* if CR4.PSE = 1, must be 0*/
#define PDE_DEFAULT	(PDE_P | PDE_W | PDE_U)		/* give all priority to pde */

#define PTE_P		0x001
#define PTE_W		0x002
#define PTE_U		0x004	/* user readable */
#define PTE_PWT		0x008
#define PTE_PCD		0x010
#define PTE_A		0x020	/* access */
#define PTE_D		0x040	/* dirty */
#define PTE_PAT		0x080	/* PAT supported */
#define PTE_G		0x100	/* global */
#define PTE_COW		0x200	/* copy-on-write flag */
#define PTE_R1		0x400
#define PTE_R2		0x800
#define PTE_SOFT	0xe00	/* These bits are used by software, not hardware. */

#define pgpte(p)	*(pte_t *)(p)->pg_private

/*
 * Page-Fault Error code:
 *  P    0 The fault was caused by a non-present page.
 *       1 The fault was caused by a page-level protection violation.
 *  W/R  0 The access causing the fault was a read.
 *       1 The access causing the fault was a write.
 *  U/S  0 The access causing the fault originated when the processor
 *         was executing in supervisor mode.
 *       1 The access causing the fault originated when the processor
 *         was executing in user mode.
 *  RSVD 0 The fault was not caused by reserved bit violation.
 *       1 The fault was caused by reserved bits set to 1 in a page directory.
 *  I/D  0 The fault was not caused by an instruction fetch.
 *       1 The fault was caused by an instruction fetch.
 */
#define PF_ERROR_P	0x01
#define PF_ERROR_RW	0x02
#define PF_ERROR_US	0x04
#define PF_ERROR_RSVD	0x08
#define PF_ERROR_ID	0x10
#define PF_ERROR_COW	(PF_ERROR_P | PF_ERROR_RW)

extern struct page *map_page(pde_t *pgdir, unsigned int va, unsigned int perm);
extern void pmap_page(pde_t *pgdir, struct page *, unsigned int va, unsigned int perm);

#endif	/* paging.h */
