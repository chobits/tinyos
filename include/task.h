#ifndef __TASK_H
#define __TASK_H

#include <paging.h>
#include <file.h>
#include <list.h>
#include <int.h>

/* switch current task to this context */
struct context {
	unsigned int esp;
	unsigned int eip;
};

#ifdef VMAP
/* virtual address map */
struct vmap {
	unsigned int va_start;
	unsigned int va_size;
	unsigned int va_flags;
	struct tree_node va_node;
};
#endif

struct userspace {
#ifdef VMAP
	struct tree_node vmap_root;
#endif
	pde_t *pgdir;
};

struct task {
	struct list_head list;		/* all task list entry */
	struct list_head sibling;	/* listed in parent */
	struct list_head childs;	/* all childs */
	struct task *parent;
	struct fd_table ft;
	struct inode *current_dir;
	struct inode *root_dir;
	void *kstacktop;		/* kernel-mode stack top of task */
	struct context con;
	struct regs *reg;
	struct userspace us;
	unsigned int state;
	int priority;
	int pid;
	int retval;			/* exit value */
};

#define TASK_RUNNABLE		1
#define TASK_INTERRUPTABLE	2
#define TASK_UNINTERRUPTABLE	3
#define TASK_DYING		4
#define for_each_task(t)	list_for_each_entry(t, &task_list, list)

#define PIDS		128	/* process id range: [0, PIDS - 1] */
#define PIDBYTES	(PIDS / 8)
/* current task */
struct pidset {
	unsigned char bitmap[PIDBYTES];		/* set for used */
	int freepids;
};

/* execute arguments */
struct execute_args {
	void *stack;
	unsigned int stackoff;
};

extern struct task *ctask;	/* current task */
extern struct task *alloc_task(void);
extern void free_task(struct task *);
extern void fork_child_return(void);
extern void task_switch(struct task *);
extern void schedule(void);
extern struct tss common_tss;
extern struct list_head task_list;

#endif	/* task.h */
