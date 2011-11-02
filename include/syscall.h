#ifndef __SYSCALL_H
#define __SYSCALL_H

#include <types.h>

typedef u32 (*syscall_t)(u32, u32, u32, u32, u32);

#define SYS_puts	1
#define SYS_gets	2
#define SYS_open	3
#define SYS_read	4
#define SYS_write	5
#define SYS_fsync	6
#define SYS_close	7
#define SYS_lseek	8
#define SYS_fork	9
#define SYS_yield	10
#define SYS_getpid	11
#define SYS_exit	12
#define SYS_wait	13
#define SYS_execute	14
#define SYS_CALL_MAX	14

extern int sys_puts(char *);
extern int sys_gets(char *, int);
extern int sys_open(char *, size_t);
extern int sys_read(int, char *, size_t);
extern int sys_write(int, char *, size_t);
extern int sys_fsync(int);
extern int sys_close(int);
extern off_t sys_lseek(int, off_t, unsigned int);
extern int sys_fork(void);
extern void sys_yield(void);
extern int sys_getpid(void);
extern void sys_exit(int);
extern int sys_wait(int *);
extern int sys_execute(char *, int, char **);

#endif	/* syscall.h */
