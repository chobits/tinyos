.code32
.text

.globl _start
_start:
	call main
_exit:
	push %eax
	call exit
_die:
	jmp _die
