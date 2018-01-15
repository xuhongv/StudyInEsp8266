#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"

#include "user_interface.h"
#include "smartconfig.h"

#define DEVICE_TYPE 		"gh_9e2cff3dfa51" //wechat public number
#define DEVICE_ID 			"122475" //model ID

#define DEFAULT_LAN_PORT 	12476

uint8_t lan_buf[200];
uint16_t lan_buf_len;

void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata) {
	switch (status) {
	case SC_STATUS_WAIT:
		os_printf("SC_STATUS_WAIT\n");
		break;
	case SC_STATUS_FIND_CHANNEL:
		os_printf("SC_STATUS_FIND_CHANNEL\n");
		break;
	case SC_STATUS_GETTING_SSID_PSWD:
		os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
		sc_type *type = pdata;
		if (*type == SC_TYPE_ESPTOUCH) {
			os_printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
		} else {
			os_printf("SC_TYPE:SC_TYPE_AIRKISS\n");
		}
		break;
	case SC_STATUS_LINK:
		os_printf("SC_STATUS_LINK\n");
		struct station_config *sta_conf = pdata;

		wifi_station_set_config(sta_conf);
		wifi_station_disconnect();
		wifi_station_connect();
		break;
	case SC_STATUS_LINK_OVER:
		os_printf("SC_STATUS_LINK_OVER\n");
		if (pdata != NULL) {
			//SC_TYPE_ESPTOUCH
			uint8 phone_ip[4] = { 0 };

			os_memcpy(phone_ip, (uint8*) pdata, 4);
			os_printf("Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1],
					phone_ip[2], phone_ip[3]);
		}
		smartconfig_stop();
		break;
	}

}

void smartconfig_init(void) {
	wifi_set_opmode(STATION_MODE);
	smartconfig_set_type(SC_TYPE_ESPTOUCH);
	smartconfig_start(smartconfig_done);
}
