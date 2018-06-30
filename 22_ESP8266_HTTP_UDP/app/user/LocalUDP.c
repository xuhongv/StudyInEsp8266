#include "LocalUDP.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"
#include "gpio.h"



LOCAL struct espconn user_udp_espconn;
LOCAL os_timer_t checkTimer_wifistate;

void ICACHE_FLASH_ATTR user_udp_sent_cb(void *arg)   //发送
{
    os_printf("\r\n发送成功！\r\n");


}


LOCAL void ICACHE_FLASH_ATTR user_udp_recv_cb(void *arg, char *pdata, unsigned short length) {
	os_printf("接收数据：%s", pdata);
}


LOCAL void sendDataUDP() {

	os_printf("\r\n send data ...\r\n");
	//每次发送数据确保端口参数不变
	user_udp_espconn.proto.udp = (esp_udp *) os_zalloc(sizeof(esp_udp));
	user_udp_espconn.type = ESPCONN_UDP;
	user_udp_espconn.proto.udp->local_port = 2000;
	user_udp_espconn.proto.udp->remote_port = 8686;
	const char udp_remote_ip[4] = { 255, 255, 255, 255 };
	os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4);
	espconn_sent(&user_udp_espconn, "this is message!", strlen("this is message!"));
}

//udp远程设置模块wifir接口
void ICACHE_FLASH_ATTR udpwificfgg_init(void) {

	user_udp_espconn.proto.udp = (esp_udp *) os_zalloc(sizeof(esp_udp)); //分配空间
	user_udp_espconn.type = ESPCONN_UDP;              //设置类型为UDP协议
	user_udp_espconn.proto.udp->local_port = 2000;            //本地端口
	user_udp_espconn.proto.udp->remote_port = 8686;           //目标端口
	const char udp_remote_ip[4] = { 255, 255, 255, 255 };       //目标IP地址（广播）
	os_memcpy(user_udp_espconn.proto.udp->remote_ip, udp_remote_ip, 4);

	espconn_regist_recvcb(&user_udp_espconn, user_udp_recv_cb);	 		//接收
	espconn_regist_sentcb(&user_udp_espconn, user_udp_sent_cb);	 		//发送
	espconn_create(&user_udp_espconn); //建立 UDP 传输

	wifi_set_broadcast_if(1);

	os_timer_disarm(&checkTimer_wifistate);   //取消定时器定时
	os_timer_setfn(&checkTimer_wifistate, (os_timer_func_t *) sendDataUDP,
	NULL);    //设置定时器回调函数
	os_timer_arm(&checkTimer_wifistate, 1000, 1);      //启动定时器，单位：毫秒

}

