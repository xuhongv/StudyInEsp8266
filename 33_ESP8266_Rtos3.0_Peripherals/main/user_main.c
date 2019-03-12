/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>

#include "esp_system.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/pwm.h"
#include "driver/uart.h"

static const char *TAG = "XuHongExampleLog";

#define GPIO_OUTPUT 12 //输出管脚
#define GPIO_INPUT 13  //输入管脚：获取高低电平
#define GPIO_ISR_1 14  //中断管脚
#define GPIO_ISR_2 15  //中断管脚

static xQueueHandle gpio_evt_queue = NULL;

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
static void gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
static void Task_gpio_isr(void *arg)
{
    uint32_t io_num;

    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
static void Task_Gpio_test(void *pData)
{

    //输出模式，禁止中断
    gpio_config_t io_conf;
    //禁止中断
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //设置为输出模式
    io_conf.mode = GPIO_MODE_OUTPUT;
    //管脚的位
    io_conf.pin_bit_mask = (1ULL << GPIO_OUTPUT);
    //禁止下拉
    io_conf.pull_down_en = 0;
    //禁止上拉
    io_conf.pull_up_en = 0;
    //开始配置管脚
    gpio_config(&io_conf);

    //管脚的位
    io_conf.pin_bit_mask = (1ULL << GPIO_INPUT);
    //设置为输入模式
    io_conf.mode = GPIO_MODE_INPUT;
    //下拉
    io_conf.pull_down_en = 1;
    //下拉
    io_conf.pull_up_en = 0;
    //开始配置管脚
    gpio_config(&io_conf);

    //上升沿中断
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //配置管脚，这可以配置2个管脚都是上升沿中断
    io_conf.pin_bit_mask = ((1ULL << GPIO_ISR_1) | (1ULL << GPIO_ISR_2));
    //输入模式
    io_conf.mode = GPIO_MODE_INPUT;
    //允许上拉
    io_conf.pull_up_en = 1;
    //禁止下拉
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(Task_gpio_isr, "Task_gpio_isr", 1024, NULL, 10, NULL);
    //注册中断服务
    gpio_install_isr_service(0);
    //添加GPIO_ISR_1的中断回调函数
    gpio_isr_handler_add(GPIO_ISR_1, gpio_isr_handler, (void *)GPIO_ISR_1);
    //添加GPIO_ISR_2的中断回调函数
    gpio_isr_handler_add(GPIO_ISR_2, gpio_isr_handler, (void *)GPIO_ISR_2);
    //移除GPIO_ISR_2的中断回调函数
    //gpio_isr_handler_remove(GPIO_ISR_2);

    //闪烁
    while (1)
    {
        gpio_set_level(GPIO_OUTPUT, 0);
        printf("gpio_get_level: %d \n", gpio_get_level(GPIO_INPUT)); //获取电平, 悬空是高电平默认输出
        vTaskDelay(1000 / portTICK_PERIOD_MS);                       //延迟1000ms就是一秒
        gpio_set_level(GPIO_OUTPUT, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS); //延迟1000ms就是一秒
    }
}

//PWM 周期 100us(也就是10Khz)
#define PWM_PERIOD (100)
//pwm gpio口配置
#define CHANNLE_PWM_TOTAL 2 //一共2个通道
#define CHANNLE_PWM_CW 0
#define CHANNLE_PWM_WW 1
#define PWM_CW_OUT_IO_NUM 14
#define PWM_WW_OUT_IO_NUM 12
// pwm pin number
const uint32_t pinNum[CHANNLE_PWM_TOTAL] = {PWM_CW_OUT_IO_NUM, PWM_WW_OUT_IO_NUM};
// don't alter it !!! dutys table, (duty/PERIOD)*depth , init
uint32_t setDuties[CHANNLE_PWM_TOTAL] = {50, 50};

static int set_duty = 10;
static int dir = 1;
os_timer_t os_timer;
void Task_pwm_blank(void)
{
    if (dir == true)
    {
        if (++set_duty >= 100)
        {
            dir = false;
        }
    }
    else
    {
        if (--set_duty <= 10)
        {
            dir = true;
        }
    }
    pwm_set_duty(CHANNLE_PWM_CW, set_duty);
    pwm_set_duty(CHANNLE_PWM_WW, set_duty);
    pwm_start();
}

static void Task_Pwm_test(void *pData)
{
    //初始化
    pwm_init(PWM_PERIOD, setDuties, CHANNLE_PWM_TOTAL, pinNum);
    //呼吸效果
    os_timer_disarm(&os_timer);
    os_timer_setfn(&os_timer, (os_timer_func_t *)(Task_pwm_blank), NULL);
    os_timer_arm(&os_timer, 6, 1);
    for (;;)
    {
        /* code */
    }
}

#define BUF_SIZE (1024) //最大缓存

static void Task_Uart_test()
{

    /**
     * 
     * 
     怎么使用uart1作为串口信息打印？如果SDK版本是 v3.1 的话，可以直接在 make menuconfig 面板里面直接配置：

       Component config  --->
           ESP8266-specific  --->
              UART for console output (Custom)  --->
                  UART peripheral to use for console output (0-1) (UART1)  --->

        在 v3.0 版本配置项可能有些出入，但是大致类似。或者使用下面代码：
         
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
        uart_param_config(UART_NUM_1, &uart_config);
        uart_driver_install(UART_NUM_1, 0, BUF_SIZE + 1, BUF_SIZE + 1, NULL);

     * */

    uart_config_t uart_config = {
        .baud_rate = 9600, //波特率
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL);

    // 为收到的数组数字开辟一个内存
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

    while (1)
    {
        // 读取串口数组
        int len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);

        //打印下数组
        for (size_t i = 0; i < len; i++)
        {
            os_printf("uart_read_bytes data[%d]= %02x \n ", i, data[i]);
        }

        // 主动发送数组到串口
        uart_write_bytes(UART_NUM_0, (const char *)data, len);
    }
}

/******************************************************************************
 * FunctionName : app_main
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //获取IDF版本
    printf("SDK version:%s\n", esp_get_idf_version());

    //获取芯片可用内存
    printf("esp_get_free_heap_size : %d  \n", esp_get_free_heap_size());
    //获取从未使用过的最小内存
    printf("esp_get_minimum_free_heap_size : %d  \n", esp_get_minimum_free_heap_size());
    //获取芯片的内存分布，返回值具体见结构体 flash_size_map
    printf("system_get_flash_size_map(): %d \n", system_get_flash_size_map());
    //获取mac地址（station模式）
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    printf("esp_read_mac(): %02x:%02x:%02x:%02x:%02x:%02x \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    /*******************************以下方法创建只能选择一个，因为某些管脚是冲突使用了！************************************/

    //xTaskCreate(Task_Gpio_test, "Task_Gpio_test", 1024, NULL, 4, NULL);
    //xTaskCreate(Task_Pwm_test, "Task_Pwm_test", 1024, NULL, 4, NULL);
    xTaskCreate(Task_Uart_test, "Task_Uart_test", 1024, NULL, 10, NULL);
}
