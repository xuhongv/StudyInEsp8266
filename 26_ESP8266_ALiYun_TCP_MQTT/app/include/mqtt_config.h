#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__

#define CFG_HOLDER	0x00FF55A4	/* Change this value to load default configurations */
#define CFG_LOCATION	0x79	/* Please don't change or if you know what you doing */
#define MQTT_SSL_ENABLE




/*aliyun 设备信息*/
#define DeviceName "smartLight"
#define ProductKey "a11ozGcYRSq"
#define DeviceSecret "pHTOZ0R0kDWyU3ruXkA4atelHxNAgdFI"
#define DeviceArea "cn-shanghai"
/*aliyun 设备信息 结束*/




//MQTT域名
#define MQTT_HOST			""ProductKey".iot-as-mqtt."DeviceArea".aliyuncs.com"
//端口号
#define MQTT_PORT			1883
//用户名
#define MQTT_USER			""DeviceName"&"ProductKey""

#define MQTT_BUF_SIZE		1024
#define MQTT_KEEPALIVE		60   //心跳
#define MQTT_PASS			""
#define MQTT_CLIENT_ID		""

//连接路由器的名字和密码
#define STA_SSID "HuaWei_Text_Only"
#define STA_PASS "xlinyun@123456"



#define STA_TYPE AUTH_WPA2_PSK
#define MQTT_RECONNECT_TIMEOUT 	5	/*second*/

#define DEFAULT_SECURITY	0
#define QUEUE_BUFFER_SIZE		 		2048

#define PROTOCOL_NAMEv31	/*MQTT version 3.1 compatible with Mosquitto v0.15*/
//PROTOCOL_NAMEv311			/*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/

#endif // __MQTT_CONFIG_H__
