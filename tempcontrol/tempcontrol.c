
#include "casper.h"
#include "sensors.h"
#include "tempcontrol.h"
#include "io_gpio.h"


ema_t *ema1;
ema_t *ema2;
ema_t *ema3;
tempchanger_t *tempchanger1;
pidcontroller_t *pid1;
hyst_t *hyst1;
koelkast_t *koelkast1;

ema_t ema1_s;
ema_t ema2_s;
ema_t ema3_s;
tempchanger_t tempchanger1_s;
pidcontroller_t pid1_s;
hyst_t hyst1_s;
koelkast_t koelkast1_s;

void ema_init(ema_t* ema, float number) {

	// TODO: we nemen maar aan dat -1000 of lager nergens op slaat
	ema->last = -10000.0;
	ema->lasttime = 0;
	ema->number = number;
}

float ema_run(ema_t* ema, int time, float current) {

    	if (ema->last <= -1000.0f) {
    		ema->last = current;
    	}
    	if (ema->lasttime == 0) {
    		ema->lasttime = time;
    	}

	float dt = time - ema->lasttime;
	if(dt < 0.1) {
		dt = 0.1;
	}
	if(dt > 60.0) {
		dt = 60.0;
	}
	// TODO: eigenlijk moet ema->number >= dt
	if(ema->number <= dt) {
		ema->number = dt;
	}
	float n = ema->number / dt;
	float a = 2.0 / (n - 1.0);
	if(a > 1.0) {
		a = 1.0;
	}
	if(a < 0.0) {
		a = 0.0;
	}

/*
	os_printf("dt: %f\n", dt);
	os_printf("n: %f\n", n);
	os_printf("a: %f\n", a);
*/
    
	// TODO: dit kan ook met 1 keer minder vermenigvuldigen maarja....
    	ema->last = a*current + (1.0-a)*ema->last;
	ema->lasttime = time;

	return ema->last;
}


void tempchanger_init(tempchanger_t* tempchanger, float maxuprate, float maxdownrate) {

	tempchanger->lasttime = 0;
	tempchanger->lastgoal = -10000.0f;
	tempchanger->maxuprate = maxuprate;
	tempchanger->maxdownrate = maxdownrate;
}

float tempchanger_run(tempchanger_t* tempchanger, int time, float goal, float temp) {

    	if (tempchanger->lastgoal <= -1000.0f) {
    		tempchanger->lastgoal = temp;
    	}
    	if (tempchanger->lasttime == 0) {
    		tempchanger->lasttime = time;
    	}
    
    	float dt = time - tempchanger->lasttime;
	if(dt > 60.0) {
		// vast iets raars dan
		dt = 60.0;
	}
	if(dt <= 0) {
		// vast iets raars dan
		dt = 0.0;
	}
//	os_printf("tempchanger dt: %d\n", (int)dt);

    	tempchanger->lasttime = time;
    	float maxupdiff = dt * tempchanger->maxuprate;
    	float maxdowndiff = dt * tempchanger->maxdownrate;
    
    	float diff = goal - tempchanger->lastgoal;
//	os_printf("tempchanger diff: %d / 10000\n", (int)(diff * 10000.0));

//	os_printf("tempchanger0 %d %d %d\n", (int)dt, (int)(tempchanger->maxuprate * 100), (int)(tempchanger->maxdownrate * 100));
//	os_printf("tempchanger0 %d %d %d\n", (int)dt, (int)(maxupdiff * 100), (int)(maxdowndiff * 100));
//	os_printf("tempchanger1 %d %d %d %d %d\n", time, (int)goal, (int)temp, (int)diff, (int)(tempchanger->lastgoal));
    
    	if (diff > maxupdiff) {
    		diff = maxupdiff;
    	} else if (diff < -maxdowndiff) {
    		diff = -maxdowndiff;
    	}

//	os_printf("tempchanger diff: %d / 10000\n", (int)(diff * 10000.0));
//	os_printf("tempchanger2 %d %d %d %d %d\n", time, (int)goal, (int)temp, (int)diff, (int)(tempchanger->lastgoal));
    
    	tempchanger->lastgoal += diff;
    
    	return tempchanger->lastgoal;
}

void pid_init(pidcontroller_t *pid, float kp, float ki, float kd) {

	pid->kp = kp;
	pid->ki = ki;
	pid->kd = kd;

    	pid->previous_error = 0.0;
    	pid->previous_time = 0;
    	pid->integral = 0.0;
}

float pid_run(pidcontroller_t *pid, int time, float setpoint, float measured_value) {

	if(pid->previous_time == 0) {
		pid->previous_time = time;
	}

        int dt = time - pid->previous_time;

	if(pid->integral < -1000000.0 || pid->integral > 1000000.0) {
		os_printf("int %d / 1000\n", (int)(pid->integral * 1000.0));
	}

        float error = setpoint - measured_value;
        float output = pid->kp*error;

	if(dt > 0) {
	        pid->integral += pid->ki*error*dt;
	        float derivative = (error - pid->previous_error)/dt;
		output += pid->kd*derivative;
	}

	output += pid->integral;

        pid->previous_error = error;
        pid->previous_time = time;
        
        return output;
}


void hyst_init(hyst_t *hyst, float ha, float hb) {
	hyst->ha = ha;
	hyst->hb = hb;
}

float hyst_run(hyst_t *hyst, int time, float goal, float temp) {
        if (temp < goal - hyst->hb) {
            return 1;
            
        } else if (temp > goal + hyst->ha) {
            return -1;
        }

        return 0;
}

void koelkast_init(koelkast_t *koelkast, int minontime, int minofftime, int maxontime) {

    koelkast->laststate = -1;
    koelkast->lastchange = -1;
	koelkast->minontime = minontime;
	koelkast->minofftime = minofftime;
	koelkast->maxontime = maxontime;
}

int koelkast_dt(koelkast_t *koelkast, int time) {
        if (koelkast->lastchange == -1) {
            koelkast->lastchange = time;
        }

	int dt = time - koelkast->lastchange;
	return dt;
}

int koelkast_run(koelkast_t *koelkast, int time, float goal) {

	int dt = koelkast_dt(koelkast, time);

        if (koelkast->laststate == -1 && dt < koelkast->minofftime) {
            return 0;
        }
        if (koelkast->laststate == 1 && dt < koelkast->minontime) {
            return 0;
        }

        if (koelkast->laststate == 1 && dt > koelkast->maxontime) {
	os_printf("kk dt > maxontime: %d\n", dt);
            koelkast->lastchange = time;
            koelkast->laststate = -1;
            return koelkast->laststate;
        }
        
        if (koelkast->laststate != -1 && goal > 0) {
	os_printf("kk goal>0 -dt: %d\n", dt);
            koelkast->lastchange = time;
            koelkast->laststate = -1;
            return koelkast->laststate;
            
        } else if (koelkast->laststate != 1 && goal < 0) {
	os_printf("kk goal<0 dt: %d\n", dt);
            koelkast->lastchange = time;
            koelkast->laststate = 1;
            return koelkast->laststate;
        }

        return 0;
}


void tempcontrol_defaults() {

/*
TODO:
crash cool mode maken die reset nadat die bij dieptepunt is... werkt wel denk ik
controller uit kunnen zetten

settings kunnen saven zodat reset niet problematisch is...
*/

put_int(config, "relay1", 1);
put_int(config, "relay2", 2);

//-- moet lager staan dan de hoeveelheid warmte die de gist gaat produceren :P of die van buiten komt
put_float(config, "tempchangeuprate", 0.5 / 3600.0); // -- 0.5 c per hour
put_float(config, "tempchangedownrate", 10.0 / 3600.0); // -- 10 c per hour

put_float(config, "pid1kp", 50.0); // -- om te compenseren voor de hoge ki en kd
put_float(config, "pid1ki", 0.0025); // -- offset correctie, is alleen veilig om zo hoog te doen met de tempchangerate ... anders moet die op 0.00006 ofzo max
put_float(config, "pid1kd", 10000.0); // -- tegen overshoot :P

put_float(config, "goaltemp", 30.0);
put_float(config, "goaltemphyst_above", 0.25);
put_float(config, "goaltemphyst_below", 0.25);

put_float(config, "ema1_n", 200.0);
put_float(config, "ema2_n", 200.0);
put_float(config, "ema3_n", 200.0);

put_int(config, "kkminontime", 5*60);
put_int(config, "kkmaxontime", 30*60);

put_int(config, "kkminofftime", 30*60);

//-- vul in
put_str(config, "emmer1sensor", "287a11ee08000081");
put_str(config, "koelkasttempsensor", "2840edf008000009");

put_int(config, "keepreadings", 125); // -- seconds

put_int(config, "tempcontrolenabled", 0);



put_float(config, "28459eef08000063_cal1", 1.000213979793837);
put_float(config, "28459eef08000063_cal2", -0.5539835044428699);

put_float(config, "2838f1f00800006d_cal1", 1.000738312095865);
put_float(config, "2838f1f00800006d_cal2", -0.628428804538217);

put_float(config, "287a11ee08000081_cal1", 1.001079709577771);
put_float(config, "287a11ee08000081_cal2", 0.29027477105198907);

put_float(config, "284734ef08000083_cal1", 1.0007773101838005);
put_float(config, "284734ef08000083_cal2", 0.3109215923606046);

put_float(config, "28dd2df0080000d0_cal1", 0.9981463326786915);
put_float(config, "28dd2df0080000d0_cal2", 0.35935966810033804);

put_float(config, "28ff1bee0800003d_cal1", 0.9965637911761293);
put_float(config, "28ff1bee0800003d_cal2", -0.44550340151669715);

put_float(config, "2840edf008000009_cal1", 1.001946383303897);
put_float(config, "2840edf008000009_cal2", 0.33016669592383596);

put_float(config, "281d5bf0080000be_cal1", 0.9997560258201995);
put_float(config, "281d5bf0080000be_cal2", 0.3767338272205054);
}

void tempcontrol_init() {

	ema1 = &ema1_s;
	ema_init(ema1, get_float(config, "ema1_n"));

	ema2 = &ema2_s;
	ema_init(ema2, get_float(config, "ema2_n"));

	ema3 = &ema3_s;
	ema_init(ema3, get_float(config, "ema3_n"));

	float maxuprate = get_float(config, "tempchangeuprate");
	float maxdownrate = get_float(config, "tempchangedownrate");
	tempchanger1 = &tempchanger1_s;
	tempchanger_init(tempchanger1, maxuprate, maxdownrate);

	float pid1kp = get_float(config, "pid1kp");
	float pid1ki = get_float(config, "pid1ki");
	float pid1kd = get_float(config, "pid1kd");
	pid1 = &pid1_s;
	pid_init(pid1, pid1kp, pid1ki, pid1kd);

	float ha = get_float(config, "goaltemphyst_above");
	float hb = get_float(config, "goaltemphyst_below");
	hyst1 = &hyst1_s;
	hyst_init(hyst1, ha, hb);

	int kkminontime = get_int(config, "kkminontime");
	int kkminofftime = get_int(config, "kkminofftime");
	int kkmaxontime = get_int(config, "kkmaxontime");
	koelkast1 = &koelkast1_s;
	koelkast_init(koelkast1, kkminontime, kkminofftime, kkmaxontime);
}

void tempcontrol_run(int time) {


//    print()
//    print('doeiets '..#sensors..' '..time)

	if(time == 0) {
		os_printf("No time yet %d\n", time);
		return;
	}

//	os_printf("Temp control loop %d\n", time);


	float goaltemp = get_float(config, "goaltemp");

    put_float(sensors, "goaltemp", goaltemp);

	config_clear(sensors_cal);

		char name_cal1[256];
		char name_cal2[256];
		char name_cal3[256];

	int i;
	for(i = 0; i < config_count(sensors); i++) {
		char *name = config_name(sensors, i);

		sprintf(name_cal1, "%s%s", name, "_cal1");
		sprintf(name_cal2, "%s%s", name, "_cal2");
		sprintf(name_cal3, "%s%s", name, "_cal3");

		if(!has_float(config, name_cal1)) {
			continue;
		}
		if(!has_float(config, name_cal2)) {
			continue;
		}

		float cal1 = get_float(config, name_cal1);
		float cal2 = get_float(config, name_cal2);

		    float t = get_float(sensors, name);

		if(has_float(config, name_cal3)) {
			float cal3 = get_float(config, name_cal3);
		    t = cal1 * t * t + cal2 * t + cal3;
		} else {
		    t = cal1 * t + cal2;
		}

		put_float(sensors_cal, name, t);
	}

// TODO: exponential running average op sensors zodat de derivative niet fucking raar piekt


    if (!has_float(sensors_cal, get_str(config, "emmer1sensor"))) {
	os_printf("sensor not calibrated '%s'\n", get_str(config, "emmer1sensor"));
        return;
    }
    if (!has_float(sensors_cal, get_str(config, "koelkasttempsensor"))) {
	os_printf("sensor not calibrated '%s'\n", get_str(config, "koelkasttempsensor"));
        return;
    }

    float emmer1temp_raw = get_float(sensors_cal, get_str(config, "emmer1sensor"));
    float koelkasttemp_raw = get_float(sensors_cal, get_str(config, "koelkasttempsensor"));

    float emmer1temp = ema_run(ema1, time, emmer1temp_raw);
    float koelkasttemp = ema_run(ema2, time, koelkasttemp_raw);
    put_float(sensors, "emmertemp_ema", emmer1temp);
    put_float(sensors, "koelkasttemp_ema", koelkasttemp);


    float tempchanger1val = tempchanger_run(tempchanger1, time, goaltemp, emmer1temp);
    put_float(sensors, "tempchanger1val", tempchanger1val);

    float pid1val_raw = pid_run(pid1,time, tempchanger1val, emmer1temp);
    put_float(sensors, "pid1val", pid1val_raw);

    float pid1val = ema_run(ema3, time, pid1val_raw);
    put_float(sensors, "pid1val_ema", pid1val);

    // denk dat hier kk1temp moet ipv goaltemp?, nope goaltemp lijkt net IETS beter te werken, echt minimaal
    float hyst1goal = goaltemp + pid1val;
//    float hyst1goal = pid1val;
//    local hyst1goal = koelkasttemp + pid1val
    put_float(sensors, "hyst1goal", hyst1goal);
    
    int hyst1val = hyst_run(hyst1, time, hyst1goal, koelkasttemp);
    put_int(sensors, "hyst1val", hyst1val);
    
    int kk1lastchange = koelkast_dt(koelkast1, time);
    int kk1val = koelkast_run(koelkast1, time, hyst1val);

    put_int(sensors, "kk1val", kk1val);
    put_int(sensors, "kk1lastchange", kk1lastchange);

    if (get_int(config, "tempcontrolenabled") == 1) {
        if (kk1val == 1) {
            io_gpio_write(get_int(config, "relay1"), 1);
        } else if (kk1val == -1) {
            io_gpio_write(get_int(config, "relay1"), 0);
        }
    }

    put_int(sensors, "relay1", io_gpio_read(get_int(config, "relay1")));
    put_int(sensors, "relay2", io_gpio_read(get_int(config, "relay2")));
}

