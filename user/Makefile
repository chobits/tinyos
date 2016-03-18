SUBDIR = user
CFLAGS	= -Wall -Werror -fno-builtin -nostdinc -nostdlib -Iinclude -g -m32
export SUBDIR CFLAGS
APPS	= $(patsubst user/%, %, $(USER_APPS))
ASMS	= $(patsubst %, %.asm, $(APPS))
SYMS	= $(patsubst %, %.sym, $(APPS))

all:$(APPS)
# basic programs
hello:hello.o libc/libc.o
	$(Q)$(LD) $(LDFLAGS) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"
init:init.o libc/libc.o
	$(Q)$(LD) $(LDFLAGS) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"
sh:shell/shell.o libc/libc.o
	$(Q)$(LD) $(LDFLAGS) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"
# utilities
cat:utils/cat.o libc/libc.o
	$(Q)$(LD) $(LDFLAGS) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"

stat:utils/stat.o libc/libc.o
	$(Q)$(LD) $(LDFLAGS) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"
mkdir:utils/mkdir.o libc/libc.o
	$(Q)$(LD) $(LDFLAGS) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"
ls:utils/ls.o libc/libc.o
	$(Q)$(LD) $(LDFLAGS) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"
sync:utils/sync.o libc/libc.o
	$(Q)$(LD) $(LDFLAGS) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"
rmdir:utils/rmdir.o libc/libc.o
	$(Q)$(LD) $(LDFLAGS) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"
rm:utils/rm.o libc/libc.o
	$(Q)$(LD) $(LDFLAGS) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"
cp:utils/cp.o libc/libc.o
	$(Q)$(LD) $(LDFLAGS) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"
pwd:utils/pwd.o libc/libc.o
	$(Q)$(LD) $(LDFLAGS) $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"

# objects
%.o:%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"
utils/%.o:utils/%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SUBDIR)/utils/$@"

libc/libc.o:libc/*.c libc/*.S
	@make -C libc/
shell/shell.o:shell/*.c
	@make -C shell/

debug:$(ASMS) $(SYMS)
%.asm:%
	$(Q)$(OBJDUMP) -S $< > $@
	@echo " [DASM]  $(SUBDIR)/$@"
%.sym:%
	$(Q)$(NM) -n $< > $@
	@echo " [NM]  $(SUBDIR)/$@"
