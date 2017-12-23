#ifndef DISK_SDCARD_H
#define DISK_SDCARD_H


#include "c_types.h"


// http://elm-chan.org/docs/mmc/mmc_e.html


#define TYPE_SD1 1
#define TYPE_SD2 2
#define TYPE_SDHC 3

typedef struct sdcard_t {
	uint32_t type;
	uint32_t cdv;
} sdcard;


extern sdcard card;


int sdcard_init_card(sdcard* card);

int sdcard_readblocks(sdcard* card, uint32_t addr, uint8_t *buf, int size);

int sdcard_writeblocks(sdcard* card, uint32_t addr, const uint8_t *buf, int size);

#endif

