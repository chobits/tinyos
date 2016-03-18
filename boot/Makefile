LFLAGS	= -Ttext=0x0000 --oformat=binary -e boot_start
SUBDIR	= boot

boot.bin:boot.elf
	$(Q)$(OBJCOPY) $< -O binary $@
	@echo " [GEN]  $(SUBDIR)/$@"

boot.elf:boot.o main.o
	$(Q)$(LD) $(LDFLAGS) -N -Ttext=0x0000 -e boot_start $^ -o boot.elf
	@echo " [LD]  $(SUBDIR)/$@"

main.o:main.c
	$(Q)$(CC) $(CFLAGS) $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"

boot.o:boot.S
	$(Q)$(CC) $(CFLAGS) -D__ASM__ $< -c -o $@
	@echo " [CC]  $(SUBDIR)/$@"

debug:boot.asm boot.sym
boot.asm:boot.elf
	$(Q)$(OBJDUMP) -S $< > $@
	@echo " [DASM]  $(SUBDIR)/$@"
boot.sym:boot.elf
	$(Q)$(NM) -n $< > $@
	@echo " [NM]  $(SUBDIR)/$@"
