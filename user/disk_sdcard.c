#include "ets_sys.h"
#include <osapi.h>
#include <gpio.h>
//#include <driver/spi_interface.h>

#include "disk_sdcard.h"
#include "io_spi.h"


#define CMD0 0
#define CMD8 8
#define CMD9 9
#define CMD17 17
#define CMD24 24

#define _CMD_TIMEOUT 100000

#define R1_IDLE_STATE 1 << 0
//R1_ERASE_RESET = const(1 << 1)
#define R1_ILLEGAL_COMMAND 1 << 2
//R1_COM_CRC_ERROR = const(1 << 3)
//R1_ERASE_SEQUENCE_ERROR = const(1 << 4)
//R1_ADDRESS_ERROR = const(1 << 5)
//R1_PARAMETER_ERROR = const(1 << 6)
#define _TOKEN_CMD25 0xfc
#define _TOKEN_STOP_TRAN 0xfd
#define _TOKEN_DATA 0xfe


#define TIMEOUT_READS _CMD_TIMEOUT

sdcard card;



// http://www.pratikpanda.com/esp8266-sd-card-interfacing-part-1/
// https://hackaday.io/project/12599-esp8266-web-serverdata-logger
// https://github.com/micropython/micropython/blob/master/drivers/sdcard/sdcard.py
// https://github.com/nodemcu/nodemcu-firmware/blob/master/app/platform/sdcard.c

// https://github.com/espressif/esp-idf/tree/master/examples/storage/sd_card
// https://github.com/espressif/esp-idf/tree/master/components/sdmmc

/*
In general, the HSPI interface of the ESP8266 can send data in a continuous block made of the following data blocks (in sequence):

    Command (max 2 bytes)
    Address (max 4 bytes)
    Dummy cycles
    Data (max 64 bytes)

*/



/*

 You will now have a working configuration for the SPI interface, you may modify the following registers before initiating any data transfer:

    SPI_USER: Enable/disable phases of SPI transfer (command, data, addr, dummy, etc), select normal/DIO/QIO modes, etc.
    SPI_USER1: Bit length of address, data out, data in, dummy clocks.
    SPI_USER2: Bit length and content of command phase
    SPI_W0 – SPI_W15: Data buffer from where data bytes are sent out/received into.
    SPI_CMD: Initiate a new transfer or check status of SPI hardware.


*/


static uint8_t crc7(const uint8_t* data, uint8_t n) {
// https://github.com/greiman/SdFat/blob/master/src/SdCard/SdSpiCard.cpp
  uint8_t crc = 0;
  uint8_t i;
  for (i = 0; i < n; i++) {
    uint8_t d = data[i];
  uint8_t j;
    for (j = 0; j < 8; j++) {
      crc <<= 1;
      if ((d & 0x80) ^ (crc & 0x80)) {
        crc ^= 0x09;
      }
      d <<= 1;
    }
  }
  return (crc << 1) | 1;
}


/*
// https://github.com/greiman/SdFat/blob/master/src/SdCard/SdSpiCard.cpp


// Shift based CRC-CCITT
// uses the x^16,x^12,x^5,x^1 polynomial.
static uint16_t CRC_CCITT(const uint8_t *data, size_t n) {
  uint16_t crc = 0;
  for (size_t i = 0; i < n; i++) {
    crc = (uint8_t)(crc >> 8) | (crc << 8);
    crc ^= data[i];
    crc ^= (uint8_t)(crc & 0xff) >> 4;
    crc ^= crc << 12;
    crc ^= (crc & 0xff) << 5;
  }
  return crc;
}

*/

static int cs_active = 0;

LOCAL void sdcard_cs_low() {
	if(cs_active == 1) {
		return;
	}
	cs_active = 1;

//	os_printf("activateeee\n");
    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);
}

LOCAL void sdcard_cs_high() {
	if(cs_active == 0) {
		return;
	}
	cs_active = 0;

//	os_printf("release\n");
    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);
    io_spi_writebyte(0xff);
//    io_spi_writebyte(0xff);
}


LOCAL int sdcard_wait_busy() {
	
	int i = 0;
	uint8_t r;
//	  r = io_spi_readbyte();
//		r = io_spi_readwritebyte(0xff);

//	return 0;

        do {
//	  r = io_spi_readbyte();
		r = io_spi_readwritebyte(0xff);
		  os_delay_us(100);
        } while(r != 0xff && i++ < TIMEOUT_READS);

	if(r != 0xff) {
		os_printf("sdcard_wait_busy timeout: %x\n", r);
		return -1;
	}

	return 0;
}


// final = 0
// release = True
LOCAL int sdcard_cmd(uint8_t cmd, uint32_t arg, uint8_t crc, uint8_t final, uint8_t release) {

//	os_printf("sdcard_cmd %d %x %x %x %x\n", cmd, arg, crc, final, release);

        sdcard_cs_low();
	
//	if(cmd != CMD0) {
//	if(cmd == CMD17) {
		// wtf? misschien door snelheid?
		// dit moet eigenlijk altijd maar dan werkt init niet?
		int r = sdcard_wait_busy();
		if(r != 0) {
			os_printf("SD card busy before command %d\n", r);
			return -1;
		}
//	}

//        # create and send the command
	uint8_t buf[6];

//        buf = self.cmdbuf
        buf[0] = 0x40 | cmd;
        buf[1] = arg >> 24;
        buf[2] = arg >> 16;
        buf[3] = arg >> 8;
        buf[4] = arg;

	if(crc == 0) {
		crc = crc7(buf, 5);
	}

        buf[5] = crc;
        io_spi_write(buf, 6);

//        # wait for the response (response[7] == 0)
	int i;
	for (i = 0; i < _CMD_TIMEOUT; i++) {
//		int res = io_spi_readbyte();
		int res = io_spi_readwritebyte(0xff);

//	if(res != 0xff) {
//		os_printf("spi byte %x\n", res);
//		os_printf("res & 0x80 %x\n", res & 0x80);
//	}

            if (0 == (res & 0x80)) {
//		os_printf("yay cmd done\n");
                //# this could be a big-endian integer that we are getting here
		int j;
                for (j = 0; j < final; j++) {
                    uint8_t b = io_spi_readwritebyte(0xff);
			os_printf("final %d = %d\n", j, b);
		}
                if (release) {
//			os_printf("release after command\n");
        		sdcard_cs_high();
		}
                return res;
	    }
		os_delay_us(100);
	}

//        # timeout
        sdcard_cs_high();
        return -1;
}

LOCAL int sdcard_read(uint8_t *buf, uint32_t len) {
//        sdcard_cs_low(); was die dat nog niet dan ? wtf?

        //# read until start byte (0xfe)
	int r;
        do {
//	  r = io_spi_readbyte();
	  r = io_spi_readwritebyte(0xff);
        } while(r == 0xff);

	if(r != 0xfe) {
		os_printf("not datastart block %x\n", r);
		return -1;
	}

//	os_printf("startblock\n");


	int res = io_spi_read(buf, len);
	if(res != 0) {
		os_printf("io_spi_read %x %d %d\n", buf, len, res);
		return -1;
	}

    uint8_t crc_a = io_spi_readwritebyte(0xff);
    uint8_t crc_b = io_spi_readwritebyte(0xff);
//	os_printf("crc %x %x\n", crc_a, crc_b);

	// checksum checken...
/*
#if USE_SD_CRC
  // get crc
  crc = (spiReceive() << 8) | spiReceive();
  if (crc != CRC_CCITT(dst, count)) {
    error(SD_CARD_ERROR_READ_CRC);
    goto fail;
  }
#else
  // discard crc
  spiReceive();
  spiReceive();
#endif  // USE_SD_CRC
*/


//	sdcard_cs_high();
		return 0;
}


LOCAL int sdcard_init_card_v1(sdcard* card)
{
	int i, res;
	for (i = 0; i < _CMD_TIMEOUT; i++) {
            sdcard_cmd(55, 0, 0, 0, 1);
            res = sdcard_cmd(41, 0, 0, 0, 1);
		if ( res == 0) {
                card->cdv = 512;
                os_printf("[SDCard] v1 card %d\n");
                return 0;
		}
	}

	// met cmd1 proberen, als dat lukt is het mmcv3 OUDDD :P

        os_printf("timeout waiting for v1 card\n");
	return -1;

}

LOCAL int sdcard_init_card_v2(sdcard* card)
{

	int i, res;
	do {

//	for (i = 0; i < _CMD_TIMEOUT; i++) {
            os_delay_us(50000);

//            sdcard_cmd(58, 0, 0, 4, 1); // read ocr? waarom? waarom niet na ACMD41 ?

		// CMD55 + CMD41 = ACMD41
            sdcard_cmd(55, 0, 0, 0, 1);
            res = sdcard_cmd(41, 0x40000000, 0, 0, 1);
//		if (res == 0) {
//                res = sdcard_cmd(58, 0, 0, 4, 1); waarom nog een keer?
//                card->cdv = 1;
//                os_printf("[SDCard] v2 card\n");
//                return 0;
//		}
	} while(i < _CMD_TIMEOUT && res != 0);

	if(res != 0) {
		os_printf("timeout waiting for v2 card\n");
		return -1;
	}

                os_printf("[SDCard] v2 card\n");


            res = sdcard_cmd(58, 0, 0, 0, 0);
	    uint8_t ocr[4];
	    ocr[0] = io_spi_readwritebyte(0xff);
	    ocr[1] = io_spi_readwritebyte(0xff);
	    ocr[2] = io_spi_readwritebyte(0xff);
	    ocr[3] = io_spi_readwritebyte(0xff);
      os_printf("ocr[0] %x\n", ocr[0]);

    if ((ocr[0] & 0xC0) == 0xC0) {
      os_printf("SD_CARD_TYPE_SDHC\n");
	card->type = TYPE_SDHC;
                card->cdv = 512;
    } else {
	card->type = TYPE_SD2;
                card->cdv = 1;
	}

sdcard_cs_high();


                return 0;

}

int sdcard_init_card(sdcard* card)
{
        sdcard_cs_high();
	io_spi_init(250000); // 250kHz was 400kHz denk?

//	return 0;

	int i;
        // clock card at least 100 cycles with cs high (arduino ding doet i < 10 ?)
	for(i = 0; i < 16; i++)
	{
		io_spi_writebyte(0xff);
	}

        // CMD0: init card; should return R1_IDLE_STATE (allow 5 attempts)
	int res;
	i = 100;
	do
	{
		  os_delay_us(1000);
		// TODO: wat is 0x95? de crc...waarom :P
		res = sdcard_cmd(CMD0, 0, 0x95, 0, 1);
		os_printf("cmd0: %x   ... want %x\n", res, R1_IDLE_STATE);
	} while(res != R1_IDLE_STATE && --i > 0);

	if(res != R1_IDLE_STATE || i <= 0)
	{
		os_printf("SD card not idle?! %d\n", res);
		return -1;
	}

#if USE_SD_CRC
  if (cardCommand(CMD59, 1) != R1_IDLE_STATE) {
    error(SD_CARD_ERROR_CMD59);
    goto fail;
  }
#endif


	// waarom doen we cmd1 niet? volgens http://elm-chan.org/docs/mmc/mmc_e.html is dat nodig?
	// oh... Because ACMD41 instead of CMD1 is recommended for SDC, trying ACMD41 first and retry with CMD1 if rejected, is ideal to support both type of the cards. 
	// lol :P

	// # CMD8: determine card version
	res = sdcard_cmd(CMD8, 0x01aa, 0x87, 4, 1);
	// Arduino ding doet hier 4 bytes lezen... als de laatste 0xaa is dan v2
        if(res == R1_IDLE_STATE)
	{
		card->type = TYPE_SD2;
            sdcard_init_card_v2(card);
	}
	else if (res == (R1_IDLE_STATE | R1_ILLEGAL_COMMAND))
	{
		card->type = TYPE_SD1;
            sdcard_init_card_v1(card);
	}
        else
	{
            os_printf("couldn't determine SD card version, %d\n", res);
		return -1;
	}

	os_printf("type %d\n", card->type);
	os_printf("cdv %d\n", card->cdv);

/*
//        # get the number of sectors
//        # CMD9: response R2 (R1 byte + 16-byte block read)
//        if self.cmd(9, 0, 0, 0, False) != 0:
	res = sdcard_cmd(CMD9, 0, 0, 0, 0);
	if(res != 0) {
            os_printf("no response from SD card %d\n", res);
		return -1;
	}

        uint8_t csd[16];
        sdcard_read(csd, 16);

        if (csd[0] & 0xc0 != 0x40) {
            os_printf("SD card CSD format not supported %x\n", csd[0]);
		return -1;
	}

	// csd = 0x5e00325b5aa3afffffff80a08000893b

	int sectors = ((csd[8] << 8 | csd[9]) + 1) * 2014;
        os_printf("sectors %d\n", sectors);// sectors 90742784 wtf?

	uint32_t sectors2;

	uint16_t cs;
                if ((csd[0] >> 6) == 1) {    // SDC ver 2.00
                    cs= csd[9] + ((uint16_t)csd[8] << 8) + 1;
                    sectors2 = (uint32_t)cs << 10;
                } else {                    // SDC ver 1.XX or MMC
                    int n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                    cs = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
                    sectors2 = (uint32_t)cs << (n - 9);
                }

        os_printf("sectors2 %d\n", sectors2);// sectors

*/


/*
i byte f
res & 0x80 0
yay
can't set 512 block size: 15
before w0 ff000000

 ets Jan  8 2013,rst cause:4, boot mode:(3,6)

wdt reset
load 0x40100000,
*/



        //# CMD16: set block length to 512 bytes
        res = sdcard_cmd(16, 512, 0, 0, 1);
	if (res != 0) {
            os_printf("can't set 512 block size: %d\n", res);

		// denk het wel: The initial read/write block length can be set 1024 on 2GB cards, so that the block size should be re-initialized to 512 with CMD16 to work with FAT file system.
		return -1; // TODO: is dit nodig dan?
	}

        io_spi_init(10000000); // 10 MHz ?

	os_printf("sdcard_init done\n");

	return 0;
}

LOCAL
int sdcard_write(uint8_t token, const uint8_t* buf, uint32_t len) {

//	os_printf("sdcard_write %x %x %d\n", token, buf, len);


/*
        //# read until start byte (0xfe)
	int r;
        do {
//	  r = io_spi_readbyte();
	  r = io_spi_readwritebyte(0xff);
        } while(r == 0xff);
*/

        //# send: start of block, data, checksum
        io_spi_writebyte(token);
        int res = io_spi_write(buf, len);
	if(res != 0) {
		os_printf("io_spi_write %x %d %d\n", buf, len, res);
		return -1;
	}
	// TODO: crc?
	io_spi_writebyte(0xff);
	io_spi_writebyte(0xff);

        //# check the response
	res = io_spi_readwritebyte(0xff);
        if ((res & 0x1f) != 0x05) {
		os_printf("sdcard_write %x %x %d %x\n", token, buf, len, res);
	    sdcard_cs_high();
            return -1;
	}

	int r = sdcard_wait_busy();
	if(r != 0) {
		os_printf("SD card timeout after write %d\n", r);
		return -1;
	}

//	sdcard_cs_high();
	return 0;
}

int sdcard_readblocks(sdcard* card, uint32_t addr, uint8_t *buf, int size) {
	if(size != 512) {
		os_printf("een blok per keer lezen aub %d\n", size);
		return -1;
	}
	if(size & (512-1)) {
		os_printf("niet een meervoud van 512 %d\n", size);
		return -1;
	}
	if(size & (card->cdv - 1)) {
		os_printf("niet een meervoud van %d %d\n", card->cdv, size);
		return -1;
	}

//	os_printf("sdcard_readblocks %x %x %x\n", addr, buf, size);

//            # CMD17: set read address for single block
	// block_num * card->cdv
	int res = sdcard_cmd(CMD17, addr, 0, 0, 0);
	if(res != 0) {
		os_printf("cmd17 error %d\n", res);
	        sdcard_cs_high();
                return -1;
	}


        res = sdcard_read(buf, size);
	if(res != 0) {
		os_printf("cmd17 read error %d\n", res);
	        sdcard_cs_high();
                return -1;
	}

        sdcard_cs_high();
	return 0;
}

int sdcard_writeblocks(sdcard* card, uint32_t addr, const uint8_t *buf, int size) {
	if(size != 512) {
		os_printf("een blok per keer lezen aub %d\n", size);
		return -1;
	}
	if(size & (512-1)) {
		os_printf("niet een meervoud van 512 %d\n", size);
		return -1;
	}
	if(size & (card->cdv - 1)) {
		os_printf("niet een meervoud van %d %d\n", card->cdv, size);
		return -1;
	}

//	os_printf("sdcard_writeblocks %x %x %x\n", addr, buf, size);

	int res = sdcard_cmd(CMD24, addr, 0, 0, 0);
	if(res != 0) {
		os_printf("cmd24 error %d\n", res);
	        sdcard_cs_high();
                return -1;
	}


        res = sdcard_write(0xfe, buf, size);
	if(res != 0) {
		os_printf("cmd24 write error %d\n", res);
	        sdcard_cs_high();
                return -1;
	}

// wait ready?

        sdcard_cs_high();
	return 0;
}


/*


    def write_token(self, token):
        self.cs(0)
        self.spi.read(1, token)
        self.spi.write(b'\xff')
        # wait for write to finish
        while self.spi.read(1, 0xff)[0] == 0x00:
            pass

        self.cs(1)
        self.spi.write(b'\xff')

    def count(self):
        return self.sectors

    def readblocks(self, block_num, buf):
        nblocks, err = divmod(len(buf), 512)
        assert nblocks and not err, 'Buffer length is invalid'
        if nblocks == 1:
            # CMD17: set read address for single block
            if self.cmd(17, block_num * self.cdv, 0) != 0:
                return 1
            # receive the data
            self.readinto(buf)
        else:
            # CMD18: set read address for multiple blocks
            if self.cmd(18, block_num * self.cdv, 0) != 0:
                return 1
            offset = 0
            mv = memoryview(buf)
            while nblocks:
                self.readinto(mv[offset : offset + 512])
                offset += 512
                nblocks -= 1
            return self.cmd_nodata(b'\x0c') # cmd 12
        return 0

    def writeblocks(self, block_num, buf):
        nblocks, err = divmod(len(buf), 512)
        assert nblocks and not err, 'Buffer length is invalid'
        if nblocks == 1:
            # CMD24: set write address for single block
            if self.cmd(24, block_num * self.cdv, 0) != 0:
                return 1

            # send the data
            self.write(_TOKEN_DATA, buf)
        else:
            # CMD25: set write address for first block
            if self.cmd(25, block_num * self.cdv, 0) != 0:
                return 1
            # send the data
            offset = 0
            mv = memoryview(buf)
            while nblocks:
                self.write(_TOKEN_CMD25, mv[offset : offset + 512])
                offset += 512
                nblocks -= 1
            self.write_token(_TOKEN_STOP_TRAN)
        return 0



*/






