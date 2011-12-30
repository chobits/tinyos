#include <print.h>
#include <task.h>
#include <pit.h>
#include <int.h>

unsigned int ticks = 0;

extern void task_switch(struct task *);
void timer_interrupt(struct regs *reg)
{
	ticks++;
	schedule();
}

