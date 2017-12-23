#ifndef WEBSERVER_H
#define WEBSERVER_H


typedef struct req_t {

	int active;
	int type;
	uint32_t offset;
	uint32_t offset2;
	char id[128]; // remote ip en port 1.1.1.1:1

	char* buffer;

} req_t;


void webserver_init();


#endif

