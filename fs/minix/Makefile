MINIXSUBDIR	= $(SUBDIR)/minix
CFLAGS	+= -I../../include

minixfs.o:init.o inode.o map.o test.o
	$(Q)$(LD) $(LDFLAGS) -r $^ -o $@
	@echo " [LD]  $(MINIXSUBDIR)/$@"

%.o:%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(MINIXSUBDIR)/$@"
