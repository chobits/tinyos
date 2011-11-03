#include <minix_fs.h>
#include <file.h>
#include <slab.h>
#include <string.h>
#include <file.h>
#include <inode.h>
#include <print.h>
#include <task.h>

static void debug_inode(struct minix_d_inode *mdi)
{
	int i;
	printk("mode: %8x uid: %8x size: %d\n"
		"time: %8x gid: %8x link: %8x\n",
		mdi->i_mode, mdi->i_uid, mdi->i_size,
		mdi->i_time, mdi->i_gid, mdi->i_nlinks);
	printk("zones: ");
	for (i = 0; i < 9; i++)
		printk("%4d ", mdi->i_zone[i]);
	printk("\n");
}

void minix_fs_test(void)
{
	struct file *file;
	char *f = "/a";

	/* "/" */
	printk("Root dir:\n");
	debug_inode(i2mdi(ctask->fs.root_dir));

	/* "/b" */
	file = file_open(f, 0777);
	if (!file)
		panic("Cannot open file %s", f);
	printk("file %s:\n", f);
	debug_inode(i2mdi(file->f_inode));
//#define TEST_READ
#ifdef TEST_READ
	char buf[128];
	int size;
	int all;
	all = 0;
	while ((size = file_read(file, buf, 128)) > 0) {
		printk("%*s", size, buf);
		all += size;
	}
	printk("%dbytes is read\n", all);
#endif

#ifdef TEST_WRITE
	int i;
	int all;
	for (i = 33216+1; i > 0; i--)
		if ((all = file_write(file, "hello world1234\n", 16)) != 16)
			panic("file write %d", i);
	sync_blocks();
#endif

#ifdef TEST_LSEEK
	char buf[128];
	int r;
	if ((r = file_lseek(file, -5, SEEK_END)) == -1)
		panic("file_seek");
	printk("Seek to offset %d\n", r);
	r = file_read(file, buf, 128);
	if (r < 0)
		panic("file read");
	printk("[%d][%*s]", r, r, buf);
#endif
}

