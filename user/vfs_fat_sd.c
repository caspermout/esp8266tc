
#include "c_types.h"
#include "osapi.h"

#include "ff.h"
#include "disk_sdcard.h"

// http://wizard.ae.krakow.pl/~jb/fatlib/


// https://github.com/gallegojm/Arduino-FatFs/blob/master/FatLib/FatLib.cpp
// https://github.com/gallegojm/Arduino-FatFs/tree/master/FatFs

// https://github.com/boivie/fatlib

// https://github.com/greiman/SdFat

// http://elm-chan.org/fsw/ff/00index_e.html



FATFS fatfs_sdcard;


ICACHE_FLASH_ATTR
void fat_init(sdcard *card) 
{
	os_printf("fat_init\n");


/*
	uint8_t buf[512];
	int res = sdcard_readblocks(card, 0, buf, 512);
	os_printf("sdcard_readblock res %d\n", res);

	res = sdcard_readblocks(card, 0, buf, 512);
	os_printf("sdcard_readblock res %d\n", res);

	res = sdcard_readblocks(card, 512, buf, 512);
	os_printf("sdcard_readblock res %d\n", res);
*/

	char *drive = "1:"; //pdrv = 1
	FRESULT r = f_mount(&fatfs_sdcard, drive, 1);
	if(r != FR_OK) {
		os_printf("Mount sdcard %s failed: %d\n", drive, r);
		return;
	}

	// http://elm-chan.org/fsw/ff/doc/chdrive.html
	r = f_chdrive(drive);
	if(r != FR_OK) {
		os_printf("Could not set default drive %s: %d\n", drive, r);
		return;
	}

	// http://elm-chan.org/fsw/ff/doc/open.html
	// http://elm-chan.org/fsw/ff/doc/filename.html
	FIL fp;
	r = f_open(&fp, "/config", FA_READ);
	if(r != FR_OK) {
		os_printf("Open file /config failed: %d\n", r);
		return;
	}

	char line[256];
	os_printf("--------------\n");
	// http://elm-chan.org/fsw/ff/doc/gets.html
	while (f_gets(line, sizeof line, &fp)) {
		os_printf(line);
	}
	os_printf("--------------\n");

	// http://elm-chan.org/fsw/ff/doc/eof.html
	// http://elm-chan.org/fsw/ff/doc/error.html
	if(f_error(&fp)) {
		os_printf("Error while reading file: %d\n", f_error(&fp));
	}

	r = f_close(&fp);
	if(r != FR_OK) {
		os_printf("Close file failed: %d\n", r);
		return;
	}
}

