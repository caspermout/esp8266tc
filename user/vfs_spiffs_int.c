#include <c_types.h>

#include <spiffs.h>
#include <spiffs_nucleus.h>
#include <spi_flash.h>

#include "user_interface.h"
  
static spiffs spiffs_fs;

#define LOG_PAGE_SIZE       0x100 // SPI_FLASH_SEC_SIZE


/*
PROVIDE ( _SPIFFS_start = 0x40300000 );
PROVIDE ( _SPIFFS_end = 0x405FB000 );
PROVIDE ( _SPIFFS_page = 0x100 );
PROVIDE ( _SPIFFS_block = 0x2000 );
*/

#define _SPIFFS_page 0x100
#define _SPIFFS_erase 0x1000
#define _SPIFFS_block 0x2000

#define INTERNAL_FLASH_READ_UNIT_SIZE 4
#define INTERNAL_FLASH_WRITE_UNIT_SIZE 4

  
static u8_t spiffs_work_buf[LOG_PAGE_SIZE*2];
static u8_t spiffs_fds[sizeof(spiffs_fd)*8];
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE+32)*8];


// https://github.com/nodemcu/nodemcu-firmware/blob/8e48483c825dea9c12b37a4db3d034fccbcba0bf/app/platform/common.c
// deel hiervan gestolen


  static s32_t my_spiffs_read(u32_t addr, u32_t size, u8_t *dst) {

//os_printf("my_spiffs_read %x %x %x\r\n", addr, size, dst);

/*

SpiFlashOpResult	spi_flash_read( 
				uint32	src_addr,	 
				uint32	*	des_addr,	 
				uint32	size 
)
uint32	src_addr: source address in flash.
Parameter
uint32	*des_addr: destination address to keep data.
uint32	size: length of data; unit: byte, has to be aligned to the 4-bytes boundary.
Return
Espressif
typedef	enum	{ 
				SPI_FLASH_RESULT_OK, 
				SPI_FLASH_RESULT_ERR, 
				SPI_FLASH_RESULT_TIMEOUT 
}	SpiFlashOpResult;

u32_t org_addr = addr;
u32_t org_size = size;

addr = addr & ~0x3;
*/



/*
    if (addr & 0x3) {
			os_printf("addr(%d) & 0x3 = %d ", addr, addr & 0x3);
			return SPIFFS_ERR_INTERNAL;
    }
    if (size & 0x3) {
			os_printf("size(%d) & 0x3 = %d ", size, size & 0x3);
			return SPIFFS_ERR_INTERNAL;
    }

	SpiFlashOpResult res = spi_flash_read(addr, (uint32*)dst, size);
		if(res != SPI_FLASH_RESULT_OK) {
			os_printf("spi_flash_read res %d ", res);
			return SPIFFS_ERR_INTERNAL;
		}
*/

u32_t fromaddr = addr;

  uint32_t temp, rest, ssize = size;
  unsigned i;
  char tmpdata[ INTERNAL_FLASH_READ_UNIT_SIZE ] __attribute__ ((aligned(INTERNAL_FLASH_READ_UNIT_SIZE)));
  uint8_t *pto = ( uint8_t* )dst;
  const uint32_t blksize = INTERNAL_FLASH_READ_UNIT_SIZE;
  const uint32_t blkmask = INTERNAL_FLASH_READ_UNIT_SIZE - 1;

system_soft_wdt_feed();


SpiFlashOpResult res;

  // Align the start
  if( fromaddr & blkmask )
  {
    rest = fromaddr & blkmask;
    temp = fromaddr & ~blkmask; // this is the actual aligned address
//    platform_s_flash_read( tmpdata, temp, blksize );

//os_printf("c_spi_flash_read %x %x %x\r\n", temp, tmpdata, blksize);
	 res = spi_flash_read(temp, (uint32*)tmpdata, blksize);
		if(res != SPI_FLASH_RESULT_OK) {
			os_printf("spi_flash_read res %d ", res);
			return SPIFFS_ERR_INTERNAL;
		}


    for( i = rest; size && ( i < blksize ); i ++, size --, pto ++ )
      *pto = tmpdata[ i ];

    if( size == 0 )
	    return SPIFFS_OK;

//      return ssize;
    fromaddr = temp + blksize;
  }
  // The start address is now a multiple of blksize
  // Compute how many bytes we can read as multiples of blksize
  rest = size & blkmask;
  temp = size & ~blkmask;
  // Program the blocks now
  if( temp )
  {
//    platform_s_flash_read( pto, fromaddr, temp );
//os_printf("c_spi_flash_read %x %x %x\r\n", fromaddr, pto, temp);
	 res = spi_flash_read(fromaddr, (uint32*)pto, temp);
		if(res != SPI_FLASH_RESULT_OK) {
			os_printf("spi_flash_read res %d ", res);
			return SPIFFS_ERR_INTERNAL;
		}
    fromaddr += temp;
    pto += temp;
  }
  // And the final part of a block if needed
  if( rest )
  {
//    platform_s_flash_read( tmpdata, fromaddr, blksize );
 
	 res = spi_flash_read(fromaddr, (uint32*)tmpdata, blksize);
		if(res != SPI_FLASH_RESULT_OK) {
			os_printf("spi_flash_read res %d ", res);
			return SPIFFS_ERR_INTERNAL;
		}


   for( i = 0; size && ( i < rest ); i ++, size --, pto ++ )
      *pto = tmpdata[ i ];
  }
//  return ssize;


    return SPIFFS_OK;
  }

  static s32_t my_spiffs_write(u32_t addr, u32_t size, u8_t *src) {

os_printf("my_spiffs_write %x %x %x\r\n", addr, size, src);

/*
SpiFlashOpResult	spi_flash_write	( 
				uint32	des_addr,	 
				uint32	*src_addr,	 
				uint32	size 
)
uint32	des_addr: destination address in flash.
Parameter
uint32	*src_addr: source address of the data.
uint32	size: length of data, uint: byte, has to be aligned to the 4-byte boundary.
Return
typedef	enum{ 
				SPI_FLASH_RESULT_OK, 
				SPI_FLASH_RESULT_ERR, 
				SPI_FLASH_RESULT_TIMEOUT 
}	SpiFlashOpResult;
*/


/*
    if (addr & 0x3) {
			os_printf("addr(%d) & 0x3 = %d ", addr, addr & 0x3);
			return SPIFFS_ERR_INTERNAL;
    }
    if (size & 0x3) {
			os_printf("size(%d) & 0x3 = %d ", size, size & 0x3);
			return SPIFFS_ERR_INTERNAL;
    }


	SpiFlashOpResult res = spi_flash_write(addr, (uint32*)src, size);
		if(res != SPI_FLASH_RESULT_OK) {
			os_printf("spi_flash_write res %d ", res);
			return SPIFFS_ERR_INTERNAL;
		}
*/

u32_t toaddr = addr;


system_soft_wdt_feed();


  uint32_t temp, rest, ssize = size;
  unsigned i;
  char tmpdata[ INTERNAL_FLASH_READ_UNIT_SIZE ] __attribute__ ((aligned(INTERNAL_FLASH_READ_UNIT_SIZE)));
//  char tmpdata[ INTERNAL_FLASH_WRITE_UNIT_SIZE ];
  const uint8_t *pfrom = ( const uint8_t* )src;
  const uint32_t blksize = INTERNAL_FLASH_WRITE_UNIT_SIZE;
  const uint32_t blkmask = INTERNAL_FLASH_WRITE_UNIT_SIZE - 1;

SpiFlashOpResult res;

  // Align the start
  if( toaddr & blkmask )
  {
    rest = toaddr & blkmask;
    temp = toaddr & ~blkmask; // this is the actual aligned address

	os_printf("write start %x %x %x\n", rest, temp, tmpdata);
 

    // c_memcpy( tmpdata, ( const void* )temp, blksize );
//    platform_s_flash_read( tmpdata, temp, blksize );
	 res = spi_flash_read(temp, (uint32*)tmpdata, blksize);
		if(res != SPI_FLASH_RESULT_OK) {
			os_printf("spi_flash_read res %d ", res);
			return SPIFFS_ERR_INTERNAL;
		}

	os_printf("write start2 %x %x %x\n", rest, temp, tmpdata);


    for( i = rest; size && ( i < blksize ); i ++, size --, pfrom ++ ) {
	os_printf("write start copy 1 byte %x %x %x %x %x %x\n", i, rest, temp, tmpdata, pfrom, size);
      tmpdata[ i ] = *pfrom;
	}

//    platform_s_flash_write( tmpdata, temp, blksize );
	 res = spi_flash_write(temp, (uint32*)tmpdata, blksize);
		if(res != SPI_FLASH_RESULT_OK) {
			os_printf("spi_flash_write res %d ", res);
			return SPIFFS_ERR_INTERNAL;
		}

	os_printf("write start3 %x %x %x %x %x\n", rest, temp, tmpdata, pfrom, size);

    if( size == 0 )
    return SPIFFS_OK;
//      return ssize;

    toaddr = temp + blksize;
  }
  // The start address is now a multiple of blksize
  // Compute how many bytes we can write as multiples of blksize
  rest = size & blkmask;
  temp = size & ~blkmask;
  // Program the blocks now
  if( temp )
  {
	os_printf("write main %x %x %x %x %x\n", toaddr, pfrom, temp, rest, ((uint32_t)pfrom & blkmask));
//    platform_s_flash_write( pfrom, toaddr, temp );


	if(((uint32_t)pfrom & blkmask) == 0) {

	 res = spi_flash_write(toaddr, (uint32*)pfrom, temp);
		if(res != SPI_FLASH_RESULT_OK) {
			os_printf("spi_flash_write res %d ", res);
			return SPIFFS_ERR_INTERNAL;
		}

    toaddr += temp;
    pfrom += temp;

	} else {

		// fucking unaligned zooi... waarom
		// Fatal exception 9(LoadStoreAlignmentCause): deze wil ik voorkomen

		for(size = temp; size >= blksize; size -= blksize, toaddr += blksize, pfrom += blksize) {

			os_printf("write main %x %x %x\n", toaddr, pfrom, size);

			os_memcpy(tmpdata, pfrom, blksize);
			os_printf("%x %x %x %x\n", tmpdata[0], tmpdata[1], tmpdata[2], tmpdata[3]);

			 res = spi_flash_write(toaddr, (uint32*)tmpdata, blksize);
				if(res != SPI_FLASH_RESULT_OK) {
					os_printf("spi_flash_write res %d ", res);
					return SPIFFS_ERR_INTERNAL;
				}

		}


	}


  }
  // And the final part of a block if needed
  if( rest )
  {

	os_printf("write rest %x\n", rest);
    // c_memcpy( tmpdata, ( const void* )toaddr, blksize );
//    platform_s_flash_read( tmpdata, toaddr, blksize );
	 res = spi_flash_read(toaddr, (uint32*)tmpdata, blksize);
		if(res != SPI_FLASH_RESULT_OK) {
			os_printf("spi_flash_read res %d ", res);
			return SPIFFS_ERR_INTERNAL;
		}

    for( i = 0; i < rest; i ++, pfrom ++ )
      tmpdata[ i ] = *pfrom;

	os_printf("write rest3 %x %x %x\n", toaddr, tmpdata, blksize);
//    platform_s_flash_write( tmpdata, toaddr, blksize );
	 res = spi_flash_write(toaddr, (uint32*)tmpdata, blksize);
		if(res != SPI_FLASH_RESULT_OK) {
			os_printf("spi_flash_write res %d ", res);
			return SPIFFS_ERR_INTERNAL;
		}


  }
//  return ssize;


	os_printf("write done\n");

    return SPIFFS_OK;
  }

  static s32_t my_spiffs_erase(u32_t addr, u32_t size) {

os_printf("my_spiffs_erase %x %x\n", addr, size);

system_soft_wdt_feed();


/*
SpiFlashOpResult	spi_flash_erase_sector	(uint16	sec)
Parameter uint16	sec: Sector number, the count starts at sector 0, 4 KB per sector.
Return
typedef	enum{ 
				SPI_FLASH_RESULT_OK, 
				SPI_FLASH_RESULT_ERR, 
				SPI_FLASH_RESULT_TIMEOUT 
}	SpiFlashOpResult;
*/

	uint16 sector_number = addr / _SPIFFS_erase;

	if(_SPIFFS_erase * sector_number != addr) {
		os_printf("%d * %d = %d but expected %d", _SPIFFS_erase, sector_number, _SPIFFS_erase * sector_number, addr);
		return SPIFFS_ERR_INTERNAL;
	}
	
	while(size > 0) {
		if(size < _SPIFFS_erase) {
			os_printf("%d < %d ", size, _SPIFFS_erase);
			return SPIFFS_ERR_INTERNAL;
		}
		size -= _SPIFFS_erase;
os_printf("spi_flash_erase_sector %x\n", sector_number);
		SpiFlashOpResult res = spi_flash_erase_sector(sector_number);
		if(res != SPI_FLASH_RESULT_OK) {
			os_printf("spi_flash_erase_sector res %d ", res);
			return SPIFFS_ERR_INTERNAL;
		}
	}

	os_printf("erase ookkk1? res \n");
    return SPIFFS_OK;
  }



  static void test_spiffs() {
    char buf[12];

os_printf("spiffs_fs.mounted %d\n", spiffs_fs.mounted);
os_printf("spiffs_fs.block_count %d\n", spiffs_fs.block_count);
os_printf("spiffs_fs.free_blocks %d\n", spiffs_fs.free_blocks);

// os_printf("sizeof(spiffs_fd) %d\n", sizeof(spiffs_fd)); // 48

    int r;
	spiffs_file fd;
/*    
	// 
    spiffs_file fd = SPIFFS_open(&spiffs_fs, "/my_file3", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
os_printf("fd1 %d\n", fd);
	if(fd > 0) {

    if (SPIFFS_write(&spiffs_fs, fd, (u8_t *)"Hello world", 12) < 0) os_printf("errno %d\n", SPIFFS_errno(&spiffs_fs));
    r = SPIFFS_close(&spiffs_fs, fd); 
os_printf("r %d\n", r);
	}
  
    fd = SPIFFS_open(&spiffs_fs, "/my_file3", SPIFFS_RDWR, 0);
os_printf("fd2 %d\n", fd);
	if(fd > 0) {
    if (SPIFFS_read(&spiffs_fs, fd, (u8_t *)buf, 12) < 0) os_printf("errno %d\n", SPIFFS_errno(&spiffs_fs));
    r = SPIFFS_close(&spiffs_fs, fd);
os_printf("r %d\n", r);
    os_printf("--> %s <--\n", buf);
	}
  
  */

    fd = SPIFFS_open(&spiffs_fs, "/config.lua", SPIFFS_RDWR, 0);
os_printf("fd3 %d\n", fd);
	if(fd > 0) {
    if (SPIFFS_read(&spiffs_fs, fd, (u8_t *)buf, 12) < 0) os_printf("errno %d\n", SPIFFS_errno(&spiffs_fs));
    r = SPIFFS_close(&spiffs_fs, fd);
os_printf("r %d\n", r);
  
    os_printf("--> %s <--\n", buf);
	}
  }




void my_spiffs_mount() {

	os_printf("spi_flash_get_id: %d\n", spi_flash_get_id());

    extern char flashchip;
    // For SDK 1.5.2, either address has shifted and not mirrored in
    // eagle.rom.addr.v6.ld, or extra initial member was added.
    SpiFlashChip *flash = (SpiFlashChip*)(&flashchip + 4);
    os_printf("deviceId: %x\n", flash->deviceId);
    os_printf("chip_size: %u\n", flash->chip_size);
    os_printf("block_size: %u\n", flash->block_size);
    os_printf("sector_size: %u\n", flash->sector_size);
    os_printf("page_size: %u\n", flash->page_size);
    os_printf("status_mask: %u\n", flash->status_mask);





/*
u8_t dstbuf[4];

int r = my_spiffs_read(0x100b04, 4, dstbuf);

os_printf("%x %x %x %x\n", dstbuf[0], dstbuf[1], dstbuf[2], dstbuf[3]);

r = my_spiffs_read(0x100b08, 4, dstbuf);
os_printf("%x %x %x %x\n", dstbuf[0], dstbuf[1], dstbuf[2], dstbuf[3]);

r = my_spiffs_read(0x100b0c, 4, dstbuf);
os_printf("%x %x %x %x\n", dstbuf[0], dstbuf[1], dstbuf[2], dstbuf[3]);

r = my_spiffs_read(0x100b10, 4, dstbuf);
os_printf("%x %x %x %x\n", dstbuf[0], dstbuf[1], dstbuf[2], dstbuf[3]);





return;
*/

/*
============   spi init master   ============= 
mount bla: %i
spi_flash_get_id: 1458400
deviceId: 1640ef
chip_size: 524288
block_size: 65536
sector_size: 4096
page_size: 256
status_mask: 65535
my_spiffs_read 1000fe 2 3ffffb30
addr(1048830) & 0x3 = 2 mount res: %i

*/

/*
 flash->chip_size = (4 * 1024 * 1024);

    os_printf("\t\t sizeof u16_t %d \n\r", sizeof(u16_t)); // 2

	uint32 tmpdata[4];
	 SpiFlashOpResult spires = spi_flash_read(0x0, tmpdata, 4);
    os_printf("\t\t spi_flash_read 0x0 %d %x %x %x %x \n\r", spires, tmpdata[0], tmpdata[1], tmpdata[2], tmpdata[3]);
	  spires = spi_flash_read(0x1000, tmpdata, 4);
    os_printf("\t\t spi_flash_read 0x1000 %d %x %x %x %x \n\r", spires, tmpdata[0], tmpdata[1], tmpdata[2], tmpdata[3]);
	  spires = spi_flash_read(0x10000, tmpdata, 4);
    os_printf("\t\t spi_flash_read 0x10000 %d %x %x %x %x \n\r", spires, tmpdata[0], tmpdata[1], tmpdata[2], tmpdata[3]);
	  spires = spi_flash_read(0x70000, tmpdata, 4);
    os_printf("\t\t spi_flash_read 0x70000 %d %x %x %x %x \n\r", spires, tmpdata[0], tmpdata[1], tmpdata[2], tmpdata[3]);
	  spires = spi_flash_read(0x100000, tmpdata, 4);
    os_printf("\t\t spi_flash_read 0x100000 %d %x %x %x %x \n\r", spires, tmpdata[0], tmpdata[1], tmpdata[2], tmpdata[3]);

	  spires = spi_flash_read(0x1000f0, tmpdata, 16);
    os_printf("\t\t spi_flash_read 0x1000f0 %d %x %x %x %x \n\r", spires, tmpdata[0], tmpdata[1], tmpdata[2], tmpdata[3]);


	  spires = my_spiffs_read(0x100000, 16, (uint8_t*)tmpdata);
    os_printf("\t\t my_spiffs_read 0x100000 %d %x %x %x %x \n\r", spires, tmpdata[0], tmpdata[1], tmpdata[2], tmpdata[3]);

	  spires = my_spiffs_read(0x1000f0, 16, (uint8_t*)tmpdata);
    os_printf("\t\t my_spiffs_read 0x1000f0 %d %x %x %x %x \n\r", spires, tmpdata[0], tmpdata[1], tmpdata[2], tmpdata[3]);

	  spires = my_spiffs_read(0x101100, 16, (uint8_t*)tmpdata);
    os_printf("\t\t my_spiffs_read 0x101100 %d %x %x %x %x \n\r", spires, tmpdata[0], tmpdata[1], tmpdata[2], tmpdata[3]);

	  spires = my_spiffs_read(0x101110, 16, (uint8_t*)tmpdata);
    os_printf("\t\t my_spiffs_read 0x101110 %d %x %x %x %x \n\r", spires, tmpdata[0], tmpdata[1], tmpdata[2], tmpdata[3]);
*/

// https://github.com/pellepl/spiffs/wiki/Integrate-spiffs
	spiffs_config cfg;
//	cfg.phys_size = 0x405FB000 - 0x40300000; // use all spi flash
	cfg.phys_size = 0x100000; // 1m?
	cfg.phys_addr = 0x40300000 - 0x40200000; // 1m?
	cfg.phys_erase_block = _SPIFFS_erase;
	cfg.log_block_size = _SPIFFS_erase; // let us not complicate things
	cfg.log_page_size = 0x100; // as we said

    cfg.hal_read_f = my_spiffs_read;
    cfg.hal_write_f = my_spiffs_write;
    cfg.hal_erase_f = my_spiffs_erase;

	int res = SPIFFS_mount(&spiffs_fs,
		&cfg,
		spiffs_work_buf,
		spiffs_fds,
		sizeof(spiffs_fds),
		spiffs_cache_buf,
		sizeof(spiffs_cache_buf),
		0);
	os_printf("mount res: %d\n", res);

/*

	SPIFFS_unmount(&spiffs_fs);
	os_printf("unmount res\n");


system_soft_wdt_stop();

	s32_t weetniet = SPIFFS_format(&spiffs_fs);
	os_printf("format res %d\n", weetniet);

system_soft_wdt_restart();

	res = SPIFFS_mount(&spiffs_fs,
		&cfg,
		spiffs_work_buf,
		spiffs_fds,
		sizeof(spiffs_fds),
		spiffs_cache_buf,
		sizeof(spiffs_cache_buf),
		0);
	os_printf("mount res: %d\n", res);
*/

test_spiffs();

/*

r = my_spiffs_read(0x100500, 4, dstbuf);

os_printf("%x %x %x %x\n", dstbuf[0], dstbuf[1], dstbuf[2], dstbuf[3]);

r = my_spiffs_read(0x100504, 4, dstbuf);
os_printf("%x %x %x %x\n", dstbuf[0], dstbuf[1], dstbuf[2], dstbuf[3]);

r = my_spiffs_read(0x100508, 4, dstbuf);
os_printf("%x %x %x %x\n", dstbuf[0], dstbuf[1], dstbuf[2], dstbuf[3]);
*/



}




