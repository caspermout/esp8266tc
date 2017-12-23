#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "stdlib.h"

#include "sntp.h"

#include "ff.h"

#include "ds18b20.h"
#include "disk_io.h"
#include "vfs.h"

#include "../tempcontrol/sensors.h"
#include "../tempcontrol/tempcontrol.h"


static os_timer_t loop_timer;
static os_timer_t ds_timer;

int logpos1;
int logpos2;

ICACHE_FLASH_ATTR
void write_sensor_log(int time) {

	FIL log;
	FRESULT r = f_open(&log, "/log.csv", FA_OPEN_APPEND | FA_WRITE | FA_READ);
	if(r != FR_OK) {
		os_printf("Open file /log.csv for write failed: %d\n", r);
		return;
	}

	logpos1 = f_tell(&log);
	if(logpos2 <= 0) {
		logpos2 = logpos1;
	}

	put_int(sensors, "logpos1", logpos1);

	char csvline[512];
	
	int i;
	for(i = 0; i < config_count(sensors); i++) {
		char* name = config_name(sensors, i);
		int type = map_type(sensors, name);

		if(type == 1) {
			char* values = get_str(sensors, name);
			os_sprintf(csvline, "%d,%s,%s\n", time, name, values);

		} else if(type == 2 && has_float(sensors_cal, name)) {
			float valuefc = get_float(sensors_cal, name);
			int ic = (int)valuefc;
			int fc = (int)(valuefc * 10000) % 10000;
			if(fc < 0) {
				fc = - fc;
			}

			float valuef = get_float(sensors, name);
			int i = (int)valuef;
			int f = (int)(valuef * 10000) % 10000;
			if(f < 0) {
				f = - f;
			}
			os_sprintf(csvline, "%d,%s,%d.%04d,%d.%04d\n", time, name, ic, fc, i, f);

		} else if(type == 2) {
			float valuef = get_float(sensors, name);
			int i = (int)valuef;
			int f = (int)(valuef * 10000) % 10000;
			if(f < 0) {
				f = - f;
			}
			os_sprintf(csvline, "%d,%s,%d.%04d\n", time, name, i, f);

		} else {
			int valuei = get_int(sensors, name);
			os_sprintf(csvline, "%d,%s,%d\n", time, name, valuei);
		}

		uint32_t bw;
		uint32_t len = strlen(csvline);
		r = f_write(&log, csvline, len, &bw);
		if(r != FR_OK) {
			os_printf("Write log file failed: %d\n", r);
		}
		if(bw != len) {
			os_printf("Write log file failed %d != %d\n", bw, len);
		}

	}

	r = f_sync(&log);
	if(r != FR_OK) {
		os_printf("Sync log file failed: %d\n", r);
	}


	int size = f_size(&log);
	r = f_lseek(&log, logpos2);
	int cur = f_tell(&log);
	if(r != FR_OK || cur != logpos2) {
		os_printf("Seek to end of log file failed: %d %d %d %d\n", r, cur, logpos2, size);

	} else {

		int keepreadings = get_int(config, "keepreadings");
//		os_printf("keepreadings %d - %d = %d\n", time, keepreadings, time - keepreadings);
		int curtime;

		do {
			curtime = -1;
			logpos2 = f_tell(&log);

			if (!f_gets(csvline, sizeof csvline, &log)) {
				break;
			}

//			os_printf("%d: %s", logpos2, csvline);

			char* comma = os_strchr(csvline, ',');
			if(comma == NULL) {
				break;
			}

			*comma = 0;

			curtime = atoi(csvline);
//			os_printf("curtime %d\n", curtime);

		} while(logpos2 < logpos1 && curtime != -1 && curtime < time - keepreadings);

	}


	r = f_close(&log);
	if(r != FR_OK) {
		os_printf("Close log file failed: %d\n", r);
		return;
	}
}


void ICACHE_FLASH_ATTR
do_read_sensors()
{
//    os_printf("\t\t read sensorssss %d \n\r", ds18b20_device_count());

	int i = 0;
	uint8_t addr[8];
	for(i = 0; i < ds18b20_device_count(); i++)
	{
		ds18b20_device_addr(i, addr);
		float temp = ds18b20_read_temp(addr);

//		int i = (int)temp;
//		int f = (int)(temp * 10000) % 10000;
//		if(f < 0) {
//			f = - f;
//		}

//		os_printf("%02x%02x%02x%02x%02x%02x%02x%02x : %d.%04d\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7], i, f);

		char sensorname[256];
		os_sprintf(sensorname, "%02x%02x%02x%02x%02x%02x%02x%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);

		put_float_cn(sensors, sensorname, temp);
	}

	put_int(sensors, "heap", system_get_free_heap_size());
	put_int(sensors, "uptime", system_get_time() / 1000000);

	int time = sntp_get_current_timestamp();

	tempcontrol_run(time);

	write_sensor_log(time);
}


void ICACHE_FLASH_ATTR
do_sensors()
{

    uint32 current_stamp = sntp_get_current_timestamp();
    os_printf("current_stamp3 %d system_get_free_heap_size %d\n\r", current_stamp, system_get_free_heap_size());

    ds18b20_req_temp();

	os_timer_arm(&ds_timer, 1000, 0); // na 1s uitlezen


/*
data  : 0x3ffe8000 ~ 0x3ffe878c, len: 1932
rodata: 0x3ffe8790 ~ 0x3ffe9158, len: 2504
bss   : 0x3ffe9158 ~ 0x3ffef7c8, len: 26224
heap  : 0x3ffef7c8 ~ 0x3fffc000, len: 51256
system_get_free_heap_size 49120

*/

}

void ICACHE_FLASH_ATTR
reload_config() {

	char line[256];
	FIL conf;
	FRESULT r = f_open(&conf, "/config", FA_READ);
	if(r == FR_OK) {
		os_printf("-------------- config\n");
		while (f_gets(line, sizeof line, &conf)) {

			int linelen = os_strlen(line);
			os_printf(line);

			char* eq = os_strchr(line, '=');
			char* nl = os_strchr(line, '\r');
			char* nl2 = os_strchr(line, '\n');
			if(nl == NULL || (nl2 != NULL && nl2 < nl)) {
				nl = nl2;
			}
			if(eq != NULL && nl != NULL) {
				*eq = 0;
				*nl = 0;
				eq++;

				os_printf("'%s' = '%s'\n", line, eq);

				int type = map_type(config, line);
				if(type == 1) {
					put_str_cnv(config, line, eq);

				} else if(type == 2) {
					char* end;
					float f = strtof(eq, &end);
					if((*end == 0 || *end == '\r' || *end == '\n')) {
						os_printf("'%s' = %d / 10000\n", line, (int)(f * 10000));
						put_float_cn(config, line, f);
					} else {
						os_printf("Invalid float\n");
					}

				} else {
					char* end;
					long i = strtol(eq, &end, 10);
					if((*end == 0 || *end == '\r' || *end == '\n')) {
						os_printf("'%s' = %d\n", line, i);
						put_int_cn(config, line, i);
					} else {
						os_printf("Invalid number\n");
					}
				}



			}
		}
		os_printf("--------------\n");

		if(f_error(&conf)) {
			os_printf("Error while /config reading file: %d\n", f_error(&conf));
		}


		r = f_close(&conf);
		if(r != FR_OK) {
			os_printf("Close file /config failed: %d\n", r);
		}

	} else {
		os_printf("Open file /config failed: %d\n", r);
	}

	tempcontrol_init();
}


void ICACHE_FLASH_ATTR
sensor_loop_init()
{

	logpos1 = 0;
	logpos2 = 0;
	
	FIL log;
	FRESULT r = f_open(&log, "/log.csv", FA_READ);
	if(r == FR_OK) {
		logpos1 = f_size(&log);
		logpos2 = f_size(&log);

		r = f_close(&log);
		if(r != FR_OK) {
			os_printf("Close log file for pos failed: %d\n", r);
		}

	} else {
		os_printf("Open file /log.csv for pos failed: %d\n", r);
	}



	ds18b20_init();

	tempcontrol_defaults();

	reload_config();

	os_timer_disarm(&loop_timer);
	os_timer_setfn(&loop_timer, (os_timer_func_t *)do_sensors, NULL);
	os_timer_arm(&loop_timer, 5000, 1); // repeat every 5000 ms

	os_timer_disarm(&ds_timer);
	os_timer_setfn(&ds_timer, (os_timer_func_t *)do_read_sensors, NULL);
}



