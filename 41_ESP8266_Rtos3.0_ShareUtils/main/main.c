#include <stdio.h>
#include "esp_system.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_err.h"
#include "internal/esp_wifi_internal.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/pwm.h"
#include "button.h"
#include "rom/ets_sys.h"
#include "driver/uart.h"
#include "xLight_pwm.h"

//打印日志用
static const char *TAG = "XuHongLog";

#define BUTTON_GPIO 4

/**
 * @description: xPWM使用示范
 * @param {type} 
 * @return: 
 */
void TaskPWM(void *pvParameters)
{
    user_light_init();
    //light_set_aim_all(50, 50, 100, 100, 100);
    while (1)
    {
        light_set_aim_all(100, 100, 100, 100, 100);
        vTaskDelay(2000 / portTICK_RATE_MS);
        light_set_aim_all(0, 0, 0, 0, 0);
        vTaskDelay(2000 / portTICK_RATE_MS);
        // light_set_aim_all(50, 50, 50, 30, 80);
        // vTaskDelay(3000 / portTICK_RATE_MS);
        // light_set_aim_all(0, 0, 0, 0, 0);
        // vTaskDelay(3000 / portTICK_RATE_MS);
        // light_set_aim_all(5, 63, 0, 0, 0);
        // vTaskDelay(3000 / portTICK_RATE_MS);
        // light_set_aim_all(100, 0, 0, 0, 0);
        // vTaskDelay(3000 / portTICK_RATE_MS);
        // light_set_aim_all(0, 5, 0, 0, 0);
        // vTaskDelay(3000 / portTICK_RATE_MS);
    }
}

static void ButtonShortPressCallBack(void *arg)
{
    ESP_LOGI(TAG, "ButtonShortPressCallBack  esp_get_free_heap_size(): %d ", esp_get_free_heap_size());
}

static void ButtonLongPressCallBack(void *arg)
{
    ESP_LOGI(TAG, "ButtonLongPressCallBack  esp_get_free_heap_size(): %d ", esp_get_free_heap_size());
}

/**
 * @description: 按键驱动
 * @param {type} 
 * @return: 
 */
void TaskButton(void *pvParameters)
{
    //定义一个 gpio 下降沿触发
    button_handle_t btn_handle = button_dev_init(BUTTON_GPIO, 1, BUTTON_ACTIVE_LOW);
    // 50ms按钮短按
    // BUTTON_PUSH_CB 表示按下就触发回调函数，如果设置了长按，这个依然会同时触发！
    // BUTTON_RELEASE_CB 表示释放才回调，如果设置了长按，这个依然会同时触发！
    // BUTTON_TAP_CB 此选项释放才回调，如果设置了长按，这个不会同时触发！
    button_dev_add_tap_cb(BUTTON_PUSH_CB, ButtonShortPressCallBack, "TAP", 50 / portTICK_PERIOD_MS, btn_handle);
    // 设置长按 2s后触发
    button_dev_add_press_cb(0, ButtonLongPressCallBack, NULL, 2000 / portTICK_PERIOD_MS, btn_handle);

    vTaskDelete(NULL);
}

/******************************************************************************
 * FunctionName : app_main
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    printf("\n\n-------------------------------- Get Systrm Info------------------------------------------\n");
    //获取IDF版本
    printf("     SDK version:%s\n", esp_get_idf_version());
    //获取芯片可用内存
    printf("     esp_get_free_heap_size : %d  \n", esp_get_free_heap_size());
    //获取从未使用过的最小内存
    printf("     esp_get_minimum_free_heap_size : %d  \n", esp_get_minimum_free_heap_size());
    //获取芯片的内存分布，返回值具体见结构体 flash_size_map
    printf("     system_get_flash_size_map(): %d \n", system_get_flash_size_map());
    //获取mac地址（station模式）
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    printf("esp_read_mac(): %02x:%02x:%02x:%02x:%02x:%02x \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("--------------------------------------------------------------------------\n\n");

    xTaskCreate(TaskPWM, "TaskPWM", 1024 * 3, NULL, 4, NULL);
    //xTaskCreate(TaskButton, "TaskButton", 1024 * 3, NULL, 4, NULL);
}