/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: key.c
 *
 * Description: key driver, now can use different gpio and install different function
 *
 * Modification history:
 *     2014/5/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "gpio.h"
#include "user_interface.h"

#include "driver/key.h"

LOCAL void key_intr_handler(struct keys_param *keys);

/******************************************************************************
 * FunctionName : key_init_single
 * Description  : init single key's gpio and register function
 * Parameters   : uint8 gpio_id - which gpio to use
 *                uint32 gpio_name - gpio mux name
 *                uint32 gpio_func - gpio function
 *                key_function long_press - long press function, needed to install
 *                key_function short_press - short press function, needed to install
 * Returns      : singleKey_param - single key parameter, needed by key init
*******************************************************************************/
struct singleKey_param *ICACHE_FLASH_ATTR
key_init_single(uint8 gpio_id, uint32 gpio_name, uint8 gpio_func, key_function long_press, key_function short_press)
{
    struct singleKey_param *singleKey = (struct singleKey_param *)os_zalloc(sizeof(struct singleKey_param));

    singleKey->gpio_id = gpio_id;
    singleKey->gpio_name = gpio_name;
    singleKey->gpio_func = gpio_func;
    singleKey->long_press = long_press;
    singleKey->short_press = short_press;

    return singleKey;
}

/******************************************************************************
 * FunctionName : key_init
 * Description  : init keys
 * Parameters   : key_param *keys - keys parameter, which inited by key_init_single
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
key_init(struct keys_param *keys)
{
    uint8 i;

    ETS_GPIO_INTR_ATTACH(key_intr_handler, keys);

    ETS_GPIO_INTR_DISABLE();

    for (i = 0; i < keys->key_num; i++) {
        keys->singleKey[i]->key_level = 1;

        PIN_FUNC_SELECT(keys->singleKey[i]->gpio_name, keys->singleKey[i]->gpio_func);

        gpio_output_set(0, 0, 0, GPIO_ID_PIN(keys->singleKey[i]->gpio_id));

        gpio_register_set(GPIO_PIN_ADDR(keys->singleKey[i]->gpio_id), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
                          | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
                          | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

        //clear gpio14 status
        GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(keys->singleKey[i]->gpio_id));

        //enable interrupt
        gpio_pin_intr_state_set(GPIO_ID_PIN(keys->singleKey[i]->gpio_id), GPIO_PIN_INTR_NEGEDGE);
    }

    ETS_GPIO_INTR_ENABLE();
}

/******************************************************************************
 * FunctionName : key_5s_cb
 * Description  : long press 5s timer callback
 * Parameters   : singleKey_param *singleKey - single key parameter
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
key_5s_cb(struct singleKey_param *singleKey)
{
    os_timer_disarm(&singleKey->key_5s);

    // low, then restart
    if (0 == GPIO_INPUT_GET(GPIO_ID_PIN(singleKey->gpio_id))) {
        if (singleKey->long_press) {
            singleKey->long_press();
        }
    }
}

/******************************************************************************
 * FunctionName : key_50ms_cb
 * Description  : 50ms timer callback to check it's a real key push
 * Parameters   : singleKey_param *singleKey - single key parameter
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
key_50ms_cb(struct singleKey_param *singleKey)
{
    os_timer_disarm(&singleKey->key_50ms);

    // high, then key is up
    if (1 == GPIO_INPUT_GET(GPIO_ID_PIN(singleKey->gpio_id))) {
        os_timer_disarm(&singleKey->key_5s);
        singleKey->key_level = 1;
        gpio_pin_intr_state_set(GPIO_ID_PIN(singleKey->gpio_id), GPIO_PIN_INTR_NEGEDGE);

        if (singleKey->short_press) {
            singleKey->short_press();
        }
    } else {
        gpio_pin_intr_state_set(GPIO_ID_PIN(singleKey->gpio_id), GPIO_PIN_INTR_POSEDGE);
    }
}

/******************************************************************************
 * FunctionName : key_intr_handler
 * Description  : key interrupt handler
 * Parameters   : key_param *keys - keys parameter, which inited by key_init_single
 * Returns      : none
*******************************************************************************/
LOCAL void
key_intr_handler(struct keys_param *keys)
{
    uint8 i;
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    for (i = 0; i < keys->key_num; i++) {
        if (gpio_status & BIT(keys->singleKey[i]->gpio_id)) {
            //disable interrupt
            gpio_pin_intr_state_set(GPIO_ID_PIN(keys->singleKey[i]->gpio_id), GPIO_PIN_INTR_DISABLE);

            //clear interrupt status
            GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(keys->singleKey[i]->gpio_id));

            if (keys->singleKey[i]->key_level == 1) {
                // 5s, restart & enter softap mode
                os_timer_disarm(&keys->singleKey[i]->key_5s);
                os_timer_setfn(&keys->singleKey[i]->key_5s, (os_timer_func_t *)key_5s_cb, keys->singleKey[i]);
                os_timer_arm(&keys->singleKey[i]->key_5s, 5000, 0);
                keys->singleKey[i]->key_level = 0;
                gpio_pin_intr_state_set(GPIO_ID_PIN(keys->singleKey[i]->gpio_id), GPIO_PIN_INTR_POSEDGE);
            } else {
                // 50ms, check if this is a real key up
                os_timer_disarm(&keys->singleKey[i]->key_50ms);
                os_timer_setfn(&keys->singleKey[i]->key_50ms, (os_timer_func_t *)key_50ms_cb, keys->singleKey[i]);
                os_timer_arm(&keys->singleKey[i]->key_50ms, 50, 0);
            }
        }
    }
}

