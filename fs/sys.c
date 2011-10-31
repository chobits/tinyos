#include <file.h>
#include <task.h>
#include <print.h>

static void fd_save_file(int fd, struct file *file)
{
	ctask->ft.files[fd] = file;
}

static void fd_free(int fd)
{
	ctask->ft.files[fd] = NULL;
}

static int fd_alloc(void)
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

struct file *fd_put_file(unsigned int fd)
{
	struct file *file = NULL;
	if (fd < FD_SIZE) {
		file = ctask->ft.files[fd];
		ctask->ft.files[fd] = NULL;
	}
	return file;
}

struct file *fd_get_file(unsigned int fd)
{
	struct file *file = NULL;
	if (fd < FD_SIZE) {
		file = ctask->ft.files[fd];
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

int sys_close(int fd)
{
	struct file *file = fd_put_file(fd);
	if (!file)
		return -1;
	file_close(file);
	return 0;
}
