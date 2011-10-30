#include <print.h>
#include <task.h>
#include <x86.h>
#include <mm.h>

extern struct list_head task_list;
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

void schedule(void)
{
	struct task *task, *next = NULL;
	for_each_task(task) {
		if (task == ctask || task->state != TASK_RUNNABLE)
			continue;
		if (!next || task->priority > next->priority)
			next = task;
	}
	if (next) {
		next->priority--;
		task_switch(next);
	}
}

void sys_yield(void)
{
	schedule();
}

int sys_getpid(void)
{
	return ctask->pid;
}
