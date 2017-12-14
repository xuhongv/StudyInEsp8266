/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2017 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "esp_common.h" 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "alink_export.h"
#include "user_config.h"
#include "esp_alink.h"
#include "esp_alink_log.h"
#include "alink_json.h"

#ifdef ALINK_PASSTHROUGH

static const char *TAG = "sample_passthrough";

typedef struct {
	uint8_t power;
	uint8_t work_mode;
	uint8_t temp_value;
	uint8_t light_value;
	uint8_t time_delay;
}virtual_dev_t;

typedef struct {
    uint8_t header1;
    uint8_t header2;
    virtual_dev_t virtual_dev_state;
    uint8_t check;
}virtual_dev_cmd_t;

void user_show_rst_info(void)
{
    struct rst_info *rtc_info = system_get_rst_info();
    ALINK_LOGI("reset reason: %x", rtc_info->reason);

    if ((rtc_info->reason == REASON_WDT_RST) 
        || (rtc_info->reason == REASON_EXCEPTION_RST) 
        ||(rtc_info->reason == REASON_SOFT_WDT_RST)) {

        if (rtc_info->reason == REASON_EXCEPTION_RST) 
        {           
            ALINK_LOGI("Fatal exception (%d):", rtc_info->exccause);       
        }

        ALINK_LOGI("dbg: epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x",rtc_info->epc1, rtc_info->epc2, rtc_info->epc3, rtc_info->excvaddr, rtc_info->depc);
    }
}

static virtual_dev_t virtual_device;
static alink_err_t proactive_report_data()
{
    virtual_dev_cmd_t up_cmd;
    
    up_cmd.header1 = 0xaa;
    up_cmd.header2 = 0x07;
    up_cmd.virtual_dev_state.power = virtual_device.power;
    up_cmd.virtual_dev_state.work_mode = virtual_device.work_mode;
    up_cmd.virtual_dev_state.temp_value = virtual_device.temp_value;
    up_cmd.virtual_dev_state.light_value = virtual_device.light_value;
    up_cmd.virtual_dev_state.time_delay = virtual_device.time_delay;
    up_cmd.check = 0x55;
    
    if (alink_write(&up_cmd, ALINK_PASSTHROUGH_DATA_LEN, 0) < 0) {
        ALINK_LOGE("alink_write is error");
    }

    return ALINK_OK;
}

static void read_task_test(void *arg)
{
    while (1) {
        ALINK_LOGI("read down cmd param");
        virtual_dev_cmd_t down_cmd;

        if (alink_read(&down_cmd, ALINK_DATA_LEN, portMAX_DELAY) < 0) {
            ALINK_LOGW("alink_read is error");
            continue;
        }

        virtual_device.power = down_cmd.virtual_dev_state.power;
        virtual_device.work_mode = down_cmd.virtual_dev_state.work_mode;
        virtual_device.temp_value = down_cmd.virtual_dev_state.temp_value;
        virtual_device.light_value = down_cmd.virtual_dev_state.light_value;
        virtual_device.time_delay = down_cmd.virtual_dev_state.time_delay;
        ALINK_LOGI("read: OnOff_Power:%d, WorkMode_MasterLight: %d, Color_Temperature: %d, Light_Brightness: %d, TimeDelay_PowerOff: %d, free heap: %d\n",
                   virtual_device.power, virtual_device.work_mode, virtual_device.temp_value, virtual_device.light_value, virtual_device.time_delay,
                   system_get_free_heap_size());
        
        /* post device status to cloud after read */
        proactive_report_data();
    }
}

static int count = 0; /*!< Count the number of packets received */
static alink_err_t alink_event_handler(alink_event_t event)
{
    switch (event) {
        case ALINK_EVENT_CLOUD_CONNECTED:
            ALINK_LOGD("Alink cloud connected!");
            proactive_report_data();
            break;

        case ALINK_EVENT_CLOUD_DISCONNECTED:
            ALINK_LOGD("Alink cloud disconnected!");
            break;

        case ALINK_EVENT_GET_DEVICE_DATA:
            ALINK_LOGD("The cloud initiates a query to the device");
            break;

        case ALINK_EVENT_SET_DEVICE_DATA:
            count++;
            ALINK_LOGD("The cloud is set to send instructions");
            break;

        case ALINK_EVENT_POST_CLOUD_DATA:
            ALINK_LOGD("The device post data success!");
            break;

        case ALINK_EVENT_STA_WIFI_DISCONNECTED:
            ALINK_LOGD("Wifi disconnected");
            break;

        case ALINK_EVENT_CONFIG_NETWORK:
            ALINK_LOGD("Enter the network configuration mode");
            break;

        case ALINK_EVENT_UPDATE_ROUTER:
            ALINK_LOGD("Requests update router");
            alink_update_router();
            break;

        case ALINK_EVENT_FACTORY_RESET:
            ALINK_LOGD("Requests factory reset");
            alink_factory_setting();
            break;

        default:
            break;
    }

    return ALINK_OK;
}

static xTaskHandle read_handle = NULL;
void start_alink_task(void *pvParameters)
{
    ALINK_LOGD("start demo task, free heap size %d", system_get_free_heap_size());
    
    /* fill main device info here */
    alink_product_t product_info = {
        .sn             = DEV_SN,
        .name           = DEV_NAME,
        .brand          = DEV_BRAND,
        .type           = DEV_TYPE,
        .category       = DEV_CATEGORY,
        .manufacturer   = DEV_MANUFACTURE,
        .version        = DEV_VERSION,
        .model          = DEV_MODEL,
        .key            = ALINK_KEY,
        .secret         = ALINK_SECRET,
        .key_sandbox    = ALINK_KEY_SANDBOX,
        .secret_sandbox = ALINK_SECRET_SANDBOX,
    };
    alink_key_trigger();
    esp_info_init();
    esp_alink_init(&product_info, alink_event_handler);
    xTaskCreate(read_task_test, "read_task_test", (1024 + 512) / 4, NULL, tskIDLE_PRIORITY+5, &read_handle);
    vTaskDelete(NULL);
}

#ifdef SAMPLE_JSON_DEBUG
extern xTaskHandle event_handle;
extern xTaskHandle post_handle;
static void alink_debug_task(void *arg)
{
    while (1) {
        ALINK_LOGI("total free heap size: %d, count: %d", system_get_free_heap_size(), count);

        if (event_handle) {
            ALINK_LOGI("event_handle free heap size: %dB", uxTaskGetStackHighWaterMark(event_handle) * 4);
        }

        if (post_handle) {
            ALINK_LOGI("post_handle free heap size: %dB", uxTaskGetStackHighWaterMark(post_handle) * 4);
        }

        if (read_handle) {
            ALINK_LOGI("read_handle free heap size: %dB", uxTaskGetStackHighWaterMark(read_handle) * 4);
        }

        vTaskDelay(5000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}
#endif

void user_demo(void)
{   
#ifdef SAMPLE_JSON_DEBUG
    user_show_rst_info();    /* print reset reason and information */
    xTaskCreate(alink_debug_task, "debug_task", 512 / 4 , NULL, tskIDLE_PRIORITY+3, NULL);
#endif

    ALINK_LOGI("****************************");
    ALINK_LOGI("SDK version:%s", system_get_sdk_version());
    ALINK_LOGI("Alink version:%s", USER_ALINK_GLOBAL_VER);
    ALINK_LOGI("User fiemware version:1.0.0(20170906@esp)");
    ALINK_LOGI("Current userbin start address: %x", system_get_userbin_addr());
    ALINK_LOGI("Esp debug heap size %d", system_get_free_heap_size());
    ALINK_LOGI("****************************");

    xTaskCreate(start_alink_task, "start_alink_task",405 + 200, NULL, tskIDLE_PRIORITY+5, NULL);
}

#endif

