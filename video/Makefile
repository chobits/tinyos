SUBDIR	= video

video.o:text.o sys.o
	$(Q)$(LD) -r $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"

%.o:%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"
