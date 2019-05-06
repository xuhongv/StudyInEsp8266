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
#include "rom/ets_sys.h"
#include "driver/uart.h"
#include "tcpip_adapter.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "dht.h"


/**
 *    这是一个基于 esp8266 3.0 sdk上驱动 dht11 获取温湿度的代码工程：接线 esp8266 gpio5 ---- DHT11 data
 *    有任何技术问题邮箱： 870189248@qq.com
 *    本人GitHub仓库：https://github.com/xuhongv
 *    本人博客：https://blog.csdn.net/xh870189248
 * 
 **/

uint8_t deviceMac[16];

void TaskCreatDht11(void *p)
{
    uint8_t curTem = 0;
    uint8_t curHum = 0;
    ESP_LOGI(TAG, " dh11Init() : %d ", dh11Init());
    while (1)
    {
        vTaskDelay(5000 / portTICK_RATE_MS);
        dh11Read(&curTem, &curHum);
        ESP_LOGI(TAG, "Temperature : %d , Humidity : %d", curTem, curHum);
    }
    vTaskDelete(NULL);
}
static void initialise_dht11()
{
    xTaskCreate(TaskCreatDht11, "TaskCreatDht11", 1024 * 3, NULL, 4, NULL);
}

/******************************************************************************
 * FunctionName : app_main
 * Description  : 程序入口
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
    //获取xMqtt版本
    printf("     xMqtt version:%s\n", getXMqttVersion());
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
    //获取mac地址
    sprintf((char *)deviceMac, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    initialise_dht11();
}
