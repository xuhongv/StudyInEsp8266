#include "client.h"
#include "iconv.h"
#include "stdio.h"
#include "string.h"

//成功接收到服务器返回数据函数
void ICACHE_FLASH_ATTR user_tcp_recv_cb(void *arg, char *pdata,
		unsigned short len) {
	uart0_sendStr("\r\n ----- 开始接受数据----- \r\n ");
	uart0_tx_buffer(pdata, strlen(pdata));
	uart0_sendStr("\r\n -----结束接受数据-----  \r\n ");

}

//发送数据到服务器成功的回调函数
void ICACHE_FLASH_ATTR user_tcp_sent_cb(void *arg) {
	uart0_sendStr("发送数据成功！\r\n ");
}

//断开服务器成功的回调函数
void ICACHE_FLASH_ATTR user_tcp_discon_cb(void *arg) {
	uart0_sendStr("断开连接成功！\r\n ");
}

//连接失败的回调函数，err为错误代码
void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err) {
	uart0_sendStr("连接错误，错误代码为%d\r\n", err);
	espconn_connect((struct espconn *) arg);
}

//成功连接到服务器的回调函数
void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg) {
	struct espconn *pespconn = arg;
	espconn_regist_recvcb(pespconn, user_tcp_recv_cb);
	espconn_regist_sentcb(pespconn, user_tcp_sent_cb);
	espconn_regist_disconcb(pespconn, user_tcp_discon_cb);

	uart0_sendStr("\r\n ----- 请求数据开始----- \r\n");
	uart0_tx_buffer(buffer, strlen(buffer));
	uart0_sendStr("\r\n -----请求数据结束-----  \r\n");

	espconn_sent(pespconn, buffer, strlen(buffer));

}
void ICACHE_FLASH_ATTR my_station_init(struct ip_addr *remote_ip,
		struct ip_addr *local_ip, int remote_port) {
	//配置
	user_tcp_conn.type = ESPCONN_TCP;
	user_tcp_conn.state = ESPCONN_NONE;
	user_tcp_conn.proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));
	os_memcpy(user_tcp_conn.proto.tcp->local_ip, local_ip, 4);
	os_memcpy(user_tcp_conn.proto.tcp->remote_ip, remote_ip, 4);
	user_tcp_conn.proto.tcp->local_port = espconn_port();
	user_tcp_conn.proto.tcp->remote_port = remote_port;
	//注册
	espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb);
	espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb);
	//连接服务器
	espconn_connect(&user_tcp_conn);
}
