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
#include "user_config.h"
#include "esp_alink_log.h"

static const char *TAG = "alink_os_info";

char *platform_get_os_version(char os_ver[PLATFORM_OS_VERSION_LEN])
{
    ALINK_PARAM_CHECK(!os_ver);
    return strncpy(os_ver, system_get_sdk_version(), PLATFORM_OS_VERSION_LEN);
}

char *platform_get_module_name(char name_str[PLATFORM_MODULE_NAME_LEN])
{
     ALINK_PARAM_CHECK(!name_str);
    return strncpy(name_str, CONFIG_ALINK_MODULE_NAME, PLATFORM_MODULE_NAME_LEN);
}


