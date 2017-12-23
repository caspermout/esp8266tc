
#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>


void io_gpio_write(int pin, int value) {
	if(pin == 1) {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(5), value);
	}
	if(pin == 2) {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(4), value);
	}
}

int io_gpio_read(int pin) {
	if(pin == 1) {
		return GPIO_INPUT_GET(GPIO_ID_PIN(5));
	}
	if(pin == 2) {
		return GPIO_INPUT_GET(GPIO_ID_PIN(4));
	}
	return -1;
}

