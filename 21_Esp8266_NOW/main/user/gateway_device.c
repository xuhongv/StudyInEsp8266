#include "osapi.h"
#include "espnow.h"
#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "spi_flash.h"
#include "user_interface.h"
#include "c_types.h"
#include "smartconfig.h"

os_timer_t gateway_esp_now_timer;

//下面是要发送的三个子设备的mac地址，此地址请替换到你要测试的设备的mac地址
u8 slaveDeviceMac_1[6] = { 0x84, 0xF3, 0xEB, 0xB3, 0xA7, 0x40 };
u8 slaveDeviceMac_2[6] = { 0x60, 0x01, 0x94, 0x35, 0x26, 0xDA };

static void ICACHE_FLASH_ATTR gateway_esp_now_recv_cb(u8 *macaddr, u8 *data,
		u8 len) {
	int i;
	static u16 ack_count = 0;
	u8 ack_buf[16];
	u8 recv_buf[17];
	os_printf("recieve from subDevice[");
	for (i = 0; i < 6; i++) {
		os_printf("%02X ", macaddr[i]);
	}
	os_printf(" len: %d]:", len);

	os_bzero(recv_buf, 17);
	os_memcpy(recv_buf, data, len < 17 ? len : 16);
	os_printf(recv_buf);
	os_printf("\r\n\r\n");
}

void ICACHE_FLASH_ATTR gateway_esp_now_send_cb(u8 *mac_addr, u8 status) {

	if (1 == status) {
		os_printf("send message to subDevice fail ! the fail send macAdress:");
		int i;
		for (i = 0; i < 6; i++) {
			os_printf("%02X-", mac_addr[i]);
		}
		os_printf("\r\n");
	} else if (0 == status) {
		os_printf("send message to subDevice successful ! the send macAdress:");
		int i;
		for (i = 0; i < 6; i++) {
			os_printf("%02X-", mac_addr[i]);
		}
		os_printf("\r\n");
	}
}

void gateway_esp_now_timer_cb() {

	u8* send_data_sub = "Hello World ! I am message from gateWay ...";
	//第一个参数传NUll，表示传所有已经记录的子设备，如果要传给具体的子设备，要写入其mac地址；
	user_esp_now_send(NULL, send_data_sub, os_strlen(send_data_sub));

}

void GateWay_Device_init() {

	wifi_set_opmode(STATION_MODE);	//设置为STATION模式
	struct station_config stationConf;
	os_strcpy(stationConf.ssid, "iPhone");	  //改成你自己的   路由器的用户名
	os_strcpy(stationConf.password, "xh870189248"); //改成你自己的   路由器的密码
	wifi_station_set_config(&stationConf);	//设置WiFi station接口配置，并保存到 flash
	wifi_station_connect();	//连接路由器
	os_printf("As a gateWay ...\r\n");

	if (esp_now_init() == 0) {

		os_printf("esp_now gateWay device init ok! \n");
		// 注册 ESP-NOW 收包的回调函数
		esp_now_register_recv_cb(gateway_esp_now_recv_cb);
		// 注册发包回调函数
		esp_now_register_send_cb(gateway_esp_now_send_cb);

		esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
		//添加子设备，信道为1，不加密，
		esp_now_add_peer(slaveDeviceMac_1, ESP_NOW_ROLE_COMBO, 1, NULL, 16);
		esp_now_add_peer(slaveDeviceMac_2, ESP_NOW_ROLE_COMBO, 1, NULL, 16);

		//开始定时发送消息给所有子设备
		os_timer_disarm(&gateway_esp_now_timer);
		os_timer_setfn(&gateway_esp_now_timer,
				(os_timer_func_t *) gateway_esp_now_timer_cb, NULL);
		os_timer_arm(&gateway_esp_now_timer, 3000, 1);

	}

}
