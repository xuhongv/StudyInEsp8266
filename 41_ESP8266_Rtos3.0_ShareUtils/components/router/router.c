#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "rom/ets_sys.h"
#include "esp_wifi.h"
#include "quick.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "router";

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
void router_wifi_clean_info(void)
{
    nvs_handle out_handle;
    if (nvs_open("wifi_info", NVS_READWRITE, &out_handle) == ESP_OK)
    {
        nvs_erase_all(out_handle);
        nvs_close(out_handle);
    }
}

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
void router_wifi_save_info(uint8_t *ssid, uint8_t *password)
{
    nvs_handle out_handle;
    char data[65];
    if (nvs_open("wifi_info", NVS_READWRITE, &out_handle) != ESP_OK)
    {
        return;
    }

    memset(data, 0x0, sizeof(data));
    strncpy(data, (char *)ssid, strlen((char *)ssid));
    if (nvs_set_str(out_handle, "ssid", data) != ESP_OK)
    {
        printf("--set ssid fail");
    }

    memset(data, 0x0, sizeof(data));
    strncpy(data, (char *)password, strlen((char *)password));
    if (nvs_set_str(out_handle, "password", data) != ESP_OK)
    {
        printf("--set password fail");
    }
    nvs_close(out_handle);
}

/**
 * @description: 是否连接服务器
 * @param {type} 
 * @return: 
 */
bool routerStartConnect()
{

    if (get_sc_flag() > 4)
    {
        return false;
    }

    size_t size = 0;
    nvs_handle out_handle;
    bool isGetSSID = false;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    //从本地存储读取是否存在ssid和password
    if (nvs_open("wifi_info", NVS_READONLY, &out_handle) == ESP_OK)
    {
        wifi_config_t config;
        memset(&config, 0x0, sizeof(config));
        size = sizeof(config.sta.ssid);
        if (nvs_get_str(out_handle, "ssid", (char *)config.sta.ssid, &size) == ESP_OK)
        {
            if (size > 0)
            {
                size = sizeof(config.sta.password);
                if (nvs_get_str(out_handle, "password", (char *)config.sta.password, &size) == ESP_OK)
                {
                    ESP_LOGI(TAG, "-- get ssid: %s", config.sta.ssid);
                    ESP_LOGI(TAG, "-- get password: %s", config.sta.password);
                    isGetSSID = true;
                    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &config));
                    ESP_ERROR_CHECK(esp_wifi_start());
                    ESP_ERROR_CHECK(esp_wifi_connect());
                }
                else
                {
                    ESP_LOGI(TAG, "--get password fail");
                }
            }
        }
        nvs_close(out_handle);
    }

    return isGetSSID;
}
