#ifndef __INT_H
#define __INT_H

#define NR_IRQS		16
#define INTNO_DE	0
/* Vector 1 is reserved */
#define INTNO_NMI	2
#define INTNO_BP	3
#define INTNO_OF	4
#define INTNO_BR	5
#define INTNO_UD	6
#define INTNO_NM	7
#define INTNO_DF	8
/* Vector 9 is reserved */
#define INTNO_TS	10
#define INTNO_NP	11
#define INTNO_SS	12
#define INTNO_GP	13
#define INTNO_PF	14
/* Vector 15 is reserved */
#define INTNO_MF	16
#define INTNO_AC	17
#define INTNO_MC	18
#define INTNO_XM	19
/* Vector 20-31 are reserved */
#define INTNO_IRQ0	32
#define INTNO_IRQ1	33
#define INTNO_IRQ2	34
#define INTNO_IRQ3	35
#define INTNO_IRQ4	36
#define INTNO_IRQ5	37
#define INTNO_IRQ6	38
#define INTNO_IRQ7	39
#define INTNO_IRQ8	40
#define INTNO_IRQ9	41
#define INTNO_IRQ10	42
#define INTNO_IRQ11	43
#define INTNO_IRQ12	44
#define INTNO_IRQ13	45
#define INTNO_IRQ14	46
#define INTNO_IRQ15	47
#define INTNO_SYSCALL	48
#define INTNO_MAX	INTNO_SYSCALL
#define IDT_SIZE	256

#define IRQ_TIMER	0	/* Programmable Interrupt Timer Interrupt */
#define IRQ_KEYBOARD	1	/* Keyboard Interrupt */
#define IRQ_CASCADE	2	/* Cascade (used internally by the two PICs. never raised) */
#define IRQ_COM2	3	/* COM2 (if enabled) */
#define IRQ_COM1	4	/* COM1 (if enabled) */
#define IRQ_FLOPPY	6	/* Floppy Disk */
#define IRQ_SPURIOUS	7	/* Unreliable "spurious" interrupt (usually) */
#define IRQ_CMOS	8	/* CMOS real-time clock (if enabled) */
#define IRQ_PRI_DISK	14	/* Primary ATA Hard Disk */
#define IRQ_SEC_DISK	15	/* Secondary ATA Hard Disk */

#ifndef __ASM__
#include <types.h>

struct regs {
	u32 ds:16, ds_pad:16;
	u32 es:16, es_pad:16;
	u32 fs:16, fs_pad:16;
	u32 gs:16, gs_pad:16;
	u32 ebp;
	u32 esi;
	u32 edi;
	u32 edx;
	u32 ecx;
	u32 ebx;
	u32 eax;
	u32 error;	/* error code */
	u32 eip;
	u32 cs:16, cs_pad:16;
	u32 eflags;
	u32 esp;
	u32 ss:16, ss_pad:16;
};

extern void de_entry(void);
extern void nmi_entry(void);
extern void bp_entry(void);
extern void of_entry(void);
extern void br_entry(void);
extern void ud_entry(void);
extern void nm_entry(void);
extern void df_entry(void);
extern void ts_entry(void);
extern void np_entry(void);
extern void ss_entry(void);
extern void gp_entry(void);
extern void pf_entry(void);
extern void mf_entry(void);
extern void ac_entry(void);
extern void mc_entry(void);
extern void xm_entry(void);
extern void irq0_entry(void);
extern void irq1_entry(void);
extern void irq2_entry(void);
extern void irq3_entry(void);
extern void irq4_entry(void);
extern void irq5_entry(void);
extern void irq6_entry(void);
extern void irq7_entry(void);
extern void irq8_entry(void);
extern void irq9_entry(void);
extern void irq10_entry(void);
extern void irq11_entry(void);
extern void irq12_entry(void);
extern void irq13_entry(void);
extern void irq14_entry(void);
extern void irq15_entry(void);
extern void syscall_entry(void);

#endif	/* !__ASM__ */

#endif	/* int.h */
