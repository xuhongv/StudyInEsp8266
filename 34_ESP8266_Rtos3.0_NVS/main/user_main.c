

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
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/pwm.h"
#include "driver/uart.h"

static const char *TAG = "XuHongLog";

//自定义一个结构体
typedef struct
{
    char name[10];
    int8_t age;
    bool sex;
} User_Info;

//数据库的表名
static const char *TB_SELF = "Tb_Self";
//保存与读取 int8_t 类型的 字段名
static const char *FILED_SELF_i8 = "int8_t_Self";
//保存与读取 字符串类型的 字段名
static const char *FILED_SELF_Str = "str_Self";
//保存与读取 数组类型的 字段名
static const char *FILED_SELF_Group = "group_Self";
//保存与读取 结构体类型的 字段名
static const char *FILED_SELF_Struct = "struct_Self";

/**
 * @description:  读取数据进去nvs里面的任务
 * @param {type} 
 * @return: 
 */
static void Task_NVS_Read(void *pvParameters)
{

    ESP_LOGI(TAG, "--------------------------- Start Task_NVS_Read  --------------------------");

    //NVS操作的句柄，类似于 rtos系统的任务创建返回的句柄！
    nvs_handle mHandleNvsRead;
    int8_t nvs_i8 = 0;

    esp_err_t err = nvs_open(TB_SELF, NVS_READWRITE, &mHandleNvsRead);
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

    //读取 i8
    err = nvs_get_i8(mHandleNvsRead, FILED_SELF_i8, &nvs_i8);
    if (err == ESP_OK)
        ESP_LOGI(TAG, "get nvs_i8 = %d ", nvs_i8);
    else
        ESP_LOGI(TAG, "get nvs_i8 error");

    //读取 字符串
    char data[65];
    uint32_t len = sizeof(data);
    err = nvs_get_str(mHandleNvsRead, FILED_SELF_Str, data, &len);

    if (err == ESP_OK)
        ESP_LOGI(TAG, "get str data = %s ", data);
    else
        ESP_LOGI(TAG, "get str data error");

    //读取数组
    uint8_t group_myself_read[8];
    size_t size = sizeof(group_myself_read);
    err = nvs_get_blob(mHandleNvsRead, FILED_SELF_Group, group_myself_read, &size);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "get group_myself_read data OK !");
        for (uint32_t i = 0; i < size; i++)
        {
            ESP_LOGI(TAG, "get group_myself_read data : [%d] =%02x", i, group_myself_read[i]);
        }
    }

    //读取结构体
    User_Info user;
    memset(&user, 0x0, sizeof(user));
    uint32_t length = sizeof(user);
    err = nvs_get_blob(mHandleNvsRead, FILED_SELF_Struct, &user, &length);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "get user Struct name = %s !", user.name);
        ESP_LOGI(TAG, "get user Struct age = %d !", user.age);
        ESP_LOGI(TAG, "get user Struct sex = %d !", user.sex);
    }

    //关闭数据库，关闭面板！
    nvs_close(mHandleNvsRead);

    ESP_LOGI(TAG, "--------------------------- End Task_NVS_Read  --------------------------");
    vTaskDelete(NULL);
}

/**
 * @description:  创建一个写数据进去nvs里面的任务
 * @param {type} 
 * @return: 
 */
static void Task_NVS_Write(void *pvParameters)
{

    ESP_LOGI(TAG, "--------------------------- Start Task_NVS_Write  --------------------------");

    //NVS操作的句柄，类似于 rtos系统的任务创建返回的句柄！
    nvs_handle mHandleNvs;
    //注意int8_t的取值范围，根据自身的业务需求来做保存类型
    int8_t nvs_i8 = 11;

    //打开数据库，打开一个数据库就相当于会返回一个句柄
    if (nvs_open(TB_SELF, NVS_READWRITE, &mHandleNvs) != ESP_OK)
    {
        ESP_LOGE(TAG, "Open NVS Table fail");
        vTaskDelete(NULL);
    }

    //保存一个 int8_t
    esp_err_t err = nvs_set_i8(mHandleNvs, FILED_SELF_i8, nvs_i8);
    if (err != ESP_OK)
        ESP_LOGE(TAG, "Save NVS i8 error !!");
    else
        ESP_LOGI(TAG, "Save NVS i8 ok !! nvs_i8 = %d ", nvs_i8);

    //提交下！相当于软件面板的 “应用” 按钮，并没关闭面板！
    nvs_commit(mHandleNvs);

    //自定义一个字符串
    char data[65] = {"https://xuhong.blog.csdn.net"};

    //保存一个字符串
    if (nvs_set_str(mHandleNvs, FILED_SELF_Str, data) != ESP_OK)
        ESP_LOGE(TAG, "Save NVS String Fail !!  ");
    else
        ESP_LOGI(TAG, "Save NVS String ok !! data : %s ", data);

    //提交下！相当于软件面板的 “应用” 按钮，并没关闭面板！
    nvs_commit(mHandleNvs);

    //自定义一个数组
    uint8_t group_myself[8] = {0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};

    if (nvs_set_blob(mHandleNvs, FILED_SELF_Group, group_myself, sizeof(group_myself)) != ESP_OK)
        ESP_LOGE(TAG, "Save group  Fail !!  ");
    else
        ESP_LOGI(TAG, "Save group  ok !!  ");

    //提交下！相当于软件面板的 “应用” 按钮，并没关闭面板！
    nvs_commit(mHandleNvs);

    //保存一个结构体
    User_Info user = {
        .name = "xuhong",
        .age = 18,
        .sex = 10};

    if (nvs_set_blob(mHandleNvs, FILED_SELF_Struct, &user, sizeof(user)) != ESP_OK)
        ESP_LOGE(TAG, "Save Struct  Fail !!  ");
    else
        ESP_LOGI(TAG, "Save Struct  ok !!  ");

    //提交下！相当于软件面板的 “应用” 按钮，并没关闭面板！
    nvs_commit(mHandleNvs);

    //关闭数据库，关闭面板！
    nvs_close(mHandleNvs);

    ESP_LOGI(TAG, "---------------------------End  Task_NVS_Write  --------------------------\n");
    //创建一个读取的任务
    xTaskCreate(Task_NVS_Read, "Task_NVS_Read", 1024, NULL, 5, NULL);

    vTaskDelay(500 / portTICK_RATE_MS);
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
    xTaskCreate(Task_NVS_Write, "Task_NVS_Write", 1024, NULL, 5, NULL);
}
