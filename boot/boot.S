#include <asm.h>
#include <boot.h>
#include <x86.h>

.code16

_ENTRY(boot_start)
	/* close interrupt */
	cli
	cld

	/* init registers */
	movw $0x07c0, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %gs
	movw %ax, %fs
	movw %ax, %ss
	movw $0x00, %sp		/* set stack top to 0x7c00 */

	/*
	 * Enable A20: although there are other methods, this method
	 * using keyboard controller can work well in any case.
	 * We can access physical memory above 1MB.
	 */
	call wait_A20
	movb $0xd1, %al		/* command: write */
	outb %al, $0x64
	call wait_A20
	movb $0xdf, %al		/* enable */
	outb %al, $0x60
	call wait_A20

	/* load second-part boot loader to memory: [0x7e00, 0x8c00) */
load_drive:
	movw $0x0207, %ax	/* ah: 02h        al: 07h(sectors) */
	movw $0x0002, %cx	/* ch: 00h(track) cl: 02h(sector) */
	movw $0x0080, %dx	/* dh: 00h(head)  cl: 80h(drive: disk) */
	movw $0x0200, %bx	/* es:bx (buffer address pointer) */
	int $0x13
	jnc load_done		/* Carry bit is set for error. */
reset_drive:
	/* reset drive */
	movb $0x00, %ah
	movb $0x80, %dl
	int $0x13
	jc reset_drive
	jmp load_drive

load_done:
	jmp boot_next		/* jmp to 0x7e00, not changing CS */

wait_A20:
	inb $0x64, %al
	test $0x2, %al
	jnz wait_A20
	ret

.org 508
.word 0x0000	/* for sects of kernel image */
.word 0xaa55

_ENTRY(boot_next)
	/*
	 * Detecting memory via E820 method
	 * If this method fails, we dont try other methods,
	 *  just killing the machine.
	 * E820 entries is saved in [0x8c00, 0x9800). (128 entries:3KB)
	 * E820 and other parameter are saved in [0x9800, 0x10000)
	 */
	xorl %ebp, %ebp			/* entry number */
	xorl %ebx, %ebx			/* ebx must be 0 to start */
	movl $0x1000, %edi		/* e820 list(es:edi 0x07c0:0x1000) */
loop_e820:
	movl $SMAP, %edx 		/* magical number: "SMAP" */
	movl $0xe820, %eax
	movl $E820_ENTRY_SIZE, %ecx
	int $0x15
	jc end_e820
	cmpl $SMAP, %eax
	jne fail_e820			/* %eax != SMAP for error */
	jcxz next_e820			/* ignore 0-length entry */
	incl %ebp			/* increase entry number */
	addl $E820_ENTRY_SIZE, %edi	/* %edi points next entry space */
next_e820:
	test %ebx, %ebx			/* if ebx equals 0, list ends */
	je end_e820
	movl %ebp, %eax
	cmpl $E820_MAX, %eax
	je fail_e820			/* entry number >= E820_MAX for error */
	jmp loop_e820
end_e820:
	movl %ebp, %eax
	cmpl $0x01, %eax		/* entry number <= 1 for error */
	jbe fail_e820
	movl %ebp, (0x1c00)		/* save e820 entry number to 0x9800 */
	jmp next_protect_mode
fail_e820:
	jmp fail_e820			/* end of boot trap */

	/* go to protect mode */
next_protect_mode:
	/* Note: we dont load idt, but interrupt has been disabled! */
	/* load gdt: [ds:boot_gdt_desc]*/
	lgdt boot_gdt_desc
	/* turn on protected mode */
	movl %cr0, %eax
	orl $CR0_PE, %eax
	movl %eax, %cr0
	ljmp $KERN_CODE_SEG, $BOOT_PADDR(start_protect_mode)

start_protect_mode:
.code32
	movw $KERN_DATA_SEG, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	movl $0x7c00, %esp		/* reset stack top to 0x7c00 */

	call boot_main

die:	/* Should it be here? */
	jmp die

.align 4
boot_gdt:
		/* none seg */
		GDT_SEG_NONE
		/* code seg: base 0, limit 4G, execute/read, */
		GDT_SEG(0x0, 0xffffffff, STA_X|STA_R)
		/* data seg: base 0, limit 4G, write */
		GDT_SEG(0x0, 0xffffffff, STA_W)
boot_gdt_desc:
		.word 0x17			/* gdt limit: gdt size - 1 */
		.long BOOT_PADDR(boot_gdt)	/* gdt address */


