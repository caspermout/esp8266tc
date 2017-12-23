#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"

#include "ip_addr.h"
#include "espconn.h"
#include "sntp.h"
//#include "spi.h"

#include "driver/uart.h"
#include "driver/onewire.h"

#include "disk_io.h"
#include "vfs.h"
#include "webserver.h"
#include "user_sensors.h"


#include "user_config.h"

// https://github.com/dimonomid/umm_malloc

// TODO: ets_loop_iter ... is dat net als yield? :D

// yield van arduino
// https://github.com/esp8266/Arduino/blob/4897e0006b5b0123a2fa31f67b14a3fff65ce561/cores/esp8266/cont.S
// https://github.com/esp8266/Arduino/blob/2126146e20042878026f03a19107555f32e3431c/cores/esp8266/core_esp8266_main.cpp

// https://github.com/esp8266/Arduino/blob/master/cores/esp8266/esp8266_peri.h


#define TEST_QUEUE_LEN 8
os_event_t * testQueue;

// http://hackaday.com/2015/03/18/how-to-directly-program-an-inexpensive-esp8266-wifi-module/

// http://naberius.de/2015/05/14/esp8266-gpio-output-performance/
// https://github.com/fasmide/esp_dht22/blob/master/espdht.c


// http://bbs.espressif.com/viewtopic.php?f=5&t=481
void ICACHE_FLASH_ATTR
user_rf_pre_init()
{
	// iets met system_phy_set_rfoption ??
}

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

            rf_cal_sec = 4096 - 5;

/*

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }
*/
    return rf_cal_sec;
}


void ICACHE_FLASH_ATTR
setup_wifi()
{
    char ssid[32] = SSID;
    char password[64] = SSID_PASSWORD;
    struct station_config stationConf;

    //Set station mode
    wifi_set_opmode(STATION_MODE);
//	wifi_set_opmode_current(STATION_MODE);

    //Set ap settings
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    wifi_station_set_config(&stationConf);

    wifi_station_connect();
}





void ICACHE_FLASH_ATTR
init_done()
{
    os_printf("\t\t system init done?!  \n\r");
    uint32 current_stamp = sntp_get_current_timestamp();
    os_printf("\t\t current_stamp2 %d \n\r", current_stamp);

	system_print_meminfo();


}


typedef void test_func(uint32_t);

void ICACHE_FLASH_ATTR
    test_task(os_event_t *e)
{
//    os_printf("\t\t test task %08x %08x \n", e->sig, e->par);

	test_func* f = (test_func*)e->sig;

	f((uint32_t)e->par);
}


// https://github.com/micropython/micropython/tree/master/esp8266

// TODO: fatfs http://elm-chan.org/fsw/ff/00index_e.html
// https://github.com/pellepl/spiffs
// https://github.com/pellepl/spiflash_driver

// https://github.com/nodemcu/nodemcu-firmware/tree/master/app/fatfs
// https://github.com/nodemcu/nodemcu-firmware/blob/master/app/platform/sdcard.c

// spi zooi: https://github.com/nodemcu/nodemcu-firmware/blob/master/app/platform/platform.c
// https://github.com/nodemcu/nodemcu-firmware/blob/master/app/driver/spi.c
// ./ESP8266_SDK/examples/driver_lib/driver/spi.c


// https://github.com/espressif/ESP8266_NONOS_SDK
// https://espressif.com/en/support/download/sdks-demos
void ICACHE_FLASH_ATTR
user_init()
{


/*

https://github.com/esp8266/esp8266-wiki/wiki/Memory-Map

flash start op: 40200000h

NodeMCU zegt dit:

 ets Jan  8 2013,rst cause:2, boot mode:(3,6)

IROM
load 0x40100000, len 27548, room 16 
tail 12
chksum 0x94
ho 0 tail 12 room 4

DRAM
load 0x3ffe8000, len 2444, room 12 
tail 0
chksum 0x73
load 0x3ffe898c, len 136, room 8 
tail 0
chksum 0xfb
csum 0xfb


40Mhz flash



Espressif refers to this area as "System Param" and it resides in the last four 4 kB sectors of flash.
The default init data is provided as part of the SDK in the file esp_init_data_default.bin. NodeMCU will automatically flash this file to the right place on first boot if the sector appears to be empty.

init_data moet hier
>>> (0x3fc000 + (4*4*1024)) / 1024.0/1024.0
4.0


blank.bin hier
0x3FE000

master_device_key.bin
non-ota 0x3E000
512+512 = 0x7E000
1024+1024 = 0xFE000


wtf?
40000h 	240k 	app.v6.irom0text.bin 	SDK libraries






		 system_get_cpu_freq 80 

 =============   spi init master   ============= 
mount bla: %i
spi_flash_get_id: 1458400
deviceId: 1640ef
chip_size: 524288
block_size: 65536
sector_size: 4096
page_size: 256
status_mask: 65535
my_spiffs_read 1000fe scandone
no Your SSID found, reconnect after 1s
reconnect
f 0, scandone

f 0, 		 test task 00000000 00000000 





�rlS	 ESP8266 user_init application 

		 SDK version:1.2.0    
		 Complie time:21:57:00  
		 init gpio  
		 spi flash id 1458400 
		 system_get_flash_size_map 0 
		 system_get_userbin_addr 0 
		 system_get_boot_mode 1 
		 system_get_boot_version 0 
		 system_get_cpu_freq 80 

 =============   spi init master   ============= 
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
please start sntp first !
		 current_stamp 0 
time 265290 52639mode : sta(60:01:94:1a:cc:85)
add if0
f 0, 		 system init done?!  
scandone
no Your SSID found, reconnect after 1s
reconnect
f 0, 
PORT CLOSED







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
please start sntp first !
		 current_stamp 0 
time 259985 51638mode : sta(60:01:94:1a:cc:85)
add if0
f 0, 		 system init done?!  
scandone





time 272017 57325
mode : sta(60:01:94:1a:cc:85)
add if0
f 0, 		 system init done?!  
please start sntp first !
		 current_stamp2 0 
scandone
add 0
aid 2
pm open phy_2,type:2 0 0
cnt 

connected with AP02, channel 13
dhcp client start...
ip:192.168.6.107,mask:255.255.255.0,gw:192.168.6.1
		 sensorssss  
		 sensorssss  
		 sensorssss  






	int i;
	for(i = 0; i < 100000; i++) {
	os_delay_us(60000);
	
	}


long long 8
float 4
void* 4
double 8


*/

// TODO: io tijden loggen? :P




  uart_div_modify(0, UART_CLK_FREQ / 115200);
//  uart_div_modify(0, UART_CLK_FREQ / 74880);
//	uart_init(BIT_RATE_115200, BIT_RATE_115200);
//	uart_init(BIT_RATE_74880, BIT_RATE_74880);
	os_delay_us(60000);

system_set_os_print(1);

    os_printf("\t ESP8266 %s application \n\r", __func__);
    os_printf("\t\t SDK version:%s    \n\r", system_get_sdk_version());
    os_printf("\t\t Compile time:%s  \n\r", __TIME__);



    os_printf("\t\t init gpio  \n\r");

	struct rst_info* rst = system_get_rst_info();

    os_printf("\t\t rst cause  %d %d: %x %x %x %x\n\r", rst->reason, rst->exccause, rst->epc1, rst->epc2, rst->epc3, rst->excvaddr);
    os_printf("\t\t spi flash id %d \n\r", spi_flash_get_id());

// https://github.com/espressif/ESP8266_NONOS_SDK/blob/18260135dfd17f40a6e0afc614bc7ce8d997c270/include/user_interface.h
    os_printf("\t\t system_get_flash_size_map %d \n\r", system_get_flash_size_map()); // 4 ????
    os_printf("\t\t system_get_userbin_addr %x \n\r", system_get_userbin_addr()); // 0

    os_printf("\t\t system_get_boot_mode %d \n\r", system_get_boot_mode()); // 1 ?
    os_printf("\t\t system_get_boot_version %d \n\r", system_get_boot_version()); // 0 ?

    os_printf("\t\t system_get_cpu_freq %d \n\r", system_get_cpu_freq()); // 80 :D

//	setup_wifi(); // hoeft maar 1 keer?!


/*
	int i;
	for(i = 0; i < 100000; i++) {
	os_delay_us(60000);
	
	}


    os_printf("\t\t sizeof int %d \n\r", sizeof(int));
    os_printf("\t\t sizeof long %d \n\r", sizeof(long));

    os_printf("\t\t sizeof uint8 %d \n\r", sizeof(uint8));
    os_printf("\t\t sizeof uint16 %d \n\r", sizeof(uint16));
    os_printf("\t\t sizeof uint32 %d \n\r", sizeof(uint32));
    os_printf("\t\t sizeof uint64 %d \n\r", sizeof(uint64));

		 sizeof int 4 
		 sizeof long 4 
		 sizeof uint8 1 
		 sizeof uint16 2 
		 sizeof uint32 4 
		 sizeof uint64 8 


*/


    gpio_init();

    //Set 1 en GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);

    //Set GPIO2 low
//    gpio_output_set(0, BIT1, BIT1, 0);
//    gpio_output_set(0, BIT2, BIT2, 0);

	GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 0);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 0);

//-- gpio.write(3, gpio.LOW) -- 0

//-- one wire input pin
//gpio.mode(4, gpio.INPUT, gpio.FLOAT)
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
//    G PIO_DIS_OUTPUT(PERIPHS_IO_MUX_GPIO2_U)
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO2_U);
//    PIN_PULLDWN_DIS(PERIPHS_IO_MUX_GPIO2_U);

    GPIO_DIS_OUTPUT(GPIO_ID_PIN(2)); // set gpio 2 as input



disk_io_init();

vfs_init();

/*

	os_printf("vfs_init fat init\n");

	fat_init2(&card);

	os_printf("vfs_init done\n");
*/




//-- https://nodemcu.readthedocs.io/en/master/en/modules/sntp/#sntpsync
//sntp.sync(nil, nil, nil, 1)
    sntp_setservername(0, "pool.ntp.org");
//    sntp_setservername(1, "asia.pool.ntp.org");
    sntp_set_timezone(0);
    sntp_init();

// TODO: rtc time?



    uint32 current_stamp = sntp_get_current_timestamp();
    os_printf("\t\t current_stamp %d \n\r", current_stamp);


testQueue = (os_event_t*)os_malloc(sizeof(os_event_t)*TEST_QUEUE_LEN);
system_os_task(test_task, USER_TASK_PRIO_0, testQueue, TEST_QUEUE_LEN);


webserver_init();

sensor_loop_init();

    system_init_done_cb(init_done);
}

