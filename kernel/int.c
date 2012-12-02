#include <print.h>
#include <8259A.h>
#include <int.h>
#include <x86.h>
#include <mm.h>

static void init_8259A(void)
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

static int interrupt_level;
void first_open_interrupt(void)
{
	interrupt_level = 0;
	sti();
}

void open_interrupt(void)
{
	interrupt_level--;
	if (interrupt_level == 0)
		sti();
	else if (interrupt_level < 0)
		panic("interrupt_level = %d", interrupt_level);
}

void close_interrupt(void)
{
	if (interrupt_level == 0)
		cli();
	interrupt_level++;
}

extern void do_page_fault(struct regs *);
extern void do_sys_call(struct regs *);
extern void keyboard_interrupt(struct regs *);
extern void timer_interrupt(struct regs *);

/* All traps, faults, fatals and hardware interrupt handler */
void interrupt_handler(int nr, struct regs *reg)
{
	if (nr < 0 || nr > INTNO_MAX) {
		dump_stack(reg);
		panic("Unknown interrupt %d", nr);
	}

	interrupt_level++;

	/*
	 * IRQs are mutually exclusive.
	 * IRQ can preempt other traps and syscalls.
	 */
	switch (nr) {
	case INTNO_IRQ0:
		timer_interrupt(reg);
		break;
	case INTNO_IRQ1:
		keyboard_interrupt(reg);
		break;
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

	/*
	 * send end of interrupt signal to PIC chip (not autoeoi mode)
	 * (If no EOI is sent, other hardware irq will not be accepted!)
	 */
	if (nr >= INTNO_IRQ0 && nr <= INTNO_IRQ15)
		EOI_8259A(nr - INTNO_IRQ0);

	interrupt_level--;
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
	set_int_gate(INTNO_PF, pf_entry);	/* page fault cannot allow interrupt. */
	/* Vector 15 iS reserved */
	set_trap_gate(INTNO_MF, mf_entry);
	set_trap_gate(INTNO_AC, ac_entry);
	set_trap_gate(INTNO_MC, mc_entry);
	set_trap_gate(INTNO_XM, xm_entry);
	/* Vector 20-31 are reserved */
	/* Interrupt Gate ensures there is no recursion interrupt. */
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
	init_8259A();
	printk("interrupt/trap/fault init!\n");
}
