
#include "osapi.h"
#include "user_interface.h"

#include "vfs.h"


// https://github.com/espressif/ESP8266_RTOS_SDK/blob/master/third_party/spiffs/esp_spiffs.c


// #define FLASH_UNIT_SIZE 4
//  return spi_flash_erase_sector(addr / fs.cfg.phys_erase_block);



ICACHE_FLASH_ATTR
void vfs_init() {

	my_spiffs_mount();

	os_printf("vfs_init fat init\n");

	fat_init(&card);

	os_printf("vfs_init done\n");
}




/*


in linker script
PROVIDE ( _SPIFFS_start = 0x40300000 );
PROVIDE ( _SPIFFS_end = 0x405FB000 );
PROVIDE ( _SPIFFS_page = 0x100 );
PROVIDE ( _SPIFFS_block = 0x2000 );





https://github.com/espressif/ESP8266_RTOS_SDK/blob/master/include/spiffs/spiffs_config.h

#if SPIFFS_SINGLETON
// Instead of giving parameters in config struct, singleton build must
// give parameters in defines below.
#ifndef SPIFFS_CFG_PHYS_SZ
#define SPIFFS_CFG_PHYS_SZ(ignore)        (1024*1024*2)
#endif
#ifndef SPIFFS_CFG_PHYS_ERASE_SZ
#define SPIFFS_CFG_PHYS_ERASE_SZ(ignore)  (65536)
#endif
#ifndef SPIFFS_CFG_PHYS_ADDR
#define SPIFFS_CFG_PHYS_ADDR(ignore)      (0)
#endif
#ifndef SPIFFS_CFG_LOG_PAGE_SZ
#define SPIFFS_CFG_LOG_PAGE_SZ(ignore)    (256)
#endif
#ifndef SPIFFS_CFG_LOG_BLOCK_SZ
#define SPIFFS_CFG_LOG_BLOCK_SZ(ignore)   (65536)
#endif
#endif
*/

