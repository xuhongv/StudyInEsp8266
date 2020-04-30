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
#define CHANNLE_PWM_CW 0
#define CHANNLE_PWM_WW 1
#define CHANNLE_PWM_RED 2
#define CHANNLE_PWM_GREEN 3
#define CHANNLE_PWM_BLUE 4

/***************  用户可修改 start **********************/
#define LEDC_FADE_TIME (2000)  //渐变时间
#define APK_MAX_COLORTEMP 6500 //用户定义最大的色温数值
#define APK_MID_COLORTEMP 4800 //用户定义中间的色温数值
#define APK_MIN_COLORTEMP 2700 //用户定义最小的色温数值
#define APK_MAX_COLOR 255      //用户定义最大的RGB数值
//pwm gpio口配置
#define CHANNLE_PWM_TOTAL 5

#define PWM_CW_OUT_IO_NUM 14
#define PWM_WW_OUT_IO_NUM 13
#define PWM_RED_OUT_IO_NUM 12
#define PWM_GREEN_OUT_IO_NUM 15
#define PWM_BLUE_OUT_IO_NUM 5

//是否带有记忆功能
#define IS_SAVE_PARAMS true

/***************  用户可修改  end**********************/

typedef struct User_dev_status_t
{
    int Power;
    int Mode;
    int Brightness;
    int Colortemp;
    int Red;
    int Green;
    int Blue;
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
esp_err_t light_driver_set_rgb(const uint8_t red, const uint8_t green, const uint8_t blue);
esp_err_t light_driver_set_switch(bool status);
esp_err_t light_driver_set_mode(uint8_t mode);
esp_err_t light_driver_set_cycle(uint8_t nums);
esp_err_t light_driver_set_ctb_from_last();

/**
 * @brief  Get the status of the light
 */
int light_driver_get_color_temperature();
uint8_t light_driver_get_brightness();
esp_err_t light_driver_get_ctb(uint8_t *p_color_temperature, uint8_t *p_brightness);
esp_err_t light_driver_get_rgb(uint8_t *p_red, uint8_t *p_green, uint8_t *p_blue);
bool light_driver_get_switch();
uint8_t light_driver_get_mode();

/**
 * @brief  Get the version of the light
 */
uint8_t light_driver_get_version();
#endif

