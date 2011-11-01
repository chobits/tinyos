#include <disk.h>
#include <boot.h>
#include <x86.h>
#include <mm.h>

static void disk_wait(void)
{
	/* wait for disk ready */
	while (((inb(ATA_MASTER_CMD) & (ATA_STATUS_RDY|ATA_STATUS_BSY)))
		!= ATA_STATUS_RDY)
		/* do nothing */;
}

static void disk_readsect(void *dst, int lba)
{
	/* wait for disk to be ready */
	disk_wait();
	/* 28 bit LBA PIO mode read on the Primary bus */
	outb(ATA_MASTER_SECS, 1);		/* read sector count = 1 */
	outb(ATA_MASTER_LBALOW, lba & 0xff);
	outb(ATA_MASTER_LBAMID, (lba >> 8) & 0xff);
	outb(ATA_MASTER_LBAHI, (lba >> 16) & 0xff);
	outb(ATA_MASTER_LBAEX, ((lba >> 24) & 0xf) | 0xe0);	/* 0xe0 for master */
	outb(ATA_MASTER_CMD, ATA_CMD_READ);
	/* wait for disk to be ready */
	disk_wait();
	/* read sector to memory */
	insl(ATA_MASTER_DATA, dst, SECT_SIZE / sizeof(int));
}

static void load_kernel(void)
{
	void *kern_addr = (void *)KERNEL_START;
	int i = KERNEL_START_SECT;
	int n = KERNEL_SECTS;
	/*
	 * Note:
	 *  1. We dont init ata for convience.
	 *  2. Assert that ata master disk exists.
	 *  3. Assert that ata master drive I/O port start from 0x1f7
	 */
	while (i <= KERNEL_START_SECT + n) {
		disk_readsect(kern_addr, i);
		kern_addr += SECT_SIZE;
		i++;
	}
}

/*
 * Read kernel from disk to memory [0x100000, 0xf00000)
 * Assert that kernel image size is smaller than 14MB(0xe00000)
 */
void boot_main(void)
{
	/* copy kernel from disk to memory */
	load_kernel();
	/* run kernel */
	((void (*)(void))KERNEL_START)();
}

