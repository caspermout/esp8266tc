
#include <c_types.h>
#include <string.h>

#include "casper.h"
#include "sensors.h"


config_t config_s;
config_t sensors_s;
config_t sensors_cal_s;

config_t *config = &config_s;
config_t *sensors = &sensors_s;
config_t *sensors_cal = &sensors_cal_s;




void config_clear(config_t *config) {

	int i;
	for(i = 0; i < MAX_ITEMS; i++) {
//		config->names[i] = NULL; (zodat geen memory leak bij de put_..._cn dingen
		config->values_s[i] = NULL;
		config->values_i[i] = 0;
		config->values_f[i] = 0.0;
	}
}

int config_count(config_t *config) {

	int i;
	for(i = 0; i < MAX_ITEMS; i++) {
		if(config->names[i] == NULL) {
			break;
		}
	}
	return i;
}

char* config_name(config_t *config, int idx) {
	return config->names[idx];
}

int config_idx(config_t *config, char *name) {

	int i;
	for(i = 0; i < MAX_ITEMS; i++) {
		if(config->names[i] == NULL) {
			continue;
		}
		
		if(config->names[i] == name || (config->names[i] != NULL && strcmp(config->names[i], name) == 0)) {
			return i;
		}
	}
	return -1;
}

int map_type(config_t *config, char* name) {
	int idx = config_idx(config, name);
	if(idx < 0) {
		return -1;
	}

	if(config->values_s[idx] != NULL) {
		return 1;
	}

	if(config->values_f[idx] != 0.0) {
		return 2;
	}

	if(config->values_i[idx] != 0) {
		return 3;
	}

	return -1;
}


int get_int(config_t *config, char* name) {
	int idx = config_idx(config, name);
	if(idx < 0) {
		return -1;
	}

	return config->values_i[idx];
}

void put_int(config_t *config, char* name, int value) {
	int idx = config_idx(config, name);

	if(idx < 0 || idx >= MAX_ITEMS) {
		idx = config_count(config);
		if(idx < 0) {
			return; //TODO: wat nu??
		}

		config->names[idx] = name;
	}

	config->values_i[idx] = value;
}

void put_int_cn(config_t *config, char* name, int value) {
	int idx = config_idx(config, name);

	if(idx < 0 || idx >= MAX_ITEMS) {
		idx = config_count(config);
		if(idx < 0) {
			return; //TODO: wat nu??
		}

		char *name_cn = malloc(strlen(name)+1);
		strcpy(name_cn, name);

		config->names[idx] = name_cn;
	}

	config->values_i[idx] = value;
}

int has_float(config_t *config, char* name) {
	int idx = config_idx(config, name);
	if(idx < 0) {
		return 0;
	}

	return 1;
}

float get_float(config_t *config, char* name) {
	int idx = config_idx(config, name);
	if(idx < 0) {
		return -1;
	}

	return config->values_f[idx];
}

void put_float(config_t *config, char* name, float value) {
	int idx = config_idx(config, name);

	if(idx < 0 || idx >= MAX_ITEMS) {
		idx = config_count(config);
		if(idx < 0) {
			return; //TODO: wat nu??
		}

		config->names[idx] = name;
	}

	config->values_f[idx] = value;
}

void put_float_cn(config_t *config, char* name, float value) {
	int idx = config_idx(config, name);

	if(idx < 0 || idx >= MAX_ITEMS) {
		idx = config_count(config);
		if(idx < 0) {
			return; //TODO: wat nu??
		}

		char *name_cn = malloc(strlen(name)+1);
		strcpy(name_cn, name);

		config->names[idx] = name_cn;
	}

	config->values_f[idx] = value;
}

char* get_str(config_t *config, char* name) {
	int idx = config_idx(config, name);
	if(idx < 0) {
		return NULL;
	}

	return config->values_s[idx];
}

void put_str(config_t *config, char *name, char *value) {
	int idx = config_idx(config, name);

	if(idx < 0 || idx >= MAX_ITEMS) {
		idx = config_count(config);
		if(idx < 0) {
			return; //TODO: wat nu??
		}

		config->names[idx] = name;
	}

	config->values_s[idx] = value;
}

void put_str_cnv(config_t *config, char *name, char *value) {
	int idx = config_idx(config, name);

	if(idx < 0 || idx >= MAX_ITEMS) {
		idx = config_count(config);
		if(idx < 0) {
			return; //TODO: wat nu??
		}

		char *name_cn = malloc(strlen(name)+1);
		strcpy(name_cn, name);

		config->names[idx] = name_cn;
	}


	// TODO: hoe voorkomen we memory leak? :P
	char *value_cn = malloc(strlen(value)+1);
	strcpy(value_cn, value);

	config->values_s[idx] = value_cn;
}


