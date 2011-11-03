#include <inode.h>
#include <print.h>
#include <fs.h>

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

struct inode *inode_open(char *path)
{
	return inode_path_lookup(path);
}

void inode_close(struct inode *inode)
{
	if (inode->i_ops && inode->i_ops->close)
		inode->i_ops->close(inode);
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
}
