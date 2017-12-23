
#include "driver/onewire.h"
#include "ds18b20.h"

#define MAX_OW_DEVICES 16

uint8_t ow_pin = 2;

uint8_t ow_addrs[8][MAX_OW_DEVICES];
uint8_t ow_devices = 0;

void ICACHE_FLASH_ATTR
ds18b20_init()
{
 // https://github.com/esp8266/Basic/blob/master/libraries/dallas-temperature-control/DallasTemperature.cpp

	uint8_t addr[8]; // 64 bit?

	onewire_init(ow_pin);

	ow_devices = 0;

	while(ow_devices < MAX_OW_DEVICES && onewire_search(ow_pin, addr)) {
		os_printf("onewire addr %d = %x %x %x %x %x %x %x %x\n", ow_devices, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);

		// check crc

		os_memcpy(ow_addrs[ow_devices], addr, 8 * sizeof(uint8_t));

		ow_devices++;
	}

}

uint8_t ICACHE_FLASH_ATTR ds18b20_device_count()
{
	return ow_devices;
}


void ICACHE_FLASH_ATTR ds18b20_req_temp()
{
  onewire_reset(ow_pin);
	// check present?
  onewire_skip(ow_pin);
  onewire_write(ow_pin, STARTCONVO, 0);
}


void ICACHE_FLASH_ATTR ds18b20_read(uint8_t* deviceAddress, uint8_t* scratchPad)
{
  onewire_reset(ow_pin);

	// check present?

  onewire_select(ow_pin, deviceAddress);
  onewire_write(ow_pin, READSCRATCH, 0);


  // TODO => collect all comments &  use simple loop
  // byte 0: temperature LSB  
  // byte 1: temperature MSB
  // byte 2: high alarm temp
  // byte 3: low alarm temp
  // byte 4: DS18S20: store for crc
  //         DS18B20 & DS1822: configuration register
  // byte 5: internal use & crc
  // byte 6: DS18S20: COUNT_REMAIN
  //         DS18B20 & DS1822: store for crc
  // byte 7: DS18S20: COUNT_PER_C
  //         DS18B20 & DS1822: store for crc
  // byte 8: SCRATCHPAD_CRC
  //
  // for(int i=0; i<9; i++)
  // {
  //   scratchPad[i] = _wire->read();
  // }

int i;
  for( i = 0; i < 9; i++) {
	scratchPad[i] = onewire_read(ow_pin);
  }  

	// check crc?

//  onewire_reset(ow_pin);
}

void ICACHE_FLASH_ATTR ds18b20_device_addr(uint8_t index, uint8_t *addr)
{
	os_memcpy(addr, ow_addrs[index], 8);
}

float ICACHE_FLASH_ATTR ds18b20_read_temp(uint8_t addr[8])
{
	uint8_t scratchPad[9];
	ds18b20_read(addr, scratchPad);

	// check read result?
//os_printf("scratchPad[4] %x\n", scratchPad[4]);

	// 12 bit .... vast wel :P

  int16_t rawTemperature = (((int16_t)scratchPad[1]) << 8) | scratchPad[0];
//os_printf("rawTemperature %d\n", rawTemperature);
          return (float)rawTemperature * 0.0625;
//          return (uint32_t)rawTemperature * 625;
//          return rawTemperature;
}


/*
float DallasTemperature::calculateTemperature(uint8_t* deviceAddress, uint8_t* scratchPad)
{
  int16_t rawTemperature = (((int16_t)scratchPad[TEMP_MSB]) << 8) | scratchPad[TEMP_LSB];

  switch (deviceAddress[0])
  {
    case DS18B20MODEL:
    case DS1822MODEL:
      switch (scratchPad[CONFIGURATION])
      {
        case TEMP_12_BIT:
          return (float)rawTemperature * 0.0625;
          break;
        case TEMP_11_BIT:
          return (float)(rawTemperature >> 1) * 0.125;
          break;
        case TEMP_10_BIT:
          return (float)(rawTemperature >> 2) * 0.25;
          break;
        case TEMP_9_BIT:
          return (float)(rawTemperature >> 3) * 0.5;
          break;
      }
      break;
    case DS18S20MODEL:
      / *
      Resolutions greater than 9 bits can be calculated using the data from
      the temperature, COUNT REMAIN and COUNT PER �C registers in the
      scratchpad. Note that the COUNT PER �C register is hard-wired to 16
      (10h). After reading the scratchpad, the TEMP_READ value is obtained
      by truncating the 0.5�C bit (bit 0) from the temperature data. The
      extended resolution temperature can then be calculated using the
      following equation:
                                       COUNT_PER_C - COUNT_REMAIN
      TEMPERATURE = TEMP_READ - 0.25 + --------------------------
                                               COUNT_PER_C
      * /

      // Good spot. Thanks Nic Johns for your contribution
      return (float)(rawTemperature >> 1) - 0.25 +((float)(scratchPad[COUNT_PER_C] - scratchPad[COUNT_REMAIN]) / (float)scratchPad[COUNT_PER_C] );
      break;
  }
}
*/


