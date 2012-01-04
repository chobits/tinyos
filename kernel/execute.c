#include <paging.h>
#include <string.h>
#include <print.h>
#include <inode.h>
#include <file.h>
#include <page.h>
#include <task.h>
#include <elf.h>
#include <fs.h>
#include <x86.h>
#include <int.h>
#include <mm.h>

static void elf_read_proghdr(struct file *file, struct elf *elf,
					int n, struct proghdr *ph)
{
	off_t off = elf->e_phoff + n * ELF_PROGHDR_SIZE;
	if (file_lseek(file, off, SEEK_SET) != off)
		panic("file_lseek");
	if (file_read(file, (char *)ph, ELF_PROGHDR_SIZE) != ELF_PROGHDR_SIZE)
		panic("file read");
}

int map_userspace(struct userspace *us, struct proghdr *ph, struct file *file)
{
	struct page *page;
	void *p;
	unsigned int rsize, sz;
	unsigned int va, off;
	unsigned int perm;

#ifdef VMAP
	vmap_add(&us->vmap_root, ph->p_va, ph->p_memsz, ph->p_flags);
#endif
	if (file_lseek(file, ph->p_offset, SEEK_SET) != ph->p_offset)
		panic("file lseek");
	rsize = 0;
	perm = (ph->p_flags & ELF_PROG_FLAG_WRITE) ? (PTE_U | PTE_W) : PTE_U;
	va = ph->p_va;
	off = PGOFFSET(va);
	sz = min(PGSIZE - off, ph->p_filesz - rsize);
	p = NULL;
	while (rsize < ph->p_filesz) {
		off = PGOFFSET(va);
		sz = min(PGSIZE - off, ph->p_filesz - rsize);
		/* map */
		page = map_page(us->pgdir, va - off, perm);
		if (!page)
			panic("error: map page");
		/* FIXME: associate page to vmap */
		/* copy */
		if (!(p = kva_page(page)))
			panic("cannot be converted to kva");
		if (file_read(file, p + off, sz) != sz)
			panic("file read");
		rsize += sz;
		va += sz;
	}
	if (off + sz < PGSIZE) {
		if (!p) {
			page = map_page(us->pgdir, va - off, perm);
			if (!page)
				panic("error: map page");
			if (!(p = kva_page(page)))
				panic("cannot be converted to kva");
		}
		memset(p + off + sz, 0x0, PGSIZE - off - sz);
		va += PGSIZE - off - sz;
		rsize += PGSIZE - off - sz;
	}
	while (rsize < ph->p_memsz) {
		off = PGOFFSET(va);
		sz = min(PGSIZE - off, ph->p_memsz - rsize);
		page = map_page(us->pgdir, va - off, perm);
		if (!(p = kva_page(page)))
			panic("cannot be converted to kva");
		memset(p, 0x0, sz);
		va += sz;
		rsize += sz;
	}
	if (off + sz < PGSIZE)
		memset(p + sz + off, 0x0, PGSIZE - sz - off);
	return 0;
}

unsigned int execute_userstack(struct execute_args *eargs)
{
	/* map usermode stack: [KERNEL_BASE - 2PGSIZE, KERNEL_BASE - PGSIZE) */
	return eargs->stackoff + USER_STACK;
}

int execute_context(struct task *task, struct elf *elf, struct execute_args *eargs)
{
	struct regs *reg;
	/* Although we overlap current kernel stack, it is safe! */
	task->con.esp = (unsigned int)(task->kstacktop - sizeof(struct regs));
	task->con.eip = (unsigned int)fork_child_return;
	/* rebuild kernel stack */
	reg = (struct regs *)task->con.esp;
	memset(reg, 0x0, sizeof(struct regs));
	reg->ds = USER_DATA_SEG;
	reg->es = USER_DATA_SEG;
	reg->fs = USER_DATA_SEG;
	reg->gs = USER_DATA_SEG;
	reg->ss = USER_DATA_SEG;
	reg->cs = USER_CODE_SEG;
	/* IOPL:0, IF */
	reg->eflags = EFLAGS_BIT1 | EFLAGS_IF;
	/* user code and data environment */
	reg->eip = elf->e_entry;
	reg->esp = USER_STACK + eargs->stackoff;
	pmap_page(task->us.pgdir, VADDR2PG(eargs->stack), USER_STACK, PTE_U | PTE_W);
	return 0;
}

void execute_map_userspace(struct userspace *us, struct file *file, struct elf *elf)
{
	struct proghdr ph;
	int i;

	for (i = 0; i < elf->e_phnum; i++) {
		elf_read_proghdr(file, elf, i, &ph);
		if (ph.p_type != ELF_PROG_LOAD)
			continue;
		if (map_userspace(us, &ph, file) < 0)
			panic("Error for map userspace");
	}
}

void exit_task_userspace(struct task *);
int execute_userspace(struct task *task, struct file *file, struct elf *elf)
{
	/* free orignal user space map */
	exit_task_userspace(task);
	/* create new user space map */
	execute_map_userspace(&task->us, file, elf);
	return 0;
}

void execute_arguments(int argc, char **argv, struct execute_args *eargs)
{
	int i, len;
	void *p;
	char **tmpargv;
	/* prealloc user stack */
	if (!(p = get_free_page()))
		panic("No free page");
	tmpargv = p;
	eargs->stack = p;
	p += PGSIZE - 16;	/* reserve 16 bytes for stack security */
	/* copy arguments */
	for (i = argc - 1; i >= 0 ; i--) {
		len = strlen(argv[i]);
		p -= len + 1;
		strcpy(p, argv[i]);
		tmpargv[i] = (char *)(USER_STACK + PGOFFSET(p));
	}
	tmpargv[argc] = NULL;	/* assure NULL-terminal */
	/* copy pointers of arguments */
	len = sizeof(char *) * (argc + 1);
	p -= len;
	p = ALIGN_DOWN(p, sizeof(char *));	/* alignment */
	memcpy(p, tmpargv, len);
	/* save @argv */
	p -= sizeof(char **);
	*(char ***)p = (char **)(USER_STACK + PGOFFSET(p) + sizeof(char **));
	/* save @argc */
	p -= sizeof(int);
	*(int *)p = argc;
	eargs->stackoff = PGOFFSET(p);
}

void execute_fs(struct task *task)
{
	/* close orignal fd table */
	ft_close(&task->fs.ft);
}

void task_execute(struct task *task, struct file *file, struct elf *elf,
		int argc, char **argv)
{
	struct execute_args eargs;
	execute_arguments(argc, argv, &eargs);
	execute_userspace(task, file, elf);
	execute_fs(task);
	execute_context(task, elf, &eargs);
	/* goto new execution */
	file_close(file);
	task_switch(task);
	panic("Error for task switch during execution!");
}

int sys_execute(char *path, int argc, char **argv)
{
	struct elf elf;
	struct file *file;
	if (argc <= 0)
		goto err_out;
	/* open file */
	if (!(file = file_open(path, 0)))
		goto err_out;
	if ((file_read(file, (char *)&elf, sizeof(elf))) != sizeof(elf))
		goto err_close;
	if (elf.e_magic != ELF_MAGIC)
		goto err_close;
	/* If this returns, error occurs during execution. */
	task_execute(ctask, file, &elf, argc, argv);
err_close:
	file_close(file);
err_out:
	return -1;
}

pde_t *kern_page_dir;
int create_userspace(struct task *t, struct file *file, struct elf *elf)
{
	struct userspace *us = &t->us;
	/* alloc new page dir */
	us->pgdir = (pde_t *)get_free_page();
	if (!us->pgdir)
		panic("No memory for us page dir");
	memset(&us->pgdir[0], 0x0, PGSIZE - KBASE_PDE_SIZE);
	/* share the same kernel address space */
	memcpy(&us->pgdir[KBASE_PDE], &kern_page_dir[KBASE_PDE], KBASE_PDE_SIZE);
	/* map user address space */
	execute_map_userspace(us, file, elf);
	return 0;
}

static struct task *create_task(void)
{
	struct task *task;
	task = alloc_task();
	if (!task || task->pid != 1)
		panic("alloc task error");
	task->fs.current_dir = get_inode_ref(ctask->fs.current_dir);
	task->fs.root_dir = get_inode_ref(ctask->fs.root_dir);
	return task;
}

void load_first_program(char *path, int argc, char **argv)
{
	struct execute_args eargs;
	struct elf elf;
	struct file *file;
	struct task *task;

	/* read elf head and check it */
	file = file_open(path, 0);
	if (!file)
		panic("Cannot open %s, try `make loaduser`", path);
	if ((file_read(file, (char *)&elf, sizeof(elf))) != sizeof(elf))
		panic("Cannot read elf header");
	if (elf.e_magic != ELF_MAGIC)
		panic("Error elf magic value:%08x", elf.e_magic);
	/* create new process */
	task = create_task();
	execute_arguments(argc, argv, &eargs);
	if (create_userspace(task, file, &elf) < 0)
		panic("create userspace");
	if (execute_context(task, &elf, &eargs) < 0)
		panic("create context");
	file_close(file);
	/* mark it runnable */
	task->state = TASK_RUNNABLE;
}

