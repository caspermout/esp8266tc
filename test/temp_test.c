#include <stdio.h>

#include "../tempcontrol/sensors.h"
#include "../tempcontrol/tempcontrol.h"


char* relay1 = "relay1";
char* relay2 = "relay2";


int gpio[100];

void io_gpio_write(int pin, int value) {
	gpio[pin] = value;
}

int io_gpio_read(int pin) {
	return gpio[pin];
}


void print_sensors(int time) {

	int i;
	for(i = 0; i < config_count(sensors); i++) {
		char* name = config_name(sensors, i);
		int type = map_type(sensors, name);

		if(type == 1) {
			char* values = get_str(sensors, name);
			printf("%d,%s,%s\n", time, name, values);

		} else if(type == 2) {
			float valuef = get_float(sensors, name);
			int i = (int)valuef;
			int f = (int)(valuef * 10000) % 10000;
			if(f < 0) {
				f = - f;
			}
			printf("%d,%s,%d.%04d,%f\n", time, name, i, f, valuef);

		} else {
			int valuei = get_int(sensors, name);
			printf("%d,%s,%d\n", time, name, valuei);
		}
	}

}

float roundtemp(float t) {
	int ti = t / 0.0625;
	float tr = (t / 0.0625) - ti;
	printf("tr %f %d\n", tr, (int)(tr * 100.0) % 2);
	if(tr > 0.65) {
		ti += 1;
	} else if(tr > 0.35) {
		ti += (int)(tr * 1700.0) % 2;
	}
	return (float)ti * 0.0625;
}



void main()
{


config_clear(config);
tempcontrol_defaults();


put_float(config, "goaltemp", 15);
//-- goaltemphyst = 0.25

/*
-- pid1kp = 1.0
pid1ki = 0.00005
pid1kd = 5000.0

tempchangerate = 0.7 / 3600 -- 0.7 c per hour
pid1ki = 0.0001
pid1kd = 1000.0


tempchangerate = 0.5 / 3600
pid1ki = 0.001
pid1kd = 0

tempchangerate = 0.8 / 3600
pid1ki = 0.001
pid1kd = 20000.0 -- overshoot beperking


tempchangerate = 0.5 / 3600
pid1ki = 0.001
pid1kd = 20000.0 -- soort van acceptabel


tempchangerate = 0.7 / 3600
pid1ki = 0.00025
pid1kd = 1000.0

pid1ki = 0.0005
pid1kd = 10000.0


pid1kp = 25.0

*/

//put_float(config, "pid1kp", 50.0);
//put_float(config, "pid1ki", 0.0025);
//put_float(config, "pid1ki", 0.000000015);
//put_float(config, "pid1kd", 10000.0);

//put_float(config, "tempchangeuprate", 0.5 / 3600.0);
//put_float(config, "tempchangedownrate", 10 / 3600.0);


char* emmer1sensor_r = "emmer1temp_r";
char* koelkasttempsensor_r = "kk1temp_r";

char* emmer1sensor_s = "emmer1temp";
char* koelkasttempsensor_s = "kk1temp";
put_str(config, "emmer1sensor", emmer1sensor_s);
put_str(config, "koelkasttempsensor", koelkasttempsensor_s);


put_float(config, "emmer1temp_cal1", 1.0);
put_float(config, "emmer1temp_cal2", 0.0);
put_float(config, "kk1temp_cal1", 1.0);
put_float(config, "kk1temp_cal2", 0.0);


put_int(config, "tempcontrolenabled", 1);

//put_float(config, "ema1_n", 200.0);
//put_float(config, "ema2_n", 200.0);
put_float(config, "ema3_n", 100.0);
//put_float(config, "ema3_n", 2.0);



put_int(config, "kkminofftime", 600);
//put_int(config, "kkminofftime", 30*60);
//-- kkminofftime = 5*60
//-- kkminontime = 5*60
//-- kkminontime = 5*60



// testje
put_float(config, "pid1kp", 1.0);
put_float(config, "pid1ki", 0.00025);
put_float(config, "pid1kd", 100.0);

// goed zodra die stabiel is
put_float(config, "pid1kp", 1.0);
put_float(config, "pid1ki", 0.0007);
put_float(config, "pid1kd", 0.0);

//put_float(config, "pid1kp", 1.0);
//put_float(config, "pid1ki", 0.0);



//put_int(config, "kkminontime", 60);
//put_float(config, "goaltemphyst_above", 0.25);
//put_float(config, "goaltemphyst_below", 0.25);

/*
put_float(config, "ema1_n", 600 * 2);
put_float(config, "ema2_n", 20.0); // heeft weinig nodig, is alleen voor hyst en die boeit het niet
put_float(config, "ema3_n", 600 * 2);
put_float(config, "goaltemphyst_above", 0.95);
put_float(config, "goaltemphyst_below", 0.25);
put_float(config, "pid1kp", 0.0);
put_float(config, "pid1ki", 0.0);
put_float(config, "pid1kd", 0.0);
*/


put_float(config, "ema1_n", 600 * 2);
put_float(config, "ema2_n", 20.0); // heeft weinig nodig, is alleen voor hyst en die boeit het niet
put_float(config, "ema3_n", 600 * 2);
put_float(config, "goaltemphyst_above", 0.25);
put_float(config, "goaltemphyst_below", 0.25);
put_float(config, "pid1kp", 0.0);
put_float(config, "pid1ki", 0.0001);
put_float(config, "pid1kd", 0.0);



config_clear(sensors);
put_float(sensors, emmer1sensor_r, 15);
put_float(sensors, koelkasttempsensor_r, 15.75 - 0.75);
put_float(sensors, emmer1sensor_s, roundtemp(get_float(sensors, emmer1sensor_r)));
put_float(sensors, koelkasttempsensor_s, roundtemp(get_float(sensors, koelkasttempsensor_r)));

config_clear(sensors_cal);






	tempcontrol_init();

	int i;
	for(i = 1000005; i < 1060000; i += 5) {


	print_sensors(i);
	tempcontrol_run(i);

//	http://mhi-inc.com/Converter/watt_calculator.htm

//	3.963 gallon (15l) verwarmen met 0.005 c in 5 sec = 34.27 watt ofzo

	// TODO: even niet verwarmen :P
//	put_float(sensors, emmer1sensor_r, get_float(sensors, emmer1sensor_r) + 0.005);

	float buitenlucht = 25;
	float kk = get_float(sensors, koelkasttempsensor_r);

	kk += (buitenlucht - kk) * 0.00045;

	if(get_int(sensors, relay1) == 1) {
		kk += (0.0 - kk) * 0.0025;
	}

	put_float(sensors, koelkasttempsensor_r, kk);


// http://www.physicsclassroom.com/class/thermalP/Lesson-1/Rates-of-Heat-Transfer
// Air (g) k=0.024
// Polyethylene (HDPE) (s) k=0.5
// Water (l) k=0.58

	float diff = get_float(sensors, emmer1sensor_r) - get_float(sensors, koelkasttempsensor_r);
//	print('diff '..diff)
//	temps[emmer1sensor] = temps[emmer1sensor] - diff * 0.5

	// heat door een muur... doe net alsof de emmer een muur is
	// Rate = k•A•(T1 - T2)/d
	// https://www.calculatorsoup.com/calculators/geometry-solids/tube.php

	float rate = 0.5 * 0.28274333882 * diff / (2.0 / 1000.0);
	rate /= 7.0;
//	print(rate..' watt')
//	print((rate*5)..' joules')
//	print((rate*5/34270)..' c voor 15l water')

	put_float(sensors, emmer1sensor_r, get_float(sensors, emmer1sensor_r) - (rate*5.0/34270.0));
	// we doen maar net of de thermal mass van de koelkast veeeeeel hoger is en rekenen hem niet mee :P

put_float(sensors, emmer1sensor_s, roundtemp(get_float(sensors, emmer1sensor_r)));
put_float(sensors, koelkasttempsensor_s, roundtemp(get_float(sensors, koelkasttempsensor_r)));


	}

}



