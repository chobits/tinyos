#ifndef __SYSCALL_H
#define __SYSCALL_H

#include <types.h>

struct file_stat;
struct dir_stat;

#define INTNO_SYSCALL	48

#define SYS_puts	1
#define SYS_gets	2
#define SYS_open	3
#define SYS_read	4
#define SYS_write	5
#define SYS_fsync	6
#define SYS_fstat	7
#define SYS_close	8
#define SYS_lseek	9
#define SYS_fork	10
#define SYS_yield	11
#define SYS_getpid	12
#define SYS_exit	13
#define SYS_wait	14
#define SYS_execute	15
#define SYS_fchdir	16
#define SYS_fgetdir	17
#define SYS_mkdir	18
#define SYS_sync	19
#define SYS_rmdir	20
#define SYS_rm		21
#define SYS_truncate	22
#define SYS_getcwd	23

extern int usys_puts(char *);
extern int usys_gets(char *, int);
extern int usys_open(char *, unsigned int);
extern int usys_read(int, char *, unsigned int);
extern int usys_write(int, char *, unsigned int);
extern int usys_fsync(int);
extern int usys_fstat(int, struct file_stat *);
extern int usys_close(int);
extern off_t usys_lseek(int, off_t, unsigned int);
extern int usys_fork(void);
extern void usys_yield(void);
extern int usys_getpid(void);
extern void usys_exit(int);
extern int usys_wait(int *);
extern int usys_execute(char *, int, char **);
extern int usys_fchdir(int);
extern int usys_fgetdir(int fd, int s, int n, struct dir_stat *ds);
extern int usys_mkdir(char *, unsigned int);
extern void usys_sync(void);
extern int usys_rmdir(char *);
extern int usys_rm(char *);
extern int usys_truncate(int);
extern int usys_getcwd(char *buf, size_t size);

#endif	/* syscall.h */
