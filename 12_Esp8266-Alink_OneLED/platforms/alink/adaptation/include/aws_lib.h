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

#ifndef _AWS_LIB_H_
#define _AWS_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

/* link type */
enum AWS_LINK_TYPE {
    AWS_LINK_TYPE_NONE,
    AWS_LINK_TYPE_PRISM,
    AWS_LINK_TYPE_80211_RADIO,
    AWS_LINK_TYPE_80211_RADIO_AVS,
    AWS_LINK_TYPE_HT40_CTRL,
};

struct ht40_ctrl {
    unsigned short length;
    unsigned char filter;
    char rssi;
};

/* auth type */
enum AWS_AUTH_TYPE {
    AWS_AUTH_TYPE_OPEN,
    AWS_AUTH_TYPE_SHARED,
    AWS_AUTH_TYPE_WPAPSK,
    AWS_AUTH_TYPE_WPA8021X,
    AWS_AUTH_TYPE_WPA2PSK,
    AWS_AUTH_TYPE_WPA28021X,
    AWS_AUTH_TYPE_WPAPSKWPA2PSK,
    AWS_AUTH_TYPE_MAX = AWS_AUTH_TYPE_WPAPSKWPA2PSK,
    AWS_AUTH_TYPE_INVALID = 0xff,
};

/* encry type */
enum AWS_ENC_TYPE {
    AWS_ENC_TYPE_NONE,
    AWS_ENC_TYPE_WEP,
    AWS_ENC_TYPE_TKIP,
    AWS_ENC_TYPE_AES,
    AWS_ENC_TYPE_TKIPAES,
    AWS_ENC_TYPE_MAX = AWS_ENC_TYPE_TKIPAES,
    AWS_ENC_TYPE_INVALID = 0xff,
};

/**
  * @brief  print aws lib version
  *
  * @return
  *     - aws lib version
  */
const char *aws_version(void);

/**
  * @brief  handle data package got in monitor mode
  *
  * @param  buf frame buffer
  * @param  length frame length
  * @param  link_type link type, defined by enum AWS_LINK_TYPE
  * @param  with_fcs frame include 80211 fcs field, the tailing 4bytes
  *
  * @return
  *     - void
  * 
  * @note
  *        execute following command before adapt, check link_type and with_fcs
  *        a) iwconfig wlan0 mode monitor    //enter monitor mode
  *        b) iwconfig wlan0 channel 6    //switch to channel 6 (based on router)
  *        c) tcpdump -i wlan0 -s0 -w file.pacp    //get package and save the file
  *        d) use wireshark or ommipeek open the file ,then check the frame header and whether there are 4 FCS bytes in the end of frame
  *        
  *        usual frame header type:
  *        no extra frame header: AWS_LINK_TYPE_NONE
  *        radio header: hdr_len = *(unsigned short *)(buf + 2)
  *        avs header: hdr_len = *(unsigned long *)(buf + 4)
  *        prism header: hdr_len = 144
  */
void aws_80211_frame_handler(char *buf, int length,
        enum AWS_LINK_TYPE link_type, int with_fcs);

/**
  * @brief  start smart config, this function will block until wifi connect success or time out
  *
  * @param  model product model
  * @param  secret product secret
  * @param  mac product mac address
  * @param  sn product sn code, usually set as NULL
  *
  * @return
  *     - void
  *
  * @note
  *        time out value can be set by aws_timeout_period_ms
  */
void aws_start(char *model, char *secret, char *mac, char *sn);

/**
  * @brief  get ssid and password after aws_start
  *
  * @param  ssid pointer of ssid buffer
  * @param  passwd pointer of password buffer
  * @param  bssid pointer of bssid buffer
  * @param  auth auth type, defined by enum AWS_AUTH_TYPE
  * @param  encry encry type, defined by enum AWS_ENC_TYPE
  * @param  channel pointer of channel
  *
  * @return
  *     - 1 success
  *     - 0 fail
  */
int aws_get_ssid_passwd(char ssid[32 + 1], char passwd[64 + 1], char bssid[6],
    char *auth, char *encry, char *channel);

/**
  * @brief  notify app smart config success
  *
  * @return
  *     - void
  *
  * @note
  *        the default time out value of notic is 2min
  */
void aws_notify_app(void);

/**
  * @brief  free memory aws lib used
  *
  * @return
  *     - void
  *
  * @note
  *        user shoud call this funtion after smart config end
  */
void aws_destroy(void);

#ifdef __cplusplus
}
#endif

#endif
