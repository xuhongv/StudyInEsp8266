/*

  This example code is created by gengyuchao used for Latest ESP8266 
  SDK based on FreeRTOS, esp-idf style.  (update:2019-06-22).

  Unless required by applicable law or agreed to in writing, this
  software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
  CONDITIONS OF ANY KIND, either express or implied.

 */
// ws2812_driver.hpp 
#ifndef __WS2812_DRIVER_DATA__
#define __WS2812_DRIVER_DATA__

#ifdef   __cplusplus 
extern "C"{
#endif


    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>

    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/queue.h"

    #include "driver/gpio.h"
    #include "driver/hw_timer.h"

    #include "esp_log.h"
    #include "esp_system.h"

    #include "esp_libc.h"
    #include "rom/gpio.h"



//WS2812初始化函数
void WS2812_Init(void);

//发送一个灯的颜色
void  WS2812BSend_24bit(uint8_t R, uint8_t G, uint8_t B);

void setAllPixelColor(uint8_t r, uint8_t g, uint8_t b);
void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
void SetPixelColor(uint16_t n, uint32_t c);
uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
uint32_t Wheel(uint8_t WheelPos);
void rainbowCycle(uint8_t wait) ;
void rainbow(uint8_t wait);
void theaterChase(uint32_t c, uint8_t wait);
void theaterChaseRainbow(uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);
void WS2812B_Test(void);


#ifdef   __cplusplus 
}
#endif


#endif