#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

static EventGroupHandle_t wifi_event_group; //定义一个事件的句柄
const int SCAN_DONE_BIT = BIT0;             //定义事件，占用事件变量的第0位，最多可以定义32个事件。
static wifi_scan_config_t scanConf = {
    .ssid = NULL,
    .bssid = NULL,
    .channel = 0,
    .show_hidden = 1}; //定义scanConf结构体，供函数esp_wifi_scan_start调用

static const char *TAG = "example";

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    if (event->event_id == SYSTEM_EVENT_SCAN_DONE)
    {
        xEventGroupSetBits(wifi_event_group, SCAN_DONE_BIT); //设置事件位
    }
    return ESP_OK;
}

static void initialise_wifi(void) //define a static function ,it's scope is this file
{
    wifi_event_group = xEventGroupCreate();                    //创建一个事件标志组
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL)); //创建事件的任务
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();       //设置默认的wifi栈参数
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));                      //初始化WiFi Alloc资源为WiFi驱动，如WiFi控制结构，RX / TX缓冲区，WiFi NVS结构等，此WiFi也启动WiFi任务。
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));   // Set the WiFi API configuration storage type
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));         //Set the WiFi operating mode
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void scan_task(void *pvParameters)
{
    while (1)
    {
        xEventGroupWaitBits(wifi_event_group, SCAN_DONE_BIT, 0, 1, portMAX_DELAY); //等待事件被置位，即等待扫描完成
        ESP_LOGI(TAG, "WIFI scan done");
        xEventGroupClearBits(wifi_event_group, SCAN_DONE_BIT); //清除事件标志位

        uint16_t apCount = 0;
        esp_wifi_scan_get_ap_num(&apCount); //Get number of APs found in last scan
        printf("Number of access points found: %d\n", apCount);
        if (apCount != 0)
        {

            //如果apCount没有受到数据，则说明没有路由器
            wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount); //定义一个wifi_ap_record_t的结构体的链表空间
            ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, list));                           //获取上次扫描中找到的AP列表。
            int i;
            printf("======================================================================\n");
            printf("             SSID             |    RSSI    |           AUTH     |     CHANNLE     \n");
            printf("======================================================================\n");
            for (i = 0; i < apCount; i++)
            {
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
                printf("%26.26s    |    % 4d    |    %22.22s |  %d  \n", list[i].ssid, list[i].rssi, authmode, scanConf.channel);
            }               //将链表的数据信息打印出来
            free(list);     //释放链表
            printf("\n\n"); //换行
        }
        else
        {
            ESP_LOGI(TAG, "Nothing AP found");
        }

        ESP_LOGI(TAG, "Now scanConf.channel : %d", scanConf.channel);
        //延迟一秒
        vTaskDelay(1000 / portTICK_PERIOD_MS); //调用延时函数，再次扫描
        //扫描开始
        ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, 1)); //扫描所有可用的AP。

        //扫描信道更变
        if (scanConf.channel++ > 13)
        {
            scanConf.channel = 0;
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
    //获取mac地址（station模式）
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    printf("esp_read_mac(): %02x:%02x:%02x:%02x:%02x:%02x \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("--------------------------------------------------------------------------\n\n");                                         
    tcpip_adapter_init();                                      
    initialise_wifi();                                         
    xTaskCreate(&scan_task, "scan_task", 2048, NULL, 15, NULL); 
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, 1));   
}