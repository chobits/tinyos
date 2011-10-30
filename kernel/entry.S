#include <asm.h>
#include <x86.h>
#include <mm.h>

.code32
.section ".entry.text", "ax"

/*
 * All kernel is mapped to KERNEL_BASE + 1MB, which is at physical address 1MB
 * We play a trick to make the virtual address `change` to physical address
 * via segment base address: -KERNEL_BASE.
 */

_ENTRY(kernel_start)
	/* ignore interrupt still */
	lgdt PADDR(gdt_desc)
	/* init segments and registers */
	movw $KERN_DATA_SEG, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	movl $ENTRY_STACK_TOP, %esp	/* init entry stack */
	xorl %ebp, %ebp			/* clear frame pointer */
	ljmp $KERN_CODE_SEG, $init	/* kernel/init.c:init() */

.section ".entry.data", "a"
.align 4
_ENTRY(gdt)
		/*
		 * GDT: first inited in entry.S
		 *       last inited in init.c
		 * NOTE: limit must be set 4G.
		 *       limit means offset range of `seg:offset`.
		 */
		/* none seg */
		GDT_SEG_NONE
		/* kernel code seg: base -3G, limit 4G, execute/read */
		GDT_SEG(-KERNEL_BASE, 0xffffffff, STA_X|STA_R)
		/* kernel data seg: base -3G, limit 4G, write */
		GDT_SEG(-KERNEL_BASE, 0xffffffff, STA_W)
		/* user code seg */
		GDT_SEG_NONE
		/* user data seg */
		GDT_SEG_NONE
		/* tss seg */
		GDT_SEG_NONE
		/* ldt seg */
		GDT_SEG_NONE
		/* none seg (align to 8 segs) */
		GDT_SEG_NONE

_ENTRY(gdt_desc)
		.word 63		/* gdt limit: gdt size - 1 */
		.long PADDR(gdt)	/* gdt linear address(using physical
					 * address, the paging mode is not open
					 * now) */
