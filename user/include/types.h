#ifndef __TYPES_H
#define __TYPES_H

/* precise n-bit type */
typedef unsigned char u8;
typedef char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef long long s64;
typedef unsigned long long u64;

typedef unsigned int size_t;
typedef int off_t;

#define offsetof(type, member) ((size_t)&((type *)0)->member)

#define container_of(ptr, type, member)				\
({								\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)((char *)__mptr - offsetof(type, member));	\
})

#define min(a, b)		\
({				\
	typeof(a) __a = (a);	\
	typeof(b) __b = (b);	\
	__a < __b ? __a : __b;	\
})

#define max(a, b)		\
({				\
	typeof(a) __a = (a);	\
	typeof(b) __b = (b);	\
	__a >= __b ? __a : __b;	\
})

#define ALIGN_DOWN(val, align)				\
({							\
	u32 __val = (u32)(val);				\
	(typeof(val))(__val - (__val % (u32)(align)));	\
})

#define ALIGN_UP(val, align)				\
	((typeof(val))ALIGN_DOWN((u32)(val) + (u32)(align) - 1, (align)))

#define DIV_UP(dividend, divisor)	\
	(((u32)(dividend) + (u32)(divisor) - 1) / (u32)divisor)

#define MIN_INT ((~0U >> 1) + 1)
#define NULL ((void *)0)

#undef _inline
#define _inline inline __attribute__((always_inline))

#endif	/* types.h */
