#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "stdlib.h"

// https://github.com/esp8266/Arduino/blob/f73457de0d6b703b96a809e48b2163f16aa367e8/hardware/esp8266com/esp8266/cores/esp8266/libc_replacements.c


int isspace(int c) {
    switch(c) {
        case 0x20: // ' '
        case 0x09: // '\t'
        case 0x0a: // '\n'
        case 0x0b: // '\v'
        case 0x0c: // '\f'
        case 0x0d: // '\r'
            return 1;
    }
    return 0;
}

// based on Source:
// https://github.com/anakod/Sming/blob/master/Sming/system/stringconversion.cpp#L93
double ICACHE_FLASH_ATTR strtod(const char* str, char** endptr) {
    double result = 0.0;
    double factor = 1.0;
    bool decimals = false;
    char c;

    while(isspace(*str)) {
        str++;
    }

    if(*str == 0x00) {
        // only space in str?
        *endptr = (char*) str;
        return result;
    }

    if(*str == '-') {
        factor = -1;
        str++;
    } else if(*str == '+') {
        str++;
    }

    while((c = *str)) {
        if(c == '.') {
            decimals = true;
            str++;
            continue;
        }

        int d = c - '0';
        if(d < 0 || d > 9) {
            break;
        }

        result = 10.0 * result + d;
        if(decimals) {
            factor *= 0.1;
        }

        str++;
    }
    *endptr = (char*) str;
    return result * factor;
}

float strtof(const char* str, char** endptr) {
	return strtod(str, endptr);
}





