#include <paging.h>
#include <string.h>
#include <print.h>
#include <task.h>
#include <slab.h>
#include <x86.h>
#include <int.h>
#include <mm.h>

static struct pidset pidmap = {
	.bitmap = { 0, },
	.freepids = PIDS,
};

static int alloc_pid(void)
{
	int pid;
	if (pidmap.freepids <= 0)
		return -1;
	pid = searchzerobit(pidmap.bitmap ,PIDS, 1);
	if (pid < 0)
		panic("pid bitmap corrupts");
	pidmap.freepids--;
	return pid;
}

static void free_pid(int pid)
{
	if (pid >= PIDS) {
		printk("try to free unknown pid %d\n", pid);
		return;
	}
	if (clearbit(pidmap.bitmap, pid))
		pidmap.freepids++;
	else
		printk("try to free free pid\n");
}

struct task *ctask;		/* current task */
struct task init_task;
static struct slab *task_slab;
LIST_HEAD(task_list);		/* all tasks */

/*
 * We set tss::limit to 107(the least tss structure),
 *  excluding I/O Permission Bit Map and set tss::iomapbase
 *  to zero, in which case the I/O Permission Bit Map is
 *  not used duiring I/O permissions.
 * This makes it easy for us to handle I/O permissions,
 *  just considering IOPL in Eflags!
 * ( For more information, see Section 13.5, "PROTECTED-MODE I/O",
 *   in the Intel Manual, Volume 1: Basic Architecture )
 */
struct tss common_tss = {
	.prev_tss = 0,		/* GDT 0 is reserved! */
	.ss0 = KERN_DATA_SEG,
	.ldt = LDT_SEG,
	.io_bitmap_addr = 0x8000,
};

struct ldt common_ldt;	/* Not used */

void free_task(struct task *task)
{
	list_del(&task->list);
	put_free_page(task->kstacktop - PGSIZE);
	free_pid(task->pid);
	slab_free_object(task_slab, task);
}

struct task *alloc_task(void)
{
	struct task *task;
	void *stack;

	/* alloc task */
	task = slab_alloc_object(task_slab);
	if (!task)
		goto err;
	memset(task, 0x0, sizeof(struct task));

	/* alloc pid */
	task->pid = alloc_pid();
	if (task->pid < 0)
		goto err_free_task;

	/* alloc kernel stack */
	stack = get_free_page();
	if (!stack)
		goto err_free_pid;
	task->kstacktop = stack + PGSIZE;

	/* add to task list */
	list_add(&task->list, &task_list);
	list_init(&task->sibling);
	list_init(&task->childs);
	task->state = TASK_SLEEP;
	return task;

err_free_pid:
	free_pid(task->pid);
err_free_task:
	slab_free_object(task_slab, task);
err:
	return NULL;
}

extern void *kern_stack_top;
extern pde_t *kern_page_dir;
void task_init(void)
{
	/* init task structure allocator */
	task_slab = alloc_slab(sizeof(struct task));
	if (!task_slab)
		panic("No memory for task slab");
	/* init first task */
	init_task.kstacktop = kern_stack_top;
	init_task.pid = alloc_pid();
	init_task.us.pgdir = kern_page_dir;
	init_task.state = TASK_RUNNABLE;
	if (init_task.pid != 0)
		panic("init task get error pid: %d", init_task.pid);
	ctask = &init_task;
	/* init tss */
	common_tss.esp0 = (unsigned int)kern_stack_top;
	/* init other segments in gdt */
	set_usercode_seg(0, KERNEL_BASE - 1);
	set_userdata_seg(0, KERNEL_BASE - 1);
	set_tss_desc((unsigned int)&common_tss);
	set_ldt_desc((unsigned int)&common_ldt);
	lldt(LDT_SEG);
	ltr(TSS_SEG);
}
