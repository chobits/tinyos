#include <print.h>
#include <boot.h>
#include <disk.h>
#include <x86.h>
#include <mm.h>

static int ide_wait(int check)
{
	int r;
	/* wait for disk ready */
	while ((((r = inb(ATA_MASTER_CMD)) & (ATA_STATUS_RDY|ATA_STATUS_BSY)))
		!= ATA_STATUS_RDY)
		/* do nothing */;
	if (check && (r & (ATA_STATUS_DF | ATA_STATUS_ERR )))
		return -1;
	return 0;
}

int ide_read_sect(int sects, void *dst, int lba)
{
	int r;
#ifdef DEBUG_IDE
	debug("read sects %d lba %d", sects, lba);
#endif
	if (sects <= 0)
		return -1;
	/* wait for disk to be ready */
	ide_wait(0);
	/* 28 bit LBA PIO mode read on the Primary bus */
	outb(ATA_MASTER_SECS, sects);		/* read sector count = 1 */
	outb(ATA_MASTER_LBALOW, lba & 0xff);
	outb(ATA_MASTER_LBAMID, (lba >> 8) & 0xff);
	outb(ATA_MASTER_LBAHI, (lba >> 16) & 0xff);
	outb(ATA_MASTER_LBAEX, ((lba >> 24) & 0xf) | 0xe0);	/* 0xe0 for master */
	outb(ATA_MASTER_CMD, ATA_CMD_READ);
	/* copy to dest buffer */
	for (r = 0; r < sects; r++) {
		/* wait for disk to be ready */
		if (ide_wait(1) < 0)
			break;
		/* read sector to memory */
		insl(ATA_MASTER_DATA, dst, SECT_SIZE / sizeof(int));
		dst += SECT_SIZE;
	}
	return r;
}

int ide_write_sect(int sects, void *src, int lba)
{
	int r;
#ifdef DEBUG_IDE
	debug("write sects %d lba %d", sects, lba);
#endif
	if (sects <= 0)
		return -1;
	/* wait for disk to be ready */
	ide_wait(0);
	/* 28 bit LBA PIO mode read on the Primary bus */
	outb(ATA_MASTER_SECS, sects);		/* read sector count = 1 */
	outb(ATA_MASTER_LBALOW, lba & 0xff);
	outb(ATA_MASTER_LBAMID, (lba >> 8) & 0xff);
	outb(ATA_MASTER_LBAHI, (lba >> 16) & 0xff);
	outb(ATA_MASTER_LBAEX, ((lba >> 24) & 0xf) | 0xe0);	/* 0xe0 for master */
	outb(ATA_MASTER_CMD, ATA_CMD_WRITE);
	/* copy to ide bus */
	for (r = 0; r < sects; r++) {
		/* wait for disk to be ready */
		if (ide_wait(1) < 0)
			break;
		/* read sector to memory */
		outsl(ATA_MASTER_DATA, src, SECT_SIZE / sizeof(int));
		src += SECT_SIZE;
	}
	return r;
}

int ide_blocks(void)
{
	return IDE_DISK_SECTS / 2;
}
