#include <print.h>
#include <8259A.h>
#include <pit.h>
#include <int.h>
#include <x86.h>

void pit_init(void)
{
	/* channel 0, rate generator, low&high bytes */
	outb(PIT_MODE, PIT_MODE_CH0 | PIT_MODE_2 | PIT_MODE_LOHI);
	outb(PIT_CH0, FREQ_DIV & 255);	/* low byte */
	outb(PIT_CH0, FREQ_DIV >> 8);	/* high byte */
	unmask_8259A(IRQ_TIMER);
	printk("pit init\n");
}

