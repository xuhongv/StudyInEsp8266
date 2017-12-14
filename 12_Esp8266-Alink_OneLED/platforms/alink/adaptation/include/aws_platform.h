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

#ifndef _AWS_PLATFORM_H_
#define _AWS_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * platform porting API
 */

/* smart config time out, suggested time out value is 1~3min */
extern int aws_timeout_period_ms;

/* scanning time for every channel, suggested value is 200~400ms */
extern int aws_chn_scanning_period_ms;

/**
  * @brief  get the time after boot, use to estimate the time interval of receive data package
  *
  * @return
  *     - time in ms
  */
unsigned int vendor_get_time_ms(void);

/**
  * @brief  enter sleep when switch channel
  *
  * @param ms sleep tiem in ms
  *
  * @return
  *     - void
  */
void vendor_msleep(int ms);

/**
  * @brief  malloc memory
  *
  * @param size memory size needed
  *
  * @return
  *     - pointer of the memory
  */
void *vendor_malloc(int size);

/**
  * @brief  free memory
  *
  * @param ptr pointer of the memory
  *
  * @return
  *     - void
  */
void vendor_free(void *ptr);

/**
  * @brief  get product model
  *
  * @return
  *     - product model string
  *
  * @note
  *        model example: "ALINKTEST_LIVING_LIGHT_SMARTLED"
  */
char *vendor_get_model(void);

/**
  * @brief  get product key
  *
  * @return
  *     - product key string
  *
  * @note
  *        key example: "ljB6vqoLzmP8fGkE6pon"
  */
char *vendor_get_key(void);

/**
  * @brief  get product secret
  *
  * @return
  *     - product secret string
  *
  * @note
  *        secret example: "YJJZjytOCXDhtQqip4EjWbhR95zTgI92RVjzjyZF"
  */
char *vendor_get_secret(void);

/**
  * @brief  get wifi module mac address
  *
  * @return
  *     - wifi module mac address string
  *
  * @note
  *        wifi mac string, format xx:xx:xx:xx:xx:xx
  *        return NULL if product is registered by mac
  */
char *vendor_get_mac(void);

/**
  * @brief  get product sn string
  *
  * @return
  *     - product sn string
  *
  * @note
  *        return NULL if product is registered by mac
  */
char *vendor_get_sn(void);

/**
  * @brief  get alink version
  *
  * @return
  *     -10  alink 1.0
  *     -11  alink 1.1
  *     -20  alink 2.0
  */
int vendor_alink_version(void);

/**
  * @brief  receive 802.11 frame
  *
  * @return
  *     - 0 success
  *     - others fail 
  *
  * @note
  *        if user had register callback function (aws_80211_frame_handler()) to receive frame
  *        this function just need to call vendor_msleep(100)
  */
int vendor_recv_80211_frame(void);

/**
  * @brief  enter monitor mode
  *
  * @return
  *     - void
  */
void vendor_monitor_open(void);

/**
  * @brief  quit monitor mode, return to station mode, and recover resources
  *
  * @return
  *     - void
  */
void vendor_monitor_close(void);

/**
  * @brief  wifi channel switch
  *
  * @param primary_channel
  * @param secondary_channel
  * @param bssid
  *
  * @return
  *     - void
  *
  * @note
  *        channel: 1~13
  */
void vendor_channel_switch(char primary_channel, char secondary_channel, char bssid[6]);

/**
  * @brief  notify app wifi connect success
  *
  * @param msg message string
  * @param msg_num message number
  *
  * @return
  *     - 0 success
  *     - others fail  
  */
int vendor_broadcast_notification(char *msg, int msg_num);

#ifdef __cplusplus
}
#endif

#endif
