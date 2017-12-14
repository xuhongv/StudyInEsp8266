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

#include <stdio.h>
#include <string.h>
#include "c_types.h"
#include "alink_export_rawdata.h"
#include "esp_common.h"
#include "freertos/semphr.h"
#include "alink_adaptation.h"
#include "esp_alink_log.h"
#include "esp_info_store.h"
#include "user_config.h"

static const char *TAG = "esp_alink_data_transport";
static bool post_data_enable = ALINK_TRUE;

static xQueueHandle xQueueDownCmd = NULL;
static xQueueHandle xQueueUpCmd = NULL;
extern xSemaphoreHandle s_need_notify_app;

alink_err_t alink_device_notify_app(void)
{
    xSemaphoreGive(s_need_notify_app);
    return ALINK_OK;
}

ssize_t alink_write(_IN_ const void *up_cmd, size_t len, int micro_seconds)
{
    ALINK_PARAM_CHECK(!up_cmd);
    ALINK_PARAM_CHECK(len == 0 || len > ALINK_DATA_LEN);

    ALINK_ERROR_CHECK(!xQueueUpCmd, ALINK_ERR ,"upcmd queue is NULL");

    int i = 0;
    alink_err_t ret = ALINK_OK;
    char *q_data = alink_malloc(ALINK_DATA_LEN);
    memcpy(q_data, up_cmd, len);
    ret = xQueueSend(xQueueUpCmd, &q_data, micro_seconds / portTICK_RATE_MS);

    if (ret == pdFALSE) {
        ALINK_LOGE("xQueueSend xQueueUpCmd, wait_time: %d", micro_seconds);
        alink_free(q_data);
    } else {
        ret = len;
    }
    
    return ret;
}

ssize_t alink_read(_OUT_ void *down_cmd, size_t size, int  micro_seconds)
{
    ALINK_PARAM_CHECK(!down_cmd);
    ALINK_PARAM_CHECK(size == 0 || size > ALINK_DATA_LEN);

    ALINK_ERROR_CHECK(!xQueueDownCmd, ALINK_ERR, "downcmd queue is NULL");
    
    alink_err_t ret = ALINK_OK;
    
    char *q_data = NULL;
    ret = xQueueReceive(xQueueDownCmd, &q_data, micro_seconds / portTICK_RATE_MS);

    if (ret == pdFALSE) {
        ALINK_LOGE("xQueueReceive xQueueDownCmd fail, ret:%d, wait_time: %d", ret, 
            micro_seconds);
        alink_free(q_data);
        return ALINK_ERR;
    }

    int size_tmp = 0;
    
#ifdef ALINK_PASSTHROUGH
    size_tmp = ALINK_PASSTHROUGH_DATA_LEN;
#else 
    size_tmp = strlen(q_data) + 1;
#endif

    size = (size_tmp > size) ? size : size_tmp;

    if (size > ALINK_DATA_LEN) {
        ALINK_LOGW("read len > ALINK_DATA_LEN, len: %d", size);
        size = ALINK_DATA_LEN;
        q_data[size - 1] = '\0';
    }
    
    memcpy(down_cmd, q_data, size);
    alink_free(q_data);

    return size;
}

#ifdef ALINK_PASSTHROUGH
alink_err_t rawdata_get_callback(const char *in_rawdata, int in_len, 
                char *out_rawdata, int *out_len)
{
    ALINK_PARAM_CHECK(!in_rawdata);
    ALINK_PARAM_CHECK(in_len < 0);
    
    alink_err_t ret = ALINK_OK;
    char *rawdata_tmp = alink_malloc(in_len);
    alink_event_send(ALINK_EVENT_SET_DEVICE_DATA);
    ALINK_LOGD("rawdata:%s  len:%d", in_rawdata, in_len);
    memcpy(rawdata_tmp, in_rawdata, in_len);

    if (xQueueSend(xQueueDownCmd, &rawdata_tmp, 0) != pdTRUE) {
        ALINK_LOGE("xQueueSend xQueueDownCmd fail");
        ret = ALINK_ERR;
        alink_free(rawdata_tmp);
    }
    
    return ret;
}

alink_err_t rawdata_set_callback(char *rawdata, int len)
{
    ALINK_PARAM_CHECK(!rawdata);
    
    alink_err_t ret = ALINK_OK;
    char *rawdata_tmp = alink_malloc(len);
    alink_event_send(ALINK_EVENT_SET_DEVICE_DATA);
    ALINK_LOGD("rawdata:%s  len:%d", rawdata, len);
    memcpy(rawdata_tmp, rawdata, len);

    if (xQueueSend(xQueueDownCmd, &rawdata_tmp, 0) != pdTRUE) {
        ALINK_LOGE("xQueueSend xQueueDownCmd fail");
        ret = ALINK_ERR;
        alink_free(rawdata_tmp);
    }
    
    return ret;
}

alink_err_t alink_post_raw_data(void)
{
    int ret = ALINK_ERR;
    char *up_cmd = NULL;
    
    if (xQueueReceive(xQueueUpCmd, &up_cmd, portMAX_DELAY) != pdTRUE) {
        ALINK_LOGE("xQueueSend xQueueDownCmd fail");
        return ALINK_ERR;
    }
    
    ret = alink_post_device_rawdata(up_cmd, ALINK_PASSTHROUGH_DATA_LEN);

    if (ret == ALINK_ERR) {
        ALINK_LOGE("post failed!");
        alink_sleep(1000);
    } else {
        ALINK_LOGI("dev post data success!");
    }

    if (up_cmd) {
        alink_free(up_cmd);
    }
    
    return ret;
}

#else
/* get json data from server, and parse the json data to normal data */
alink_err_t main_dev_set_device_status_callback(alink_down_cmd_ptr down_cmd)
{
    ALINK_PARAM_CHECK(!down_cmd);
    
    alink_err_t ret = ALINK_OK;
    alink_event_send(ALINK_EVENT_SET_DEVICE_DATA);

    ALINK_LOGD("%s %d\n%s", down_cmd->uuid, down_cmd->method, down_cmd->param);
    ALINK_LOGD("downcmd param length: %d", strlen(down_cmd->param));

    int size = strlen(down_cmd->param) + 1;
    char *q_data = alink_malloc(size);
    memcpy(q_data, down_cmd->param, size);

    if (xQueueSend(xQueueDownCmd, &(q_data), 0) != pdTRUE) {
        ALINK_LOGE("xQueueSend xQueueDownCmd is error");
        ret = ALINK_ERR;
        alink_free(q_data);
    }
    
    return ret;
}

/* server inquire device status, the device should post current status */
alink_err_t main_dev_get_device_status_callback(alink_down_cmd_ptr down_cmd)
{
    ALINK_PARAM_CHECK(!down_cmd);
    
    alink_err_t ret = ALINK_OK;
    alink_event_send(ALINK_EVENT_SET_DEVICE_DATA);
    
    ALINK_LOGD("%s %d\n%s", down_cmd->uuid, down_cmd->method, down_cmd->param);

    int size = strlen(down_cmd->param) + 1;
    char *q_data = alink_malloc(size);
    memcpy(q_data, down_cmd->param, size);

    if (xQueueSend(xQueueDownCmd, &(q_data), 0) != pdTRUE) {
        ALINK_LOGE("xQueueSend xQueueDownCmd is error");
        ret = ALINK_ERR;
        alink_free(q_data);
    }
    
    return ret;
}

alink_err_t alink_post_json_data(void)
{
    alink_up_cmd cmd;
    char *up_cmd = NULL;
    int ret = ALINK_ERR;

    if (xQueueReceive(xQueueUpCmd, &up_cmd, portMAX_DELAY) != pdTRUE) {
        ALINK_LOGE("xQueueReceive xQueueUpCmd fail");
        return ALINK_ERR;
    }

    ALINK_LOGD("alink_free heap size:%d", system_get_free_heap_size());
    ALINK_LOGI("up_cmd param(json):%s", up_cmd);

    cmd.param = up_cmd;
    cmd.target = NULL;
    cmd.resp_id = -1;
    ret = alink_post_device_data(&cmd);

    if (ret == ALINK_ERR) {
        ALINK_LOGE("post failed!");
        alink_sleep(1000);
    } else {
        alink_event_send(ALINK_EVENT_POST_CLOUD_DATA);
        ALINK_LOGI("dev post data success!");
    }

    if (up_cmd) {
        alink_free(up_cmd);
    }
    
    return ret;
}
#endif

void alink_set_thread_stack_size(struct thread_stacksize *p_thread_stacksize)
{
    p_thread_stacksize->alink_main_thread_size = 0xc00; //0xc00;
    p_thread_stacksize->send_work_thread_size = 0x780; //0x800;
    p_thread_stacksize->wsf_thread_size = 0xa80; //0x1000;  
    p_thread_stacksize->func_thread_size = 0x800; //0x800;
}

void alink_device_post_data(void)
{
    while (post_data_enable) {

#ifdef ALINK_PASSTHROUGH
        alink_post_raw_data();
#else
        alink_post_json_data();
#endif

        ALINK_LOGD("Aviable Memory: %d", system_get_free_heap_size());
    }
}

alink_err_t alink_trans_init(alink_product_t* info)
{
    ALINK_PARAM_CHECK(!info);
    
    alink_err_t ret  = ALINK_OK;
    post_data_enable = ALINK_TRUE;
    xQueueDownCmd = xQueueCreate(DOWN_CMD_QUEUE_NUM, sizeof(char *));
    xQueueUpCmd = xQueueCreate(UP_CMD_QUEUE_NUM, sizeof(char *));

    uint8 macaddr[6];
    char mac_str[STR_MAC_LEN] = {0};
    wifi_get_macaddr(0, macaddr);
    snprintf(info->mac, sizeof(info->mac), MACSTR, MAC2STR(macaddr));
    snprintf(info->cid, sizeof(info->cid), "%024d", system_get_chip_id());

    info->sys_callback[ALINK_FUNC_SERVER_STATUS] = alink_handler_systemstates_callback;
    info->dev_callback[ACB_SET_DEVICE_STATUS_ARRAY] = NULL;
    info->dev_callback[ACB_REQUEST_REMOTE_SERVICE] = NULL;
    info->dev_callback[ACB_GET_DEVICE_STATUS_BY_RAWDATA] = NULL;
    info->dev_callback[ACB_SET_DEVICE_STATUS_BY_RAWDATA] = NULL;
    info->dev_callback[ACB_REQUEST_DEVICE_UPGRADE] = NULL;
    info->dev_callback[ACB_REQUEST_DEVICE_UPGRADE_CANCEL] = NULL;
    info->dev_callback[ACB_REQUEST_DEVICE_UNUPGRADE] = NULL;

    //alink_enable_sandbox_mode();       //if in online environment, this function should be closed
    alink_set_callback(ALINK_FUNC_AVAILABLE_MEMORY, print_mem_callback);
    /* ALINK_CONFIG_LEN 2048 */
    alink_register_cb(ALINK_FUNC_READ_CONFIG, (void *) &alink_read_flash_config);
    alink_register_cb(ALINK_FUNC_WRITE_CONFIG, (void *) &alink_write_flash_config);
    alink_register_cb(ALINK_FUNC_GET_STATUS, alink_get_debuginfo);
    alink_register_cb(ALINK_FUNC_OTA_FIRMWARE_SAVE, esp_ota_firmware_save);
    alink_register_cb(ALINK_FUNC_OTA_UPGRADE, esp_ota_upgrade);
    /*start alink-sdk */
    alink_set_thread_stack_size(&g_thread_stacksize);

    alink_set_product_info(info);

#ifdef ALINK_PASSTHROUGH
    ret = alink_start_rawdata(info, rawdata_get_callback, rawdata_set_callback);
#else
    info->dev_callback[ACB_GET_DEVICE_STATUS] = main_dev_get_device_status_callback;
    info->dev_callback[ACB_SET_DEVICE_STATUS] = main_dev_set_device_status_callback;
    ret = alink_start(info);
#endif

    if(ret != ALINK_OK) {
        post_data_enable = ALINK_FALSE;
        alink_end();
        return ret;
    } 
    
    alink_wait_ap_connected();
    ALINK_LOGD("connected to ap!");
    struct station_config wifi_config;

    if (esp_info_load(NVS_KEY_WIFI_CONFIG, &wifi_config, sizeof(struct station_config)) <= 0) {
        wifi_station_get_config_default(&wifi_config);
        int cfg_len = esp_info_save(NVS_KEY_WIFI_CONFIG, &wifi_config, sizeof(struct station_config));
        ALINK_ERROR_CHECK(cfg_len < 0, ALINK_ERR, "write wifi cfg err");
    }
    
    if (xSemaphoreTake(s_need_notify_app, 1000 / portTICK_RATE_MS) == pdPASS) {
        aws_notify_app();
    }
    
    return ret;
}


