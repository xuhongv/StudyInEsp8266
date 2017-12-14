/*
 * Copyright (c) 2014-2015 Alibaba Group. All rights reserved.
 *
 * Alibaba Group retains all right, title and interest (including all
 * intellectual property rights) in and to this computer program, which is
 * protected by applicable intellectual property laws.  Unless you have
 * obtained a separate written license from Alibaba Group., you are not
 * authorized to utilize all or a part of this computer program for any
 * purpose (including reproduction, distribution, modification, and
 * compilation into object code), and you must immediately destroy or
 * return to Alibaba Group all copies of this computer program.  If you
 * are licensed by Alibaba Group, your rights to utilize this computer
 * program are limited by the terms of that license.  To obtain a license,
 * please contact Alibaba Group.
 *
 * This computer program contains trade secrets owned by Alibaba Group.
 * and, unless unauthorized by Alibaba Group in writing, you agree to
 * maintain the confidentiality of this computer program and related
 * information and to not disclose this computer program and related
 * information to any other person or entity.
 *
 * THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND
 * Alibaba Group EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * INCLUDING THE WARRANTIES OF MERCHANTIBILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE, AND NONINFRINGEMENT.
 */

#include "c_types.h"
#include <stdio.h>
#include <string.h>
#include "esp_common.h"
#include "freertos/semphr.h"
#include "esp_alink_log.h"
#include "esp_info_store.h"

#define EVENT_QUEUE_NUM     (5)

static const char *TAG = "esp_alink_main";

static xQueueHandle xQueueEvent = NULL;
xSemaphoreHandle s_need_notify_app = NULL;

alink_err_t alink_event_send(alink_event_t event)
{
    if (!xQueueEvent) {
        xQueueEvent = xQueueCreate(EVENT_QUEUE_NUM, sizeof(alink_event_t));
    }
    
    ALINK_LOGD("send event, event value:%d", event);    
    alink_err_t ret = xQueueSend(xQueueEvent, &event, 0);
    ALINK_ERROR_CHECK(ret != pdTRUE, ALINK_ERR, "xQueueSendToBack fail!");

    return ALINK_OK;
}

alink_err_t alink_update_router()
{
    int ret = ALINK_ERR;
    ALINK_LOGI("clear wifi config");
    ret = esp_info_erase(NVS_KEY_WIFI_CONFIG);
    system_restart();

    return ret;
}

void alink_factory_reset_task()
{
    alink_factory_reset();
    vTaskDelay(500 / portTICK_RATE_MS);
    vTaskDelete(NULL);
}

void alink_factory_setting()
{
    ALINK_LOGI("*********************************");
    ALINK_LOGI("*         FACTORY RESET         *");
    ALINK_LOGI("*********************************");
    
    xTaskCreate(alink_factory_reset_task, "alink_factory_reset_task", 512, NULL,
        tskIDLE_PRIORITY + 5, NULL);
}

xTaskHandle event_handle = NULL;
void alink_event_loop_task(void *pvParameters)
{
    alink_err_t ret = ALINK_OK;
    alink_event_cb_t s_event_handler_cb = (alink_event_cb_t)pvParameters;

    while (1) {
        alink_event_t event;

        if (xQueueReceive(xQueueEvent, &event, portMAX_DELAY) != pdPASS) {
            continue;
        }
        
        ALINK_LOGD("receive event, event value:%d", event);

        if (!s_event_handler_cb) {
            continue;
        }

        ret = (*s_event_handler_cb)(event);

        if (ret != ALINK_OK) {
            ALINK_LOGE("Event handling failed");
        }
    }
    vTaskDelete(NULL);
}

xTaskHandle post_handle = NULL;
void alink_post_data_task(void *pvParameters)
{
    alink_device_post_data();
}

alink_err_t esp_alink_init(alink_product_t* info, alink_event_cb_t event_handler_cb)
{
    ALINK_PARAM_CHECK(!info);
    ALINK_PARAM_CHECK(!event_handler_cb);

    int ret = ALINK_OK;
    
    if (s_need_notify_app == NULL) {
        portENTER_CRITICAL();
        vSemaphoreCreateBinary(s_need_notify_app);
        if (s_need_notify_app) {
            xSemaphoreTake(s_need_notify_app, 0);
        }
        portEXIT_CRITICAL();
    }
    
    if (!xQueueEvent) {
        xQueueEvent = xQueueCreate(EVENT_QUEUE_NUM, sizeof(alink_event_t));
    }
    
    alink_set_loglevel(ALINK_SDK_LOG_LEVEL);
    xTaskCreate(alink_event_loop_task, "alink_event_loop_task", 256, event_handler_cb,
        tskIDLE_PRIORITY + 5, &event_handle);

    ret = alink_connect_ap();
    ALINK_ERROR_CHECK(ret != ALINK_OK, ALINK_ERR, "connect ap fail");

    ret = alink_trans_init(info);
    ALINK_ERROR_CHECK(ret != ALINK_OK, ALINK_ERR, "alink trans init fail");

    xTaskCreate(alink_post_data_task, "alink_post_data_task", 512+32, NULL,
        tskIDLE_PRIORITY + 5, &post_handle);

    return ret;
}


