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

#ifndef _ESP_ALINK_APPLICATION_H_
#define _ESP_ALINK_APPLICATION_H_

#include "alink_export.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  callback function of alink system status handle
 *
 * @param  dev_mac  device mac address
 * @param  sys_state  system state
 *
 * @return
 *     - ALINK_OK : success
 *     - Others : fail
 */
int alink_handler_systemstates_callback(void *dev_mac, void *sys_state);

/**
 * @brief  callback function of write alink config into flash
 *
 * @param  buffer  alink config
 * @param  len  length of config
 *
 * @return
 *     - ALINK_OK : success
 *     - Others : fail
 */
int alink_write_flash_config(unsigned char *buffer, unsigned int len);

/**
 * @brief  callback function of write alink config from flash
 *
 * @param  buffer  alink config
 * @param  len  length of config
 *
 * @return
 *     - ALINK_OK : success
 *     - Others : fail
 */
int alink_read_flash_config(unsigned char *buffer, unsigned int len);

/**
 * @brief  callback function of get new firmware and save in the flash
 *
 * @param  buffer  ota config
 * @param  len  length of config
 *
 * @return
 *     - ALINK_OK : success
 *     - Others : fail
 */
int esp_ota_firmware_save(char * buffer, int len);

/**
 * @brief  callback function of ota upgrade
 *
 * @return
 *     - ALINK_OK : success
 *     - Others : fail
 */
int esp_ota_upgrade(void);

/**
 * @brief  callback function of get free heap size
 *
 * @return
 *     - free heap size
 */
int print_mem_callback(void *a, void *b);

/**
 * @brief  callback function of get debug information
 *
 * @param  type  info type
 * @param  status  status
 *
 * @return
 *     - ALINK_OK : success
 *     - Others : fail
 */
int alink_get_debuginfo(info_type type, char *status);

/**
 * @brief  wait device connected to ap, this function will block if device did not connected with ap
 *
 * @return
 *     - void
 */
void alink_wait_ap_connected(void);

/**
 * @brief  delay function
 *
 * @param  time_in_ms  delay time (ms)
 *
 * @return
 *     - void
 */
void alink_sleep(int time_in_ms);

#ifdef __cplusplus
}
#endif
/******************************************/
#endif

