#ifndef __ASM_H
#define __ASM_H

#define _ENTRY(name)\
	.global name;\
	name:

#define ENTRY(name)\
	.global name;\
	.align 4, 0x90;\
	name:

#define ENTRY_NOERRCODE(func, num)\
	ENTRY(func);\
	pushl $0;\
	pushl $(num);\
	jmp interrupt_entry

#define ENTRY_ERRCODE(func, num)\
	ENTRY(func);\
	pushl $(num);\
	jmp interrupt_entry

#define ENTRY_INT(func, num) ENTRY_NOERRCODE(func, num)

#endif	/* asm.h */
