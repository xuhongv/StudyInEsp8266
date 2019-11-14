/*
 * @Author: your name
 * @Date: 2019-10-13 12:49:47
 * @LastEditTime: 2019-11-14 21:33:00
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \ESP8266_RTOS_SDK\examples\get-started\project_template\main\user_main.c
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

//这个宏的数值是从 自定义分区表获取
#define MFG_PARTITION_NAME "fctry"
//这个宏的数值是从 csv文件定义获取
#define NVS_PRODUCT "user-info"

static const char *TAG = "wrapper_product";

/**
 * @description:  读取数据进去nvs里面的任务
 * @param {type} null
 * @return: 
 */
static void Task_NVS_Read(void *pvParameters)
{

    ESP_LOGI(TAG, "--------------------------- Start Task_NVS_Read  --------------------------");
    
    nvs_handle mHandleNvsRead;
    uint8_t nvs_i8 = 0;
    esp_err_t err;

    //初始化分区
    if ((err = nvs_flash_init_partition(MFG_PARTITION_NAME)) != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS Flash init %s failed, Please check that you have flashed fctry partition!!!", MFG_PARTITION_NAME);
        vTaskDelete(NULL);
    }

    //NVS操作的句柄，类似于 rtos系统的任务创建返回的句柄！
    err = nvs_open_from_partition(MFG_PARTITION_NAME, NVS_PRODUCT, NVS_READONLY, &mHandleNvsRead);

    //打开数据库，打开一个数据库就相当于会返回一个句柄
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Open NVS Table fail");
        vTaskDelete(NULL);
    }
    else
    {
        ESP_LOGI(TAG, "Open NVS Table ok.");
    }

    //读取 字符串 --> 名字
    char data[65];
    uint32_t len = sizeof(data);
    err = nvs_get_str(mHandleNvsRead, "Name", data, &len);

    if (err == ESP_OK)
        ESP_LOGI(TAG, "get str data = %s ", data);
    else
        ESP_LOGI(TAG, "get str data error");

    //读取 u8 年龄
    err = nvs_get_u8(mHandleNvsRead, "Age", &nvs_i8);
    if (err == ESP_OK)
        ESP_LOGI(TAG, "get nvs_i8 = %d ", nvs_i8);
    else
        ESP_LOGI(TAG, "get nvs_i8 error");

    //读取 字符串 --> Csdn博客地址
    char Csdn[65];
    uint32_t lenCsdn = sizeof(Csdn);
    err = nvs_get_str(mHandleNvsRead, "Csdn", Csdn, &lenCsdn);

    if (err == ESP_OK)
        ESP_LOGI(TAG, "get str data = %s ", Csdn);
    else
        ESP_LOGI(TAG, "get Csdn data error");

    //读取 字符串 --> GitHub地址
    char GitHub[65];
    uint32_t lenGitHub = sizeof(GitHub);
    err = nvs_get_str(mHandleNvsRead, "GitHub", GitHub, &lenGitHub);

    if (err == ESP_OK)
        ESP_LOGI(TAG, "get GitHub data = %s ", GitHub);
    else
        ESP_LOGI(TAG, "get GitHub data error");

    //关闭数据库，关闭面板！
    nvs_close(mHandleNvsRead);

    ESP_LOGI(TAG, "--------------------------- End Task_NVS_Read  --------------------------");
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

    //创建一个读取的任务
    xTaskCreate(Task_NVS_Read, "Task_NVS_Read", 1024, NULL, 5, NULL);
}
