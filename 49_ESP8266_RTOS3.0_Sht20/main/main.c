#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "sht20.h"

#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "main";

void app_main(void)
{
    while (true)
    {
        ESP_LOGI(TAG, "-----------------");
        ESP_LOGI(TAG, "T=%.2f℃", get_sht20_Temperature());
        ESP_LOGI(TAG, "H=%.2f%%", get_sht20_Humidity());
        vTaskDelay( 2000 / portTICK_RATE_MS );
    }
}


