#ifndef __SYSCALL_H
#define __SYSCALL_H

#include <types.h>

#define INTNO_SYSCALL	48

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

extern int usys_puts(char *);
extern int usys_open(char *, unsigned int);
extern int usys_read(int, char *, unsigned int);
extern int usys_write(int, char *, unsigned int);
extern int usys_sync(int);
extern int usys_close(int);
extern off_t usys_lseek(int, off_t, unsigned int);
extern int usys_fork(void);
extern void usys_yield(void);
extern int usys_getpid(void);
extern void usys_exit(int);
extern int usys_wait(int *);
extern int usys_execute(char *, int, char **);

#endif	/* syscall.h */
