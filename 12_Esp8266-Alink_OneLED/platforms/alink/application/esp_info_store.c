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

#include "c_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_libc.h"
#include "esp_info_store.h"
#include "spi_flash.h"
#include "esp_alink_log.h"

static const char *TAG = "esp_info_store";

typedef struct alink_info {
    char key[INFO_STORE_KEY_LEN];
    uint16_t offset;
    uint16_t length;
} alink_info_t;

typedef struct alink_info_manager {
    uint8_t num;
    uint16_t use_length;
    alink_info_t alink_info[INFO_STORE_KEY_NUM];
} alink_info_manager_t;

static alink_info_manager_t g_info_manager;

static xSemaphoreHandle info_store_lock = NULL;
#define INFO_MUTEX_INIT()       info_store_lock = xSemaphoreCreateMutex()
#define INFO_MUTEX_LOCK()       xSemaphoreTake(info_store_lock, portMAX_DELAY)
#define INFO_MUTEX_UNLOCK()     xSemaphoreGive(info_store_lock)
#define INFO_MUTEX_DEINIT()     vSemaphoreDelete(info_store_lock)

#define ALINK_ERROR_GOTO(err, lable, format, ...) do{\
        if(err) {\
            ALINK_LOGE(format, ##__VA_ARGS__); \
            goto lable;\
        }\
    }while(0)

static int get_alink_info_index(const char *key)
{
    int i = 0;

    for (i = 0; i < g_info_manager.num; ++i) {
        if (!strncmp(g_info_manager.alink_info[i].key, key, INFO_STORE_KEY_LEN)) {
            return i;
        }
    }

    return ALINK_ERR;
}

alink_err_t esp_info_init()
{
    int ret = 0;

    INFO_MUTEX_INIT();

    ret = system_param_load(INFO_STORE_MANAGER_ADDR / 4096, 0, &g_info_manager, sizeof(alink_info_manager_t));
    ALINK_ERROR_CHECK(ret == ALINK_FALSE, ALINK_ERR, "read flash data error");

    if (g_info_manager.num > INFO_STORE_KEY_NUM || g_info_manager.num == 0x0) {
        spi_flash_erase_sector(INFO_STORE_MANAGER_ADDR / 4096);
        g_info_manager.num = 0;
        g_info_manager.use_length = sizeof(alink_info_manager_t);
    }

    ALINK_LOGD("alink info addr: 0x%x, num: %d", INFO_STORE_MANAGER_ADDR, g_info_manager.num);

    return ALINK_OK;
}

int esp_info_erase(const char *key)
{
    ALINK_PARAM_CHECK(!key);
    INFO_MUTEX_LOCK();

    uint8_t *data_tmp = NULL;
    alink_err_t ret = ALINK_FALSE;
    int info_index = 0;

    if (!strcmp(key, ALINK_SPACE_NAME)) {
        spi_flash_erase_sector(INFO_STORE_MANAGER_ADDR / 4096);
        spi_flash_erase_sector(INFO_STORE_MANAGER_ADDR / 4096 + 1);
        spi_flash_erase_sector(INFO_STORE_MANAGER_ADDR / 4096 + 2);
        ret = ALINK_TRUE;
        goto EXIT;
    }

    info_index = get_alink_info_index(key);
    ALINK_ERROR_GOTO(info_index < 0, EXIT, "get_alink_info_index");

    data_tmp = malloc(g_info_manager.use_length);
    ret = system_param_load(INFO_STORE_MANAGER_ADDR / 4096, 0, data_tmp, g_info_manager.use_length);
    ALINK_ERROR_GOTO(ret == ALINK_FALSE, EXIT, "system_param_load");

    memset(data_tmp + g_info_manager.alink_info[info_index].offset, 0xff,
           g_info_manager.alink_info[info_index].length);

    ret = system_param_save_with_protect(INFO_STORE_MANAGER_ADDR / 4096, data_tmp, g_info_manager.use_length);
    ALINK_ERROR_GOTO(ret == ALINK_FALSE, EXIT, "system_param_save_with_protect");

EXIT:
    INFO_MUTEX_UNLOCK();

    if (data_tmp) {
        free(data_tmp);
    }

    return ret ? ALINK_OK : ALINK_ERR;
}

ssize_t esp_info_save(const char *key, const void *value, size_t length)
{
    ALINK_PARAM_CHECK(!key);
    ALINK_PARAM_CHECK(!value);
    ALINK_PARAM_CHECK(length <= 0);

    INFO_MUTEX_LOCK();

    int i = 0;
    int ret = 0;
    uint8_t *data_tmp = NULL;
    int info_index = get_alink_info_index(key);

    if (info_index < 0) {
        ALINK_ERROR_CHECK(g_info_manager.num == INFO_STORE_KEY_NUM, ALINK_ERR,
                          "Has reached the upper limit of the number of stores");
        alink_info_t *alink_info = &g_info_manager.alink_info[g_info_manager.num];
        strncpy(alink_info->key, key, INFO_STORE_KEY_LEN);
        alink_info->length = length;
        alink_info->offset = g_info_manager.use_length;
        /*!< Four bytes aligned */
        g_info_manager.use_length += alink_info->length + (4 - (length & 0x3));
        info_index = g_info_manager.num;
        g_info_manager.num++;
    }

    data_tmp = malloc(g_info_manager.use_length);
    ret = system_param_load(INFO_STORE_MANAGER_ADDR / 4096, 0, data_tmp, g_info_manager.use_length);
    ALINK_ERROR_GOTO(ret == ALINK_FALSE, EXIT, "system_param_load");

    memcpy(data_tmp, &g_info_manager, sizeof(alink_info_manager_t));
    memcpy(data_tmp + g_info_manager.alink_info[info_index].offset, value,
           g_info_manager.alink_info[info_index].length);
    ret = system_param_save_with_protect(INFO_STORE_MANAGER_ADDR / 4096, data_tmp, g_info_manager.use_length);
    ALINK_ERROR_GOTO(ret == ALINK_FALSE, EXIT, "system_param_save_with_protect");

EXIT:
    INFO_MUTEX_UNLOCK();

    if (data_tmp) {
        free(data_tmp);
    }

    return ret ? length : ALINK_ERR;
}

ssize_t esp_info_load(const char *key, void *value, size_t length)
{
    ALINK_PARAM_CHECK(!key);
    ALINK_PARAM_CHECK(!value);
    ALINK_PARAM_CHECK(length <= 0);

    INFO_MUTEX_LOCK();

    int ret = ALINK_FALSE;
    int info_index = get_alink_info_index(key);

    if (info_index < 0) {
        ALINK_LOGW("The data has been erased");
        goto EXIT;
    }

    ALINK_ERROR_GOTO(length < g_info_manager.alink_info[info_index].length, EXIT,
                     "The buffer is too small, length: %d, flash length: %d",
                     length, g_info_manager.alink_info[info_index].length);

    ret = system_param_load(INFO_STORE_MANAGER_ADDR / 4096, g_info_manager.alink_info[info_index].offset,
                            value, g_info_manager.alink_info[info_index].length);
    ALINK_ERROR_GOTO(ret == ALINK_FALSE, EXIT, "system_param_load");

    if (*((uint8_t *)value) == 0xff && *((uint8_t *)value + 1) == 0xff) {
        ALINK_LOGW("The data has been erased");
        ret = ALINK_FALSE;
        goto EXIT;
    }

EXIT:
    INFO_MUTEX_UNLOCK();
    return ret ? length : ALINK_ERR;
}
