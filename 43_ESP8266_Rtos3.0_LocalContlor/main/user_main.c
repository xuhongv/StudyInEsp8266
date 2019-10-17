/*
 * @Description: 本地局域网通讯的例子
 * @Author: 徐宏
 * @Date: 2019-10-17 09:36:23
 * @LastEditTime: 2019-10-17 15:42:07
 * @LastEditors: Please set LastEditors
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "event_groups.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "esp_event_loop.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

void findStr(char *input, char *output)
{
    char *outputStart = strchr(input, '{');
    char *outputEnd = NULL;
    if (NULL != outputStart)
    {
        outputEnd = strrchr(input, '}');
        if (NULL != outputEnd)
        {
            memcpy(output, outputStart, outputEnd - outputStart + 1);
        }
    }
    output[outputEnd - outputStart + 1] = '\0';
}

static const char *TAG = "main";

static EventGroupHandle_t wifi_event_group;

//MQTT Message
typedef struct esp_mqtt_msg_type
{
    int power;
    int cw;
    int ww;
    int r;
    int g;
    int b;
    int workMode;
    int skill;
    char allData[1024];
} xMessage;
xMessage tempMsg;

static xQueueHandle ParseJSONQueueHandler = NULL;                                                       //解析json数据的队列
static xTaskHandle mHandlerParseJSON = NULL, mHandlerLocalServer = NULL, mHandlerLocalBroadcast = NULL; //任务队列

char udp_msg[102]; //固定的本地广播数据

void Task_ParseJSON(void *pvParameters)
{
    printf("[SY] Task_ParseJSON_Message creat ... \n");
    while (1)
    {
        struct esp_mqtt_msg_type *pMqttMsg;

        printf("[SY] Task_ParseJSON_Message xQueueReceive wait ... \n");
        xQueueReceive(ParseJSONQueueHandler, &pMqttMsg, portMAX_DELAY);
        printf("[SY] xQueueReceive  data [%s] \n", pMqttMsg->allData);
    }
}
void Task_local_server(void *ab)
{

    char rx_buffer[512];
    char addr_str[128];
    char output[1024] = {0};
    int addr_family;
    int ip_protocol;

    while (1)
    {

        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(80);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (listen_sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err != 0)
        {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket binded");

        err = listen(listen_sock, 1);
        if (err != 0)
        {
            ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket listening");
        int sock;
        while (1)
        {
            struct sockaddr_in sourceAddr;
            uint addrLen = sizeof(sourceAddr);
            sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
            if (sock < 0)
            {
                ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Socket accepted");

            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occured during receiving
            if (len > 0)
            {
                char *sendstr = (char *)malloc(1024);
                inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);
                findStr(rx_buffer, output);
                ESP_LOGI(TAG, "output : %s", output);
                //发送数据到队列
                struct esp_mqtt_msg_type *pTmp;
                tempMsg.workMode = 0;
                sprintf(tempMsg.allData, "%s", output);
                pTmp = &tempMsg;
                xQueueSend(ParseJSONQueueHandler, (void *)&pTmp, portMAX_DELAY);

                sprintf(sendstr, "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\nContent-Length: %d\nContent-Type: text/html\nServer: ESP8266\n\n%s", strlen(output), output);
                int err = send(sock, sendstr, strlen(sendstr), 0);
                free(sendstr);
                int result = close(sock);
                ESP_LOGI(TAG, "Shutting down socket %d result , esp_get_free_heap_size(): %d", result, esp_get_free_heap_size());
            }
        }

        if (sock != -1)
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

void Task_local_broadcast(void *pData)
{
    struct sockaddr_in client_addr;
    int sock_fd;
    int err;
    int count = 0;

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1)
    {
        printf("[SY] failed to create sock_fd!\n");
    }

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    client_addr.sin_port = htons(8989);

    while (1)
    {
        //sprintf(udp_msg,"");
        vTaskDelay(2000 / portTICK_RATE_MS);
        err = sendto(sock_fd, (char *)udp_msg, sizeof(udp_msg), 0,
                     (struct sockaddr *)&client_addr, sizeof(client_addr));
        if (err < 0)
        {
            printf("[SY] failed to send local ! ... \n");
        }
    }
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{

    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, "ESP8266_XUHONG");
        esp_wifi_connect();
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");

        if (mHandlerLocalServer == NULL)
        {
            xTaskCreate(Task_local_server, "Task_Sync_net_time", 4096, NULL, 4,
                        &mHandlerLocalServer);
            printf("[SY] create Task_local_server thread ...\n");
        }
        //锟斤拷锟斤拷锟斤拷锟斤拷状态锟姐播锟竭筹拷
        if (mHandlerLocalBroadcast == NULL)
        {
            xTaskCreate(Task_local_broadcast, "Task_local_broadcast", 512, NULL, 3, &mHandlerLocalBroadcast);
            printf("[SY] create Task_local_broadcast thread ...\n");
        }

        sprintf(udp_msg, "{\"name\":\"xuhong\",\"age\":18\",\"ip\":\"%s\"}", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        break;

    default:
        break;
    }
    return ESP_OK;
}

/**
 * @description: wifi连接路由器初始化
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
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "MZX5L",
            .password = "mzx@123456",
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

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

    initialise_wifi();
    if (ParseJSONQueueHandler == NULL)
        ParseJSONQueueHandler = xQueueCreate(5, sizeof(struct esp_mqtt_msg_type *));

    //开启json解析线程
    if (mHandlerParseJSON == NULL)
    {
        xTaskCreate(Task_ParseJSON, "Task_ParseJSON", 1024, NULL, 3, &mHandlerParseJSON);
    }
}
