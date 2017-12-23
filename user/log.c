#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "espmissingincludes.h"

#define LOGBUFFERSIZE (1024*2)

static int logstartpos = 0;
static int logendpos = 0;
static char logbuffer[LOGBUFFERSIZE];


ICACHE_FLASH_ATTR
void casper_log(const char* format, ...)
{
	char buffer[256];
	int ret;
	va_list arglist;
	va_start(arglist, format);
	ret = ets_vsnprintf(buffer, sizeof buffer, format, arglist);
	va_end(arglist);

	os_printf(buffer);

	int len = os_strlen(buffer);
	int start = logendpos;
	logendpos += len;
	if(logendpos > LOGBUFFERSIZE) {
		int l = LOGBUFFERSIZE - logendpos;
		logendpos -= LOGBUFFERSIZE;
		logstartpos = logendpos;
		os_memcpy(&logbuffer[start], buffer, l);
		os_memcpy(logbuffer, &buffer[l], logendpos);
		
	} else {
		os_memcpy(&logbuffer[start], buffer, len);
	}
}

ICACHE_FLASH_ATTR
int casper_log_copy(char* dst, uint32_t maxsize) {

	if(maxsize > LOGBUFFERSIZE) {
		maxsize = LOGBUFFERSIZE;
	}

	os_memcpy(dst, logbuffer, maxsize);
	return maxsize;
}


