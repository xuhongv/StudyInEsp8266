/*
 * user_main.c
 *
 *  Created on: 2015年7月13日
 *      Author: Administrator
 */
#include "driver/uart.h"
#include "user_main.h"

os_timer_t checkTimer_wifistate;

void Check_WifiState(void) {
	uint8 getState;
	getState = wifi_station_get_connect_status();

	//如果状态正确，证明已经成功连接到路由器
	if (getState == STATION_GOT_IP) {
		os_printf("WIFI连接成功！");
		os_timer_disarm(&checkTimer_wifistate);
		os_timer_disarm(&connect_timer);

		uint8 status = wifi_station_get_connect_status();
		if (status == STATION_GOT_IP) {
			uart0_sendStr("WIFI连接成功！");
			startHttpQuestByGET(
					"https://api.seniverse.com/v3/weather/daily.json?key=rrpd2zmqkpwlsckt&location=guangzhou&language=en&unit=c&start=0&days=3");
			return;
		}
	}

}
void user_init() {

	uart_init(57600, 57600);
	wifi_set_opmode(0x01); //设置为STATION模式
	struct station_config stationConf;
	os_strcpy(stationConf.ssid, "TP-LINK_AliYun");	  //改成你自己的   路由器的用户名
	os_strcpy(stationConf.password, "aliyun_123456"); //改成你自己的   路由器的密码
	wifi_station_set_config(&stationConf); //设置WiFi station接口配置，并保存到 flash
	wifi_station_connect(); //连接路由器

	os_timer_disarm(&checkTimer_wifistate); //取消定时器定时
	os_timer_setfn(&checkTimer_wifistate, (os_timer_func_t *) Check_WifiState,
	NULL); //设置定时器回调函数
	os_timer_arm(&checkTimer_wifistate, 500, true); //启动定时器，单位：毫秒
	os_printf("57600 end... \n\r");
}
void user_rf_pre_init() {
}

