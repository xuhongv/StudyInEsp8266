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

#ifndef __ESP_ALINK_LOG_H__
#define __ESP_ALINK_LOG_H__

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include "alink_export.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ESP_OK
#define ESP_OK    0
#endif
#ifndef ESP_FAIL
#define ESP_FAIL   -1
#endif

/**
 * @brief Log level
 *
 */
typedef enum {
    ESP_LOG_NONE,       /*!< No log output */
    ESP_LOG_FATAL,     /*!< Fatal error log will output */
    ESP_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    ESP_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    ESP_LOG_INFO,       /*!< Information messages which describe normal flow of events */
    ESP_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    ESP_LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} esp_log_level_t;

#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D
#define LOG_COLOR_V

#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " %s: " format LOG_RESET_COLOR "\n"

#define ESP_EARLY_LOGE( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_ERROR)   { printf(LOG_FORMAT(E, format), tag, ##__VA_ARGS__); }
#define ESP_EARLY_LOGW( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_WARN)    { printf(LOG_FORMAT(W, format), tag, ##__VA_ARGS__); }
#define ESP_EARLY_LOGI( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_INFO)    { printf(LOG_FORMAT(I, format), tag, ##__VA_ARGS__); }
#define ESP_EARLY_LOGD( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG)   { printf(LOG_FORMAT(D, format), tag, ##__VA_ARGS__); }
#define ESP_EARLY_LOGV( tag, format, ... )  if (LOG_LOCAL_LEVEL >= ESP_LOG_VERBOSE) { printf(LOG_FORMAT(V, format), tag, ##__VA_ARGS__); }

#define ESP_LOGE( tag, format, ... )  ESP_EARLY_LOGE(tag, format, ##__VA_ARGS__)
#define ESP_LOGW( tag, format, ... )  ESP_EARLY_LOGW(tag, format, ##__VA_ARGS__)
#define ESP_LOGI( tag, format, ... )  ESP_EARLY_LOGI(tag, format, ##__VA_ARGS__)
#define ESP_LOGD( tag, format, ... )  ESP_EARLY_LOGD(tag, format, ##__VA_ARGS__)
#define ESP_LOGV( tag, format, ... )  ESP_EARLY_LOGV(tag, format, ##__VA_ARGS__)

#define ALINK_LOGE( format, ... ) ESP_LOGE(TAG, "[%s, %d]:" format, __func__, __LINE__, ##__VA_ARGS__)
#define ALINK_LOGW( format, ... ) ESP_LOGW(TAG, "[%s, %d]:" format, __func__, __LINE__, ##__VA_ARGS__)
#define ALINK_LOGI( format, ... ) ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define ALINK_LOGD( format, ... ) ESP_LOGD(TAG, "[%s, %d]:" format, __func__, __LINE__, ##__VA_ARGS__)
#define ALINK_LOGV( format, ... ) ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define ALINK_ERROR_CHECK(con, err, format, ...) if(con) {ALINK_LOGE(format, ##__VA_ARGS__); return err;}
#define ALINK_PARAM_CHECK(con) if(con) {ALINK_LOGE("Parameter error: %s", #con); system_restart();}
#define ESP_ALINK_ERROR_CHECK(con)   if((con) != ALINK_TRUE) {ALINK_LOGE("Parameter error: %s", #con); system_restart();}

#define alink_malloc(num_bytes) ({void *ptr = malloc(num_bytes); \
        ALINK_LOGV("malloc size: %d, ptr: %p, heap free: %d", num_bytes, ptr, system_get_free_heap_size()); ptr;})

#define alink_calloc(n, size) ({void *ptr = calloc(n, size); \
        ALINK_LOGV("calloc size: %d, ptr: %p, heap free: %d", n * size, ptr, system_get_free_heap_size()); ptr;})

#define alink_free(arg) {if(arg){ free(arg); \
        ALINK_LOGV("free ptr: %p, heap free: %d", arg, system_get_free_heap_size()); arg = NULL;}}while(0)

#ifndef CONFIG_LOG_ALINK_LEVEL
#define CONFIG_LOG_ALINK_LEVEL ESP_LOG_DEBUG
#endif
#ifndef CONFIG_ALINK_SDK_LOG_LEVEL
#define CONFIG_ALINK_SDK_LOG_LEVEL ALINK_LL_INFO
#endif

#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL CONFIG_LOG_ALINK_LEVEL
#define ALINK_SDK_LOG_LEVEL 1<<CONFIG_ALINK_SDK_LOG_LEVEL-1

#ifdef __cplusplus
}
#endif

#endif /*!< __ESP_ALINK_LOG_H__ */
