#ifndef __STRING_H
#define __STRING_H

extern int setbit(void *src, int bits);
extern int clearbit(void *src, int bits);
extern int searchzerobit(void *src, int bits, int set);
extern int strnncpy(char *dst, char *src, int n);
extern int testbit(void *src, int bits);
extern void memmove(void *dst, void *src, int n);
extern int strtoi(char *nptr, char **endptr, int base);
extern int itostr(int n, char *nptr, int base);
extern int utostr(unsigned int n, char *nptr, int base);
extern void memset(void *dst, int c, int n);
extern void wmemset(void *dst, int c, int n);
extern void memcpy(void *dst, void *src, int n);
extern int strcpy(char *dst, char *src);
extern int strncpy(char *dst, char *src, int n);
extern int log2(int n);
extern int log2up(int n);
extern int strncmp(char *s1, char *s2, unsigned int n);
extern int strcmp(char *s1, char *s2);

#endif	/* string.h */
