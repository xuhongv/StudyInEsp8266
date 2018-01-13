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
#include "hal_key.h"

//MQTT客户端
MQTT_Client mqttClient;
typedef unsigned long u32_t;
static ETSTimer sntp_timer;

//检查wifi是否已经连接的定时器
os_timer_t checkTimer_wifistate;

//按键定义
#define GPIO_KEY_NUM                            1
#define KEY_0_IO_MUX                            PERIPHS_IO_MUX_MTMS_U
#define KEY_0_IO_NUM                            14
#define KEY_0_IO_FUNC                           FUNC_GPIO14
LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM];
LOCAL keys_typedef_t keys;

void sntpfn() {
	u32_t ts = 0;
	ts = sntp_get_current_timestamp();
	os_printf("current time : %s\n", sntp_get_real_time(ts));
	if (ts != 0) {
		os_timer_disarm(&sntp_timer);
		MQTT_Connect(&mqttClient);
	}
}

void wifiConnectCb(uint8_t status) {
	if (status == STATION_GOT_IP) {
		MQTT_Connect(&mqttClient);
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}

void mqttConnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	MQTT_Subscribe(client, "/xuhong/LED/in", 0); //订阅主题/xuhong/LED/in
}

void mqttDisconnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len,
		const char *data, uint32_t data_len) {

	MQTT_Client* client = (MQTT_Client*) args;

	//如果接收到指令是1，GPIO15输出为低,也就是LED开灯，同时发布消息，主题是/xuhong/LED/out，信息是LED status is open ...
	if (data[0] == '1') {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);
		MQTT_Publish(client, "/xuhong/LED/out", "LED status is open ...",
				strlen("LED status is open ..."), 0, 0);
	}

	//如果接收到指令是0，GPIO15为高,也就是LED关灯，同时发布消息，主题是/xuhong/LED/out，信息是LED status is off ...
	if (data[0] == '0') {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);
		MQTT_Publish(client, "/xuhong/LED/out", "LED status is off ...",
				strlen("LED status is off ..."), 0, 0);
	}

}

uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void) {
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

void Check_WifiState(void) {
	uint8 getState;
	struct ip_info ipConfig;
	wifi_get_ip_info(STATION_IF, &ipConfig);
	getState = wifi_station_get_connect_status();
	//查询 ESP8266 WiFi station 接口连接 AP 的状态
	if (getState == STATION_GOT_IP && ipConfig.ip.addr != 0) {


		os_printf("----- wifi 连接成功！ 红灯关闭啦啦！---\r\n");
		GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);

		os_printf("----- wifi 连接成功！ 断开定时器啦！---\r\n");
		os_timer_disarm(&checkTimer_wifistate);

		sntp_setservername(0, "pool.ntp.org"); // set sntp server after got ip address
		sntp_init();
		os_timer_disarm(&sntp_timer);
		os_timer_setfn(&sntp_timer, (os_timer_func_t *) sntpfn, NULL);
		os_timer_arm(&sntp_timer, 1000, 1); //1s

	}
}

LOCAL void ICACHE_FLASH_ATTR keyLongPress(void) {

}

LOCAL void ICACHE_FLASH_ATTR keyShortPress(void) {
	os_printf("---------- 按键触发 ，开始进去SmartConfig配网 \n\n\n-----");
	smartconfig_init();
}

//按键初始化
LOCAL void ICACHE_FLASH_ATTR keyInit(void) {
	singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
			keyLongPress, keyShortPress);
	keys.singleKey = singleKey;
	keyParaInit(&keys);
}

void user_init(void) {
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(60000);

	//LED初始化
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15); //选择GPIO15，此GPIO连接是绿灯
	GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1); //默认GPIO15为高,也就是关灯

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); //选择GPIO12，此GPIO连接是红灯
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0); //默认GPIO12为低,也就是开灯，表示配网不成功！



	CFG_Load();
	//开始MQTT连接
	MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port,
			sysCfg.security);
	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user,
			sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
	MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);

	os_timer_disarm(&checkTimer_wifistate);	//启动定时器前先取消定时器定时
	os_timer_setfn(&checkTimer_wifistate, (os_timer_func_t *) Check_WifiState,
	NULL);	//设置定时器回调函数
	os_timer_arm(&checkTimer_wifistate, 1000, 1);	//启动定时器
	keyInit();
	INFO("\r\nSystem started ...\r\n");
}
