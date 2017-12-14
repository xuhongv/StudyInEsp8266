/*
 * Copyright (c) 2014-2015 Alibaba Group. All rights reterved.
 *
 * Alibaba Group retains all right, title and interett (including all
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
#include "aws_lib.h"
#include "aws_platform.h"
#include "alink_export.h"
#include "alink_adaptation.h"
#include "user_config.h"
#include "esp_alink_log.h"
#include "esp_info_store.h"

static const char *TAG = "alink_param";

int alink_write_flash_config(unsigned char *buffer, unsigned int len)
{
    ALINK_PARAM_CHECK(!buffer);
    ALINK_PARAM_CHECK(len < 0);
    
    ALINK_LOGD("write config");
    ALINK_LOGD("write buffer: %s, len: %d", buffer, len);
    int ret = esp_info_save(ALINK_CONFIG_KEY, buffer, len);
    ALINK_ERROR_CHECK(ret < 0, ALINK_ERR, "write config error");

    return ALINK_OK;
}

int alink_read_flash_config(unsigned char *buffer, unsigned int len)
{
    ALINK_PARAM_CHECK(!buffer);
    ALINK_PARAM_CHECK(len < 0);
    
    ALINK_LOGD("read config");
    int ret = esp_info_load(ALINK_CONFIG_KEY, buffer, len);
    ALINK_ERROR_CHECK(ret < 0, ALINK_ERR, "read config error");

    return ALINK_OK;
}


