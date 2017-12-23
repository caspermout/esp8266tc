
#include "osapi.h"
#include "eagle_soc.h"

#include "espmissingincludes.h"
#include "driver/spi_register.h"

#include "io_spi.h"
//#include "hspi.h"


// https://github.com/espressif/ESP8266_RTOS_SDK/blob/master/include/espressif/esp8266/pin_mux_register.h
#define SPI1_CLK_EQU_SYS_CLK BIT9
#define FUNC_HSPIQ_MISO 2
#define FUNC_HSPID_MOSI 2
#define FUNC_HSPI_CLK 2
#define FUNC_HSPI_CS0 2
/*


TODO: klooien met interrupts

#define ETS_SPI_INUM	    2

#define ETS_SPI_INTR_ATTACH(func, arg) \
    ets_isr_attach(ETS_SPI_INUM, (func), (void *)(arg))

#define ETS_SPI_INTR_ENABLE() \
    ETS_INTR_ENABLE(ETS_SPI_INUM)

#define ETS_SPI_INTR_DISABLE() \
    ETS_INTR_DISABLE(ETS_SPI_INUM)

#define ETS_INTR_ENABLE(inum) \
    ets_isr_unmask((1<<inum))

#define ETS_INTR_DISABLE(inum) \
    ets_isr_mask((1<<inum))


ESP8266_SDK.old/examples/driver_lib/include/driver/spi_register.h:#define SPI_SLV_WR_STA_DONE_EN (BIT(8))
ESP8266_SDK.old/examples/driver_lib/include/driver/spi_register.h:#define SPI_SLV_RD_STA_DONE_EN (BIT(7))
ESP8266_SDK.old/examples/driver_lib/include/driver/spi_register.h:#define SPI_SLV_WR_BUF_DONE_EN (BIT(6))
ESP8266_SDK.old/examples/driver_lib/include/driver/spi_register.h:#define SPI_SLV_RD_BUF_DONE_EN (BIT(5))
ESP8266_SDK.old/examples/driver_lib/include/driver/spi_register.h:#define SLV_SPI_INT_EN   0x0000001f
ESP8266_SDK.old/examples/driver_lib/include/driver/spi_register.h:#define SLV_SPI_INT_EN_S 5






void ICACHE_FLASH_ATTR SPIIntClear(SpiNum spiNum)
{
    if (spiNum > SpiNum_HSPI) {
        return;
    }
    CLEAR_PERI_REG_MASK(SPI_SLAVE(spiNum), SpiIntSrc_TransDone
                        | SpiIntSrc_WrStaDone
                        | SpiIntSrc_RdStaDone
                        | SpiIntSrc_WrBufDone
                        | SpiIntSrc_RdBufDone);
}







*/


// https://github.com/micropython/micropython/blob/ad166857bc93e519bca8c4f14523dcce654a5994/esp8266/machine_hspi.c


// https://github.com/espruino/Espruino/search?utf8=%E2%9C%93&q=spi_doutdin&type=
#define SPI_DOUTDIN (BIT(0)) //From previous SDK

#define SPI_CLK_USE_DIV 0
#define SPI_CLK_80MHZ_NODIV 1

#define SPI_BYTE_ORDER_HIGH_TO_LOW 1
#define SPI_BYTE_ORDER_LOW_TO_HIGH 0

//#ifndef CPU_CLK_FREQ //Should already be defined in eagle_soc.h
//#define CPU_CLK_FREQ (80 * 1000000)
//#endif

// Define some default SPI clock settings
//#define SPI_CLK_PREDIV 10
//#define SPI_CLK_CNTDIV 2
//#define SPI_CLK_FREQ (CPU_CLK_FREQ / (SPI_CLK_PREDIV * SPI_CLK_CNTDIV))
// 80 / 20 = 4 MHz

#define spi_busy(spi_no) READ_PERI_REG(SPI_CMD(spi_no))&SPI_USR


void io_spi_isr(void *para)
{

// 0x3ff00020 = 2080
// io_spi_isr 8003f0


	uint32 regvalue,calvalue;
    	static uint8 state =0;
	uint32 recv_data,send_data;

	// 0x3ff00020 is ISR register?!
	// http://wiki.jackslab.org/ESP8266_Memory_Map
	//  Interrupt Status Register for SPI and I2S peripherals 

	if(READ_PERI_REG(0x3ff00020) != 0x2080 && READ_PERI_REG(0x3ff00020) != 0x80 && READ_PERI_REG(0x3ff00020) != 0x2081 && READ_PERI_REG(0x3ff00020) != 0x81) {
	os_printf("0x3ff00020 = %x\n", READ_PERI_REG(0x3ff00020));
	}

	if(READ_PERI_REG(0x3ff00020)&BIT4){		
        //following 3 lines is to clear isr signal
        	CLEAR_PERI_REG_MASK(SPI_SLAVE(SPI), 0x3ff);
    	}else if(READ_PERI_REG(0x3ff00020)&BIT7){ //bit7 is for hspi isr,
        	regvalue=READ_PERI_REG(SPI_SLAVE(HSPI));
         	CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI),  
								SPI_TRANS_DONE_EN|
								SPI_SLV_WR_STA_DONE_EN|
								SPI_SLV_RD_STA_DONE_EN|
								SPI_SLV_WR_BUF_DONE_EN|
								SPI_SLV_RD_BUF_DONE_EN);
        	SET_PERI_REG_MASK(SPI_SLAVE(HSPI), SPI_SYNC_RESET);
        	CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI),  
								SPI_TRANS_DONE|
								SPI_SLV_WR_STA_DONE|
								SPI_SLV_RD_STA_DONE|
								SPI_SLV_WR_BUF_DONE|
								SPI_SLV_RD_BUF_DONE); 
		SET_PERI_REG_MASK(SPI_SLAVE(HSPI),  
								SPI_TRANS_DONE_EN|
								SPI_SLV_WR_STA_DONE_EN|
								SPI_SLV_RD_STA_DONE_EN|
								SPI_SLV_WR_BUF_DONE_EN|
								SPI_SLV_RD_BUF_DONE_EN);

if(regvalue != 0x8003f0) {
os_printf("io_spi_isr %x\n", regvalue);
}

		if(regvalue&SPI_SLV_WR_BUF_DONE){ 

/*
            		GPIO_OUTPUT_SET(0, 0);
            		idx=0;
            		while(idx<8){
            			recv_data=READ_PERI_REG(SPI_W0(HSPI)+(idx<<2));
            			spi_data[idx<<2] = recv_data&0xff;
            			spi_data[(idx<<2)+1] = (recv_data>>8)&0xff;
            			spi_data[(idx<<2)+2] = (recv_data>>16)&0xff;
            			spi_data[(idx<<2)+3] = (recv_data>>24)&0xff;
            			idx++;
			}
			//add system_os_post here
            		GPIO_OUTPUT_SET(0, 1);
*/
		}
        	if(regvalue&SPI_SLV_RD_BUF_DONE){
/*
			//it is necessary to call GPIO_OUTPUT_SET(2, 1), when new data is preped in SPI_W8-15 and needs to be sended.
           		GPIO_OUTPUT_SET(2, 0);
			//add system_os_post here
			//system_os_post(USER_TASK_PRIO_1,WR_RD,regvalue);

*/
        	}
    
    }else if(READ_PERI_REG(0x3ff00020)&BIT9){ //bit7 is for i2s isr,

    }
}



void io_spi_init(uint32_t speed) {

	uint8_t spi_no = HSPI;

            os_printf("before w0 %x\n", READ_PERI_REG(SPI_W0(spi_no)));


    uint32_t clock_div_flag = 0;
//    if (sysclk_as_spiclk) {
//        clock_div_flag = 0x0001;
//    }

// todo: waarom niet nodig?
//        CLEAR_PERI_REG_MASK(SPI_PIN(spiNum), SPI_IDLE_EDGE);
//        CLEAR_PERI_REG_MASK(SPI_USER(spiNum),  SPI_CK_OUT_EDGE);


//            os_printf("PERIPHS_IO_MUX %x\n", READ_PERI_REG(PERIPHS_IO_MUX));
	CLEAR_PERI_REG_MASK(PERIPHS_IO_MUX, SPI1_CLK_EQU_SYS_CLK); // BIT9
//            os_printf("PERIPHS_IO_MUX %x\n", READ_PERI_REG(PERIPHS_IO_MUX));


    //clear bit9,bit8 of reg PERIPHS_IO_MUX
    //bit9 should be cleared when HSPI clock doesn't equal CPU clock
    //bit8 should be cleared when SPI clock doesn't equal CPU clock
    ////WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105); //clear bit9//TEST


        // Set bit 9 if 80MHz sysclock required
// todo: dit sloopt alles
//        WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105 | (clock_div_flag<<9));


	// https://github.com/espressif/ESP8266_RTOS_SDK/blob/master/include/espressif/esp8266/pin_mux_register.h
        // GPIO12 is HSPI MISO pin (Master Data In)
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_HSPIQ_MISO); // 2
        // GPIO13 is HSPI MOSI pin (Master Data Out)
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_HSPID_MOSI); // 2
        // GPIO14 is HSPI CLK pin (Clock)
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_HSPI_CLK); // 2
        // GPIO15 is HSPI CS pin (Chip Select / Slave Select)
//        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_HSPI_CS0); // 2

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);

	// pullup doen op MOSI ? zou dat werken? ipv de 0xff schrijven? met DOUTDIN?
os_printf("peri reg mux mosi %x\n", READ_PERI_REG(PERIPHS_IO_MUX_MTCK_U));
 PIN_PULLUP_EN(PERIPHS_IO_MUX_MTCK_U);
os_printf("peri reg mux mosi %x\n", READ_PERI_REG(PERIPHS_IO_MUX_MTCK_U));


/*

interrupt niet echt nodig :P

    // Clear the interrupt source and disable all of the interrupt.
os_printf("peri reg spi_slave %x\n", READ_PERI_REG(SPI_SLAVE(spi_no)));
//peri reg spi_slave 200
//0x3ff00020 = 10
// peri reg spi_slave 3e0


    CLEAR_PERI_REG_MASK(SPI_SLAVE(spi_no), 0x3FF);
    SET_PERI_REG_MASK(SPI_SLAVE(spi_no), ((SPI_TRANS_DONE 
        |SPI_SLV_WR_STA_DONE 
        |SPI_SLV_RD_STA_DONE 
        |SPI_SLV_WR_BUF_DONE 
        |SPI_SLV_RD_BUF_DONE) << SLV_SPI_INT_EN_S));
    ETS_SPI_INTR_ATTACH(io_spi_isr, NULL);
    ETS_SPI_INTR_ENABLE();    
*/



//return;



// Define some default SPI clock settings
//#define SPI_CLK_PREDIV 10
//#define SPI_CLK_CNTDIV 2
//#define SPI_CLK_FREQ (CPU_CLK_FREQ / (SPI_CLK_PREDIV * SPI_CLK_CNTDIV))
// 80 / 20 = 4 MHz

uint16_t prediv = 10; //SPI_CLK_PREDIV;

if(speed >= 4000000) {
	prediv = 1;
}

uint8_t cntdiv = (80000000 / prediv) / speed; //SPI_CLK_CNTDIV;

// http://d.av.id.au/blog/hardware-spi-clock-registers/
os_printf("cntdiv %d\n", cntdiv);
os_printf("speed %d kHz\n", 80000 / (prediv * cntdiv));

    if (prediv == 0 || cntdiv == 0) {
        WRITE_PERI_REG(SPI_CLOCK(spi_no), SPI_CLK_EQU_SYSCLK);
    } else {
        WRITE_PERI_REG(SPI_CLOCK(spi_no),
           (((prediv - 1) & SPI_CLKDIV_PRE) << SPI_CLKDIV_PRE_S) |
           (((cntdiv - 1) & SPI_CLKCNT_N) << SPI_CLKCNT_N_S) |
           (((cntdiv >> 1) & SPI_CLKCNT_H) << SPI_CLKCNT_H_S) |
           ((0 & SPI_CLKCNT_L) << SPI_CLKCNT_L_S)
        );
    }

// todo: en deze dan?
//        CLEAR_PERI_REG_MASK(SPI_CTRL(spiNum), SPI_WR_BIT_ORDER);
//        CLEAR_PERI_REG_MASK(SPI_CTRL(spiNum), SPI_RD_BIT_ORDER);

        SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_WR_BYTE_ORDER);

        SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_RD_BYTE_ORDER);

//    SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_CS_SETUP|SPI_CS_HOLD); // gebruiken we dit wel?
    CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_FLASH_MODE); // moet dit niet eerder?

	
//os_printf("wat is dit: %x\n", READ_PERI_REG(SPI_USER(spi_no)));
//os_printf("wat is dit1: %x\n", READ_PERI_REG(SPI_USER1(spi_no)));
//os_printf("wat is dit2: %x\n", READ_PERI_REG(SPI_USER2(spi_no)));

/*
before w0 3f000000
cntdiv 32
speed 250 kHz
wat is dit: 80000c40
wat is dit1: 5c000000
wat is dit2: 70000000
write w0 ff000000

*/

}

int io_spi_writebyte(uint8_t dout_data) {

	uint8_t spi_no = HSPI;

    while (spi_busy(spi_no)) {};  // Wait for SPI to be ready

// Enable SPI Functions
    // Disable MOSI, MISO, ADDR, COMMAND, DUMMY in case previously set.
    CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MOSI | SPI_USR_MISO |
                        SPI_USR_COMMAND | SPI_USR_ADDR | SPI_USR_DUMMY);

// Setup Bitlengths
    WRITE_PERI_REG(SPI_USER1(spi_no),
        // Number of bits to Send
        ((8 - 1) & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S |
        // Number of bits to receive
        ((8 - 1) & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S);

//        SET_PERI_REG_BITS(SPI_USER1(spiNum), SPI_USR_MOSI_BITLEN, ((pInData->dataLen << 3) - 1), SPI_USR_MOSI_BITLEN_S);


// Setup DOUT data
    // Enable MOSI function in SPI module
    SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MOSI);
    // Copy data to W0
    if (READ_PERI_REG(SPI_USER(spi_no)) & SPI_WR_BYTE_ORDER) {
        WRITE_PERI_REG(SPI_W0(spi_no), dout_data << (32 - 8));
    } else {
        WRITE_PERI_REG(SPI_W0(spi_no), dout_data);
    }

//            os_printf("write w0 %x\n", READ_PERI_REG(SPI_W0(spi_no)));

// enable miso ook maar gewoon :P
//    SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MISO);


// Begin SPI Transaction
    SET_PERI_REG_MASK(SPI_CMD(spi_no), SPI_USR);

        while (spi_busy(spi_no)) {}; // Wait for SPI transaction to complete

//            os_printf("after write w0 %x\n", READ_PERI_REG(SPI_W0(spi_no)));
}

uint8_t io_spi_readbyte() {

	uint8_t spi_no = HSPI;
//	uint32_t din_bits = 8;


    while (spi_busy(spi_no)) {};  // Wait for SPI to be ready

// Enable SPI Functions
    // Disable MOSI, MISO, ADDR, COMMAND, DUMMY in case previously set.
    CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MOSI | SPI_USR_MISO |
                        SPI_USR_COMMAND | SPI_USR_ADDR | SPI_USR_DUMMY);

// Setup Bitlengths
/*
    WRITE_PERI_REG(SPI_USER1(spi_no),
//        // Number of bits in Address
//        ((addr_bits - 1) & SPI_USR_ADDR_BITLEN) << SPI_USR_ADDR_BITLEN_S |
//        // Number of bits to Send
//        ((dout_bits - 1) & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S |
//        // Number of bits to receive
        ((din_bits - 1) & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S
	// |
        // Number of Dummy bits to insert
//        ((dummy_bits - 1) & SPI_USR_DUMMY_CYCLELEN) << SPI_USR_DUMMY_CYCLELEN_S
	);
*/
    WRITE_PERI_REG(SPI_USER1(spi_no),
//        // Number of bits to Send
//        ((8 - 1) & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S |
        // Number of bits to receive
        ((8 - 1) & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S);






    SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MISO);


	//doe tegelijk maar schrijven? wtf? doet reset dus werkt niet zo... maar hoe wel? wil graag dat ding high tijdens lezen :O
	if(0) {
    SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MOSI);
    // Copy data to W0
    if (READ_PERI_REG(SPI_USER(spi_no)) & SPI_WR_BYTE_ORDER) {
        WRITE_PERI_REG(SPI_W0(spi_no), 0xff << (32 - 8));
    } else {
        WRITE_PERI_REG(SPI_W0(spi_no), 0xff);
    }
	
	}


// Begin SPI Transaction
    SET_PERI_REG_MASK(SPI_CMD(spi_no), SPI_USR);

// Return DIN data
        while (spi_busy(spi_no)) {}; // Wait for SPI transaction to complete




//            os_printf("w0 %x\n", READ_PERI_REG(SPI_W0(spi_no)));


        if (READ_PERI_REG(SPI_USER(spi_no))&SPI_RD_BYTE_ORDER) {
            // Assuming data in is written to MSB. TBC
            return READ_PERI_REG(SPI_W0(spi_no)) >> (32 - 8);
        } else {
            // Read in the same way as DOUT is sent. Note existing contents of
            // SPI_W0 remain unless overwritten!
            return READ_PERI_REG(SPI_W0(spi_no));
        }
        return 0; // Something went wrong
}


uint8_t io_spi_readwritebyte(uint8_t byte) {

	uint8_t spi_no = HSPI;
//	uint32_t din_bits = 8;


    while (spi_busy(spi_no)) {};  // Wait for SPI to be ready

// Enable SPI Functions
    // Disable MOSI, MISO, ADDR, COMMAND, DUMMY in case previously set.
    CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MOSI | SPI_USR_MISO |
                        SPI_USR_COMMAND | SPI_USR_ADDR | SPI_USR_DUMMY);

// Setup Bitlengths
/*
    WRITE_PERI_REG(SPI_USER1(spi_no),
//        // Number of bits in Address
//        ((addr_bits - 1) & SPI_USR_ADDR_BITLEN) << SPI_USR_ADDR_BITLEN_S |
//        // Number of bits to Send
//        ((dout_bits - 1) & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S |
//        // Number of bits to receive
        ((din_bits - 1) & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S
	// |
        // Number of Dummy bits to insert
//        ((dummy_bits - 1) & SPI_USR_DUMMY_CYCLELEN) << SPI_USR_DUMMY_CYCLELEN_S
	);
*/

    WRITE_PERI_REG(SPI_USER1(spi_no),
        // Number of bits to Send
        ((8 - 1) & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S |
        // Number of bits to receive
        ((8 - 1) & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S);


// Setup DOUT data
    // Enable MOSI function in SPI module
    SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MOSI);
    // Copy data to W0
    if (READ_PERI_REG(SPI_USER(spi_no)) & SPI_WR_BYTE_ORDER) {
        WRITE_PERI_REG(SPI_W0(spi_no), byte << (32 - 8));
    } else {
        WRITE_PERI_REG(SPI_W0(spi_no), byte);
    }

//        WRITE_PERI_REG(SPI_W0(spi_no), 0xffffffff);

//    SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_USR_MISO);

    SET_PERI_REG_MASK(SPI_USER(spi_no), SPI_DOUTDIN);
//os_printf("wat is dit4: %x\n", READ_PERI_REG(SPI_USER(spi_no)));
// wat is dit: 8000c41


//            os_printf("rw w0 %x\n", READ_PERI_REG(SPI_W0(spi_no)));




// Begin SPI Transaction
    SET_PERI_REG_MASK(SPI_CMD(spi_no), SPI_USR);

// Return DIN data
        while (spi_busy(spi_no)) {}; // Wait for SPI transaction to complete

    CLEAR_PERI_REG_MASK(SPI_USER(spi_no), SPI_DOUTDIN);

//            os_printf("rw r0 %x\n", READ_PERI_REG(SPI_W0(spi_no)));


        if (READ_PERI_REG(SPI_USER(spi_no))&SPI_RD_BYTE_ORDER) {
            // Assuming data in is written to MSB. TBC
            return READ_PERI_REG(SPI_W0(spi_no)) >> (32 - 8);
        } else {
            // Read in the same way as DOUT is sent. Note existing contents of
            // SPI_W0 remain unless overwritten!
            return READ_PERI_REG(SPI_W0(spi_no));
        }
        return 0; // Something went wrong
}


int io_spi_write(const uint8_t *buf, uint32_t len) {

	int i;
	for( i = 0; i < len ; i++, *buf++) {
		io_spi_writebyte(*buf);
	}
	return 0;
}

int io_spi_read(uint8_t *buf, uint32_t len) {

	int i;
	for( i = 0; i < len ; i++, *buf++) {
		*buf = io_spi_readbyte();
	}
	return 0;
}




