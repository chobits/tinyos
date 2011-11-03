#ifndef __CTYPE_H
#define __CTYPE_H

#include <types.h>

static _inline int isblank(int c)
{
	return ((c == ' ') || (c == '\t'));
}

#endif	/* ctype.h */
