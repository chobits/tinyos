#ifndef __ULIB_H
#define __ULIB_H

#include <types.h>

struct file_stat;
struct dir_stat;

extern int printf(char *, ...);
extern int gets(char *, int);
extern int fork(void);
extern void yield(void);
extern int getpid(void);
extern void exit(int);
extern int wait(int *);
extern int execute(char *, int, char **);
/* file I/O */
extern int open(char *path, unsigned int mode);
extern int close(int fd);
extern off_t lseek(int fd, off_t offset, int whence);
extern int write(int fd, char *buf, size_t size);
extern int read(int fd, char *buf, size_t size);
extern int fsync(int fd);
extern int fstat(int fd, struct file_stat *);
extern int fchdir(int fd);
extern int chdir(char *);
extern int fgetdir(int, int, int, struct dir_stat *);
extern int mkdir(char *, unsigned int);
extern int rmdir(char *);
extern int rm(char *);
extern void sync(void);
extern int truncate(int);
extern char *getcwd(char *buf, size_t size);

#define S_IFMT		00170000
#define S_IFSOCK	0140000
#define S_IFLNK		0120000
#define S_IFREG		0100000
#define S_IFBLK		0060000
#define S_IFDIR		0040000
#define S_IFCHR		0020000
#define S_IFIFO		0010000
#define S_ISUID		0004000
#define S_ISGID		0002000
#define S_ISVTX		0001000

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

#define S_IRWXU		00700
#define S_IRUSR		00400
#define S_IWUSR		00200
#define S_IXUSR		00100

#define S_IRWXG		00070
#define S_IRGRP		00040
#define S_IWGRP		00020
#define S_IXGRP		00010

#define S_IRWXO		00007
#define S_IROTH		00004
#define S_IWOTH		00002
#define S_IXOTH		00001

#define O_CREATE	0x80000000
#define O_TRUNCATE	0x40000000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* used by system call: fstat */
struct file_stat {
	unsigned int size;
	unsigned int inode;
	unsigned int mode;
	unsigned int iref;
	unsigned int link;
};

#define DIR_SIZE 32
struct dir_stat {
	char name[DIR_SIZE];
	int len;
	unsigned int inode;
};

#endif	/* ulib.h */
