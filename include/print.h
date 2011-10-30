#ifndef __PRINT_H
#define __PRINT_H

extern int text_puts(char *s);
extern int text_nputs(char *s, int n);
extern void text_putc(char c);
extern int printk(char *format, ...);
extern void panic(char *format, ...);
struct regs;
extern void dump_stack(struct regs *);

#define debug(fmt, argv...) printk("%s: "fmt"\n", __FUNCTION__, ##argv)

#endif	/* print.h */
