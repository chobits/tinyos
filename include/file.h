#ifndef __FILE_H
#define __FILE_H

#include <types.h>

struct file_stat;
struct inode;
struct file {
	off_t f_pos;
	struct inode *f_inode;
	int f_refcnt;
	unsigned int f_mode;
};

#define FD_SAVED	((struct file *)0xffffdead)
#define FD_SIZE		16

struct fd_table {
	struct file *files[FD_SIZE];
};

extern struct file *file_open(char *path, unsigned int mode);
extern void file_close(struct file *file);
extern int file_read(struct file *file, char *buf, size_t size);
extern int file_write(struct file *file, char *buf, size_t size);
extern struct file *get_file(struct file *file);
extern void put_file(struct file *file);
extern off_t file_lseek(struct file *file, off_t offset, int whence);
extern void file_sync(struct file *file);
extern int file_stat(struct file *file, struct file_stat *stat);
extern void ft_close(struct fd_table *);
extern struct file *alloc_file(struct inode *, unsigned int);

#endif	/* file.h */
