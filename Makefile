MAKEFLAGS	+= --no-print-directory

Q	= @
LD	= ld
AS	= as
CC	= gcc
DD	= dd
NM	= nm
BUILD	= tools/build
MKFS	= tools/mkfs.minix
CTAGS	= ctags
OBJDUMP	= objdump
OBJCOPY	= objcopy
CFLAGS	= -Wall -Werror -fno-builtin -nostdinc -nostdlib -I../include -g
export Q LD AS CC NM OBJDUMP OBJCOPY CFLAGS

KLINK	= -T kernel.ld

OBJS	= kernel/kernel.o mm/mm.o video/video.o fs/fs.o keyboard/keyboard.o

all:disk.img user
user:user/init
user/init:user/*.c user/libc/*.c user/libc/*.S
	@make -C user/ all

disk.img:boot/boot.bin kernel.bin tools/build tools/mkfs.minix
	$(Q)$(BUILD) boot/boot.bin kernel.bin 10000
	@echo " [BUILD]  boot/boot.bin"
	$(Q)$(DD) of=$@ if=/dev/zero bs=512 count=10000 2>/dev/null
	$(Q)$(DD) of=$@ if=boot/boot.bin conv=notrunc 2>/dev/null
	$(Q)$(DD) of=$@ if=kernel.bin conv=notrunc bs=512 seek=8 2>/dev/null
	@echo " [DD]  $@"
	$(Q)$(MKFS) $@
	@echo " [MKFS]  $@"

tools/%:tools/%.c
	$(Q)$(CC) $< -o $@
	@echo " [CC]  $@"

kernel.bin:kernel.elf
	$(Q)$(OBJCOPY) $< -O binary $@
	@echo " [GEN]  $@"

kernel.elf:$(OBJS)
	$(Q)$(LD) $(KLINK) $^ -o $@
	@echo " [LD]  $@"

video/video.o:video/*.c
	@make -C video/

keyboard/keyboard.o:keyboard/*.c
	@make -C keyboard/

fs/fs.o:fs/*.c fs/minix/*.c
	@make -C fs/

mm/mm.o:mm/*.c
	@make -C mm/

kernel/kernel.o:kernel/*.S kernel/*.c
	@make -C kernel/

boot/boot.bin:boot/boot.S boot/main.c
	@make -C boot/

debug:kernel.asm kernel.sym
	@make -C boot/ debug
	@make -C user/ debug
kernel.asm:kernel.elf
	$(Q)$(OBJDUMP) -S $< > $@
	@echo " [DASM]  $@"
kernel.sym:kernel.elf
	$(Q)$(NM) -n $< > $@
	@echo " [NM]  $@"

# load user environment into bootale disk image
loaduser:disk.img
	sudo losetup /dev/loop0 -o $(shell cat .offset) disk.img
	-sudo mount -t minix /dev/loop0 /mnt
	-sudo cp user/init user/hello /mnt/
	-sudo touch text
	-sudo cp text /mnt/text
	-sudo umount /mnt
	-sudo losetup -d /dev/loop0

bochs:
	bochs -qf tools/bochs.bxrc
# If your bochs installs nogui library, you can run this command.
nbochs:
	bochs -qf tools/bochs.bxrc 'display_library: nogui'

tag:
	$(Q)$(CTAGS) -R *
	@echo " [CTAGS]"

mount:disk.img
	-sudo losetup /dev/loop0 -o $(shell cat .offset) disk.img
	-sudo mount -t minix /dev/loop0 /mnt

umount:
	-sudo umount /mnt
	-sudo losetup -d /dev/loop0

clean:
	rm -rf kernel.elf kernel.bin kernel.sym kernel.asm
	rm -rf */*/*.o */*.o boot/boot.bin boot/boot.elf boot/boot.asm boot/boot.sym
	rm -rf user/init user/init.sym user/init.asm user/hello
	rm -rf tools/mkfs.minix tools/build tags .offset disk.img text

lines:
	@echo "code lines:"
	@wc -l `find . -name \*.[ch]` | sort -n

