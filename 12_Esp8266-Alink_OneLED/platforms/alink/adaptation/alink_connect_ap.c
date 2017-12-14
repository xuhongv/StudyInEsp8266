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

#include "esp_common.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/api.h"
#include "aws_lib.h"
#include "aws_platform.h"

#include "alink_export.h"
#include "alink_adaptation.h"
#include "esp_alink.h"
#include "esp_alink_log.h"
#include "user_config.h"
#include "esp_info_store.h"

static const char *TAG = "alink_connect_ap";

extern alink_err_t alink_device_notify_app();

/* callback function of alink system status handle */
int alink_handler_systemstates_callback(void *dev_mac, void *sys_state)
{
    ALINK_PARAM_CHECK(dev_mac == NULL);
    ALINK_PARAM_CHECK(sys_state == NULL);

    char uuid[STR_UUID_LEN];
    char *mac = (char *)dev_mac;
    enum ALINK_STATUS *state = (enum ALINK_STATUS *)sys_state;
    switch (*state) {
        sprintf(uuid, "%s", alink_get_uuid(NULL));

        case ALINK_STATUS_INITED:
            ALINK_LOGI("ALINK_STATUS_INITED, mac %s uuid %s", mac, uuid);
            break;

        case ALINK_STATUS_REGISTERED:
            ALINK_LOGI("ALINK_STATUS_REGISTERED, mac %s uuid %s", mac, uuid);
            break;

        case ALINK_STATUS_LOGGED:
            ALINK_LOGI("ALINK_STATUS_LOGGED, mac %s uuid %s", mac, uuid);
            alink_event_send(ALINK_EVENT_CLOUD_CONNECTED);
            break;

        case ALINK_STATUS_LOGOUT:
            ALINK_LOGI("ALINK_STATUS_LOGOUT, mac %s uuid %s", mac, uuid);
            alink_event_send(ALINK_EVENT_CLOUD_DISCONNECTED);
            break; 

        default:
            break;
    }

    return ALINK_OK;
}

void alink_wait_ap_connected(void)
{
    while (1) {

        if (wifi_station_get_connect_status() == STATION_GOT_IP) {
            break;
        }

        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

alink_err_t alink_connect_ap(void)
{
    struct station_config wifi_config;
    wifi_set_opmode(STATION_MODE);
     
    if (esp_info_load(NVS_KEY_WIFI_CONFIG, &wifi_config, sizeof(struct station_config)) <= 0) {
        alink_event_send(ALINK_EVENT_CONFIG_NETWORK);
        ALINK_LOGI("*********************************");
        ALINK_LOGI("*    ENTER SAMARTCONFIG MODE    *");
        ALINK_LOGI("*********************************");

        if(aws_smart_config() == ALINK_OK){
            alink_device_notify_app();
        } else {
            ALINK_LOGI("*********************************");
            ALINK_LOGI("*       ENTER SOFTAP MODE       *");
            ALINK_LOGI("*********************************");
            aws_softap_main();
        }
    }
    
    return ALINK_OK;
}
