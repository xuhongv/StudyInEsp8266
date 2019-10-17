/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-10-17 17:26:05
 * @LastEditTime: 2019-10-17 18:12:26
 * @LastEditors: Please set LastEditors
 */
/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "ds18b20.h"

/**
 * @description: ����ʱ��ͳ��
 * @param {type} 
 * @return: 
 */
void TaskRunTimesCounts(void *p)
{

    while (1)
    {
        printf("ds18b20采集的温度: %d \n\n", (int)(Ds18b20ReadTemp() * 0.0625 + 0.005));
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{

    Ds18b20Init();

    xTaskCreate(TaskRunTimesCounts, "TaskRunTimesCounts", 2048, NULL, 3, NULL);
}
