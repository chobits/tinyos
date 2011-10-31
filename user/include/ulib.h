#ifndef __ULIB_H
#define __ULIB_H

#include <types.h>

extern int printf(char *, ...);
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
extern int write(unsigned int fd, char *buf, size_t size);
extern int read(unsigned int fd, char *buf, size_t size);
extern int fsync(unsigned int fd);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#endif	/* ulib.h */
