#include <boot.h>
#include <page.h>
#include <types.h>
#include <print.h>
#include <mm.h>

struct boot_param *boot_param = (struct boot_param *)VADDR(BOOT_PARAM_ADDR);
static void *boot_memory_start;
static void *boot_memory_free;
unsigned int memory_end;

static const char *e820_str[] = {
	"ignored",
	"free",
	"reserved",
	"acpi reclaim",
	"acpi invs"
};

void boot_memory_init(void)
{
	struct e820_entry *entry = boot_param->e820_list;
	int i, num = boot_param->e820_num;
	boot_memory_start = (void *)ALIGN_UP(PADDR(_end), PGSIZE);
	boot_memory_free = boot_memory_start;
	/*
	 * We reserve memory below 1MB and use 820 map table
	 * to find end of memory, ignoring real maps.
	 */
	memory_end = 0;
	printk("Memory map:\n");
	printk(" %-10s|%-10s|%-12s|\n", "addr", "length", "type");
	for (i = 0; i < num; i++) {
		printk(" 0x%08x|0x%08x|%-12s|\n", entry[i].addr,
			entry[i].length, e820_str[entry[i].type]);
		if (entry[i].type != E820_TYPE_FREE)
			continue;
		if (entry[i].addr + entry[i].length > memory_end)
			memory_end = entry[i].addr + entry[i].length;
	}
	/* caculate the number of memory pages */
	npage = memory_end / PGSIZE;
	printk("memory size: %dKB(%dMB)\n", npage * 4, npage * 4 / 1024);
}

/*
 * Caller dont need to check the return value of it,
 *  but assure that @size an @num are positive.
 * If free memory is not enough, it panics directly!
 */
void *boot_alloc(int size, int num)
{
	void *ret;
	if (!boot_memory_free)
		panic("Boot memory allocator dies.");
	ret = ALIGN_UP(boot_memory_free, size);
	boot_memory_free = ret + size * num;
	if (boot_memory_free > (void *)memory_end)
		panic("Out of boot memory: alloc %d from %p",
			num * size, boot_memory_free);

	return (void *)VADDR(ret);
}

void e820_memory_reserved(void)
{
	struct e820_entry *entry = boot_param->e820_list;
	int num = boot_param->e820_num;
	int pn, n, i;
	for (i = 0; i < num; i++) {
		if (entry[i].type == E820_TYPE_FREE)
			continue;
		if (entry[i].addr + entry[i].length > memory_end)
			continue;
		n = PN(entry[i].addr + entry[i].length + PGSIZE - 1);
		for (pn = PN(entry[i].addr); pn < n; pn++)
			setpagereserved(&page_table[pn]);
	}
}

void *boot_memory_exit(void)
{
	void *last = ALIGN_UP(boot_memory_free, PGSIZE);
	boot_memory_free = NULL;
	return last;
}
