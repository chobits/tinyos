#ifndef __VARG_H
#define __VARG_H

typedef char *va_list;
#define va_mask (sizeof(int) - 1)
#define va_size(type) ((sizeof(type) + va_mask) & ~va_mask)
#define va_start(ap, last) (ap = (va_list)&(last) + va_size(last))
#define va_arg(ap, type) (ap += va_size(type), *(type *)(ap - va_size(type)))
#define va_end(ap)

#endif	/* varg.h */
