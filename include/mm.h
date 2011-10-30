#ifndef __MM_H
#define __MM_H

/* make entry stack overlap bootloader */
#define ENTRY_STACK_TOP	VADDR(0x8c00)
#define KERNEL_BASE	0xc0000000

#ifdef __ASM__

/* Macro for entry.S */
#define PADDR(va)	((va) - KERNEL_BASE)
#define VADDR(pa)	((pa) + KERNEL_BASE)

#else	/* !__ASM__ */

/* kernel address space: [0xc0000000, 0xffffffff] */
#define PADDR(va)	((unsigned int)(va) - KERNEL_BASE)
#define VADDR(pa)	((unsigned int)(pa) + KERNEL_BASE)

/* user stack address */
#define USER_STACK_SIZE	PGSIZE
#define USER_STACK_TOP	(KERNEL_BASE - PGSIZE)	/* reserve a page hole */
#define USER_STACK	(USER_STACK_TOP - USER_STACK_SIZE)

/* kernel stack size: 8KB */
#define KERN_STACK_PAGES 2
#define KERN_STACK_SIZE (KERN_STACK_PAGES * PGSIZE)
/* kernel image starts from physical memory 16MB */
#define KERNEL_START	0x01000000
/* high memory start from physical memory 768MB */
#define HIGH_MEM	0x30000000
/* bootloader contains 4KB in disk */
#define KERNEL_START_SECT	8

#define PGSHIFT	12
#define PTSHIFT	22
#define PGSIZE	0x1000
#define NPDE(addr)	(((unsigned int)addr) >> PTSHIFT)
#define NPTE(addr)	(((unsigned int)addr) >> PGSHIFT)
/* page table size: 4MB (1024 * PGSIZE) */
#define PTSIZE	0x400000

#define KBASE_PDE	NPDE(KERNEL_BASE)
#define KBASE_PDE_SIZE	(PGSIZE - KBASE_PDE * sizeof(pde_t))
#define KBASE_PTE	NPTE(KERNEL_BASE)
#define KSTART_PDE	NPDE(KERNEL_START)
#define VKSTART_PDE	NPDE(VADDR(KERNEL_START))

extern unsigned int _stext[];
extern unsigned int _etext[];
extern unsigned int _sdata[];
extern unsigned int _edata[];
extern unsigned int __bss_start[];
extern unsigned int __bss_end[];
extern unsigned int _end[];
extern void *boot_alloc(int, int);
extern void *get_free_page(void);
extern void put_free_page(void *);
extern void *get_free_page_pa(void);
struct page;
extern struct page *alloc_pages(int zone, int order);
extern void free_pages(struct page *page, int order);
extern void free_page(struct page *page);
extern void *kva_page(struct page *page);
extern void get_page_ref(struct page *page);
extern void hold_page(struct page *page);
extern void zone_debug(int);

#endif	/* __ASM__ */

#endif	/* mm.h */
