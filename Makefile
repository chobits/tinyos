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

KVM	= qemu-kvm
KLINK	= -T kernel.ld
USER_APPS = user/init user/hello user/sh user/cat user/stat user/ls user/mkdir\
		user/sync user/rmdir user/rm user/cp user/pwd
export USER_APPS

OBJS	= kernel/kernel.o mm/mm.o video/video.o fs/fs.o keyboard/keyboard.o

all:disk.img user
user:user/init
user/init:user/*.c user/*/*.c user/*/*.S
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

# minixdir/dir/[1-100]
MKDIRS = $(shell seq -s ' ' 1 100 | sed 's/\([0-9]\+\)/minixdir\/dir\/\1/g')
RMDIRS = $(shell seq -s ' ' 20 40 | sed 's/\([0-9]\+\)/minixdir\/dir\/\1/g')
# load user environment into bootale disk image
loaduser:disk.img
	-mkdir -p minixdir
	-sudo losetup /dev/loop0 -o $(shell cat .offset) disk.img
	-sudo mount -t minix /dev/loop0 minixdir
	-sudo cp $(USER_APPS) minixdir/
	-sudo mkdir -p minixdir/dir
	-sudo mkdir -p minixdir/dir/dir1
	-@sudo mkdir -p $(MKDIRS); echo "Create 100 dirs"
	-@sudo rmdir $(RMDIRS); echo "Remove 20 dirs"
	-echo "hello wrold" > text1; sudo mv text1 minixdir/
	-echo "RIP Dennis Ritchie" > text2; sudo mv text2 minixdir/dir/
	-sudo umount minixdir
	-sudo losetup -d /dev/loop0
	-rm -rf minixdir

kvm:disk.img
	$(KVM) -hda disk.img -m 64
bochs:disk.img
	bochs -qf tools/bochs.bxrc
# If your bochs installs nogui library, you can run this command.
nbochs:disk.img
	bochs -qf tools/bochs.bxrc 'display_library: nogui'

tag:
	$(Q)$(CTAGS) -R *
	@echo " [CTAGS]"

fsck:disk.img
	-sudo losetup /dev/loop0 -o $(shell cat .offset) disk.img
	-sudo fsck.minix /dev/loop0
	-sudo losetup -d /dev/loop0


mount:disk.img
	mkdir -p minixdir
	-sudo losetup /dev/loop0 -o $(shell cat .offset) disk.img
	-sudo mount -t minix /dev/loop0 minixdir

umount:
	-sudo umount minixdir
	-sudo losetup -d /dev/loop0
	rm -rf minixdir

clean:
	find . -regex ".*\.\(o\|sym\|asm\|elf\|bin\)" -exec rm -f {} ';'
	rm -rf tools/mkfs.minix tools/build tags .offset disk.img
	rm -rf $(USER_APPS)

lines:
	@echo "code lines:"
	@wc -l `find . -name \*.[ch]` | sort -n

