#ifndef __FS_H
#define __FS_H

#include <block.h>

struct super_block {
	struct block_device *s_bdev;	/* deive of file system */
	int s_start;			/* start block number of file system */
	struct block *s_block;		/* real super block */
};

/* used by system call: fstat */
struct file_stat {
	unsigned int size;
	unsigned int inode;
	unsigned int mode;
	unsigned int iref;	/* inode reference count */
	unsigned int link;	/* inode link number */
};

#define DIR_SIZE 32
struct dir_stat {
	char name[DIR_SIZE];
	int len;
	unsigned int inode;
};

#define S_IFMT		00170000

#define S_IFSOCK	00140000
#define S_IFLNK		00120000
#define S_IFREG		00100000
#define S_IFBLK		00060000
#define S_IFDIR		00040000
#define S_IFCHR		00020000
#define S_IFIFO		00010000

#define S_ISUID		00004000
#define S_ISGID		00002000
#define S_ISVTX		00001000

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

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define O_CREATE	0x80000000
#define O_TRUNCATE	0x40000000

#endif	/* fs.h */
