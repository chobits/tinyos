#ifndef __SYSCALL_H
#define __SYSCALL_H

#include <types.h>

typedef u32 (*syscall_t)(u32, u32, u32, u32, u32);

#define SYS_puts	1
#define SYS_open	2
#define SYS_read	3
#define SYS_write	4
#define SYS_sync	5
#define SYS_close	6
#define SYS_lseek	7
#define SYS_fork	8
#define SYS_yield	9
#define SYS_getpid	10
#define SYS_exit	11
#define SYS_wait	12
#define SYS_execute	13
#define SYS_CALL_MAX	13

extern int sys_puts(char *);
extern int sys_open(char *, size_t);
extern int sys_read(int, char *, size_t);
extern int sys_write(int, char *, size_t);
extern int sys_sync(int);
extern int sys_close(int);
extern off_t sys_lseek(int, off_t, unsigned int);
extern int sys_fork(void);
extern void sys_yield(void);
extern int sys_getpid(void);
extern void sys_exit(int);
extern int sys_wait(int *);
extern int sys_execute(char *, int, char **);

#endif	/* syscall.h */
