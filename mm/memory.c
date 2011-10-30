#include <print.h>

extern void boot_memory_init(void);
extern void pages_init(void);
extern void paging_init(void);
extern void slab_init(void);

void mm_init(void)
{
	boot_memory_init();
	paging_init();
	pages_init();
	slab_init();
}
