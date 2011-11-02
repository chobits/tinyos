#include <print.h>
#include <task.h>
#include <page.h>
#include <mm.h>

void free_page_table(pde_t *pde)
{
	pte_t *pte = (pte_t *)VADDR(PDE_PADDR(*pde));
	struct page *page;
	int i;
	/* free all physical pages in this page table */
	for (i = 0; i < PAGE_PTES; i++, pte++) {
		if (!(*pte & PTE_P))
			continue;
		page = ADDR2PG(PTE_PADDR(*pte));	/* physical page */
		if (*pte & PTE_COW)
			page->pg_cowshare--;
		free_page(page);
	}
	/* free page table */
	put_free_page((void *)VADDR(PDE_PADDR(*pde)));
	/* clear page dir entry */
	*pde = 0;
}

void exit_task_userspace(struct task *task)
{
	pde_t *pgdir = task->us.pgdir;
	int i;
	/* free user address space */
	for (i = 0; i < KBASE_PDE; i++) {
		if (pgdir[i] & PDE_P)
			free_page_table(&pgdir[i]);
	}
}

void exit_task_fs(struct task *task)
{

}

/* destroy task resources and yield CPU */
void task_exit(struct task *task)
{
	/* Unrunable */
	task->state = TASK_DYING;
	exit_task_fs(task);
	exit_task_userspace(task);
	/* 
	 * Why cannot we free total resources of task?
	 * (Why do we need parent to free task?)
	 * Now it is in current task context, the kernel
	 * stack is the exiting task resouce!
	 */
}

void sys_exit(int status)
{
	ctask->retval = status;
	task_exit(ctask);
	/* No return */
	schedule();
}

void task_wait_exit(struct task *parent, struct task *task)
{
	struct task *child;
	list_del(&task->sibling);
	/* let @parent to adopt childs of @task */
	if (!list_empty(&task->childs)) {
		list_for_each_entry(child, &task->childs, sibling)
			child->parent = parent;
		list_merge_tail(&parent->childs, &task->childs);
		
	}
	put_free_page(task->us.pgdir);
	free_task(task);
}

int task_wait(struct task *parent, int *status)
{
	struct task *task;
	int cpid = -1;

	list_for_each_entry(task, &parent->childs, sibling) {
		if (task->state == TASK_DYING) {
			cpid = task->pid;
			break;
		}

	}
	if (cpid > 0) {
		if (status)
			*status = task->retval;
		task_wait_exit(parent, task);
	}
	return cpid;
}

int sys_wait(int *status)
{
	return task_wait(ctask, status);
}
