#ifndef TEMPCONTROL_H
#define TEMPCONTROL_H

#include "sensors.h"

typedef struct ema_t {
	int lasttime;
	float last;
	float number;
} ema_t;

typedef struct tempchanger_t {
	float maxuprate;
	float maxdownrate;

	int lasttime;
	float lastgoal;
} tempchanger_t;

typedef struct pidcontroller_t {
	float kp;
	float ki;
	float kd;

    	float previous_error;
    	int previous_time;
    	float integral;

} pidcontroller_t;

typedef struct hyst_t {
	float ha;
	float hb;
} hyst_t;

typedef struct koelkast_t {
	int minontime;
	int minofftime;
	int maxontime;

    int laststate;
    int lastchange;

} koelkast_t;

extern tempchanger_t *tempchanger1;
extern pidcontroller_t *pid1;
extern hyst_t *hyst1;
extern koelkast_t *koelkast1;



void tempcontrol_defaults();
void tempcontrol_init();

void tempcontrol_run(int time);


#endif

