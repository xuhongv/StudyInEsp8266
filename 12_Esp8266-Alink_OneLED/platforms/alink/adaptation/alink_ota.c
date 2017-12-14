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
#include "aws_lib.h"
#include "aws_platform.h"
#include "alink_export.h"
#include "alink_adaptation.h"
#include "esp_alink_log.h"

static const char *TAG = "alink_ota";

int esp_ota_firmware_save(char * buffer, int len)
{
    ALINK_PARAM_CHECK(!buffer);
    ALINK_PARAM_CHECK(len < 0);
    
    ALINK_LOGI("start firmware update");
    return upgrade_download(buffer, len);
}

int esp_ota_upgrade(void)
{
    ALINK_LOGI("download firmware success, reboot system");
    system_upgrade_reboot();
    
    return ALINK_OK;
}

