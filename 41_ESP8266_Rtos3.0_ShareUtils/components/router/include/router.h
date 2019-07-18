#ifndef _IOT_ROUTER_H_
#define _IOT_ROUTER_H_

#include <string.h>
#include <stdlib.h>

/**
 * @description:  自动连接路由器
 * @param {type}  是否成功连接！ false：获取ssid失败！true：成功自动连接路由器
 * @return: 
 */
bool routerStartConnect();

/**
 * @description: 清理wifi路由器连接信息
 * @param {type} 
 * @return: 
 */
void router_wifi_clean_info(void);

/**
 * @description: 保存路由器信息
 * @param {type} 
 * @return: 
 */
void router_wifi_save_info(uint8_t *ssid, uint8_t *password);

#endif