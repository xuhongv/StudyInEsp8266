#include <stdio.h>
#include "esp_system.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
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
#include "rom/ets_sys.h"
#include "driver/uart.h"
#include "lwip/netdb.h"
#include "driver/gpio.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "driver/hw_timer.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "mbedtls/base64.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "rom/ets_sys.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "driver/ledc.h"
#include "xpwm.h"


/**
 *    这是一个在 esp8266 rtos master 分支在 ledc 上实现 平滑调光 的 亮度色温 rgb 调光功能实现
 *    有任何技术问题邮箱： 870189248@qq.com
 *    本人GitHub仓库：https://github.com/xuhongv
 *    本人博客：https://blog.csdn.net/xh870189248
 * 
 **/



/**
 * @description: 颜色设置
 * @param {type} 
 * @return: 
 */
static void Task_set_color(void *parm)
{
    light_driver_set_rgb(255, 0, 0);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_rgb(0, 255, 0);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_rgb(0, 0, 255);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_rgb(255, 128, 0);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_rgb(255, 0, 128);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);
}
/**
 * @description:  工作模式设置
 * @param {type} 
 * @return: 
 */
static void Task_set_mode(void *parm)
{
    light_driver_set_mode(0);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_mode(1);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_mode(2);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_mode(3);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_mode(4);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    xTaskCreate(Task_set_color, "Task_set_color", 1024 * 2, NULL, 8, NULL); // 创建任务
    vTaskDelete(NULL);
}
/**
 * @description:  冷暖光设置
 * @param {type} 
 * @return: 
 */
static void Task_brightness_temperature(void *parm)
{

    //亮度取值范围为 0~100 ，色温取值范围2700~6500， 均在 头文件可定义
    light_driver_set_ctb(100, 6500);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_ctb(100, 2700);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_ctb(100, 4600);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_ctb(0, APK_MIN_COLORTEMP);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_brightness(100);
    vTaskDelay(1500 / portTICK_PERIOD_MS);
    light_driver_set_colorTemperature(APK_MIN_COLORTEMP);
    vTaskDelay(1500 / portTICK_PERIOD_MS);
    light_driver_set_brightness(10);
    vTaskDelay(1500 / portTICK_PERIOD_MS);
    light_driver_set_colorTemperature(5000);
    vTaskDelay(1500 / portTICK_PERIOD_MS);

    xTaskCreate(Task_set_mode, "Task_set_mode", 1024 * 2, NULL, 8, NULL); // 创建任务
    vTaskDelete(NULL);
}

void app_main()
{

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    pwm_init_data();

    xTaskCreate(Task_brightness_temperature, "Task_brightness_temperature", 1024 * 2, NULL, 8, NULL); // 创建任务
}
