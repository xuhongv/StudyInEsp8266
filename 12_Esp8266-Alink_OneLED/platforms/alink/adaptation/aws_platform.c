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

#define _PLATFORM_ESPRESSIF_
#ifdef _PLATFORM_ESPRESSIF_

#include <stdarg.h>
#include "esp_common.h"
#include "aws_platform.h"
#include "aws_lib.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "espressif/esp_libc.h" 
#include "espressif/esp_wifi.h"
#include "alink_export.h"
#include "user_config.h"
#include "esp_alink.h"
#include "esp_alink_log.h"

static const char *TAG = "aws_platform";

#ifndef ETH_ALEN
#define ETH_ALEN    (6)
#endif

#define UDP_TX_PORT  (65123)
#define UDP_RX_PORT  (65126)
#define UDP_RX_LEN   (1024)

#define SSID_LEN     (32)
#define PASSWD_LEN   (64)
#define BSSID_LEN    (6)
#define MAC_LEN      (6)

struct RxControl {
    signed rssi:8;
    unsigned rate:4;
    unsigned is_group:1;
    unsigned:1;
    unsigned sig_mode:2;
    unsigned legacy_length:12;
    unsigned damatch0:1;
    unsigned damatch1:1;
    unsigned bssidmatch0:1;
    unsigned bssidmatch1:1;
    unsigned MCS:7;
    unsigned CWB:1;
    unsigned HT_length:16;
    unsigned Smoothing:1;
    unsigned Not_Sounding:1;
    unsigned:1;
    unsigned Aggregation:1;
    unsigned STBC:2;
    unsigned FEC_CODING:1;
    unsigned SGI:1;
    unsigned rxend_state:8;
    unsigned ampdu_cnt:8;
    unsigned channel:4;
    unsigned:12;
};

struct Ampdu_Info
{
    uint16_t length;
    uint16_t seq;
    uint8_t  address3[6];
};

struct sniffer_buf {
    struct RxControl rx_ctrl;  //12Byte
    uint8_t  buf[36];
    uint16_t cnt;
    struct Ampdu_Info ampdu_info[1];
};

struct sniffer_buf2{
    struct RxControl rx_ctrl;
    uint8_t  buf[496];
    uint16_t cnt;
    uint16_t len;  //length of packet
};

struct ieee80211_hdr {
    uint16_t frame_control;
    uint16_t duration_id;
    uint8_t  addr1[ETH_ALEN];
    uint8_t  addr2[ETH_ALEN];
    uint8_t  addr3[ETH_ALEN];
    uint16_t seq_ctrl;
    uint8_t  addr4[ETH_ALEN];
};

/* smart config time out, suggested time out value is 1~3min, here is 1min */
int aws_timeout_period_ms = 60 * 1000;

/* scanning time for every channel, suggested value is 200~400ms, here is 200 ms */
int aws_chn_scanning_period_ms = 200;

unsigned int vendor_get_time_ms(void)
{
    return (unsigned int)system_get_time() / 1000;
}

void *vendor_malloc(int size)
{
    return malloc(size);
}

void vendor_free(void *ptr)
{
    if (ptr) {
        free(ptr);
        ptr = NULL;
    }
}

void vendor_msleep(int ms)
{
    sys_msleep(ms);
}

void vendor_data_callback(unsigned char *buf, int length)
{
    ALINK_PARAM_CHECK(!buf);
    ALINK_PARAM_CHECK(length < 0);
    
    aws_80211_frame_handler(buf, length, AWS_LINK_TYPE_NONE, 0);
}

static void vendor_sniffer_wifi_promiscuous_rx(uint8 *buf, uint16 buf_len)
{
    ALINK_PARAM_CHECK(!buf);
    ALINK_PARAM_CHECK(buf_len < 0);
    
    uint8_t *data;
    uint32_t data_len;

    if (buf_len == sizeof(struct sniffer_buf2)) { /* managment frame */
        struct sniffer_buf2 *sniffer = (struct sniffer_buf2 *)buf;
        data_len = sniffer->len;

        if (data_len > sizeof(sniffer->buf)) {
            data_len = sizeof(sniffer->buf);
        }
        
        data = sniffer->buf;
        vendor_data_callback(data, data_len);
    }  else if (buf_len == sizeof(struct RxControl)) { /* mimo, HT40, LDPC */
        struct RxControl *rx_ctrl= (struct RxControl *)buf;
        struct ht40_ctrl ht40;
        ht40.rssi = rx_ctrl->rssi;

        if (rx_ctrl->Aggregation) {
            ht40.length = rx_ctrl->HT_length - 4;
        } else {
            ht40.length = rx_ctrl->HT_length;
        }

        ht40.filter = rx_ctrl->Smoothing << 5
                        | rx_ctrl->Not_Sounding << 4
                        | rx_ctrl->Aggregation << 3
                        | rx_ctrl->STBC << 1
                        | rx_ctrl->FEC_CODING;

        aws_80211_frame_handler((char *)&ht40, ht40.length, AWS_LINK_TYPE_HT40_CTRL, 1);
    } else {
        struct sniffer_buf *sniffer = (struct sniffer_buf *)buf;
        data = buf + sizeof(struct RxControl);

        struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)data;

        if (sniffer->cnt == 1) {
            data_len = sniffer->ampdu_info[0].length - 4;
            vendor_data_callback(data, data_len);
        } else {
            int i;
            for (i = 1; i < sniffer->cnt; i++) {
                hdr->seq_ctrl = sniffer->ampdu_info[i].seq;
                memcpy(&hdr->addr3, sniffer->ampdu_info[i].address3, 6);

                data_len = sniffer->ampdu_info[i].length - 4;
                vendor_data_callback(data, data_len);
            }
        }
    }
}

int vendor_recv_80211_frame(void)
{
    vendor_msleep(100);
    return ALINK_OK;
}

void vendor_monitor_open(void)
{
    wifi_set_channel(6);
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(vendor_sniffer_wifi_promiscuous_rx);
    wifi_promiscuous_enable(1);
}

void vendor_monitor_close(void)
{
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(NULL);
}

void vendor_channel_switch(char primary_channel, char secondary_channel, char bssid[6])
{
    wifi_set_channel(primary_channel);
}

int vendor_broadcast_notification(char *msg, int msg_num)
{
    ALINK_PARAM_CHECK(!msg);
    ALINK_PARAM_CHECK(msg_num <= 0);

    int i, ret, socket_fd;
    struct sockaddr_in addr_local, addr_remote;
    char *buf = vendor_malloc(UDP_RX_LEN);

    do {
        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        
        if (socket_fd == -1) {
            ALINK_LOGE("ERROR: failed to create sock!");
            vTaskDelay(1000/portTICK_RATE_MS);
        }
        
    } while (socket_fd == -1);

    memset(&addr_local, 0, sizeof(addr_local));
    addr_local.sin_family = AF_INET;
    addr_local.sin_addr.s_addr = INADDR_ANY;
    addr_local.sin_port = htons(UDP_RX_PORT);
    addr_local.sin_len = sizeof(addr_local);
    ret = bind(socket_fd, (const struct sockaddr *)&addr_local, sizeof(addr_local));
    ALINK_ERROR_CHECK(ret, ALINK_ERR, "aws bind local port error!");

    memset(&addr_remote, 0, sizeof(addr_remote));
    addr_remote.sin_family = AF_INET;
    addr_remote.sin_addr.s_addr = INADDR_BROADCAST;
    addr_remote.sin_port = htons(UDP_TX_PORT);
    addr_remote.sin_len = sizeof(addr_remote);

    /* send notification */
    for (i = 0; i < msg_num; i++) {
        ret = sendto(socket_fd, msg, strlen(msg), 0, (const struct sockaddr *)&addr_remote, sizeof(addr_remote));

        if (ret < 0) {
            ALINK_LOGE("aws send notify msg error!");
            vendor_msleep(1000);
        }

        struct timeval tv;
        fd_set rfds;

        FD_ZERO(&rfds);
        FD_SET(socket_fd, &rfds);

        tv.tv_sec = 0;
        tv.tv_usec = 1000 * (200 + i * 100); //from 200ms to N * 100ms, N = 25

        ret = select(socket_fd + 1, &rfds, NULL, NULL, &tv);

        if (ret > 0) {
            ret = recv(socket_fd, buf, UDP_RX_LEN, 0);

            if (ret) {
                buf[ret] = '\0';
                ALINK_LOGI("rx: %s", buf);
                break;
            }
        }
    }

    vendor_free(buf);
    close(socket_fd);

    return ALINK_OK;
}

/* connect to ap and got ip */
int vendor_connect_ap(char *ssid, char *passwd)
{
    ALINK_PARAM_CHECK(ssid == NULL);
    ALINK_PARAM_CHECK(passwd == NULL);

    struct station_config *config = alink_malloc(sizeof(struct station_config));
    strncpy(config->ssid, ssid, SSID_LEN);
    strncpy(config->password, passwd, PASSWD_LEN);
    wifi_station_set_config(config);
    alink_free(config);

    wifi_station_connect();
    alink_wait_ap_connected();
    alink_event_send(ALINK_EVENT_STA_WIFI_CONNECTED);
    ALINK_LOGD("waiting network ready");

    return ALINK_OK;
}

int aws_smart_config(void)
{
    int ret;
    char auth;
    char encry;
    char channel;
    char ssid[SSID_LEN + 1];
    char passwd[PASSWD_LEN + 1];
    char bssid[BSSID_LEN];
    char macaddr[MAC_LEN];
    char dev_mac[STR_MAC_LEN];

    if (!wifi_get_macaddr(0, macaddr)) {
        ALINK_LOGE("get macaddr failed!");
    }

    snprintf(dev_mac, sizeof(dev_mac), MACSTR, MAC2STR(macaddr));
    aws_start(DEV_MODEL, ALINK_SECRET, dev_mac, NULL);
    ALINK_LOGD("dev mac:%s",dev_mac);
    ret = aws_get_ssid_passwd(ssid, passwd, bssid, &auth, &encry, &channel);

    if (!ret) {
        ALINK_LOGW("alink wireless setup timeout!");
        aws_destroy();
        return ALINK_ERR;
    }

    ALINK_LOGD("ssid:%s, passwd:%s", ssid, passwd);
    aws_destroy();  // add for ht40
    vendor_connect_ap(ssid, passwd);
    alink_device_notify_app();

    return ALINK_OK;
}

#endif
