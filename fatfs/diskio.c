/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */

#include "disk_io.h"
#include "sntp.h"


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	uint32_t result;

	if(pdrv > MAX_DISKS) {
		return STA_NOINIT;
	}

	disk_io_t *disk = &disks[pdrv];

	if(disk->type == 0) {
		return STA_NOINIT;
	}

	stat = ((disk_io_status_t*)disk->status)(disk);


		// translate the reslut code here

		return stat;


}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	if(pdrv > MAX_DISKS) {
		return STA_NOINIT;
	}

	disk_io_t *disk = &disks[pdrv];

	if(disk->type == 0) {
		return STA_NOINIT;
	}

	stat = ((disk_io_init_t*)disk->init)(disk);

	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	if(pdrv > MAX_DISKS) {
		return RES_PARERR;
	}

	disk_io_t *disk = &disks[pdrv];

	if(disk->type == 0) {
		return RES_PARERR;
	}

	result = ((disk_io_read_t*)disk->read)(disk, sector * disk->blksize, buff, count * disk->blksize);


	return result;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	if(pdrv > MAX_DISKS) {
		return RES_PARERR;
	}

	disk_io_t *disk = &disks[pdrv];

	if(disk->type == 0) {
		return RES_PARERR;
	}

	result = ((disk_io_write_t*)disk->write)(disk, sector * disk->blksize, buff, count * disk->blksize);

	return result;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;


	if(pdrv > MAX_DISKS) {
		return RES_PARERR;
	}

	disk_io_t *disk = &disks[pdrv];

	if(disk->type == 0) {
		return RES_PARERR;
	}

	result = ((disk_io_ioctl_t*)disk->ioctl)(disk, cmd, buff);

	return result;
}

DWORD get_fattime (void) {
	return sntp_get_current_timestamp();
}


