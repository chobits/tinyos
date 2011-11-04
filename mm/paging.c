#include <paging.h>
#include <string.h>
#include <print.h>
#include <types.h>
#include <page.h>
#include <task.h>
#include <int.h>
#include <x86.h>
#include <mm.h>

pde_t *kern_page_dir;
pte_t *kern_page_table;
void *kern_stack;
void *kern_stack_top;
extern struct table_desc gdt_desc[];
//#define MOVE_TO_USERMODE

void paging_init(void)
{
	pte_t *pte;
	u32 cr0;
	int i, n;
	/* alloc page dir/entry table */
	kern_page_dir = (pde_t *)boot_alloc(PGSIZE, 1);
	kern_page_table = (pte_t *)boot_alloc(PGSIZE, PAGE_PDES);
	/*
	 * Reserve 2 pages for kernel space stack
	 * Copy current stack contents to new stack
	 *   (assert: size of current stack contents < PGSIZE)
	 * Swap stack before returning from pagint_init()
	 */
	kern_stack = boot_alloc(PGSIZE, KERN_STACK_PAGES);
	kern_stack_top = kern_stack + KERN_STACK_SIZE;
	i = PGSIZE - PGOFFSET(&n);	/* offset of stack top */
	memcpy(kern_stack_top - i, (void *)ENTRY_STACK_TOP - i, i);

	/*
	 * Map virtual address [KERNEL_BASE, KERNEL_BASE + HIGH_MEM)
	 * to physical address [0, HIGH_MEM)
	 */
	/* None map for address space [0, KERNEL_BASE) and high memory space */
	memset(kern_page_dir, 0x0, PGSIZE);
	/* Map for regular memory */
	n = KBASE_PDE + min(DIV_UP(npage, PAGE_PTES), NPDE(HIGH_MEM));
	pte = &kern_page_table[KBASE_PTE];
	for (i = KBASE_PDE; i < n; i++) {
		kern_page_dir[i] = PDE(PADDR(pte), PDE_DEFAULT);
		pte += PAGE_PTES;
	}
	/*
	 * Map va [0, 4M) to pa [0, 4M) for accessing entry
	 *  stack 0x7c00~0x8c00
	 * Map va [16, 32M) to pa [16, 32M) for accessing
	 *  instructions between `load_cr0` and `ljmp` safely.
	 *  Assert that the instructions is in [16, 32M).
	 */
	kern_page_dir[0] = kern_page_dir[KBASE_PDE];
	kern_page_dir[KSTART_PDE] = kern_page_dir[VKSTART_PDE];
	kern_page_dir[KSTART_PDE + 1] = kern_page_dir[VKSTART_PDE + 1];
	kern_page_dir[KSTART_PDE + 2] = kern_page_dir[VKSTART_PDE + 2];
	kern_page_dir[KSTART_PDE + 3] = kern_page_dir[VKSTART_PDE + 3];
	/* init the ptes of regular memory */
	memset(kern_page_table, 0x0, PTSIZE);
	pte = &kern_page_table[KBASE_PTE];
	n = min(npage, HIGH_MEM / PGSIZE);
	for (i = 0; i < n; i++)
#ifdef MOVE_TO_USERMODE
		pte[i] = PTE(i * PGSIZE, PTE_P | PTE_W|PTE_U);
#else
		pte[i] = PTE(i * PGSIZE, PTE_P | PTE_W);
#endif

	/*
	 * Make kernel stack bottom unaccessable for safe check.
	 * We can catch the overflow of kernel stack from pagefault.
	 */
	kern_page_table[NPTE(kern_stack)] = 0;

	/* open paging mode */
	load_cr3(PADDR(kern_page_dir));
	cr0 = read_cr0();
	cr0 |= CR0_PG | CR0_WP;		/* Paging & Write protect */
	load_cr0(cr0);
	/* update gdt and segments/registers */
	set_kerncode_seg(0, 0xffffffff);	/* base 0, limit 4G */
	set_kerndata_seg(0, 0xffffffff);	/* base 0, limit 4G */
	/* updating gdt base to kernel space */
	gdt_desc->base = VADDR(gdt_desc->base);
	asm volatile (
		"lgdt gdt_desc;"		/* reload kva of gdt_desc */
		"movw %%ax, %%ds;"
		"movw %%ax, %%es;"
		"movw %%ax, %%fs;"
		"movw %%ax, %%gs;"
		"movw %%ax, %%ss;"
		"ljmp %1, $1f;"
		"1:;"
		:: "a" (KERN_DATA_SEG), "i" (KERN_CODE_SEG));
	/*
	 * Now we dont need entry stack and can access
	 *  instructions in kernel text segment regularly.
	 * Unmap temp va [0, 4MB) of entry stack and
	 *  [16MB, 32) of kernel text segment.
	 */
 	kern_page_dir[0] = 0;
	kern_page_dir[KSTART_PDE] = 0;
	kern_page_dir[KSTART_PDE + 1] = 0;
	kern_page_dir[KSTART_PDE + 2] = 0;
	kern_page_dir[KSTART_PDE + 3] = 0;
	load_cr3(PADDR(kern_page_dir));

	printk("paging init\n");
	/* Swap stack pointer */
	asm volatile ("addl %0, %%esp;"
		:: "a"((u32)kern_stack_top - ENTRY_STACK_TOP));
}

pte_t *pte_lookup(pde_t *pgdir, unsigned int va, int create)
{
	pde_t pde = pgdir[VA_NPDE(va)];
	pte_t *pgtbl;

	if (!(pde & PDE_P)) {
		if (!create)
			return NULL;
		pde = (pde_t)get_free_page_pa();
		if (!pde)
			return NULL;
		memset((void *)VADDR(pde), 0x0, PGSIZE);
		pde |= PDE_DEFAULT;
		pgdir[VA_NPDE(va)] = pde;
	}
	pgtbl = (pte_t *)VADDR(PTE_PADDR(pde));
	return &pgtbl[VA_NPTE(va)];
}

void pmap_page(pde_t *pgdir, struct page *page, unsigned int va, unsigned int perm)
{
	pte_t *pte;
	if (PGOFFSET(va))
		panic("map unaligned page %08x", va);
	pte = pte_lookup(pgdir, va, 1);
	if (!pte)
		panic("No pte");
	if (*pte & PTE_P)
		put_free_page((void *)VADDR(PTE_PADDR(*pte)));
	*pte = PTE(PG2ADDR(page), perm | PTE_P);
}

struct page *map_page(pde_t *pgdir, unsigned int va, unsigned int perm)
{
	pte_t *pte, e;
	if (PGOFFSET(va))
		panic("map unaligned page %08x", va);
	pte = pte_lookup(pgdir, va, 1);
	e = *pte;
	if (!(e & PTE_P)) {
		e = (pte_t)get_free_page_pa();
		if (!e)
			return NULL;
		memset((void *)VADDR(e), 0x0, PGSIZE);
		e |= PTE_P | (perm & 0xfff);
		*pte = e;
	}
	return ADDR2PG(PTE_PADDR(e));
}

int pf_copyonwrite(struct regs *reg, unsigned int addr)
{
	struct page *page;
	void *p;
	pte_t *pte;
	if (addr >= KERNEL_BASE)
		return -1;
	pte = pte_lookup(ctask->us.pgdir, addr, 0);
	if (!pte || !(*pte & PTE_COW) || !PTE_PADDR(*pte) || !(*pte & PTE_U) || (*pte & PTE_W))
		return -1;

	/*
	 * handle orignal cow page
	 * free ref page = ADDR2PG(PTE_PADDR(*pte));
	 */
	page = ADDR2PG(PTE_PADDR(*pte));
	if (page->pg_cowshare == 0) {
		*pte &= ~PTE_COW;
		*pte |= PTE_W;
		return 0;
	}
	page->pg_cowshare--;
	free_page(page);
	/* alloc and copy orignal page */
	if (!(p = get_free_page_pa()))
		panic("Cannot get free page for copy-on-write page");
	memcpy((void *)VADDR(p), (void *)VADDR(PTE_PADDR(*pte)), PGSIZE);
	*pte = PTE(p, PTE_U | PTE_P | PTE_W);
	return 0;
}

void do_page_fault(struct regs *reg)
{
	unsigned int erraddr = read_cr2();
	unsigned int error = reg->error;
	int r = -1;

	if (((error & PF_ERROR_COW) == PF_ERROR_COW))
		r = pf_copyonwrite(reg, erraddr);
	if (r < 0) {
		printk("page fault at pid:%d erraddr:%08x error:%08x\n",
			ctask->pid, erraddr, error);
		dump_stack(reg);
		panic("!");
	}
}

