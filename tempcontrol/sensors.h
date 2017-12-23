#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

#define MAX_ITEMS 128


typedef struct config_t {
	char* names[MAX_ITEMS];
	float values_f[MAX_ITEMS];
	int values_i[MAX_ITEMS];
	char* values_s[MAX_ITEMS];

} config_t;

extern config_t *config;
extern config_t *sensors;
extern config_t *sensors_cal;


void config_clear(config_t *config);
int config_count(config_t *config);
char* config_name(config_t *config, int idx);

int map_type(config_t *config, char* name);


char* get_str(config_t *config, char* name);
void put_str(config_t *config, char* name, char* value);
void put_str_cnv(config_t *config, char* name, char* value);


int get_int(config_t *config, char* name);
void put_int(config_t *config, char* name, int value);
void put_int_cn(config_t *config, char* name, int value);

int has_float(config_t *config, char* name);
float get_float(config_t *config, char* name);
void put_float(config_t *config, char* name, float value);
void put_float_cn(config_t *config, char* name, float value);

#endif

