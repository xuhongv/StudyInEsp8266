#include "driver/uart.h"  //串口0需要的头文件
#include "osapi.h"  //串口1需要的头文件
#include "user_interface.h" //WIFI连接需要的头文件
#include "espconn.h"//TCP连接需要的头文件
#include "mem.h" //系统操作需要的头文件
#include "gpio.h"

struct espconn user_tcp_espconn;

void ICACHE_FLASH_ATTR server_recv(void *arg, char *pdata, unsigned short len) {
	os_printf("收到PC发来的数据：%s", pdata);
	espconn_sent((struct espconn *) arg, "已经收到啦！", strlen("已经收到啦！"));

}
void ICACHE_FLASH_ATTR server_sent(void *arg) {
	os_printf("发送成功！");
}
void ICACHE_FLASH_ATTR server_discon(void *arg) {
	os_printf("连接已经断开！");
}

void ICACHE_FLASH_ATTR server_listen(void *arg)  //注册 TCP 连接成功建立后的回调函数
{
	struct espconn *pespconn = arg;
	espconn_regist_recvcb(pespconn, server_recv);  //接收
	espconn_regist_sentcb(pespconn, server_sent);  //发送
	espconn_regist_disconcb(pespconn, server_discon);  //断开
}
void ICACHE_FLASH_ATTR server_recon(void *arg, sint8 err) //注册 TCP 连接发生异常断开时的回调函数，可以在回调函数中进行重连
{
	os_printf("连接错误，错误代码为：%d\r\n", err); //%d,用来输出十进制整数
}

void Inter213_InitTCP(uint32_t Local_port) {
	user_tcp_espconn.proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp)); //分配空间
	user_tcp_espconn.type = ESPCONN_TCP; //设置类型为TCP协议
	user_tcp_espconn.proto.tcp->local_port = Local_port; //本地端口

	espconn_regist_connectcb(&user_tcp_espconn, server_listen); //注册 TCP 连接成功建立后的回调函数
	espconn_regist_reconcb(&user_tcp_espconn, server_recon); //注册 TCP 连接发生异常断开时的回调函数，可以在回调函数中进行重连
	espconn_accept(&user_tcp_espconn); //创建 TCP server，建立侦听
	espconn_regist_time(&user_tcp_espconn, 180, 0); //设置超时断开时间 单位：秒，最大值：7200 秒

}

void WIFI_Init() {
	struct softap_config apConfig;
	wifi_set_opmode(0x02);    //设置为AP模式，并保存到 flash
	apConfig.ssid_len = 10;						//设置ssid长度
	os_strcpy(apConfig.ssid, "xuhongLove");	    //设置ssid名字
	os_strcpy(apConfig.password, "12345678");	//设置密码
	apConfig.authmode = 3;                      //设置加密模式
	apConfig.beacon_interval = 100;            //信标间隔时槽100 ~ 60000 ms
	apConfig.channel = 1;                      //通道号1 ~ 13
	apConfig.max_connection = 4;               //最大连接数
	apConfig.ssid_hidden = 0;                  //隐藏SSID

	wifi_softap_set_config(&apConfig);		//设置 WiFi soft-AP 接口配置，并保存到 flash
}

void tcp_service_init()		//初始化
{
	WIFI_Init();
	Inter213_InitTCP(8266);		//本地端口
}

