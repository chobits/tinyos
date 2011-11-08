#include <string.h>
#include <print.h>
#include <inode.h>
#include <slab.h>
#include <file.h>
#include <fs.h>

struct slab *file_slab;

static void free_file(struct file *file)
{
	slab_free_object(file_slab, file);
}

void put_file(struct file *file)
{
	file->f_refcnt--;
	if (file->f_refcnt <= 0)
		free_file(file);
}

struct file *get_file(struct file *file)
{
	file->f_refcnt++;
	return file;
}

struct file *alloc_file(struct inode *inode, unsigned int mode)
{
	struct file *file = slab_alloc_object(file_slab);
	memset(file, 0x0, sizeof(file));
	file->f_inode = inode;
	file->f_mode = mode;
	file->f_refcnt = 1;
	return file;
}

struct file *file_open(char *path, unsigned int mode)
{
	struct file *file;
	struct inode *inode;
	inode = inode_open(path, mode);
	if (!inode)
		return NULL;
	file = alloc_file(inode, mode);
	return file;
}

void file_close(struct file *file)
{
	if (file->f_inode)
		inode_close(file->f_inode);
	else
		printk("close file without inode\n");
	file->f_inode = NULL;
	put_file(file);
}

int file_read(struct file *file, char *buf, size_t size)
{
	int r = -1;
	get_file(file);
	if (file->f_inode) {
		r = inode_read(file->f_inode, buf, size, file->f_pos);
		if (r > 0)
			file->f_pos += r;
	}
	put_file(file);
	return r;
}

int file_write(struct file *file, char *buf, size_t size)
{
	int r = -1;
	get_file(file);
	if (file->f_inode) {
		r = inode_write(file->f_inode, buf, size, file->f_pos);
		if (r > 0)
			file->f_pos += r;
	}
	put_file(file);
	return r;
}

off_t file_lseek(struct file *file, off_t offset, int whence)
{
	off_t off;
	get_file(file);
	off = file->f_pos;
	switch (whence) {
	case SEEK_SET:
		off = offset;
		break;
	case SEEK_END:
		off = file->f_inode->i_size;
	case SEEK_CUR:
		off += offset;
		break;
	default:
		off = -1;
		break;
	}
	if (off >= 0)
		file->f_pos = off;
	else
		off = -1;
	put_file(file);
	return off;
}

void file_sync(struct file *file)
{
	get_file(file);
	if (file->f_inode)
		inode_sync(file->f_inode);
	put_file(file);
}

int file_stat(struct file *file, struct file_stat *stat)
{
	int r = -1;
	get_file(file);
	if (file->f_inode) {
		inode_stat(file->f_inode, stat);
		r = 0;
	}
	put_file(file);
	return r;
}

int file_chdir(struct file *file)
{
	int r = -1;
	get_file(file);
	if (file->f_inode) {
		inode_chdir(file->f_inode);
		r = 0;
	}
	put_file(file);
	return r;
}

int file_getdir(struct file *file, int start, int num, struct dir_stat *ds)
{
	int r = -1;
	get_file(file);
	if (file->f_inode)
		r = inode_getdir(file->f_inode, start, num, ds);
	put_file(file);
	return r;
}

struct file *file_mkdir(char *path, unsigned int mode)
{
	struct file *file;
	struct inode *inode;
	inode = inode_mkdir(path);
	if (!inode)
		return NULL;
	file = alloc_file(inode, mode);
	return file;
}

int file_rmdir(char *path)
{
	return inode_rmdir(path);
}

int file_rm(char *path)
{
	return inode_rm(path);
}

int file_truncate(struct file *file)
{
	int r = -1;
	get_file(file);
	if (file->f_inode)
		r = inode_truncate(file->f_inode);
	put_file(file);
	return r;

}
