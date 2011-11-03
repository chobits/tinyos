#include <syscall.h>

int open(char *path, unsigned int mode)
{
	return usys_open(path, mode);
}

int close(int fd)
{
	return usys_close(fd);
}

int read(unsigned int fd, char *buf, size_t size)
{
	return usys_read(fd, buf, size);
}

int write(unsigned int fd, char *buf, size_t size)
{
	return usys_write(fd, buf, size);
}


off_t lseek(int fd, off_t offset, int whence)
{
	return usys_lseek(fd, offset, whence);
}

int fsync(int fd)
{
	return usys_fsync(fd);
}

int fstat(int fd, struct file_stat *stat)
{
	return usys_fstat(fd, stat);
}
