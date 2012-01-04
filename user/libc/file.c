#include <syscall.h>
#include <ulib.h>

int open(char *path, unsigned int mode)
{
	return usys_open(path, mode);
}

int close(int fd)
{
	return usys_close(fd);
}

int read(int fd, char *buf, size_t size)
{
	return usys_read(fd, buf, size);
}

int write(int fd, char *buf, size_t size)
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

int fchdir(int fd)
{
	return usys_fchdir(fd);
}

int chdir(char *path)
{
	struct file_stat stat;
	int fd, r;
	fd = open(path, 0);
	if (fd < 0) {
		printf("Cannot open file: %s\n", path);
		return -1;
	}
	if ((r = fstat(fd, &stat)) < 0) {
		printf("Cannot get file stat: %s\n", path);
	} else if (!S_ISDIR(stat.mode)) {
		printf("%s is not dir\n", path);
	} else {
		r = fchdir(fd);
		if (r < 0)
			printf("error for fchdir\n");
	}
	close(fd);
	return r;
}

int fgetdir(int fd, int s, int n, struct dir_stat *ds)
{
	if (s < 0 || n <= 0)
		return -1;
	return usys_fgetdir(fd, s, n, ds);
}

int mkdir(char *path, unsigned int mode)
{
	return usys_mkdir(path, mode);
}

int rmdir(char *path)
{
	return usys_rmdir(path);
}

int rm(char *path)
{
	return usys_rm(path);
}

void sync(void)
{
	usys_sync();
}

int truncate(int fd)
{
	return usys_truncate(fd);
}

char *getcwd(char *buf, size_t size)
{
	if (usys_getcwd(buf, size))
		return NULL;

	return buf;
}
