SUBDIR	= video

video.o:text.o sys.o
	$(Q)$(LD) $(LDFLAGS) -r $^ -o $@
	@echo " [LD]  $(SUBDIR)/$@"

%.o:%.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"
