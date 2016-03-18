SHELLSUBDIR = $(SUBDIR)/libc
CFLAGS += -I../include

shell.o:main.o builtin.o process.o parse.o
	$(Q)$(LD) $(LDFLAGS) -r $^ -o $@
	@echo " [LD]  $(SHELLSUBDIR)/$@"

%.o:%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SHELLSUBDIR)/$@"

