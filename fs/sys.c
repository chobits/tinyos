#include <inode.h>
#include <print.h>
#include <file.h>
#include <slab.h>
#include <task.h>
#include <fs.h>

void ft_close(struct fd_table *ft)
{
	int i;
	for (i = 0; i < FD_SIZE; i++) {
		if (ft->files[i])
			file_close(ft->files[i]);
	}
}

static void fd_save_file(int fd, struct file *file)
{
	ctask->fs.ft.files[fd] = file;
}

static void fd_free(int fd)
{
	ctask->fs.ft.files[fd] = NULL;
}

static int fd_alloc(void)
{
	struct fd_table *ft = &ctask->fs.ft;
	int fd;
	for (fd = 0; fd < FD_SIZE; fd++)
		if (!ft->files[fd])
			break;
	if (fd < FD_SIZE)
		ft->files[fd] = FD_SAVED;
	return fd;
}

struct file *fd_put_file(unsigned int fd)
{
	struct file *file = NULL;
	if (fd < FD_SIZE) {
		file = ctask->fs.ft.files[fd];
		ctask->fs.ft.files[fd] = NULL;
	}
	return file;
}

struct file *fd_get_file(unsigned int fd)
{
	struct file *file = NULL;
	if (fd < FD_SIZE) {
		file = ctask->fs.ft.files[fd];
		if (file)
			get_file(file);
	}
	return file;
}

int sys_read(unsigned int fd, char *buf, size_t size)
{
	struct file *file;
	int r = -1;
	if (size <= 0)
		return size;
	file = fd_get_file(fd);
	if (file) {
		r = file_read(file, buf, size);
		put_file(file);
	}
	return r;
}

int sys_write(unsigned int fd, char *buf, size_t size)
{
	struct file *file;
	int r = -1;
	if (size <= 0)
		return size;
	file = fd_get_file(fd);
	if (file) {
		r = file_write(file, buf, size);
		put_file(file);
	}
	return r;
}

int sys_open(char *name, unsigned int mode)
{
	struct file *file;
	int fd = fd_alloc();
	if (fd < 0)
		goto out;
	file = file_open(name, mode);
	if (!file)
		goto out_free_fd;
	fd_save_file(fd, file);
	return fd;
out_free_fd:
	fd_free(fd);
out:
	return -1;
}

off_t sys_lseek(int fd, off_t offset, int whence)
{
	struct file *file = fd_get_file(fd);
	if (!file)
		return -1;
	offset = file_lseek(file, offset, whence);
	put_file(file);
	return offset;
}

int sys_fsync(int fd)
{
	struct file *file = fd_get_file(fd);
	if (!file)
		return -1;
	file_sync(file);
	put_file(file);
	return 0;
}


int sys_fstat(int fd, struct file_stat *stat)
{
	struct file *file = fd_get_file(fd);
	int r = -1;
	if (!file)
		return -1;
	r = file_stat(file, stat);
	put_file(file);
	return r;
}

int sys_close(int fd)
{
	struct file *file = fd_put_file(fd);
	if (!file)
		return -1;
	file_close(file);
	return 0;
}

int sys_fchdir(int fd)
{
	struct file *file = fd_get_file(fd);
	int r = -1;
	if (!file)
		return -1;
	r = file_chdir(file);
	put_file(file);
	return r;
}

/* get dir entry from @fd */
int sys_fgetdir(int fd, int start, int num, struct dir_stat *ds)
{
	struct file *file = fd_get_file(fd);
	int r = -1;
	if (!file)
		return -1;
	r = file_getdir(file, start, num, ds);
	put_file(file);
	return r;
}

int sys_mkdir(char *path, unsigned int mode)
{
	struct file *file;
	int fd = fd_alloc();
	if (fd < 0)
		goto out;
	file = file_mkdir(path, mode);
	if (!file)
		goto out_free_fd;
	fd_save_file(fd, file);
	return fd;
out_free_fd:
	fd_free(fd);
out:
	return -1;
}

int sys_rmdir(char *path)
{
	return file_rmdir(path);
}

int sys_rm(char *path)
{
	return file_rm(path);
}

int sys_truncate(int fd)
{
	struct file *file = fd_get_file(fd);
	int r = -1;
	if (!file)
		return -1;
	r = file_truncate(file);
	put_file(file);
	return r;
}

int sys_getcwd(char *buf, size_t size)
{
	struct inode *inode;

	inode = ctask->fs.current_dir;
	if (!inode)
		return -1;

	return inode_get_pathname(inode, buf, size);
}
