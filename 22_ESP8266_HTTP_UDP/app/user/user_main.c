
#include "driver/uart.h"  //串口0需要的头文件
#include "osapi.h"  //串口1需要的头文件
#include "user_interface.h"  //串口1需要的头文件



os_timer_t checkTimer;



void user_rf_pre_init(void)
{

}


LOCAL void checkWifiStation() {

	//查询 ESP8266 WiFi station 接口连接 AP 的状态
	if (wifi_station_get_connect_status() == STATION_GOT_IP) {
		sntp_setservername(0, "0.cn.pool.ntp.org");
		sntp_setservername(1, "1.cn.pool.ntp.org");
		sntp_setservername(2, "2.cn.pool.ntp.org");
		sntp_init();
		udpwificfgg_init();
		gpioctr_init();
		os_timer_disarm(&checkTimer); //取消定时器定时
	}
}



void user_init(void) {

	os_printf("\r\n\r\n");
	os_printf("SDK version:%s\n", system_get_sdk_version());
	os_printf("\r\n\r\n");

	uart_init(115200, 115200);
	wifi_set_opmode(STATIONAP_MODE);

	os_timer_disarm(&checkTimer);   //取消定时器定时
	os_timer_setfn(&checkTimer, (os_timer_func_t *) checkWifiStation,
	NULL);    //设置定时器回调函数
	os_timer_arm(&checkTimer, 500, 1);      //启动定时器，单位：毫秒

}

