#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "xpwm.h"

esp_err_t pwm_init_data()
{

    esp_err_t result = ESP_OK;
    int ch;
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 1000,                      // frequency of PWM signal
        .speed_mode = LEDC_MODE,              // timer mode
        .timer_num = LEDC_TIMER               // timer index
    };

    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel[CHANNLE_PWM_TOTAL] = {
        {.channel = LEDC_CHANNEL_1,
         .duty = 0,
         .gpio_num = PWM_CW_OUT_IO_NUM,
         .speed_mode = LEDC_MODE,
         .hpoint = 0,
         .timer_sel = LEDC_TIMER},
        {.channel = LEDC_CHANNEL_1,
         .duty = 0,
         .gpio_num = PWM_WW_OUT_IO_NUM,
         .speed_mode = LEDC_MODE,
         .hpoint = 0,
         .timer_sel = LEDC_TIMER},
        {.channel = LEDC_CHANNEL_1,
         .duty = 0,
         .gpio_num = PWM_RED_OUT_IO_NUM,
         .speed_mode = LEDC_MODE,
         .hpoint = 0,
         .timer_sel = LEDC_TIMER},
        {.channel = LEDC_CHANNEL_1,
         .duty = 0,
         .gpio_num = PWM_GREEN_OUT_IO_NUM,
         .speed_mode = LEDC_MODE,
         .hpoint = 0,
         .timer_sel = LEDC_TIMER},
        {.channel = LEDC_CHANNEL_1,
         .duty = 0,
         .gpio_num = PWM_BLUE_OUT_IO_NUM,
         .speed_mode = LEDC_MODE,
         .hpoint = 0,
         .timer_sel = LEDC_TIMER},
    };

    // Set LED Controller with previously prepared configuration
    for (ch = 0; ch < CHANNLE_PWM_TOTAL; ch++)
    {
        ledc_channel_config(&ledc_channel[ch]);
    }

    // Initialize fade service.
    ledc_fade_func_install(0);

    //本地存储取出数据
    if (IS_SAVE_PARAMS)
    {

        nvs_handle out_handle;

        memset(&dev_status, 0x0, sizeof(dev_status));

        uint32_t length = sizeof(dev_status);

        if (nvs_open(NVS_CONFIG_NAME, NVS_READWRITE, &out_handle) != ESP_OK)
        {
            printf("open innet_conf fail\n");
            return ESP_FAIL;
        }
        if (nvs_get_blob(out_handle, NVS_TABLE_NAME, &dev_status, &length) != ESP_OK)
            return ESP_FAIL;

        printf(" outBrightnessChannle : %d , outColortempChannle %d \n", dev_status.Brightness, dev_status.Colortemp);

        nvs_close(out_handle);
    }

    return result;
}
esp_err_t light_driver_set_ctb(const int brightness, const int color_temperature)
{
    int part = (APK_MAX_COLORTEMP - APK_MIN_COLORTEMP) / 100;
    int temp = APK_MAX_COLORTEMP - color_temperature;
    int tempWW = ((temp) / part);
    uint8_t outCW = (brightness - (brightness * tempWW / 100));
    uint8_t outWW = (brightness * tempWW / 100);
    int outBrightnessChannle = 8192 * outCW / 100;
    int outColortempChannle = 8192 * outWW / 100;

    printf(" light_driver_set_ctb=> outBrightnessChannle : %d , outColortempChannle %d \n", outBrightnessChannle, outColortempChannle);

    dev_status.Brightness = brightness;
    dev_status.Colortemp = color_temperature;

    if (5 == CHANNLE_PWM_TOTAL)
    {
        dev_status.Red = 0;
        dev_status.Green = 0;
        dev_status.Blue = 0;
    }

    if (dev_status.Brightness == 0)
    {
        dev_status.Power = 0;
    }
    else
    {
        dev_status.Power = 1;
    }

    ledc_set_fade_with_time(LEDC_MODE, 1, outColortempChannle, LEDC_FADE_TIME);
    ledc_set_fade_with_time(LEDC_MODE, 0, outBrightnessChannle, LEDC_FADE_TIME);

    if (CHANNLE_PWM_TOTAL == 5)
    {
        ledc_set_fade_with_time(LEDC_MODE, CHANNLE_PWM_RED, 0, LEDC_FADE_TIME);
        ledc_set_fade_with_time(LEDC_MODE, CHANNLE_PWM_GREEN, 0, LEDC_FADE_TIME);
        ledc_set_fade_with_time(LEDC_MODE, CHANNLE_PWM_BLUE, 0, LEDC_FADE_TIME);
    }

    int ch = 0;
    for (ch = 0; ch < CHANNLE_PWM_TOTAL; ch++)
    {
        ledc_fade_start(LEDC_MODE, ch, LEDC_FADE_NO_WAIT);
    }

    if (IS_SAVE_PARAMS)
    {
        nvs_handle out_handle;
        if (nvs_open(NVS_CONFIG_NAME, NVS_READWRITE, &out_handle) != ESP_OK)
        {
            printf("open innet_conf fail\n");
            return ESP_FAIL;
        }
        if (nvs_set_blob(out_handle, NVS_TABLE_NAME, &dev_status, sizeof(dev_status)) != ESP_OK)
            printf("Save Struct  Fail !!  \n");
        else
            printf("Save Struct  ok !!  \n");
        //提交下！相当于软件面板的 “应用” 按钮，并没关闭面板！
        nvs_commit(out_handle);
        nvs_close(out_handle);
    }
    return ESP_OK;
}
esp_err_t light_driver_set_switch(bool power)
{

    // 如果当前为关灯状态则不做处理
    if (power && dev_status.Power == 0)
    {
        //开灯默认是中性光
        light_driver_set_ctb(100, APK_MID_COLORTEMP);
    }
    else
    {
        light_driver_set_ctb(0, APK_MIN_COLORTEMP);
    }

    return ESP_OK;
}
esp_err_t light_driver_set_mode(uint8_t mode)
{
    dev_status.Mode = mode;
    switch (mode)
    {
    case 0:
        /* code */ light_driver_set_ctb(100, APK_MID_COLORTEMP);
        break;
    case 1:
        /* code */ light_driver_set_ctb(20, APK_MID_COLORTEMP);
        break;
    case 2:
        /* code */ light_driver_set_ctb(50, APK_MIN_COLORTEMP);
        break;
    case 3:
        /* code */ light_driver_set_ctb(80, APK_MID_COLORTEMP);
        break;
    case 4:
        /* code */ light_driver_set_ctb(100, APK_MAX_COLORTEMP);
        break;
    default:
        break;
    }
    return ESP_OK;
}
esp_err_t light_driver_set_brightness(uint8_t brightness)
{
    light_driver_set_ctb(brightness, dev_status.Colortemp);
    return ESP_OK;
}
esp_err_t light_driver_set_colorTemperature(int colorTemperature)
{
    if (dev_status.Brightness == 0)
    {
        dev_status.Brightness = 100;
    }

    light_driver_set_ctb(dev_status.Brightness, colorTemperature);
    return ESP_OK;
}

int light_driver_get_color_temperature()
{
    return dev_status.Colortemp;
}
uint8_t light_driver_get_brightness() { return dev_status.Brightness; }

esp_err_t light_driver_get_ctb(uint8_t *p_color_temperature, uint8_t *p_brightness)
{
    *p_brightness = dev_status.Brightness;
    *p_color_temperature = dev_status.Colortemp;
    return ESP_OK;
}
bool light_driver_get_switch()
{
    return dev_status.Power;
}
uint8_t light_driver_get_mode()
{
    return dev_status.Mode;
}
uint8_t light_driver_get_version()
{
    return 1;
}

esp_err_t light_driver_set_rgb(const uint8_t red, const uint8_t green, const uint8_t blue)
{

    dev_status.Brightness = 0;
    dev_status.Colortemp = 0;
    dev_status.Red = red;
    dev_status.Green = green;
    dev_status.Blue = blue;

    if (!dev_status.Red || !dev_status.Green || !dev_status.Blue)
    {
        dev_status.Power = 0;
    }
    else
    {
        dev_status.Power = 1;
    }

    int outRedChannle = 8192 * red / APK_MAX_COLOR;
    int outGreenChannle = 8192 * green / APK_MAX_COLOR;
    int outBlueChannle = 8192 * blue / APK_MAX_COLOR;

    // printf("outRedChannle: %d \n", outRedChannle);
    // printf("outGreenChannle: %d \n", outGreenChannle);
    // printf("outBlueChannle: %d\n\n", outBlueChannle);

    ledc_set_fade_with_time(LEDC_MODE, CHANNLE_PWM_CW, 0, LEDC_FADE_TIME);
    ledc_set_fade_with_time(LEDC_MODE, CHANNLE_PWM_WW, 0, LEDC_FADE_TIME);
    ledc_set_fade_with_time(LEDC_MODE, CHANNLE_PWM_RED, outRedChannle, LEDC_FADE_TIME);
    ledc_set_fade_with_time(LEDC_MODE, CHANNLE_PWM_GREEN, outGreenChannle, LEDC_FADE_TIME);
    ledc_set_fade_with_time(LEDC_MODE, CHANNLE_PWM_BLUE, outBlueChannle, LEDC_FADE_TIME);

    int ch = 0;
    for (ch = 0; ch < CHANNLE_PWM_TOTAL; ch++)
    {
        ledc_fade_start(LEDC_MODE, ch, LEDC_FADE_NO_WAIT);
    }

    if (IS_SAVE_PARAMS)
    {
        nvs_handle out_handle;
        if (nvs_open(NVS_CONFIG_NAME, NVS_READWRITE, &out_handle) != ESP_OK)
        {
            printf("open innet_conf fail\n");
            return ESP_FAIL;
        }
        if (nvs_set_blob(out_handle, NVS_TABLE_NAME, &dev_status, sizeof(dev_status)) != ESP_OK)
            printf("Save Struct  Fail !!  \n");
        else
            printf("Save Struct  ok !!  \n");
        //提交下！相当于软件面板的 “应用” 按钮，并没关闭面板！
        nvs_commit(out_handle);
        nvs_close(out_handle);
    }
    return ESP_OK;
}

esp_err_t light_driver_get_rgb(uint8_t *p_red, uint8_t *p_green, uint8_t *p_blue)
{
    *p_red = dev_status.Red;
    *p_green = dev_status.Green;
    *p_blue = dev_status.Blue;
    return ESP_OK;
}

esp_err_t light_driver_set_ctb_not_save(const int brightness, const int color_temperature)
{
    int part = (APK_MAX_COLORTEMP - APK_MIN_COLORTEMP) / 100;
    int temp = APK_MAX_COLORTEMP - color_temperature;
    int tempWW = ((temp) / part);
    uint8_t outCW = (brightness - (brightness * tempWW / 100));
    uint8_t outWW = (brightness * tempWW / 100);
    int outBrightnessChannle = 8192 * outCW / 100;
    int outColortempChannle = 8192 * outWW / 100;

    // printf(" outBrightnessChannle : %d , outColortempChannle %d \n", outBrightnessChannle, outColortempChannle);

    dev_status.Brightness = brightness;
    dev_status.Colortemp = color_temperature;

    if (5 == CHANNLE_PWM_TOTAL)
    {
        dev_status.Red = 0;
        dev_status.Green = 0;
        dev_status.Blue = 0;
    }

    if (dev_status.Brightness == 0)
    {
        dev_status.Power = 0;
    }
    else
    {
        dev_status.Power = 1;
    }

    ledc_set_fade_with_time(LEDC_MODE, 1, outColortempChannle, 800);
    ledc_set_fade_with_time(LEDC_MODE, 0, outBrightnessChannle, 800);

    if (CHANNLE_PWM_TOTAL == 5)
    {
        ledc_set_fade_with_time(LEDC_MODE, CHANNLE_PWM_RED, 0, LEDC_FADE_TIME);
        ledc_set_fade_with_time(LEDC_MODE, CHANNLE_PWM_GREEN, 0, LEDC_FADE_TIME);
        ledc_set_fade_with_time(LEDC_MODE, CHANNLE_PWM_BLUE, 0, LEDC_FADE_TIME);
    }

    int ch = 0;
    for (ch = 0; ch < CHANNLE_PWM_TOTAL; ch++)
    {
        ledc_fade_start(LEDC_MODE, ch, LEDC_FADE_NO_WAIT);
    }

    return ESP_OK;
}

esp_err_t light_driver_set_cycle(uint8_t nums)
{
    while (nums--)
    {
        light_driver_set_ctb_not_save(100, APK_MID_COLORTEMP);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        light_driver_set_ctb_not_save(0, APK_MID_COLORTEMP);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    light_driver_set_ctb(100, APK_MID_COLORTEMP);
    return ESP_OK;
}

esp_err_t light_driver_set_ctb_from_last()
{
    light_driver_set_ctb(dev_status.Brightness, dev_status.Colortemp);
    return ESP_OK;
}
