#include <inode.h>
#include <print.h>
#include <task.h>
#include <fs.h>

void put_inode(struct inode *inode)
{
	inode->i_refcnt--;
	if (inode->i_refcnt == 0) {
		if (inode->i_ops && inode->i_ops->release)
			inode->i_ops->release(inode);
	} else if (inode->i_refcnt < 0) {
		printk("Free inode too many times(%d)\n", inode->i_refcnt);
	}
}

struct inode *get_inode_ref(struct inode *inode)
{
	inode->i_refcnt++;
	return inode;
}

int inode_read(struct inode *inode, char *buf, size_t size, off_t off)
{
	/*
	 * We cannot use min(i_size - off, size) directly here,
	 * the negative value of (i_size - off) will be changd to
	 * positive value of `unsigned int` type.
	 */
	int r = -1;
	if (off >= inode->i_size)
		return 0;
	size = min(inode->i_size - off, size);
	if (inode->i_ops && inode->i_ops->read)
		r = inode->i_ops->read(inode, buf, size, off);
	return r;
}

void inode_update_size(struct inode *inode, size_t size)
{
	inode->i_size = size;
	if (inode->i_ops && inode->i_ops->update_size)
		inode->i_ops->update_size(inode, size);
}

int inode_write(struct inode *inode, char *buf, size_t size, off_t off)
{
	int r = -1;
	if (inode->i_ops && inode->i_ops->write)
		r = inode->i_ops->write(inode, buf, size, off);
	if ((r > 0) && (r + off > inode->i_size))
		inode_update_size(inode, r + off);
	return r;
}

int inode_truncate(struct inode *inode)
{
	int r = -1;
	if (S_ISREG(inode->i_mode) && inode->i_ops && inode->i_ops->truncate) {
		inode->i_ops->truncate(inode);
		r = 0;
	}
	return r;
}

struct inode *inode_open(char *path, unsigned int mode)
{
	struct inode *dir, *inode;
	char *basename;
	int len;
	dir = path_lookup_dir(path, &basename, &len);
	if (!dir)
		return NULL;
	if (len == 0) {
		/* for "/" ,"//", "///", ... */
		if (dir == ctask->fs.root_dir)
			return dir;
		return NULL;
	}
	inode = inode_sub_lookup_put(dir, basename, len);
	/* Not exist and create it? */
	if (!inode && (mode & O_CREATE)) {
		if (dir->i_ops && dir->i_ops->create)
			inode = dir->i_ops->create(dir, basename, len);
	}
	if (inode && (mode & O_TRUNCATE))
		inode_truncate(inode);
	return inode;
}

void inode_close(struct inode *inode)
{
	put_inode(inode);
}

void inode_sync(struct inode *inode)
{
	if (inode->i_ops && inode->i_ops->sync)
		inode->i_ops->sync(inode);
}

void inode_stat(struct inode *inode, struct file_stat *stat)
{
	stat->size = inode->i_size;
	stat->inode = inode->i_ino;
	stat->mode = inode->i_mode;
	stat->iref = inode->i_refcnt;
	if (inode->i_ops && inode->i_ops->stat)
		inode->i_ops->stat(inode, stat);
}

void inode_chdir(struct inode *inode)
{
	struct inode *old;
	old = ctask->fs.current_dir;
	ctask->fs.current_dir = get_inode_ref(inode);
	if (old)
		put_inode(old);
}

int inode_getdir(struct inode *inode, int start, int num, struct dir_stat *ds)
{
	int r = -1;
	if (!S_ISDIR(inode->i_mode))
		return -1;
	if (inode->i_ops && inode->i_ops->getdir)
		r = inode->i_ops->getdir(inode, start, num, ds);
	return r;
}

struct inode *inode_mkdir(char *path)
{
	struct inode *dir, *inode = NULL;
	char *basename;
	int len;
	dir = path_lookup_dir(path, &basename, &len);
	if (dir) {
		if (len > 0 && dir->i_ops && dir->i_ops->mkdir)
			inode = dir->i_ops->mkdir(dir, basename, len);
		put_inode(dir);
	}
	return inode;
}

int inode_rmdir(char *path)
{
	struct inode *dir;
	char *basename;
	int len, r = -1;
	dir = path_lookup_dir(path, &basename, &len);
	if (dir) {
		if (len > 0 && dir->i_ops && dir->i_ops->rmdir)
			r = dir->i_ops->rmdir(dir, basename, len);
		put_inode(dir);
	}
	return r;
}

int inode_rm(char *path)
{
	struct inode *dir;
	char *basename;
	int len, r = -1;
	dir = path_lookup_dir(path, &basename, &len);
	if (dir) {
		if (len > 0 && dir->i_ops && dir->i_ops->rm)
			r = dir->i_ops->rm(dir, basename, len);
		put_inode(dir);
	}
	return r;
}

int inode_get_pathname(struct inode *inode, char *pathname, size_t len)
{
	return inode->i_ops->get_pathname(inode, pathname, len);
}
