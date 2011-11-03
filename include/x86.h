#ifndef _X86_H
#define _X86_H

#define EFLAGS_CF	0x1
#define EFLAGS_BIT1	0x2		/* always 1 */
#define EFLAGS_PF	0x4
#define EFLAGS_BIT3	0x8		/* always 0 */
#define EFLAGS_AF	0x10
#define EFLAGS_BIT5	0x20		/* always 0 */
#define EFLAGS_ZF	0x40
#define EFLAGS_SF	0x80
#define EFLAGS_TF	0x100
#define EFLAGS_IF	0x200
#define EFLAGS_DF	0x400
#define EFLAGS_OF	0x800
#define EFLAGS_IOPL	0x3000		/* IOPL: 2 bits */
#define EFLAGS_NT	0x4000		/* if set, iret will do task switch, not interrupt entering! */
#define EFLAGS_BIT15	0x8000		/* always 0 */
#define EFLAGS_RF	0x10000
#define EFLAGS_VM	0x20000
#define EFLAGS_AC	0x40000
#define EFLAGS_VIF	0x80000
#define EFLAGS_VIP	0x100000
#define EFLAGS_ID	0x200000
/* bit 22-31 is reserved(set to 0) */
#define EFLAGS_IOPL_MASK	(~EFLAGS_IOPL)
#define EFLAGS_IOPL_0		0x0000
#define EFLAGS_IOPL_1		0x1000
#define EFLAGS_IOPL_2		0x2000
#define EFLAGS_IOPL_3		0x3000

#define CR0_PG 0x80000000	/* Paging */
#define CR0_CD 0x40000000	/* Cache disable */
#define CR0_NW 0x20000000	/* Not-write through */
#define CR0_AM 0x40000		/* Alignment mask */
#define CR0_WP 0x10000		/* Write protect */
#define CR0_NE 0x20		/* Numeric error */
#define CR0_ET 0x10		/* Extension type */
#define CR0_TS 0x8		/* Task switched */
#define CR0_EM 0x4		/* Emulation */
#define CR0_MP 0x2		/* Monitor co-processor */
#define CR0_PE 0x1		/* Protected Mode Enable */

/* for *.S */
#define GDT_SEG_NONE	.quad 0
#define GDT_SEG(base, limit, type)				\
	.word (((limit) >> 12) & 0xffff), ((base) & 0xffff);	\
	.byte (((base) >> 16) & 0xff), (0x90 | (type)),		\
	(0xC0 | (((limit) >> 28) & 0xf)), (((base) >> 24) & 0xff)

/* application segment type bits */
#define STA_X	0x8	/* Executable segment */
#define STA_E	0x4	/* Expand down (non-executable segments) */
#define STA_C	0x4	/* Conforming code segment (executable only) */
#define STA_W	0x2	/* Writeable (non-executable segments) */
#define STA_R	0x2	/* Readable (executable segments) */
#define STA_A	0x1	/* Accessed */

/* system segment type bits */
#define STS_T16A	0x1	/* Available 16-bit TSS */
#define STS_LDT		0x2	/* Local Descriptor Table */
#define STS_T16B	0x3	/* Busy 16-bit TSS */

/* gdt entry index */
#define GDT_ENTRY_RESERVED	0	/* reserved entry */
#define GDT_ENTRY_KERN_CS	1
#define GDT_ENTRY_KERN_DS	2
#define GDT_ENTRY_USER_CS	3
#define GDT_ENTRY_USER_DS	4
#define GDT_ENTRY_TSS		5
#define GDT_ENTRY_LDT		6
#define GDT_ENTRY_END		7	/* dummy entry */

/* segment selector */
#define KERN_CODE_SEG (GDT_ENTRY_KERN_CS * 8)
#define KERN_DATA_SEG (GDT_ENTRY_KERN_DS * 8)
/* user cs selector */
#define USER_CODE_SEG ((GDT_ENTRY_USER_CS * 8) | 3)
/* user ds(gs,..) selector */
#define USER_DATA_SEG ((GDT_ENTRY_USER_DS * 8) | 3)
/* segment selector , offset from gdt */
#define TSS_SEG	(GDT_ENTRY_TSS * 8)
#define LDT_SEG (GDT_ENTRY_LDT * 8)

#ifndef __ASM__

#include <types.h>
#include <compile.h>

static _inline void cli(void)
{
	asm volatile ("cli" ::);
}

static _inline void sti(void)
{
	asm volatile ("sti" ::);
}

static _inline u8 inb(u16 port)
{
	u8 __value;
	asm volatile ("inb %1, %0" : "=a"(__value) : "d"(port));
	return __value;
}

static _inline u8 inb_p(u16 port)
{
	u8 __value;
	asm volatile ("inb %1, %0;"
			"jmp 1f;"
			"1: jmp 1f;"
			"1:"
			 : "=a"(__value) : "d"(port));
	return __value;
}


static _inline void outb(u16 port, u8 value)
{
	asm volatile ("outb %0,%1" :: "a"(value),"d"(port));
}

static _inline void outb_p(u16 port, u8 value)
{
	asm("outb %0,%1;"
		"jmp 1f;"
		"1:;"
		"jmp 1f;"
		"1:"
		:: "a"(value), "d"(port));
}

static _inline void insl(u16 port, void *addr, int count)
{
        asm volatile ("cld\n\trepne\n\tinsl"
			: "=D" (addr), "=c" (count)
			: "d" (port), "0" (addr), "1" (count)
			: "memory", "cc");
}

static _inline void outsl(u16 port, void *addr, int count)
{
        asm volatile ("cld\n\trepne\n\toutsl"
			: "=S" (addr), "=c" (count)
			: "d" (port), "0" (addr), "1" (count)
			: "memory", "cc");
}

static _inline void ltr(u16 value)
{
	asm volatile ("ltr %0" ::"a" (value));
}

static _inline void lldt(u16 value)
{
	asm volatile ("lldt %0" ::"a" (value));
}

static _inline void load_cr3(u32 value)
{
	asm volatile ("movl %0, %%cr3"::"r" (value));
}

static _inline void load_cr0(u32 value)
{
	asm volatile ("movl %0, %%cr0"::"r" (value));
}

static _inline u32 read_cr0(void)
{
	u32 cr0;
	asm volatile ("movl %%cr0, %0" : "=r"(cr0):);
	return cr0;
}

static _inline u32 read_cr2(void)
{
	u32 cr2;
	asm volatile ("movl %%cr2, %0" : "=r"(cr2):);
	return cr2;
}

static _inline void flush_tlb(void)
{
	u32 cr3;
	asm volatile ("movl %%cr3,%0" : "=r" (cr3));
	asm volatile ("movl %0,%%cr3" :: "r" (cr3));
}

static _inline void hlt(void)
{
	asm volatile ("hlt;" ::);
}

static _inline void lidt(u32 idt_desc)
{
	asm volatile ("lidt (%0)" ::"r" (idt_desc));
}

struct desc {
	unsigned int low;
	unsigned int high;
};

struct table_desc {
	u16 limit;
	u32 base;
} __attribute__((packed));

extern struct desc gdt[];

struct tss {
	unsigned int prev_tss;	/* segment selector for the TSS of the previous task */
	unsigned int esp0,ss0;
	unsigned int esp1,ss1;
	unsigned int esp2,ss2;
	unsigned int cr3;	/* reloaded on a hardware task switch */
	unsigned int eip;
	unsigned int eflags;
	unsigned int eax,ecx,edx,ebx;
	unsigned int esp,ebp,esi,edi;
	unsigned int es,cs,ss,ds,fs,gs;
	unsigned int ldt;	/* the segment selector for the LDT */
	unsigned short T;	/* bit 0 for the debug trap flag */
	/*
	 * offset from the base of the TSS to the I/O permission bit map
	 * and interrupt redirection bitmap
	 */
	unsigned short io_bitmap_addr;
} __attribute__((packed));


struct ldt {
	struct desc none;
	struct desc code;
	struct desc data;
};

/*
 * L	 — 64-bit code segment (IA-32e mode only) -- 0
 * AVL	 — Available for use by system software -- 0
 * BASE	 — Segment base address
 * D/B	 — Default operation size (0 = 16-bit segment; 1 = 32-bit segment) -- 1
 * DPL	 — Descriptor privilege level -- 0/3
 * G	 — Granularity (0 = LIMIXT:Byte unit; 1 = LIMIT:4-Kbyte units) -- 1
 * LIMIT — Segment Limit
 * P	 — Segment present -- 1
 * S	 — Descriptor type (0 = system; 1 = code or data) -- 1
 * TYPE	 — Segment type
 */
static _inline void set_seg_desc(struct desc *seg_desc, u32 base,
					u32 limit, u8 g_db_l_avl, u8 p_dpl_s,
					u8 type)
{
	seg_desc->low = (limit & 0xffff) | (base << 16);
	seg_desc->high = (base & 0xff000000) | (g_db_l_avl << 20) |
			(limit & 0x000f0000) | (p_dpl_s << 12) |
			(type << 8) | ((base >> 16) & 0xff);
}
/* g,d/b,l,avl,dpl,s = 0, type = 9, limit = 103(No I/O Permissions Bit Map) */
#define set_tss_desc(base)\
	set_seg_desc(&gdt[GDT_ENTRY_TSS], base, 0x67, 0x0, 0x8, 0x9)
/* g,d/b,l,avl,dpl,s = 0, type = 9, limit = 23 */
#define set_ldt_desc(base)\
	set_seg_desc(&gdt[GDT_ENTRY_LDT], base, 0x17, 0x0, 0x8, 0x2)
/* g,d,p,s = 1 type = 0xa (exec/read) */
#define set_kerncode_seg(base, limit)\
	set_seg_desc(&gdt[GDT_ENTRY_KERN_CS], base, (limit) >> 12, 0xc, 0x9, 0xa)
/* g,d,p,s = 1 type = 0x2 (read/write) */
#define set_kerndata_seg(base, limit)\
	set_seg_desc(&gdt[GDT_ENTRY_KERN_DS], base, (limit) >> 12, 0xc, 0x9, 0x2)
/* g,d,p,s = 1 type = 0xa (exec/read) dpl = 3 */
#define set_usercode_seg(base, limit)\
	set_seg_desc(&gdt[GDT_ENTRY_USER_CS], base, (limit) >> 12, 0xc, 0xf, 0xa)
/* g,d,p,s = 1 type = 0x2 (read/write) dpl = 3 */
#define set_userdata_seg(base, limit)\
	set_seg_desc(&gdt[GDT_ENTRY_USER_DS], base, (limit) >> 12, 0xc, 0xf, 0x2)

static _inline void set_gate(struct desc *gate, u8 type, u8 dpl, u32 addr)
{
	/*
	 * 0x0008 --> 16~31bit --> segment selector
	 * addr   --> 0~15bit and 48~63bit --> offset
	 * dpl    --> 45~46bit
	 * 0      --> 44bit
	 * type   --> 40~43bit
	 * P      --> 47bit
	 */
	gate->low = (0x00080000 | (addr & 0xffff));
	gate->high = ((addr & 0xffff0000) | 0x8000 |
			((dpl << 13) & 0x6000) | ((type << 8) & 0x0f00));
}

#define set_trap_gate(n, addr) set_gate(&idt[n], 0xf, 0, (u32)(addr))
#define set_system_gate(n, addr) set_gate(&idt[n], 0xf, 3, (u32)(addr))
#define set_int_gate(n, addr) set_gate(&idt[n], 0xe, 0, (u32)(addr))

#endif	/* !__ASM__ */

#endif	/* x86.h */
