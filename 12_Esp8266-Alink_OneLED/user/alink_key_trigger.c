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

#include "esp_common.h"

#include "esp_alink.h"
#include "esp_alink_log.h"

#include "key.h"

const char *TAG = "alink_key_trigger";

struct single_key_param *single_key[2];
struct keys_param keys;

static void key_13_short_press(void)
{
    ALINK_LOGD("short press..");
}

static void key_13_long_press(void)
{
    ALINK_LOGD("long press..");
    alink_event_send(ALINK_EVENT_UPDATE_ROUTER);
}

static void key_sw2_short_press(void)
{
    ALINK_LOGD("short press..");
}

static void key_sw2_long_press(void)
{
    ALINK_LOGD("long press..");
    alink_event_send(ALINK_EVENT_FACTORY_RESET);
}

void alink_key_trigger()
{
    single_key[0] = key_init_single(14, PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14,
                                    key_13_long_press, key_13_short_press);
    single_key[1] = key_init_single(4 , PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4,
                                    key_sw2_long_press, key_sw2_short_press);
    keys.key_num = 2;
    keys.single_key = single_key;
    key_init(&keys);
}
