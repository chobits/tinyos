#include <int.h>
#include <x86.h>
#include <mm.h>
#include <print.h>

#define ICW1_BIT4	0x10	/* must be set */
#define ICW1_LTIM	0x08	/* level triggered mode (0 for edge) */
#define ICW1_ADI	0x04	/* call address interval:4 (0 for 8) */
#define ICW1_SNGL	0x02	/* sigle (0 for cascade mode)*/
#define ICW1_IC4	0x01	/* IC4 is needed */

#define ICW1		(ICW1_BIT4 | ICW1_IC4)	/* edge cascade 8interval IC4 */
#define ICW3_MASTER	0x04	/* slave PIC is cascaded on IRQ2 */
#define ICW3_SLAVE	0x02	/* ?? */

#define ICW4_uPM	0x01	/* 8086/8088 mode(0 for MCS-80/85 mode) */
#define ICW4_AEOI	0x02	/* auto EOI (0 for normal EOI) */
#define ICW4_MS		0x04	/* X non buffer|0 buffer slave|1 buffer master */
#define ICW4_BUF	0x08	/* 0 non buffer|1 buffer slave|1 buffer master */
#define ICW4_SFNM	0x10	/* special fully nested mode(non nested) */

#define ICW4_MASTER	(ICW4_uPM)	/* non-buffer 8086 NEOI non-nested */
#define ICW4_SLAVE	(ICW4_uPM)	/* non-buffer 8086 NEOI non-nested */

#define REG_PIC1	0x20
#define REG_PIC2	0xa0
#define PIC_A0		0x01	/* A0 bit */
#define PIC_MASTER_CMD	REG_PIC1		/* master command register */
#define PIC_MASTER_DATA	(REG_PIC1 | PIC_A0)	/* master data register */
#define PIC_SLAVE_CMD	REG_PIC2		/* slave command register */
#define PIC_SLAVE_DATA	(REG_PIC2 | PIC_A0)	/* slave data register */
#define PIC_MASTER_IMR	PIC_MASTER_DATA		/* interrupt mask register */
#define PIC_SLAVE_IMR	PIC_SLAVE_DATA		/* interrupt mask register */

#define OCW2_R		0x80
#define OCW2_SL		0x40
#define OCW2_EOI	0x20
#define OCW2_L2		0x04
#define OCW2_L1		0x02
#define OCW2_L0		0x01

#define EOI		0x20		/* non-specific EOI command */

void init_8259A(void)
{
	/* ICW1: */
	outb_p(PIC_MASTER_CMD, ICW1);
	outb_p(PIC_SLAVE_CMD, ICW1);
	/* ICW2: remap IRQ number */
	outb_p(PIC_MASTER_DATA, INTNO_IRQ0);
	outb_p(PIC_SLAVE_DATA, INTNO_IRQ8);
	/* ICW3: */
	outb_p(PIC_MASTER_DATA, ICW3_MASTER);
	outb_p(PIC_SLAVE_DATA, ICW3_SLAVE);
	/* ICW4: cascaded PIC environment */
	outb_p(PIC_MASTER_DATA, ICW4_MASTER);
	outb_p(PIC_SLAVE_DATA, ICW4_SLAVE);
	/* disable all interrupts: mask on all bit of IMR */
	outb_p(PIC_MASTER_IMR, 0xff);
	outb_p(PIC_SLAVE_IMR, 0xff);
}

void mask_8259A(int irq)
{
	u16 port;
	u8 value;
	port = (irq < 8) ? PIC_MASTER_IMR : PIC_SLAVE_IMR;
	value = inb(port) | (1 << irq);
	outb(port, value);
}

void EOI_8259A(int irq)
{
	/*
	 * If IRQ comes from slave PIC, it is necessary to
	 * send an EOI to master PIC.
	 */
	if (irq >= 8)
		outb(PIC_SLAVE_CMD, EOI);
	outb(PIC_MASTER_CMD, EOI);
}

void unmask_8259A(int irq)
{
	u16 port;
	u8 value;
	port = (irq < 8) ? PIC_MASTER_IMR : PIC_SLAVE_IMR;
	value = inb(port) & (~(1 << irq));
	outb(port, value);
}

extern void do_page_fault(struct regs *);
extern void do_sys_call(struct regs *);

/* All traps, faults, fatals and hardware interrupt handler */
void interrupt_handler(int nr, struct regs *reg)
{
	if (nr < 0 || nr > INTNO_MAX) {
		dump_stack(reg);
		panic("Unknown interrupt %d", nr);
	}
	switch (nr) {
	case INTNO_PF:
		do_page_fault(reg);
		break;
	case INTNO_SYSCALL:
		do_sys_call(reg);
		break;
	default:
		dump_stack(reg);
		printk("error code: %08x\n", reg->error);
		printk("interrupt on %d\n", nr);
		panic("interrupt unfinished");
		break;
	}
}

static struct desc idt[IDT_SIZE];
static struct table_desc idt_desc = {
	.limit = sizeof(idt) - 1,
	.base = (u32)idt,		/* linear address */
};

void int_init(void)
{
	/* Init idt table */
	set_trap_gate(INTNO_DE, de_entry);
	/* Vector 1 is reserved */
	set_trap_gate(INTNO_NMI, nmi_entry);
	set_trap_gate(INTNO_BP, bp_entry);
	set_trap_gate(INTNO_OF, of_entry);
	set_trap_gate(INTNO_BR, br_entry);
	set_trap_gate(INTNO_UD, ud_entry);
	set_trap_gate(INTNO_NM, nm_entry);
	set_trap_gate(INTNO_DF, df_entry);
	/* Vector 9 is reserved */
	set_trap_gate(INTNO_TS, ts_entry);
	set_trap_gate(INTNO_NP, np_entry);
	set_trap_gate(INTNO_SS, ss_entry);
	set_trap_gate(INTNO_GP, gp_entry);
	set_trap_gate(INTNO_PF, pf_entry);
	/* Vector 15 iS reserved */
	set_trap_gate(INTNO_MF, mf_entry);
	set_trap_gate(INTNO_AC, ac_entry);
	set_trap_gate(INTNO_MC, mc_entry);
	set_trap_gate(INTNO_XM, xm_entry);
	/* Vector 20-31 are reserved */
	set_int_gate(INTNO_IRQ0, irq0_entry);
	set_int_gate(INTNO_IRQ1, irq1_entry);
	set_int_gate(INTNO_IRQ2, irq2_entry);
	set_int_gate(INTNO_IRQ3, irq3_entry);
	set_int_gate(INTNO_IRQ4, irq4_entry);
	set_int_gate(INTNO_IRQ5, irq5_entry);
	set_int_gate(INTNO_IRQ6, irq6_entry);
	set_int_gate(INTNO_IRQ7, irq7_entry);
	set_int_gate(INTNO_IRQ8, irq8_entry);
	set_int_gate(INTNO_IRQ9, irq9_entry);
	set_int_gate(INTNO_IRQ10, irq10_entry);
	set_int_gate(INTNO_IRQ11, irq11_entry);
	set_int_gate(INTNO_IRQ12, irq12_entry);
	set_int_gate(INTNO_IRQ13, irq13_entry);
	set_int_gate(INTNO_IRQ14, irq14_entry);
	set_int_gate(INTNO_IRQ15, irq15_entry);
	set_system_gate(INTNO_SYSCALL, syscall_entry);

	/* load idt table */
	lidt((u32)&idt_desc);

	/* init interrupt processor */
//	init_8259A();
}
