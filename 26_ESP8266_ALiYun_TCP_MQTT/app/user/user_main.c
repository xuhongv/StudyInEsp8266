/* main.c -- MQTT client example
 *
 * Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of Redis nor the names of its contributors may be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
 *******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void) {
	enum flash_size_map size_map = system_get_flash_size_map();
	uint32 rf_cal_sec = 0;

	switch (size_map) {
	case FLASH_SIZE_4M_MAP_256_256:
		rf_cal_sec = 128 - 5;
		break;

	case FLASH_SIZE_8M_MAP_512_512:
		rf_cal_sec = 256 - 5;
		break;

	case FLASH_SIZE_16M_MAP_512_512:
	case FLASH_SIZE_16M_MAP_1024_1024:
		rf_cal_sec = 512 - 5;
		break;

	case FLASH_SIZE_32M_MAP_512_512:
	case FLASH_SIZE_32M_MAP_1024_1024:
		rf_cal_sec = 1024 - 5;
		break;

	case FLASH_SIZE_64M_MAP_1024_1024:
		rf_cal_sec = 2048 - 5;
		break;
	case FLASH_SIZE_128M_MAP_1024_1024:
		rf_cal_sec = 4096 - 5;
		break;
	default:
		rf_cal_sec = 0;
		break;
	}

	return rf_cal_sec;
}

MQTT_Client mqttClient;
static ETSTimer WenDuTimer;

static void ICACHE_FLASH_ATTR sendMsg() {
	char *random = "13";
	os_sprintf(random, "%d", os_random() % 100);
	INFO("WenDu:%s\r\n", random);

	u8 topic[48];
	os_sprintf(topic, "/%s/%s/update", ProductKey, DeviceName);

	MQTT_Publish(&mqttClient, topic, random,
	os_strlen(random), 0, 0);
}

void wifiConnectCb(uint8_t status) {
	if (status == STATION_GOT_IP) {
		MQTT_Connect(&mqttClient);
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}
void mqttConnectedCb(uint32_t *args) {
	MQTT_Client *client = (MQTT_Client *) args;
	INFO("MQTT: Connected succeed !!!\r\n");

	u8 topic[48];
	os_sprintf(topic, "/%s/%s/get", ProductKey, DeviceName);

	MQTT_Subscribe(client, topic, 0);

	os_timer_disarm(&WenDuTimer);
	os_timer_setfn(&WenDuTimer, (os_timer_func_t *) sendMsg, NULL);
	os_timer_arm(&WenDuTimer, 10000, 1);
}

void mqttDisconnectedCb(uint32_t *args) {
	MQTT_Client *client = (MQTT_Client *) args;
	INFO("MQTT: Disconnected\r\n");
	os_timer_disarm(&WenDuTimer);
}

void mqttPublishedCb(uint32_t *args) {
	MQTT_Client *client = (MQTT_Client *) args;
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char *topic, uint32_t topic_len,
		const char *data, uint32_t data_len) {
	char *topicBuf = (char *) os_zalloc(topic_len + 1), *dataBuf =
			(char *) os_zalloc(data_len + 1);

	MQTT_Client *client = (MQTT_Client *) args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);

	os_free(topicBuf);
	os_free(dataBuf);
}

uint8 tempMessage[6];
void ICACHE_FLASH_ATTR initGetMacAdress(void) {

	//获取station下的mac地址
	uint8 macAdress[6];
	if (!wifi_get_macaddr(STATION_IF, macAdress)) {
		INFO("Failed to get mac... \r\n");
	} else {
		INFO("succeed to get mac...\r\n");
	}
	os_sprintf(tempMessage, "%02x%02x%02x%02x%02x%02x", macAdress[0],
			macAdress[1], macAdress[2], macAdress[3], macAdress[4],
			macAdress[5]);
	INFO(" MacAdress: %s\r\n", tempMessage);
}

static uint8_t randclient[60];
void user_init(void) {

	//获取mac地址··············
	initGetMacAdress();

	//获取随机码··············
	int randtimestamp = os_random() % 1000;

	uint8_t mydigest[20], str[40], dst[41], message[80];

	os_delay_us(60000);

	//tempMessage是设备的mac地址，这个可以自定义为mac地址或者sn序列号
	os_sprintf(message, "clientId%sdeviceName%sproductKey%stimestamp%d",
			tempMessage, DeviceName, ProductKey, randtimestamp);

	os_sprintf(randclient, "%s|securemode=3,signmethod=hmacsha1,timestamp=%d|",
			tempMessage, randtimestamp);

	os_printf(" message: %s \n", message);
	os_printf(" before ssl_hmac_sha1 client_id: %s \n", randclient);

	os_delay_us(60000); //不加延迟貌似会影响randclient

	//	待加密的信息 message 、密钥 secret、 加密之后的信息 mydigest
	ssl_hmac_sha1(message, os_strlen(message), DeviceSecret,
	os_strlen(DeviceSecret), mydigest);

	//  再一次转换为字符串
	int i;
	for (i = 0; i < 20; i++) {
		str[2 * i] = mydigest[i] >> 4;
		str[2 * i + 1] = mydigest[i] & 0xf;
	}
	for (i = 0; i < 40; i++) {
		os_sprintf(&dst[i], "%x", str[i]);
	}
	dst[40] = '\0';

	os_delay_us(60000);

	//打印下
	os_printf(" after client_id -> %s\r\n", randclient);
	os_printf(" user -> %s\r\n", MQTT_USER);
	os_printf(" pass -> %s\r\n", dst);

	MQTT_InitConnection(&mqttClient, MQTT_HOST, MQTT_PORT, DEFAULT_SECURITY);
	MQTT_InitClient(&mqttClient, randclient, MQTT_USER, dst, MQTT_KEEPALIVE, 1);

	MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);

	WIFI_Connect(STA_SSID, STA_PASS, wifiConnectCb);

	INFO("\r\nSystem started ...\r\n");
}
