#include <file.h>
#include <task.h>

void fd_save_file(int fd, struct file *file)
{
	ctask->ft.files[fd] = file;
}

void fd_free(int fd)
{
	ctask->ft.files[fd] = NULL;
}

int fd_alloc(void)
{
	struct fd_table *ft = &ctask->ft;
	int fd;
	for (fd = 0; fd < FD_SIZE; fd++)
		if (!ft->files[fd])
			break;
	if (fd < FD_SIZE)
		ft->files[fd] = FD_SAVED;
	return fd;
}

struct file *fd_get_file(unsigned int fd)
{
	struct fd_table *ft = &ctask->ft;
	struct file *file = ft->files[fd];
	if (!file)
		return NULL;
	return get_file(file);
}

int sys_read(unsigned int fd, char *buf, size_t size)
{
	struct file *file = fd_get_file(fd);
	if (!file)
		return -1;
	size = file_read(file, buf, size);
	put_file(file);
	return size;
}

int sys_write(unsigned int fd, char *buf, size_t size)
{
	return -1;
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

int sys_sync(int fd)
{
	return -1;
}

int sys_close(int fd)
{
	struct file *file = fd_get_file(fd);
	if (!file)
		return -1;
	file_close(file);
	fd_free(fd);
	return 0;
}
