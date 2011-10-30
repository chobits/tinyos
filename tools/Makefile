#Author:chobit_s <wangxiaochen0@gmail.com>
all:mkfs.minix
mkfs.minix:mkfs.minix.c
	gcc $< -o $@

#generate 1024MB file(data:0),which can be used as block device
data:
	dd if=/dev/zero of=./data bs=1024 count=1024
install:
	sudo mount -t minix data -o loop test_mount_dir/
un:
	sudo umount test_mount_dir/
clean:
	rm -rf data mkfs.minix
