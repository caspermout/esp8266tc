
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

#include "sntp.h"
#include "espconn.h"

#ifdef SERVER_SSL_ENABLE
#include "ssl/cert.h"
#include "ssl/private_key.h"
#endif

#include "user_sensors.h"
#include "vfs.h"
#include "ff.h"
#include "disk_sdcard.h"
#include "io_gpio.h"
#include "webserver.h"
#include "log.h"

#define REQ_REQUEST 0
#define REQ_WEETNIE 1
#define REQ_LAST 2
#define REQ_INDEX 3

#define MAX_REQUESTS 10


req_t requests[MAX_REQUESTS];

// TODO:  application/x-www-form-urlencoded

LOCAL ICACHE_FLASH_ATTR
req_t* get_req(struct espconn *pesp_conn) {
	
	char id[128];

	os_sprintf(id, "%d.%d.%d.%d:%d", pesp_conn->proto.tcp->remote_ip[0],
    		pesp_conn->proto.tcp->remote_ip[1],pesp_conn->proto.tcp->remote_ip[2],
    		pesp_conn->proto.tcp->remote_ip[3],pesp_conn->proto.tcp->remote_port);

	
	int i;
	for(i = 0; i < 10; i++) {
//		if(requests[i].active == 1) {
//			os_printf("Connection: %d - %32s\n", i, requests[i].id);
//		}
		if(requests[i].active == 1 && os_strncmp(id, requests[i].id, 128) == 0) {
			return &requests[i];
		}
	}
	os_printf("Connection %s not found\n", id);
	return NULL;
}


LOCAL void ICACHE_FLASH_ATTR
webserver_disconnect_int(void *arg)
{
    struct espconn *pesp_conn = arg;

os_printf("disconnect_int %x\n", pesp_conn); 

#ifdef SERVER_SSL_ENABLE
    int r = espconn_secure_disconnect(pesp_conn);
#else
    int r = espconn_disconnect(pesp_conn);
#endif

os_printf("disconnect_int %x = %d\n", pesp_conn, r); 

}

LOCAL ICACHE_FLASH_ATTR
void req_free(req_t* req) {
		req->active = 0;
		req->type = 0;
		if(req->buffer != NULL) {
			os_free(req->buffer);
			req->buffer = NULL;
		}
		os_printf("Free request: %s\n", req->id);
}

LOCAL void ICACHE_FLASH_ATTR
webserver_disconnect(void *arg) {

    struct espconn *pesp_conn = arg;

	os_printf("disconnect %x\n", pesp_conn); 

	req_t* req = get_req(pesp_conn);

	if(req != NULL) {
		req_free(req);
	}

	system_os_post(USER_TASK_PRIO_0,	(os_signal_t)&webserver_disconnect_int,	(os_param_t)pesp_conn);
}



LOCAL void ICACHE_FLASH_ATTR
webserver_recv(void *arg, char *pusrdata, unsigned short length)
{
    struct espconn *pesp_conn = arg;

//	os_printf("recv len %x : %u \n", pesp_conn, length);


	req_t* req = get_req(pesp_conn);
	if(req == NULL) {
		os_printf("No request object\n");
		webserver_disconnect(pesp_conn);
		return;
	}

	if(req->type != REQ_REQUEST) {
		os_printf("recv %d %d\n", req->type, length);
		webserver_disconnect(pesp_conn);
		return;
	}

	if(req->buffer == NULL) {
		req->buffer = os_malloc(2048);
		req->buffer[0] = 0;
	}

	int buflen = os_strlen(req->buffer);

	if(buflen + length >= 2048) {
		req->type = REQ_WEETNIE;
		webserver_disconnect(pesp_conn);
		return;
	}

	os_memcpy(&req->buffer[buflen], pusrdata, length);
	buflen += length;
	req->buffer[buflen] = 0;


	char* end = os_strstr(req->buffer, "\r\n\r\n");
	if(end == NULL) {
		os_printf("Request not done yet %d\n", buflen);
		return;
	}
	os_printf("Request done! %d %x %d\n", buflen, end, end - req->buffer);

	// klaar met lezen van request... wat is het :D
	req->type = REQ_WEETNIE;

	char response[1024+1024+256];
	int resplen = 0;


	char* close = "HTTP/1.1 404 Not found ofzo\r\nConnection: close\r\n\r\n";

	char *reqbuf = req->buffer;

	if(os_strncmp(reqbuf, "GET ", 4) == 0) {
		os_printf("GET request\n");
		reqbuf += 4;
	}
	if(os_strncmp(reqbuf, "POST ", 5) == 0) {
		os_printf("POST request\n");
		reqbuf += 5;
	}

	char* urlend = os_strchr(reqbuf, ' ');
	char* url = NULL;
	char* query = NULL;
	if(urlend != NULL) {
		*urlend = 0;
		url = reqbuf;
		os_printf("URL '%s'\n", url);

		reqbuf = ++urlend;

		char* querybegin = os_strchr(url, '?');
		if(querybegin != NULL) {
			*querybegin = 0;
			query = ++querybegin;
			os_printf("Query '%s'\n", query);
		}
	}

//	os_printf("Rest:\n%s\n", reqbuf);


	if(os_strstr(reqbuf, "\r\nAuthorization: Basic Ymllcjptb3V3dDM=\r\n") == NULL) {

		char* auth = "HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Basic realm=\"Tja doe es auth\"\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";

		resplen = os_strlen(auth);
		os_memcpy(response, auth, resplen);
		goto finish;
	}

	char* ok200 = "HTTP/1.1 200 Ok\r\nConnection: close\r\n\r\n";

	if(url != NULL && os_strncmp(url, "/", 1+1) == 0) {

		if(req != NULL) {
			req->type = REQ_INDEX;
			req->offset = 0;

			resplen = os_strlen(ok200);
			os_memcpy(response, ok200, resplen);

		} else {
			resplen = os_strlen(close);
			os_memcpy(response, close, resplen);
		}

		while(query != NULL) {
//			os_printf("Rest:\n%s\n", query);
			char* eq = os_strchr(query, '=');
			if(eq != NULL) {
				*eq = 0;
				eq++;

				os_printf("param_raw '%s' = '%s'\n", query, eq);

				char* eqo;
				char* eqn;
				for(eqo = eq, eqn = eq; *eqo != 0; eqo++, eqn++) {

					if(*eqo == '+') {
						*eqn = ' ';

					} else if (*eqo == '%' && *(eqo+1) != 0 && *(eqo+2) != 0) {
					
						eqo++;
						char a = *eqo;
						eqo++;
						char b = *eqo;

						if(a == '0' && b == 'D') {
							*eqn = 0x0D;
						} else if(a == '0' && b == 'A') {
							*eqn = 0x0A;
						} else if(a == '3' && b == 'D') {
							*eqn = 0x3D;
						} else if(a == '2' && b == '0') {
							*eqn = 0x20;
						} else {
							*eqn = '%';
							eqn++;
							*eqn = a;
							eqn++;
							*eqn = b;
						}

					} else {
						*eqn = *eqo;
					}
				}
				*eqn = 0;

				os_printf("param '%s' = '%s'\n", query, eq);


				if(os_strcmp(query, "pin1") == 0 && os_strcmp(eq, "on") == 0) {
					io_gpio_write(1, 1);
				}
				if(os_strcmp(query, "pin1") == 0 && os_strcmp(eq, "off") == 0) {
					io_gpio_write(1, 0);
				}
				if(os_strcmp(query, "pin2") == 0 && os_strcmp(eq, "on") == 0) {
					io_gpio_write(2, 1);
				}
				if(os_strcmp(query, "pin2") == 0 && os_strcmp(eq, "off") == 0) {
					io_gpio_write(2, 0);
				}

				if(os_strcmp(query, "config") == 0) {

					FIL log;
					FRESULT r = f_open(&log, "/config.new", FA_CREATE_ALWAYS | FA_WRITE);
					if(r != FR_OK) {
						os_printf("Open file /config.new for writing failed: %d\n", r);
					} else {

						int btw = strlen(eq);
						int bw;
						int writeok = 1;
						r = f_write(&log, eq, btw, &bw);
						if(r != FR_OK || btw != bw) {
							writeok = 0;
							os_printf("Writing /config.new file failed: %d or (%d != %d)\n", r, btw, bw);
						}

						r = f_close(&log);
						if(r != FR_OK) {
							os_printf("Close /config.new file failed: %d\n", r);

						} else if(writeok == 1) {
							r = f_unlink("/config.old");
							if(r != FR_OK) {
								os_printf("Unlink /config.old failed: %d\n", r);
							}

							r = f_rename("/config", "/config.old");
							if(r != FR_OK) {
								os_printf("Rename /config to /config.old failed: %d\n", r);
							}

							r = f_rename("/config.new", "/config");
							if(r != FR_OK) {
								os_printf("Rename /config.new to /config failed: %d\n", r);
							} else {
								system_os_post(USER_TASK_PRIO_0, (os_signal_t)&reload_config, (os_param_t)NULL);
							}
						}
					}
				}
			}

			char* amp = os_strchr(query, '&');
			if(amp != NULL) {
				query = ++amp;
			} else {
				query = NULL;
			}

		}

	}

	if(url != NULL && os_strncmp(url, "/restart", 8+1) == 0) {

		system_restart();
	}

	if(url != NULL && os_strncmp(url, "/log", 4+1) == 0) {

		resplen = os_strlen(ok200);
		os_memcpy(response, ok200, resplen);

		resplen += casper_log_copy(&response[resplen], sizeof response - resplen);

	}

	if(url != NULL && os_strncmp(url, "/sd0.bin", 8+1) == 0) {

		int r = sdcard_readblocks(&card, 0, response, 512);
		r = sdcard_readblocks(&card, 512, &response[512], 512);
		if(r == 0) {
			resplen = 1024;
		} else {
			os_printf("read failed %d\n", r);
		}
	}

	if(url != NULL && os_strncmp(url, "/last.csv", 9+1) == 0) {

		req_t* req = get_req(pesp_conn);
		if(req != NULL) {
			req->type = REQ_LAST;
			req->offset = logpos2;

			resplen = os_strlen(ok200);
			os_memcpy(response, ok200, resplen);

		} else {
			resplen = os_strlen(close);
			os_memcpy(response, close, resplen);
		}

	}
	


finish:

if(resplen == 0) {
	resplen = os_strlen(close);
	os_memcpy(response, close, resplen);
}



#ifdef SERVER_SSL_ENABLE
        int r = espconn_secure_send(pesp_conn, response, resplen);
#else
        int r = espconn_send(pesp_conn, response, resplen);
#endif
	if(r != 0) {
		os_printf("failed to send data %d\n", r);
		webserver_disconnect(pesp_conn);
	}
}


LOCAL void ICACHE_FLASH_ATTR
webserver_sent(void *arg)
{
    struct espconn *pesp_conn = arg;

//    os_printf("%x sent %d.%d.%d.%d:%d \n", pesp_conn, pesp_conn->proto.tcp->remote_ip[0],
//    		pesp_conn->proto.tcp->remote_ip[1],pesp_conn->proto.tcp->remote_ip[2],
//    		pesp_conn->proto.tcp->remote_ip[3],pesp_conn->proto.tcp->remote_port);

	char response[1024];
	char *responseptr = response;
	int resplen = 0;


	req_t* req = get_req(pesp_conn);
	if(req == NULL) {
		os_printf("No active request\n");
		webserver_disconnect(pesp_conn);
		return;
	}

	if(req->type == REQ_LAST) {

		FIL log;
		FRESULT r = f_open(&log, "/log.csv", FA_READ);
		if(r != FR_OK) {
			os_printf("Open file /log.csv for reading failed: %d\n", r);
		} else {

			int size = f_size(&log);
			r = f_lseek(&log, req->offset);
			int cur = f_tell(&log);
			if(r != FR_OK || cur != req->offset) {
				os_printf("Seek to logpos2 failed: %d %d %d %d\n", r, cur, req->offset, size);

			} else {

//				os_printf("--------------\n");
				while (f_gets(&response[resplen], sizeof response - resplen, &log)) {

					int linelen = os_strlen(&response[resplen]);

					if(resplen + linelen + 10 > sizeof response) {
						// alleen hele regels, 10 is beetje willekeurig.... 1 werkte niet... :(
						break;
					}

//					os_printf(&response[resplen]);

					resplen += linelen;
					req->offset = f_tell(&log);
				}
//				os_printf("--------------\n");

				if(f_error(&log)) {
					os_printf("Error while reading file: %d\n", f_error(&log));
				}
			}

			r = f_close(&log);
			if(r != FR_OK) {
				os_printf("Close log file failed: %d\n", r);
			}
		}

	} else if(req->type == REQ_INDEX) {
		if(req->offset == 0) {
			char* index_buf0 = ""
"<h1>Mouwt bier!</h1>"
//<p>Heap: "..node.heap().."</p>";
//	    buf = buf.."<p>Uptime: "..tmr.time().."</p>";
"<p><a href=\"/\">refresh</a></p>"
"<p><a href=\"last.csv\">last.csv</a></p>"
//"<p><a href=\"log.csv\">log.csv</a></p>"
"<p>RELAY1 ";

			responseptr = index_buf0;
			resplen = os_strlen(index_buf0);
			req->offset++;
			req->offset2 = 0;

		} else if(req->offset == 1) {

			os_sprintf(response, "%d", io_gpio_read(1));
			resplen = os_strlen(response);
			req->offset++;
			req->offset2 = 0;

		} else if(req->offset == 2) {

			char* index_buf1 = " <a href=\"?pin1=on\"><button>ON</button></a>&nbsp;<a href=\"?pin1=off\"><button>OFF</button></a></p>"
				"<p>RELAY2 ";

			responseptr = index_buf1;
			resplen = os_strlen(index_buf1);
			req->offset++;
			req->offset2 = 0;

		} else if(req->offset == 3) {

			os_sprintf(response, "%d", io_gpio_read(2));
			resplen = os_strlen(response);
			req->offset++;
			req->offset2 = 0;

		} else if(req->offset == 4) {
			char* index_buf1 = ""
" <a href=\"?pin2=on\"><button>ON</button></a>&nbsp;<a href=\"?pin2=off\"><button>OFF</button></a></p>"
//"<p>goaltemp "..goaltemp.."</p>\n"
"<form method=\"GET\">"
"<textarea name=\"config\" rows=\"15\" cols=\"30\">"
;

			responseptr = index_buf1;
			resplen = os_strlen(index_buf1);
			req->offset++;
			req->offset2 = 0;

		} else if(req->offset == 5) {

			FIL conf;
			FRESULT r = f_open(&conf, "/config", FA_READ);
			if(r != FR_OK) {
				os_printf("Open file /config for reading failed: %d\n", r);
			} else {

				int size = f_size(&conf);
				if(req->offset2 < size) {
					r = f_lseek(&conf, req->offset2);
					int cur = f_tell(&conf);
					if(r != FR_OK || cur != req->offset2) {
						os_printf("Seek to logpos2 failed: %d %d %d %d\n", r, cur, req->offset2, size);

					} else {

//						os_printf("-------------- %d\n", resplen);
						while (f_gets(&response[resplen], sizeof response - resplen, &conf)) {

							int linelen = os_strlen(&response[resplen]);

							if(resplen + linelen + 10 > sizeof response) {
								// alleen hele regels, 10 is beetje willekeurig.... 1 werkte niet... :(
								break;
							}

//							os_printf("%d : %s\n", resplen, &response[resplen]);

							resplen += linelen;
							req->offset2 = f_tell(&conf);
						}
//						os_printf("--------------\n");

						if(f_error(&conf)) {
							os_printf("Error while reading file: %d\n", f_error(&conf));
						}
					}
				}

				r = f_close(&conf);
				if(r != FR_OK) {
					os_printf("Close log file failed: %d\n", r);
				}
			}
			if(resplen == 0) {
				os_printf("sending config file done\n");
				req->offset++;
			}
		}

		if(req->offset == 6) {
			char* index_buf2 = "</textarea><br/>"
			"<input type=\"submit\" value=\"saveconfig\"/>"
			"</form>\n"
			;
			responseptr = index_buf2;
			resplen = os_strlen(index_buf2);
			req->offset++;
		}
	}




	if(resplen == 0) {
		webserver_disconnect(pesp_conn);
		return;
	}


#ifdef SERVER_SSL_ENABLE
	int r = espconn_secure_send(pesp_conn, responseptr, resplen);
#else
	int r = espconn_send(pesp_conn, responseptr, resplen);
#endif
	if(r != 0) {
		os_printf("failed to send data %d\n", r);
		webserver_disconnect(pesp_conn);
	}
}



LOCAL ICACHE_FLASH_ATTR
void webserver_recon(void *arg, sint8 err)
{
    struct espconn *pesp_conn = arg;

    os_printf("webserver's %d.%d.%d.%d:%d err %d reconnect\n", pesp_conn->proto.tcp->remote_ip[0],
    		pesp_conn->proto.tcp->remote_ip[1],pesp_conn->proto.tcp->remote_ip[2],
    		pesp_conn->proto.tcp->remote_ip[3],pesp_conn->proto.tcp->remote_port, err);
}

LOCAL ICACHE_FLASH_ATTR
void webserver_discon(void *arg)
{
    struct espconn *pesp_conn = arg;

    os_printf("%x webserver's %d.%d.%d.%d:%d disconnect\n", pesp_conn, pesp_conn->proto.tcp->remote_ip[0],
        		pesp_conn->proto.tcp->remote_ip[1],pesp_conn->proto.tcp->remote_ip[2],
        		pesp_conn->proto.tcp->remote_ip[3],pesp_conn->proto.tcp->remote_port);

	// TODO: wat is espconn_delete en espconn_abort ?
	// denk dat delete = server stoppen?

	req_t* req = get_req(pesp_conn);

	if(req != NULL) {
		req_free(req);
		os_printf("webserver_discon req was nog active: %s\n", req->id);
	}
}

LOCAL void ICACHE_FLASH_ATTR
webserver_listen(void *arg)
{
    struct espconn *pesp_conn = arg;

	int i;
	for(i = 0; i < 10; i++) {
		if(requests[i].active == 0) {
			break;
		}
	}
	if(i == 10) {
		os_printf("Too many connections!\n");
	} else {

		requests[i].active = 1;
		requests[i].type = REQ_REQUEST;
		requests[i].offset = 0;
		requests[i].offset2 = 0;
		requests[i].buffer = NULL;
		os_sprintf(requests[i].id, "%d.%d.%d.%d:%d", pesp_conn->proto.tcp->remote_ip[0],
	    		pesp_conn->proto.tcp->remote_ip[1],pesp_conn->proto.tcp->remote_ip[2],
	    		pesp_conn->proto.tcp->remote_ip[3],pesp_conn->proto.tcp->remote_port);
		os_printf("New connection: %d - %s\n", i, requests[i].id);
	}

    espconn_regist_recvcb(pesp_conn, webserver_recv);
    espconn_regist_reconcb(pesp_conn, webserver_recon);
    espconn_regist_disconcb(pesp_conn, webserver_discon);

    espconn_regist_sentcb(pesp_conn, webserver_sent);
}

ICACHE_FLASH_ATTR
void webserver_init() 
{

	os_printf("time %d %d %d\r\n", system_get_time(), system_get_rtc_time(), sntp_get_current_timestamp());



	uint32 port = 888;

	espconn_tcp_set_max_con(MAX_REQUESTS);

    LOCAL struct espconn esp_conn;
    LOCAL esp_tcp esptcp;

    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = port;
    espconn_regist_connectcb(&esp_conn, webserver_listen);

#ifdef SERVER_SSL_ENABLE
    espconn_secure_set_default_certificate(default_certificate, default_certificate_len);
    espconn_secure_set_default_private_key(default_private_key, default_private_key_len);
    espconn_secure_accept(&esp_conn);
#else
    espconn_accept(&esp_conn);
#endif

	espconn_tcp_set_max_con_allow(&esp_conn, MAX_REQUESTS - 2); // 10 - 1 ... mya doe nog maar -1 :P

	// 10 s timeout?
	espconn_regist_time(&esp_conn, 10, 0);

}


