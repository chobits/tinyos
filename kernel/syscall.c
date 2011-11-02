#include <syscall.h>
#include <print.h>
#include <task.h>
#include <int.h>

#define __sys_call_entry(name) [SYS_##name] = (syscall_t)sys_##name
static syscall_t sys_call_table[SYS_CALL_MAX + 1] = {
	__sys_call_entry(puts),
	__sys_call_entry(gets),
	__sys_call_entry(open),
	__sys_call_entry(read),
	__sys_call_entry(write),
	__sys_call_entry(fsync),
	__sys_call_entry(close),
	__sys_call_entry(lseek),
	__sys_call_entry(fork),
	__sys_call_entry(yield),
	__sys_call_entry(getpid),
	__sys_call_entry(exit),
	__sys_call_entry(wait),
	__sys_call_entry(execute),
};

void do_sys_call(struct regs *reg)
{
	syscall_t sys_call;
	if (reg->eax > SYS_CALL_MAX) {
		reg->eax = -1;
		return;
	}
	ctask->reg = reg;
	sys_call = sys_call_table[reg->eax];
	reg->eax = sys_call(reg->ebx, reg->ecx, reg->edx, reg->edi, reg->esi);
}
