#include <asm.h>
#include <int.h>
#include <x86.h>

ENTRY_NOERRCODE(de_entry, INTNO_DE)
/* Vector 1 is reserved */
ENTRY_NOERRCODE(nmi_entry, INTNO_NMI)
ENTRY_NOERRCODE(bp_entry, INTNO_BP)
ENTRY_NOERRCODE(of_entry, INTNO_OF)
ENTRY_NOERRCODE(br_entry, INTNO_BR)
ENTRY_NOERRCODE(ud_entry, INTNO_UD)
ENTRY_NOERRCODE(nm_entry, INTNO_NM)
ENTRY_ERRCODE(df_entry, INTNO_DF)
/* Vector 9 is reserved */
ENTRY_ERRCODE(ts_entry, INTNO_TS)
ENTRY_ERRCODE(np_entry, INTNO_NP)
ENTRY_ERRCODE(ss_entry, INTNO_SS)
ENTRY_ERRCODE(gp_entry, INTNO_GP)
ENTRY_ERRCODE(pf_entry, INTNO_PF)
/* Vector 15 is reserved */
ENTRY_NOERRCODE(mf_entry, INTNO_MF)
ENTRY_ERRCODE(ac_entry, INTNO_AC)
ENTRY_NOERRCODE(mc_entry, INTNO_MC)
ENTRY_NOERRCODE(xm_entry, INTNO_XM)
/* Vector 20-31 are reserved */
/* Vector 32-47(IRQ 0-15) */
ENTRY_INT(irq0_entry, INTNO_IRQ0)
ENTRY_INT(irq1_entry, INTNO_IRQ1)
ENTRY_INT(irq2_entry, INTNO_IRQ2)
ENTRY_INT(irq3_entry, INTNO_IRQ3)
ENTRY_INT(irq4_entry, INTNO_IRQ4)
ENTRY_INT(irq5_entry, INTNO_IRQ5)
ENTRY_INT(irq6_entry, INTNO_IRQ6)
ENTRY_INT(irq7_entry, INTNO_IRQ7)
ENTRY_INT(irq8_entry, INTNO_IRQ8)
ENTRY_INT(irq9_entry, INTNO_IRQ9)
ENTRY_INT(irq10_entry, INTNO_IRQ10)
ENTRY_INT(irq11_entry, INTNO_IRQ11)
ENTRY_INT(irq12_entry, INTNO_IRQ12)
ENTRY_INT(irq13_entry, INTNO_IRQ13)
ENTRY_INT(irq14_entry, INTNO_IRQ14)
ENTRY_INT(irq15_entry, INTNO_IRQ15)
/* Vector 48 (system_call)*/
ENTRY_INT(syscall_entry, INTNO_SYSCALL)

ENTRY(interrupt_entry)
	xchgl %eax, (%esp)	/* push eax & eax <- interrupt number */
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %edi
	pushl %esi
	pushl %ebp
	push %gs
	push %fs
	push %es
	push %ds

	/* Reset kernel data segments */
	movl $KERN_DATA_SEG, %ecx
	movw %cx, %ds
	movw %cx, %es
	movw %cx, %fs
	movw %cx, %gs

	/* Call interrupt handler */
	pushl %esp	/* poiner of regs */
	pushl %eax	/* first argument of interrupt_handler */
	call interrupt_handler
	addl $8, %esp

interrupt_return:
	pop %ds
	pop %es
	pop %fs
	pop %gs
	popl %ebp
	popl %esi
	popl %edi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	addl $4, %esp	/* skip error code */
	iret

ENTRY(fork_child_return)
	/* send EOI of timer interrupt */
	pushl $INTNO_IRQ0
	call EOI_8259A
	addl $4, %esp
	call open_interrupt
	/* return to usermode */
	jmp interrupt_return
