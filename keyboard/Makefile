SUBDIR	= keyboard

OBJS	= key.o keybuf.o scancode.o

keyboard.o:$(OBJS)
	$(Q)$(LD) $(LDFLAGS) -r $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"

%.o:%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"

