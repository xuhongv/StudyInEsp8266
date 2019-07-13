#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

/**
 *    有任何技术问题邮箱： 870189248@qq.com
 *    本人GitHub仓库：https://github.com/xuhongv
 *    本人博客：https://blog.csdn.net/xh870189248
 *    
 *    注意事项 ------> 代码仅仅可以扫描出附近的 2.4G频段的AP！
 * 
 **/

static const char *TAG = "ScanAPList";

static void TaskScanResult(void *pvParameters);

static EventGroupHandle_t wifi_event_group;

const int BIT_DONE = BIT0;

//定义wifi_scan_config_t结构体
static wifi_scan_config_t configForScan = {
    .ssid = NULL,
    .bssid = NULL,
    .channel = 1,     //扫描的信道
    .show_hidden = 1, //是否隐藏的ssid
};

/**
 * @description: 监听wifi系统事件
 * @param {type} 
 * @return: 
 */
esp_err_t event_handler(void *ctx, system_event_t *event)
{
    system_event_info_t *info = &event->event_info;
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        xTaskCreate(&TaskScanResult, "TaskScanResult", 4096, NULL, 10, NULL);
        break;

    case SYSTEM_EVENT_SCAN_DONE:
        xEventGroupSetBits(wifi_event_group, BIT_DONE);
        break;
    default:
        break;
    }

    return ESP_OK;
}

/**
 * @description: wiif初始化
 * @param {type} 
 * @return: 
 */
static void initialise_wifi(void)
{

    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/**
 * @description: 扫描结果获取
 * @param {type} 
 * @return: 
 */
static void TaskScanResult(void *pvParameters)
{
    while (1)
    {

        //阻塞等待
        xEventGroupWaitBits(wifi_event_group, BIT_DONE, 0, 1, portMAX_DELAY);

        uint16_t counts = 0;
        //从上次扫描结果获取数量
        esp_wifi_scan_get_ap_num(&counts);

        //如果counts为0，则未在此信道扫描到可用的wifi路由器ap热点
        if (counts != 0)
        {

            wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * counts);
            //获取上次扫描中找到的AP列表
            ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&counts, list));

            int i;
            for (i = 0; i < counts; i++)
            {
                if (strlen((const char *)list[i].ssid) == 0)
                {
                    continue;
                }
                char *authmode;
                switch (list[i].authmode)
                {
                case WIFI_AUTH_OPEN:
                    authmode = "WIFI_AUTH_OPEN";
                    break;
                case WIFI_AUTH_WEP:
                    authmode = "WIFI_AUTH_WEP";
                    break;
                case WIFI_AUTH_WPA_PSK:
                    authmode = "WIFI_AUTH_WPA_PSK";
                    break;
                case WIFI_AUTH_WPA2_PSK:
                    authmode = "WIFI_AUTH_WPA2_PSK";
                    break;
                case WIFI_AUTH_WPA_WPA2_PSK:
                    authmode = "WIFI_AUTH_WPA_WPA2_PSK";
                    break;
                default:
                    authmode = "Unknown";
                    break;
                }

                printf("AP Name: %s| SSID: %d | oauthMode：%22.22s | channle:%d  \n", list[i].ssid, list[i].rssi, authmode, configForScan.channel);
            }
            free(list); //根据官网的描述，这里必须释放
        }
        else
        {
            ESP_LOGE(TAG, "No AP to find at channle[%d].", configForScan.channel);
        }
        //组成json格式数据
        xEventGroupClearBits(wifi_event_group, BIT_DONE);
        //延迟一秒
        vTaskDelay(500 / portTICK_PERIOD_MS); //调用延时函数，再次扫描
        //扫描开始
        //找到的APs将存储在WiFi驱动程序动态分配的内存中，将在esp_wifi_scan_get_ap_records中释放，
        //因此通常调用esp_wifi_scan_get_ap_records
        //扫描完成后要释放的内存
        //第二个参数如果block为真，这个API将阻塞调用者，直到扫描完成，否则了立刻返回
        ESP_ERROR_CHECK(esp_wifi_scan_start(&configForScan, true)); //开始扫描周围所有可用的AP。

        //扫描信道更变
        if (configForScan.channel++ > 12)
        {
            configForScan.channel = 1;
        }
    }
}
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
    uint8_t mac[6];
    //获取mac地址（station模式）
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    printf(" Station esp_read_mac(): %02x:%02x:%02x:%02x:%02x:%02x \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    //获取mac地址（ap模式）
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    printf(" AP esp_read_mac(): %02x:%02x:%02x:%02x:%02x:%02x \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("--------------------------------------------------------------------------\n\n");

    initialise_wifi();

    ESP_ERROR_CHECK(esp_wifi_scan_start(&configForScan, 1));
}