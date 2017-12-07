
#include "driver/uart.h"  //串口0需要的头文件
#include "osapi.h"  //串口1需要的头文件
#include "user_interface.h" //WIFI连接需要的头文件
#include "espconn.h"//TCP连接需要的头文件
#include "mem.h" //系统操作需要的头文件


struct espconn user_udp_espconn;
os_timer_t checkTimer_wifistate;

void ICACHE_FLASH_ATTR user_udp_sent_cb(void *arg)   //发送
{
	os_printf("\r\n发送成功！\r\n");

}

void ICACHE_FLASH_ATTR user_udp_recv_cb(void *arg,    //接收
		char *pdata, unsigned short len) {
	os_printf("接收数据：%s", pdata);

	//每次发送数据确保参数不变
	user_udp_espconn.proto.udp = (esp_udp *) os_zalloc(sizeof(esp_udp));
	user_udp_espconn.type = ESPCONN_UDP;
	user_udp_espconn.proto.udp->local_port = 2000;
	user_udp_espconn.proto.udp->remote_port = 8686;
	const char udp_remote_ip[4] = { 255, 255, 255, 255 };
	os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4);

	espconn_sent((struct espconn *) arg, "已经收到啦！", strlen("已经收到啦!"));
}

void Check_WifiState(void) {

	uint8 getState = wifi_station_get_connect_status();

	//如果状态正确，证明已经连接
	if (getState == STATION_GOT_IP) {

		os_printf("WIFI连接成功！");
		os_timer_disarm(&checkTimer_wifistate);

		wifi_set_broadcast_if(0x01);	 //设置 ESP8266 发送 UDP广播包时，从 station 接口发送
		user_udp_espconn.proto.udp = (esp_udp *) os_zalloc(sizeof(esp_udp));//分配空间
		user_udp_espconn.type = ESPCONN_UDP;	 		  //设置类型为UDP协议
		user_udp_espconn.proto.udp->local_port = 2000;	 		  //本地端口
		user_udp_espconn.proto.udp->remote_port = 8686;	 		  //目标端口
		const char udp_remote_ip[4] = { 255, 255, 255, 255 };	 	//目标IP地址（广播）
		os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4);

		espconn_regist_recvcb(&user_udp_espconn, user_udp_recv_cb);	 		//接收
		espconn_regist_sentcb(&user_udp_espconn, user_udp_sent_cb);	 		//发送
		espconn_create(&user_udp_espconn);	 		  //建立 UDP 传输
		espconn_sent(&user_udp_espconn, "连接服务器", strlen("连接服务器"));

	}
}

void udp_client_init() //初始化
{
	wifi_set_opmode(0x01); //设置为STATION模式
	struct station_config stationConf;
	os_strcpy(stationConf.ssid, "meizu");	  //改成你要连接的 路由器的用户名
	os_strcpy(stationConf.password, "12345678"); //改成你要连接的路由器的密码

	wifi_station_set_config(&stationConf);	  //设置WiFi station接口配置，并保存到 flash
	wifi_station_connect();	  //连接路由器
	os_timer_disarm(&checkTimer_wifistate);	  //取消定时器定时
	os_timer_setfn(&checkTimer_wifistate, (os_timer_func_t *) Check_WifiState,
	NULL);	  //设置定时器回调函数
	os_timer_arm(&checkTimer_wifistate, 500, 1);	  //启动定时器，单位：毫秒
}

