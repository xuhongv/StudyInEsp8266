/*
 * @Author: your name
 * @Date: 2019-11-21 15:59:45
 * @LastEditTime: 2020-04-28 11:01:43
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \ESP8266_RTOS_SDK\mine\ESP8266ForMqttPWM\components\pwm\include\xpwm.h
 */

#ifndef _IOT_PWMUTILS_H_
#define _IOT_PWMUTILS_H_

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"
#include "driver/pwm.h"
#include "driver/ledc.h"

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_MAX_DUTY pow(2, 13) // 请勿改变： 对应下面的 LEDC_TIMER_13_BIT 定时器
#define NVS_CONFIG_NAME "PWM_CONFIG"
#define NVS_TABLE_NAME "NVS_TABLE"

/***************  用户可修改 start **********************/
#define LEDC_FADE_TIME (2000)  //渐变时间
#define APK_MAX_COLORTEMP 6500 //用户定义最大的色温数值
#define APK_MID_COLORTEMP 4800 //用户定义中间的色温数值
#define APK_MIN_COLORTEMP 2700 //用户定义最小的色温数值

//pwm gpio口配置
#define CHANNLE_PWM_CW 0
#define CHANNLE_PWM_WW 1

#define PWM_CW_OUT_IO_NUM 14
#define PWM_WW_OUT_IO_NUM 13

//是否带有记忆功能
#define IS_SAVE_PARAMS true

/***************  用户可修改  end**********************/

typedef struct User_dev_status_t
{
    int Power;
    int Mode;
    int Brightness;
    int Colortemp;
} User_dev_status_t;

User_dev_status_t dev_status;

/**
 * @brief  Set the status of the light
 */
esp_err_t pwm_init_data();
esp_err_t light_driver_set_color_temperature(uint8_t color_temperature);
esp_err_t light_driver_set_brightness(uint8_t brightness);
esp_err_t light_driver_set_colorTemperature(int colorTemperature);
esp_err_t light_driver_set_ctb(const int brightness, const int color_temperature);
esp_err_t light_driver_set_switch(bool status);
esp_err_t light_driver_set_mode(uint8_t mode);

/**
 * @brief  Get the status of the light
 */
int light_driver_get_color_temperature();
uint8_t light_driver_get_brightness();
esp_err_t light_driver_get_ctb(uint8_t *p_color_temperature, uint8_t *p_brightness);
bool light_driver_get_switch();
uint8_t light_driver_get_mode();

/**
 * @brief  Get the version of the light
 */
uint8_t light_driver_get_version();
#endif
