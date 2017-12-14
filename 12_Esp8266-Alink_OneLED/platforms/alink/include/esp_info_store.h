/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2017 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __ESP_INFO_STORE_H__
#define __ESP_INFO_STORE_H__

#include <stdio.h>
#include "esp_alink.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef CONFIG_INFO_STORE_MANAGER_ADDR
#define CONFIG_INFO_STORE_MANAGER_ADDR 0x1f8000
#endif

#ifndef CONFIG_INFO_STORE_TOTAL_SIZE
#define CONFIG_INFO_STORE_TOTAL_SIZE   4096
#endif

#ifndef CONFIG_INFO_STORE_KEY_LEN
#define CONFIG_INFO_STORE_KEY_LEN      16
#endif

#ifndef CONFIG_INFO_STORE_KEY_NUM
#define CONFIG_INFO_STORE_KEY_NUM      6
#endif

#define INFO_STORE_MANAGER_ADDR CONFIG_INFO_STORE_MANAGER_ADDR
#define INFO_STORE_TOTAL_SIZE   CONFIG_INFO_STORE_TOTAL_SIZE
#define INFO_STORE_KEY_LEN      CONFIG_INFO_STORE_KEY_LEN
#define INFO_STORE_KEY_NUM      CONFIG_INFO_STORE_KEY_NUM

#define ALINK_SPACE_NAME        "ALINK_APP"
#define NVS_KEY_WIFI_CONFIG     "wifi_config"
#define ALINK_CONFIG_KEY        "alink_config"

/**
 * @brief  Initialize the stored information
 *
 *     - ALINK_ERR : Error, errno is set appropriately
 *     - ALINK_OK  : Success
 */
alink_err_t esp_info_init();

/**
 * @brief  Erase the alink's information
 *
 * @param key   the corresponding key, it will find the target type by the key
 *
 * @return
 *     - ALINK_ERR : Error, errno is set appropriately
 *     - ALINK_OK  : Success
 */
alink_err_t esp_info_erase(const char *key);

/**
 * @brief Save the alink's information
 *
 * @param  key    the corresponding key, it will find the target type by the key
 * @param  value  the value to be saved in the target information
 * @param  length the length of the value
 *
 *     - ALINK_ERR : Error, errno is set appropriately
 *     - others    : The length of the stored information
 */
ssize_t esp_info_save(const char *key, const void *value, size_t length);

/**
 * @brief  Load the alink's information
 *
 * @param  key    the corresponding key, it will find the target type by the key
 * @param  value  the value to be saved in the target information
 * @param  length the length of the value
 *
 *     - ALINK_ERR : Error, errno is set appropriately
 *     - others   : The length of the stored information
 */
ssize_t esp_info_load(const char *key, void *value, size_t length);

/****************************** example ******************************/
/*
void info_store()
{
    esp_info_init();
    char buf[16] = "12345678910";

    esp_info_save("test_buf", buf, sizeof(buf));
    memset(buf, 0, sizeof(buf));
    int ret = esp_info_load("test_buf", buf, sizeof(buf));
    printf("ret: %d, buf: %s\n", ret, buf);

    esp_info_erase("test_buf");
    memset(buf, 0, sizeof(buf));
    ret = esp_info_load("test_buf", buf, sizeof(buf));
    printf("ret: %d, buf: %s\n", ret, buf);
}
*/

#ifdef __cplusplus
}
#endif

#endif /* __ESP_INFO_STORE_H__ */
