SUBDIR = user
CFLAGS	= -Wall -Werror -fno-builtin -nostdinc -nostdlib -Iinclude -g
export SUBDIR CFLAGS

all:init hello
hello:hello.o libc/libc.o
	$(Q)$(LD) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"

init:init.o libc/libc.o
	$(Q)$(LD) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"

%.o:%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"

libc/libc.o:libc/*.c libc/*.S
	@make -C libc/

debug:init.asm init.sym
init.asm:init
	$(Q)$(OBJDUMP) -S $< > $@
	@echo " [DASM]  $(SUBDIR)/$@"
init.sym:init
	$(Q)$(NM) -n $< > $@
	@echo " [NM]  $(SUBDIR)/$@"
