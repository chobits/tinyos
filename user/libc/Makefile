LIBCSUBDIR = $(SUBDIR)/libc
CFLAGS += -I../include

libc.o:entry.o system.o syscall.o string.o print.o file.o
	$(Q)$(LD) $(LDFLAGS) -r $^ -o $@
	@echo " [LD]  $(LIBCSUBDIR)/$@"

%.o:%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(LIBCSUBDIR)/$@"

%.o:%.S
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(LIBCSUBDIR)/$@"
