
#include "Http_Post.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"
#include "gpio.h"

void ICACHE_FLASH_ATTR findStr(char * input, char * output) {
	char * outputStart = strchr(input, '{');
	char * outputEnd = NULL;
	if (NULL != outputStart) {
		outputEnd = strrchr(input, '}');
		if (NULL != outputEnd) {
			memcpy(output, outputStart, outputEnd - outputStart + 1);
		}
	}
	output[outputEnd - outputStart + 1] = '\0';
}



LOCAL struct espconn httperver;
char httphead[] = "HTTP/1.1 200 OK \nDate: %s  \nContent-Type: text/plain; charset=UTF-8\n\nOK!!";

LOCAL void ICACHE_FLASH_ATTR httperver_recv(void *arg, char *pdata,
		unsigned short len) {

	struct espconn *pespconn = (struct espconn *) arg;

	uart0_sendStr(pdata);
	uart0_sendStr("\n\r------ recieve over ------- \n\r");

	//确定是post请求
	if (pdata[0] == 'P' && pdata[1] == 'O' && pdata[2] == 'S'
			&& pdata[3] == 'T') {

		char input[2048] = { 0 };
		char output[2048] = { 0 };
		os_sprintf(input, "%s", pdata);
		findStr(input, output);

		char* pJSON = output;

		char temp[2048];

		os_sprintf(temp, httphead,
				sntp_get_real_time(sntp_get_current_timestamp()));

		uart0_sendStr(temp);
		espconn_sent(pespconn, temp, strlen(temp));

	}
	espconn_disconnect(pespconn);
}


static void ICACHE_FLASH_ATTR httperver_sent_cb(void *arg)
 {
	struct espconn *pespconn = (struct espconn *) arg;
	espconn_disconnect(pespconn);
}
LOCAL void ICACHE_FLASH_ATTR httperver_listen(void *arg) {
	struct espconn *pespconn = (struct espconn *) arg;
	espconn_regist_recvcb(pespconn, httperver_recv);
	espconn_regist_sentcb(pespconn, httperver_sent_cb);
}
void ICACHE_FLASH_ATTR httperver_init(void) //http协议服务器接口调用
{
	httperver.type = ESPCONN_TCP;
	httperver.state = ESPCONN_NONE;
	httperver.proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));
	httperver.proto.tcp->local_port = 80;
	espconn_regist_connectcb(&httperver, httperver_listen);
	espconn_accept(&httperver);
	espconn_regist_time(&httperver, 3, 0);
}

void ICACHE_FLASH_ATTR gpioctr_init(void) {

	httperver_init();

}
