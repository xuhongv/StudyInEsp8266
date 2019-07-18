
#ifndef __USER_LIGHT_H__
#define __USER_LIGHT_H__
#include <stdlib.h>
#include <stdio.h>
#include "esp_system.h"
#include <stdlib.h>
#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"
#include "driver/pwm.h"
#include "rom/ets_sys.h"
#include "nvs.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

//一共设计的通道
#define PWM_CHANNEL 5
//周期,同样是深度
#define PWM_PERIOD 100

enum
{
    LED_OFF = 0,
    LED_ON = 1,
    LED_1HZ,
    LED_5HZ,
    LED_20HZ,
};

#define PWM_0_OUT_IO_NUM 12 //白光 通道0
#define PWM_1_OUT_IO_NUM 13 //暖光 通道1
#define PWM_2_OUT_IO_NUM 14 //红色灯 通道2
#define PWM_3_OUT_IO_NUM 15 //绿色灯 通道3
#define PWM_4_OUT_IO_NUM 5  //蓝色灯 通道4

struct light_saved_param
{
    uint32_t pwm_period;
    uint32_t pwm_duty[PWM_CHANNEL];
};

#define FILED_NVS_PARAM "light_param"
#define TB_SELF_LIGHT "TBselfLight"

void user_light_init(void);
uint32_t user_light_get_duty(uint8_t channel);
void user_light_set_duty(uint32_t duty, uint8_t channel);
uint32_t user_light_get_period(void);
void user_light_set_period(uint32_t period);

/**
 * @description: 平滑设置全部通道
 * @param {type} 
 * @return: 
 */
void light_set_aim_all(uint32_t cw, uint32_t ww, uint32_t r, uint32_t g, uint32_t b);

/**
 * @description: 平滑设置通道2 红色灯
 * @param {type} 
 * @return: 
 */
void light_set_aim_r(uint32_t r);

/**
 * @description: 平滑设置通道3 绿色灯
 * @param {type} 
 * @return: 
 */
void light_set_aim_g(uint32_t g);

/**
 * @description: 平滑设置通道4 蓝色灯
 * @param {type} 
 * @return: 
 */
void light_set_aim_b(uint32_t b);

/**
 * @description: 平滑设置通道0 白色灯
 * @param {type} 
 * @return: 
 */
void light_set_aim_cw(uint32_t cw);

/**
 * @description: 平滑设置通道1 暖色灯
 * @param {type} 
 * @return: 
 */
void light_set_aim_ww(uint32_t ww);

#endif
