#ifndef __BOOT_H
#define __BOOT_H

/* segment base is 0x7c00 */
#define BOOT_PADDR(offset) ((offset) + 0x7c00)

/* linux uses 128 as max entries. */
#define E820_MAX	128
#define E820_ENTRY_SIZE	24
#define E820_TYPE_FREE		1
#define E820_TYPE_RESERVED	2
#define E820_TYPE_ACPIRECLAIM	3
#define E820_TYPE_ACPINVS	4
#define SMAP		0x534D4150

#ifndef __ASM__
struct e820_entry {
	unsigned int addr;
	unsigned int addr_high;
	unsigned int length;
	unsigned int length_high;
	unsigned int type;
	unsigned int pad;
};

struct boot_param {
	struct e820_entry e820_list[E820_MAX];
	unsigned int e820_num;
};

#define BOOT_PARAM_ADDR	0x8c00
/* used by bootloader */
#define KERNEL_SECTS	*(unsigned short *)(0x7c00 + 508)
/* used by kernel init */
#define MINIXFS_START	*(unsigned short *)VADDR(0x7c00 + 506)
#define IDE_DISK_SECTS	*(unsigned int *)VADDR(0x7c00 + 500)

#endif	/* __ASM__ */

#endif	/* boot.h */
