#ifndef __DISK_H
#define __DISK_H

/* ATA Master Drive */
#define ATA_MASTER_DATA		0x1f0
#define ATA_MASTER_ERR		0x1f1
#define ATA_MASTER_SECS		0x1f2
#define ATA_MASTER_LBALOW	0x1f3
#define ATA_MASTER_LBAMID	0x1f4
#define ATA_MASTER_LBAHI	0x1f5
#define ATA_MASTER_LBAEX	0x1f6
#define ATA_MASTER_CMD		0x1f7

/* ATA Status Bits */
#define ATA_STATUS_ERR		0x01	/* set for disk error */
#define ATA_STATUS_DRQ		0x08
#define ATA_STATUS_SRV		0x10
#define ATA_STATUS_DF 		0x20
#define ATA_STATUS_RDY		0x40	/* set for ready */
#define ATA_STATUS_BSY		0x80	/* set for busy */

#define ATA_CMD_READ		0x20
#define ATA_CMD_WRITE		0x30

#define SECT_SHIFT		9
#define SECT_SIZE		(1 << SECT_SHIFT)
#define SECT_MASK		(SECT_SIZE - 1)

extern int ide_read_sect(int sects, void *dst, int lba);
extern int ide_write_sect(int sects, void *src, int lba);
extern int ide_blocks(void);

#endif	/* disk.h */
