#include <slab.h>
#include <print.h>
#include <file.h>
#include <inode.h>

void minix_fs_init(void);
struct slab *file_slab;
void fs_init(void)
{
	/* init virtual file system */
	file_slab = alloc_slab(sizeof(struct file));
	if (!file_slab)
		panic("No memory for file slab");
	/* load real file system */
	minix_fs_init();
}
