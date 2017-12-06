#include "driver/uart.h"  //串口0需要的头文件
#include "osapi.h"  //串口1需要的头文件
#include "user_interface.h" //WIFI连接需要的头文件
#include "espconn.h"//TCP连接需要的头文件
#include "mem.h" //系统操作需要的头文件
#include "gpio.h"  //端口控制需要的头文件
#include "cJSON.h"
#include "hal_key.h"

struct espconn user_tcp_espconn;
os_timer_t checkTimer_wifistate;

//按键定义
#define GPIO_KEY_NUM                            1
#define KEY_0_IO_MUX                            PERIPHS_IO_MUX_MTMS_U
#define KEY_0_IO_NUM                            14
#define KEY_0_IO_FUNC                           FUNC_GPIO14

LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM]; 
LOCAL keys_typedef_t keys; 

bool isConnected = false;

void Check_WifiState(void) {

	uint8 status = wifi_station_get_connect_status();

	if (status == STATION_GOT_IP) {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
		wifi_set_opmode(0x01);
	} else {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0); //GPIO15 低电平输出
		GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1); //GPIO15 低电平输出
	}
}

void wifiStationConnect(char *ssid, char *psw) {
	os_printf(" wifiStationConnect name:%s \n", ssid);
	os_printf(" wifiStationConnect psw :%s \n", psw);
	struct station_config stationConf;
	os_strcpy(stationConf.ssid, ssid);	  //路由器的用户名
	os_strcpy(stationConf.password, psw); //路由器的密码
	wifi_station_set_config(&stationConf); //设置WiFi station接口配置，并保存到 flash
	wifi_station_connect(); //连接路由器

}

void ICACHE_FLASH_ATTR server_recv(void *arg, char *pdata, unsigned short len) {

	cJSON *root = cJSON_Parse(pdata);
	if (!root) {
		os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON * Result = cJSON_CreateObject();
		cJSON_AddNumberToObject(Result, "status", 3);
		cJSON_AddStringToObject(Result, "msg", "json error!");
		char *succeedData = cJSON_Print(Result);
		char data[1024];
		os_sprintf(data, "%s", succeedData);
		espconn_send((struct espconn *) arg, data, strlen(data));
		return;
	} else {
		char *ssid, *psw;
		int state;

		cJSON *ssid_json = cJSON_GetObjectItem(root, "ssid");
		cJSON *psw_json = cJSON_GetObjectItem(root, "psw");
		cJSON *state_json = cJSON_GetObjectItem(root, "state");

		state = state_json->valueint;

		switch (state) {
		case 0:
			if (STATION_GOT_IP == wifi_station_get_connect_status()) {

				cJSON * Result = cJSON_CreateObject();
				cJSON_AddNumberToObject(Result, "status", 0);
				cJSON_AddStringToObject(Result, "msg", "connect succeed!");
				char *succeedData = cJSON_Print(Result);
				char data[1024];
				os_sprintf(data, "%s", succeedData);
				espconn_send((struct espconn *) arg, data, strlen(data));
				isConnected = true;
			} else {

				cJSON * Result = cJSON_CreateObject();
				cJSON_AddNumberToObject(Result, "status", 1);
				cJSON_AddStringToObject(Result, "msg", "connect fail!");
				char *succeedData = cJSON_Print(Result);
				char data[1024];
				os_sprintf(data, "%s", succeedData);
				espconn_send((struct espconn *) arg, data, strlen(data));
			}

			break;

			//1表示连接AP
		case 1:

			if (ssid_json && psw_json) {
				ssid = ssid_json->valuestring;
				psw = psw_json->valuestring;
				wifiStationConnect(ssid, psw);
			} else {
				os_printf(" width null! \n");
			}

			cJSON_Delete(root);
			cJSON * Result = cJSON_CreateObject();
			cJSON_AddNumberToObject(Result, "status", 2);
			cJSON_AddStringToObject(Result, "msg", "AP connectting!");
			char *succeedData = cJSON_Print(Result);
			char data[1024];
			os_sprintf(data, "%s", succeedData);
			espconn_send((struct espconn *) arg, data, strlen(data));

			break;

		}

	}
}

void ICACHE_FLASH_ATTR server_sent(void *arg) {
	os_printf("send data succeed！\r");
	if (isConnected) {
		wifi_set_opmode(0x01); //设置为STATION模式
	}
}

void ICACHE_FLASH_ATTR server_discon(void *arg) {
	os_printf("conect diable! \r");
}

void ICACHE_FLASH_ATTR server_listen(void *arg) //注册 TCP 连接成功建立后的回调函数
{
	struct espconn *pespconn = arg;
	espconn_regist_recvcb(pespconn, server_recv); //接收
	espconn_regist_sentcb(pespconn, server_sent); //发送
	espconn_regist_disconcb(pespconn, server_discon); //断开
}

void ICACHE_FLASH_ATTR server_recon(void *arg, sint8 err) //注册 TCP 连接发生异常断开时的回调函数，可以在回调函数中进行重连
{
	os_printf("连接错误，错误代码为：%d\r\n", err); //%d,用来输出十进制整数
}

void Inter213_InitTCP(uint32_t Local_port) {
	user_tcp_espconn.proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp)); //分配空间
	user_tcp_espconn.type = ESPCONN_TCP; //设置类型为TCP协议
	user_tcp_espconn.proto.tcp->local_port = Local_port; //本地端口

	//注册连接成功回调函数和重新连接回调函数
	espconn_regist_connectcb(&user_tcp_espconn, server_listen); //注册 TCP 连接成功建立后的回调函数
	espconn_regist_reconcb(&user_tcp_espconn, server_recon); //注册 TCP 连接发生异常断开时的回调函数，可以在回调函数中进行重连
	espconn_accept(&user_tcp_espconn); //创建 TCP server，建立侦听
	espconn_regist_time(&user_tcp_espconn, 180, 0); //设置超时断开时间 单位：秒，最大值：7200 秒
	//如果超时时间设置为 0，ESP8266 TCP server 将始终不会断开已经不与它通信的 TCP client，不建议这样使用。

}

void WIFI_Init() {
	struct softap_config apConfig;
	wifi_set_opmode(0x03);    //设置为AP模式，保存到 flash
	apConfig.ssid_len = 10;						//设置ssid长度
	os_strcpy(apConfig.ssid, "xuhongWifi");	    //设置ssid名字
	os_strcpy(apConfig.password, "12345678");	//设置密码
	apConfig.authmode = 3;                      //设置加密模式
	apConfig.beacon_interval = 100;            //信标间隔时槽100 ~ 60000 ms
	apConfig.channel = 1;                      //通道号1 ~ 13
	apConfig.max_connection = 4;               //最大连接数
	apConfig.ssid_hidden = 0;                  //隐藏SSID

	wifi_softap_set_config(&apConfig);		//设置 WiFi soft-AP 接口配置，并保存到 flash
}

void wifiConnectCb(uint8_t status) {

}

LOCAL void ICACHE_FLASH_ATTR keyLongPress(void) {

}

LOCAL void ICACHE_FLASH_ATTR keyShortPress(void) {
	os_printf(" key short...\r\n");
	WIFI_Init();
	Inter213_InitTCP(8266); //本地端口
}
//按键初始化
LOCAL void ICACHE_FLASH_ATTR keyInit(void) {
	singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
			keyLongPress, keyShortPress);
	keys.singleKey = singleKey;
	keyParaInit(&keys);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); //GPIO12初始化
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15); //GPIO15初始化
}

void user_init() {

	uart_init(57600, 57600);		//设置串口0和串口1的波特率
	os_printf("-----SDK version:%s------\n", system_get_sdk_version());
	keyInit();

	wifi_station_set_auto_connect(1);
	wifi_set_sleep_type(NONE_SLEEP_T);                     //set none sleep mode

	os_timer_disarm(&checkTimer_wifistate); //取消定时器定时
	os_timer_setfn(&checkTimer_wifistate, (os_timer_func_t *) Check_WifiState,
	NULL); //设置定时器回调函数
	os_timer_arm(&checkTimer_wifistate, 1000, true); //启动定时器，单位：毫秒

}

void user_rf_pre_init() {

}
