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

/*
    this section is about use softap to connect wifi, app send ssid and password to device by tcp
    the steps as follows:
    1) wifi device open tcp server, waiting for connected by app
    2) app send ssid, password and bssid to wifi device after connected to tcp server
    3) wifi device active scan to acquire encryption type and connect to ap
    4) wifi device send success notice to app after connected to ap
*/
#include "esp_common.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/api.h"
#include "aws_lib.h"
#include "aws_platform.h"
#include "esp_alink_log.h"

static const char *TAG = "aws_softap";

#define SOFTAP_GATEWAY_IP         "172.31.254.250"
#define SOFTAP_TCP_SERVER_PORT    (65125)

#define STR_SSID_LEN        (32 + 1)
#define STR_PASSWD_LEN      (64 + 1)
#define STR_BSSID_LEN       (6)
#define TCP_DATA_LEN        (512)

static char aws_ssid[STR_SSID_LEN];
static char aws_passwd[STR_PASSWD_LEN];
static char aws_bssid[STR_BSSID_LEN];

/* parse ssid and password from json data */
static int aws_get_ssid_and_passwd(char *msg)
{
    ALINK_PARAM_CHECK(!msg);
    
    char *ptr, *end, *name;
    int len;

    /* ssid */
    name = "\"ssid\":";
    ptr = strstr(msg, name);
    ALINK_ERROR_CHECK(!ptr, ALINK_ERR, "%s not found!", name);

    ptr += strlen(name);
    while (*ptr++ == ' '); /* eating the beginning */
    end = strchr(ptr, '"');
    len = end - ptr;

    ALINK_PARAM_CHECK(len >= sizeof(aws_ssid));
    strncpy(aws_ssid, ptr, len);
    aws_ssid[len] = '\0';

    /* passwd */
    name = "\"passwd\":";
    ptr = strstr(msg, name);
    ALINK_ERROR_CHECK(!ptr, ALINK_ERR, "%s not found!", name);

    ptr += strlen(name);
    while (*ptr++ == ' '); /* eating the beginning */
    end = strchr(ptr, '"');
    len = end - ptr;

    ALINK_PARAM_CHECK(len >= sizeof(aws_passwd));
    strncpy(aws_passwd, ptr, len);
    aws_passwd[len] = '\0';

    /* bssid-mac */
    name = "\"bssid\":";
    ptr = strstr(msg, name);
    ALINK_ERROR_CHECK(!ptr, ALINK_ERR, "%s not found!", name);


    ptr += strlen(name);
    while (*ptr++ == ' '); /* eating the beginning */
    end = strchr(ptr, '"');
    len = end - ptr;

    return ALINK_OK;
}

/* setup tcp server to get ssid and passwd */
static int aws_softap_tcp_server(void)
{
    struct sockaddr_in server, client;
    socklen_t socklen = sizeof(struct sockaddr_in);
    int ret, len;
    int listen_fd = -1, conn_fd=-1;

    char *buf = alink_malloc(TCP_DATA_LEN);
    ALINK_ERROR_CHECK(!buf, ALINK_ERR, "malloc err!");
    char *msg = alink_malloc(TCP_DATA_LEN);
    ALINK_ERROR_CHECK(!msg, ALINK_ERR, "malloc err!");

    ALINK_LOGI("setup tcp server");

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    ALINK_ERROR_CHECK(listen_fd < 0, ALINK_ERR, "create socket fail");

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SOFTAP_GATEWAY_IP);
    server.sin_port = htons(SOFTAP_TCP_SERVER_PORT);

    ret = bind(listen_fd, (struct sockaddr *)&server, sizeof(server));
    ALINK_ERROR_CHECK(ret, ALINK_ERR, "bind fail");

    ret = listen(listen_fd, 10);
    ALINK_ERROR_CHECK(ret, ALINK_ERR, "listen fail");

    ALINK_LOGI("server %x %d created\n", ntohl(server.sin_addr.s_addr),
            ntohs(server.sin_port));

    conn_fd = accept(listen_fd, (struct sockaddr *)&client, &socklen);
    ALINK_ERROR_CHECK(conn_fd <= 0, ALINK_ERR, "accept fail");
    ALINK_LOGI("client %x %d connected!", ntohl(client.sin_addr.s_addr),
            ntohs(client.sin_port));

    len = recv(conn_fd, buf, TCP_DATA_LEN, 0);
    ALINK_ERROR_CHECK(len < 0, ALINK_ERR, "recieve fail");

    buf[len] = '\0';
    ALINK_LOGI("softap tcp server recv: %s", buf);

    ret = aws_get_ssid_and_passwd(buf);

    if (!ret) {
        snprintf(msg, TCP_DATA_LEN,
            "{\"code\":1000, \"msg\":\"format ok\", \"model\":\"%s\", \"mac\":\"%s\"}",
            vendor_get_model(), vendor_get_mac());
    } else {
        snprintf(msg, TCP_DATA_LEN,
            "{\"code\":2000, \"msg\":\"format error\", \"model\":\"%s\", \"mac\":\"%s\"}",
            vendor_get_model(), vendor_get_mac());
    }

    len = send(conn_fd, msg, strlen(msg), 0);
    ALINK_ERROR_CHECK(len < 0, ALINK_ERR, "send fail");
    ALINK_LOGI("ack %s", msg);

    close(conn_fd);
    close(listen_fd);

    alink_free(buf);
    alink_free(msg);

    return ALINK_OK;
}

/* setup softap */
static void aws_softap_setup(void)
{
    /* Step1: Softap config */
    char ssid[STR_SSID_LEN]={0};
    snprintf(ssid, STR_SSID_LEN, "alink_%s", vendor_get_model());
    struct softap_config ap_config;
    bzero(&ap_config,sizeof(ap_config));
    wifi_set_opmode(SOFTAP_MODE);
    memcpy(ap_config.ssid,ssid,strlen(ssid));
    ap_config.ssid_len=strlen(ap_config.ssid);
    ap_config.authmode=AUTH_OPEN;
    ap_config.channel=6;
    ap_config.max_connection=4;
    wifi_softap_set_config(&ap_config);
    
    /* Step2: Gateway ip config*/
    struct ip_info ip_info;
    ip_info.ip.addr=ipaddr_addr("172.31.254.250");
    ip_info.gw.addr=ipaddr_addr("172.31.254.250");
    ip_info.netmask.addr=ipaddr_addr("255.255.255.0");
    wifi_softap_dhcps_stop();
    wifi_set_ip_info(SOFTAP_IF,&ip_info);
    wifi_softap_dhcps_start();
    
    /* Step3 Check Step1 and Step2 */
    wifi_softap_get_config(&ap_config);
    wifi_get_ip_info(SOFTAP_IF,&ip_info);
    ALINK_LOGI("******* Softap Param *******");
    ALINK_LOGI("ssid:%s",ap_config.ssid);
    ALINK_LOGI("beacon_interval:%u",ap_config.beacon_interval);
    ALINK_LOGI("ip:%s",inet_ntoa((ip_info.ip)));
    ALINK_LOGI("gw:%s",inet_ntoa((ip_info.gw)));
    ALINK_LOGI("netmask:%s",inet_ntoa((ip_info.netmask)));
}

/* exit softap mode */
static void aws_softap_exit(void)
{
    wifi_set_opmode(STATION_MODE);
}

/* connect to ap */
static void aws_connect_to_ap(char *ssid, char *password)
{
    ALINK_PARAM_CHECK(!ssid);
    ALINK_PARAM_CHECK(!password);
    
    ALINK_LOGI("aws_connect_to ap ssid:%s, password:%s", ssid, password);
    vendor_connect_ap(ssid, password);
}

extern alink_device_notify_app();

int aws_softap_main(void)
{
    /* prepare and setup softap */
    aws_softap_setup();
    /* tcp server to get ssid & passwd */
    aws_softap_tcp_server();
    /* set device as sta mode */
    aws_softap_exit();
    aws_connect_to_ap(aws_ssid, aws_passwd);
    /* after dhcp ready, send notification to APP */
    alink_device_notify_app();

    return ALINK_OK;
}
