SUBDIR	= mm

mm.o:memory.o buddy.o boot_memory.o paging.o slab.o page.o
	$(Q)$(LD) $(LDFLAGS) -r $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"

%.o:%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"
