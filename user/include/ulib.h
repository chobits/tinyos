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

#endif	/* ulib.h */
