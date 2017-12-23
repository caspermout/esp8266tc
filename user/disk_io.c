#include <osapi.h>

#include <../fatfs/diskio.h>

#include "disk_io.h"
#include "disk_sdcard.h"



disk_io_t disks[MAX_DISKS];


uint32_t sdcard_disk_status(void *diskp) {
	
	disk_io_t *disk = (disk_io_t*)diskp;

//	os_printf("sdcard_disk_status %d\n", disk->initialized);
	if(!disk->initialized) {
		return STA_NOINIT;
	}
	return 0;
}

uint32_t sdcard_disk_init(void *diskp) {
	
	disk_io_t *disk = (disk_io_t*)diskp;

	if(disk->initialized) {
		return RES_OK;
	}

	os_printf("sdcard_disk_init %d\n", disk->initialized);

	int r = sdcard_init_card(&card);
	if(r != 0) {
		return STA_NODISK;
	}
	disk->blksize = 512;
	disk->initialized = 1;
	return 0;
}

uint32_t sdcard_disk_read(void *diskp, uint32_t sector, uint8_t* buf, uint32_t count) {
	
	disk_io_t *disk = (disk_io_t*)diskp;

	if(!disk->initialized) {
		return RES_NOTRDY;
	}

//	os_printf("sdcard_disk_read res %d %x %d\n", sector, buf, count);

	int res = sdcard_readblocks(&card, sector, buf, count);
//	os_printf("sdcard_disk_read res %d\n", res);

	if(res != 0) {
		return RES_ERROR;
	}

	return RES_OK;
}

uint32_t sdcard_disk_write(void *diskp, uint32_t sector, const uint8_t* buf, uint32_t count) {
	
	disk_io_t *disk = (disk_io_t*)diskp;

	if(!disk->initialized) {
		return RES_NOTRDY;
	}

//	os_printf("sdcard_disk_write res %d %x %d\n", sector, buf, count);

	int res = sdcard_writeblocks(&card, sector, buf, count);
//	os_printf("sdcard_disk_write res %d\n", res);

	if(res != 0) {
		return RES_ERROR;
	}

	return RES_OK;
}

uint32_t sdcard_disk_ioctl(void *diskp, uint32_t cmd, void* buf) {
	
	disk_io_t *disk = (disk_io_t*)diskp;

//	os_printf("sdcard_disk_ioctl res %d\n", cmd);

	if(cmd == CTRL_SYNC) {
		return RES_OK;
	}

	return RES_PARERR;
}


void disk_io_init() {

	int i = 0;
	for(; i < MAX_DISKS; i++) {
		disks[i].type = 0;
	}

	disks[0].type = 0; // spiffs op int flash hier doen?

/*
	int r = sdcard_init_card(&card);
	if(r != 0) {
		// partities detecten?
		// denk dat fatfs dit al doet? http://elm-chan.org/fsw/ff/doc/filename.html
		// http://elm-chan.org/fsw/ff/doc/fdisk.html


		uint8_t mbr[512];

		int res = sdcard_readblocks(&card, 0, mbr, 512);
		os_printf("sdcard_readblock res %d\n", res);

		if(res != 0) {
		}



	}
*/
	
	disks[1].type = 1; // spiffs op int flash hier doen?
	disks[1].initialized = 0;
	disks[1].status = sdcard_disk_status;
	disks[1].init = sdcard_disk_init;
	disks[1].read = sdcard_disk_read;
	disks[1].write = sdcard_disk_write;
	disks[1].ioctl = sdcard_disk_ioctl;
}


