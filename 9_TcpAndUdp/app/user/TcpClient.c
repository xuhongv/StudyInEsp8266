
#include "driver/uart.h"  //串口0需要的头文件
#include "osapi.h"  //串口1需要的头文件
#include "user_interface.h" //WIFI连接需要的头文件
#include "espconn.h"//TCP连接需要的头文件
#include "mem.h" //系统操作需要的头文件
#include "gpio.h"

os_timer_t checkTimer_wifistate;
struct espconn user_tcp_conn;

void ICACHE_FLASH_ATTR user_tcp_sent_cb(void *arg)  //发送
{
	os_printf("发送数据成功！");
}
void ICACHE_FLASH_ATTR user_tcp_discon_cb(void *arg)  //断开
{
	os_printf("断开连接成功！");
}
void ICACHE_FLASH_ATTR user_tcp_recv_cb(void *arg,  //接收
		char *pdata, unsigned short len) {

	os_printf("收到数据：%s\r\n", pdata);
	espconn_sent((struct espconn *) arg, "0", strlen("0"));

}
void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err) //注册 TCP 连接发生异常断开时的回调函数，可以在回调函数中进行重连
{
	os_printf("连接错误，错误代码为%d\r\n", err);
	espconn_connect((struct espconn *) arg);
}
void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg)  //注册 TCP 连接成功建立后的回调函数
{
	struct espconn *pespconn = arg;
	espconn_regist_recvcb(pespconn, user_tcp_recv_cb);  //接收
	espconn_regist_sentcb(pespconn, user_tcp_sent_cb);  //发送
	espconn_regist_disconcb(pespconn, user_tcp_discon_cb);  //断开
	espconn_sent(pespconn, "8226", strlen("8226"));

}

void ICACHE_FLASH_ATTR my_station_init(struct ip_addr *remote_ip,
		struct ip_addr *local_ip, int remote_port) {
	user_tcp_conn.proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));  //分配空间
	user_tcp_conn.type = ESPCONN_TCP;  //设置类型为TCP协议
	os_memcpy(user_tcp_conn.proto.tcp->local_ip, local_ip, 4);
	os_memcpy(user_tcp_conn.proto.tcp->remote_ip, remote_ip, 4);
	user_tcp_conn.proto.tcp->local_port = espconn_port();  //本地端口
	user_tcp_conn.proto.tcp->remote_port = remote_port;  //目标端口
	//注册连接成功回调函数和重新连接回调函数
	espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb);//注册 TCP 连接成功建立后的回调函数
	espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb);//注册 TCP 连接发生异常断开时的回调函数，可以在回调函数中进行重连
	//启用连接
	espconn_connect(&user_tcp_conn);
}

void Check_WifiState(void) {
	uint8 getState;
	getState = wifi_station_get_connect_status();
	//查询 ESP8266 WiFi station 接口连接 AP 的状态
	if (getState == STATION_GOT_IP) {
		os_printf("WIFI连接成功！\r\n");
		os_timer_disarm(&checkTimer_wifistate);
		struct ip_info info;
		const char remote_ip[4] = { 192, 168, 43, 1 };//目标IP地址,必须要先从手机获取，否则连接失败.
		wifi_get_ip_info(STATION_IF, &info);	//查询 WiFi模块的 IP 地址
		my_station_init((struct ip_addr *) remote_ip, &info.ip, 6000);//连接到目标服务器的6000端口
 }
}

void tcp_client_init()	//初始化
{

	wifi_set_opmode(0x01);	//设置为STATION模式

	struct station_config stationConf;
	os_strcpy(stationConf.ssid, "meizu");	  //改成你自己的   路由器的用户名
	os_strcpy(stationConf.password, "12345678"); //改成你自己的   路由器的密码
	wifi_station_set_config(&stationConf);	//设置WiFi station接口配置，并保存到 flash
	wifi_station_connect();	//连接路由器

	os_timer_disarm(&checkTimer_wifistate);	//取消定时器定时
	os_timer_setfn(&checkTimer_wifistate, (os_timer_func_t *) Check_WifiState,
	NULL);	//设置定时器回调函数
	os_timer_arm(&checkTimer_wifistate, 500, 1);	//启动定时器，单位：毫秒
}

