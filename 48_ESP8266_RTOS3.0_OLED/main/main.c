#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp8266/gpio_struct.h"
#include "esp8266/spi_struct.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_libc.h"
#include "iot_hal_oled.h"
#include "driver/gpio.h"
#include "driver/spi.h"

static const char *TAG = "spi_oled";

void app_main(void)
{

    ESP_LOGI(TAG, "init gpio");

    GPIO_OLED_InitConfig();

    //OLED_ShowChar(0,3,"Hello World");
    OLED_ShowCHinese(0, 2, 0);
    OLED_ShowCHinese(20, 2, 1);
    OLED_ShowCHinese(40, 2, 2);

}