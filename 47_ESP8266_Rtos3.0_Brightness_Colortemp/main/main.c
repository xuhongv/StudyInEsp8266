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

    //亮度取值范围为 0~100 ，色温取值范围2700~6500， 均在 头文件可定义
    light_driver_set_ctb(100, 6500);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_ctb(100, 2700);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_ctb(100, 4600);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    light_driver_set_ctb(0, 0);
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    while (1)
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
    }
}
