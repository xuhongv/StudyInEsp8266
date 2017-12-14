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

static const char *TAG = "alink_product";

#define ALINK_VERSION  (10)
static alink_product_t g_product_info;

int alink_set_product_info(alink_product_t *info)
{
    ALINK_PARAM_CHECK(!info);
    memcpy(&g_product_info, info, sizeof(alink_product_t));
    return ALINK_OK;
}

char *vendor_get_model(void)
{
    return g_product_info.model;
}

char *vendor_get_secret(void)
{
    return g_product_info.secret;
}

char *vendor_get_mac(void)
{
    return g_product_info.mac;
}

char *vendor_get_sn(void)
{
    return g_product_info.sn;
}

int vendor_alink_version(void)
{
    return ALINK_VERSION;
}


