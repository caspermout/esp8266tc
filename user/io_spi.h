

#include "c_types.h"
//#include "osapi.h"

#define SPI 0
#define HSPI 1

void io_spi_init(uint32_t speed);

int io_spi_writebyte(uint8_t byte);
uint8_t io_spi_readbyte();

uint8_t io_spi_readwritebyte(uint8_t byte);


int io_spi_write(const uint8_t *buf, uint32_t len);

int io_spi_read(uint8_t *buf, uint32_t len);



