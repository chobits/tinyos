#include <task.h>
#include <x86.h>
#include <mm.h>
#include <print.h>

struct list_head task_list;
struct task init_task;
static struct task dummy_task;	/* for switching to the same ctack */

/* It is safe to switch to current task, which is not recommended. */
void task_switch(struct task *next)
{
	struct task *prev;
	/*
	 * If swiching to current task, using dummy task as prev.
	 * Otherwise, prev_[esp|eip] will overlap next_[esp|eip]
	 */
	prev = (ctask == next) ? &dummy_task : ctask;
	ctask = next;
	/* When user jumps to kernel, tss::esp0|ss0 will be loaded. */
	common_tss.esp0 = (unsigned int)next->kstacktop;
	load_cr3(PADDR(next->us.pgdir));
	asm volatile (
		"pushfl;"
		"pushl %%ebp;"
		"movl %%esp, %[prev_esp];"
		"movl $1f, %[prev_eip];"
		"movl %[next_esp], %%esp;"
		"pushl %[next_eip];"
		"ret;"
		"1:"
		"popl %%ebp;"
		"popfl;"
		: [prev_esp] "=m" (prev->con.esp),
		  [prev_eip] "=m" (prev->con.eip)
		: [next_esp] "m"  (next->con.esp),
		  [next_eip] "m"  (next->con.eip)
		: "memory");
}

void debug_task(void)
{
	struct task *task;
	for_each_task(task) {
		if (task->state == TASK_RUNNABLE)
			printk("[%d]", task->pid);
	}
}

void schedule(void)
{
	struct task *task, *next = NULL;

	/*
	 *  FIXME: add lock to solve race condition for task_list
	 *  FIXED: close interrupt when schedule is called() via interrupt level
	 *   (schedule is called by sys_yield or timer_interrupt)
	 */

	for_each_task(task) {
		if (task == ctask || task->state != TASK_RUNNABLE)
			continue;
		if (!next || task->priority > next->priority)
			next = task;
	}

	if (!next) {
		if (ctask == &init_task) {
#ifdef DEBUG_SCHED_TASK
			debug_task();
#endif
			return;
		}
		next = &init_task;
	}
	next->priority--;
	task_switch(next);
}

extern void close_interrupt(void);
extern void open_interrupt(void);

void sys_yield(void)
{
	close_interrupt();
	schedule();
	open_interrupt();
}

int sys_getpid(void)
{
	return ctask->pid;
}
