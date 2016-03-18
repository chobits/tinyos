SUBDIR	= kernel

OBJS	= entry.o init.o interrupt.o int.o print.o string.o task.o execute.o\
	syscall.o fork.o schedule.o exit.o wait_queue.o pit.o timer.o

kernel.o:$(OBJS)
	$(Q)$(LD) $(LDFLAGS) -r $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"

%.o:%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"

%.o:%.S
	$(Q)$(CC) $(CFLAGS) -D__ASM__ $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"
