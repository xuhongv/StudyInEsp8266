#include "driver/uart.h"
#include "osapi.h"

void user_init() //初始化
{
	uart_init(57600, 57600);

	// 8266作为UDP服务器，本地和目标端口为8266，IP地址见串口打印
	//udp_services_init();

	// 8266作为UDP客户端，本地端口为2000，目标端口为8686，IP地址见串口打印
	//udp_client_init();

	// 8266作为TCP服务器，本地和目标端口为8266，IP地址见串口打印
	//tcp_service_init();

	// 8266作为TCP客户端，目标端口为6000，IP地址见串口打印，此要先获取Tcp服务端的ip地址
	tcp_client_init();

}

void user_rf_pre_init() {

}
