/*
 * user_esp_now.h
 *
 *  Created on: 2017Äê7ÔÂ8ÈÕ
 *      Author: Administrator
 */

#ifndef _USER_ESP_NOW_H_
#define _USER_ESP_NOW_H_

extern void demo_send(u8 *mac_addr, u8 *data, u8 len);
extern void user_esp_now_init(void);
extern void user_esp_now_timer_init(void);
extern void user_esp_now_set_mac_current(void);

#endif /* _USER_ESP_NOW_H_ */
