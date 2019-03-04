/* Simple WiFi Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "rom/ets_sys.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_MODE_AP CONFIG_ESP_WIFI_MODE_AP //TRUE:AP FALSE:STA
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_MAX_STA_CONN CONFIG_MAX_STA_CONN

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "simple wifi";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "station:" MACSTR " join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "station:" MACSTR "leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void funtion_wifi_clear_info(void)
{
    nvs_handle out_handle;
    if (nvs_open("wifi_info", NVS_READWRITE, &out_handle) == ESP_OK)
    {
        nvs_erase_all(out_handle);
        nvs_close(out_handle);
    }
}

void funtion_wifi_save_info(uint8_t *ssid, uint8_t *password)
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

static void wifi_init_sta(void *pvParameters)
{

    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
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
                }
                else
                {
                    ESP_LOGI(TAG, "--get password fail");
                }
            }
        }
        nvs_close(out_handle);
    }

    if (!isGetSSID)
    {
        ESP_LOGI(TAG, "--get ssid fail");
        wifi_config_t wifi_config = {
            .sta = {
                .ssid = EXAMPLE_ESP_WIFI_SSID,
                .password = EXAMPLE_ESP_WIFI_PASS},
        };
        funtion_wifi_save_info((unsigned char *)EXAMPLE_ESP_WIFI_SSID, (unsigned char *)EXAMPLE_ESP_WIFI_PASS);

        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    vTaskDelete(NULL);
}

void app_main()
{

    printf("SDK version:%s\n", esp_get_idf_version());

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    xTaskCreate(wifi_init_sta, "wifi_init_sta", 1024 * 10, NULL, 2, NULL);
}
