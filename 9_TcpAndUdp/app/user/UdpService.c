#include "driver/uart.h"  //串口0需要的头文件
#include "osapi.h"  //串口1需要的头文件
#include "user_interface.h" //WIFI连接需要的头文件
#include "espconn.h"//TCP连接需要的头文件
#include "mem.h" //系统操作需要的头文件
#include "gpio.h"

struct espconn user_udp_espconn;

static void Inter213_Receive(void *arg, char *pdata, unsigned short len) {  //接收
	os_printf("收到数据：%s\r\n", pdata); // %s,用来输出一个字符串
	espconn_sent((struct espconn *) arg, "已经收到", strlen("已经收到"));

}
static void Inter213_Send_Cb(void *arg) {  //发送
	os_printf("\r\n已发送\r\n");

}

void Inter213_InitUDP(int32_t Remote_port, uint32_t Local_port) {
	user_udp_espconn.proto.udp = (esp_udp *) os_zalloc(sizeof(esp_udp));  //分配空间
	user_udp_espconn.type = ESPCONN_UDP;  //设置类型为UDP协议
	user_udp_espconn.proto.udp->local_port = Local_port;  //本地端口
	user_udp_espconn.proto.udp->remote_port = Remote_port;  //目标端口

	espconn_regist_recvcb(&user_udp_espconn, Inter213_Receive);  //接收
	espconn_regist_sentcb(&user_udp_espconn, Inter213_Send_Cb);  //发送
	espconn_create(&user_udp_espconn);  //建立UDP传输

}

void WIFI_Init() {
	struct softap_config apConfig;

	wifi_set_opmode(0x02);  //设置为AP模式，并保存到 flash

	apConfig.ssid_len = 10;				        //设置ssid长度
	os_strcpy(apConfig.ssid, "meizu");	//设置ssid名字
	os_strcpy(apConfig.password, "12345678");	//设置密码
	apConfig.authmode = 3;                      //设置加密模式
	apConfig.beacon_interval = 100;            //信标间隔时槽100 ~ 60000 ms
	apConfig.channel = 1;                      //通道号1 ~ 13
	apConfig.max_connection = 4;               //最大连接数
	apConfig.ssid_hidden = 0;                  //隐藏SSID

	wifi_softap_set_config(&apConfig);		//设置 WiFi soft-AP 接口配置，并保存到 flash
}
void udp_services_init()		//初始化
{
	os_printf("\r\n udp_services_init ... \r\n");
	WIFI_Init();
	Inter213_InitUDP(8266, 8266);		//目标端口，本地端口

}

