SUBDIR = user
CFLAGS	= -Wall -Werror -fno-builtin -nostdinc -nostdlib -Iinclude -g
ULIB_OBJS = entry.o print.o string.o syscall.o system.o

all:init hello
hello:hello.o $(ULIB_OBJS)
	$(Q)$(LD) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"

init:init.o $(ULIB_OBJS)
	$(Q)$(LD) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"

%.o:%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"
%.o:%.S
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"

debug:init.asm init.sym
init.asm:init
	$(Q)$(OBJDUMP) -S $< > $@
	@echo " [DASM]  $(SUBDIR)/$@"
init.sym:init
	$(Q)$(NM) -n $< > $@
	@echo " [NM]  $(SUBDIR)/$@"
