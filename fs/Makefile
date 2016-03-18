SUBDIR	= fs
export SUBDIR

SRC	= $(wildcard *.c)
OBJS	= $(patsubst %.c, %.o, $(SRC)) minix/minixfs.o

fs.o:$(OBJS)
	$(Q)$(LD) $(LDFLAGS) -r $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"

minix/minixfs.o:minix/*.c
	@make -C minix/

%.o:%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"

%.o:%.S
	$(Q)$(CC) $(CFLAGS) -D__ASM__ $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"
