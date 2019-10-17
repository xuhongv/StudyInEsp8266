/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-10-17 17:29:46
 * @LastEditTime: 2019-10-17 19:01:13
 * @LastEditors: Please set LastEditors
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "driver/hw_timer.h"
#include "rom/ets_sys.h"
#include "esp_log.h"
#include "esp_system.h"
#include "ds18b20.h"

int DSPORT = GPIO_NUM_13; //(为gpio14接口)

bool isInit = true;

void Delay1ms(int y)
{
    os_delay_us(y * 1000);
}
/*******************************************************************************
 * 函 数 名         : Ds18b20Init
 * 函数功能		   : 初始化
 * 输    入         : 无
 * 输    出         : 初始化成功返回1，失败返回0
 *******************************************************************************/

uint8_t Ds18b20Init()
{
    int i = 0;

    if (isInit)
    {
        gpio_config_t io_conf;
        //disable interrupt
        io_conf.intr_type = GPIO_INTR_DISABLE;
        //set as output mode
        io_conf.mode = (GPIO_MODE_OUTPUT);
        //bit mask of the pins that you want to set,e.g.GPIO15/16
        io_conf.pin_bit_mask = (1ULL << DSPORT);
        //disable pull-down mode
        io_conf.pull_down_en = 0;
        //disable pull-up mode
        io_conf.pull_up_en = 0;
        //configure GPIO with the given settings
        gpio_config(&io_conf);
        isInit = false;
    }

    gpio_set_direction(DSPORT, GPIO_MODE_OUTPUT);
    gpio_set_level(DSPORT, 0); //将总线拉低480us~960us
    os_delay_us(642);          //延时642us
    gpio_set_level(DSPORT, 1); //然后拉高总线，如果DS18B20做出反应会将在15us~60us后总线拉低

    gpio_set_direction(DSPORT, GPIO_MODE_INPUT);
    while (gpio_get_level(DSPORT)) //等待DS18B20拉低总线
    {
        Delay1ms(1);
        i++;
        if (i > 5) //等待>5MS
        {
            return 0; //初始化失败
        }
    }
    return 1; //初始化成功
}

/*******************************************************************************
 * 函 数 名         : Ds18b20WriteByte
 * 函数功能		   : 向18B20写入一个字节
 * 输    入         : com
 * 输    出         : 无
 *******************************************************************************/

void Ds18b20WriteByte(uint8_t dat)
{
    int i, j;
    gpio_set_direction(DSPORT, GPIO_MODE_OUTPUT);
    for (j = 0; j < 8; j++)
    {
        gpio_set_level(DSPORT, 0); //每写入一位数据之前先把总线拉低1us
        i++;
        gpio_set_level(DSPORT, dat & 0x01); //然后写入一个数据，从最低位开始
        os_delay_us(70);                    //延时68us，持续时间最少60us
        gpio_set_level(DSPORT, 1);          //然后释放总线，至少1us给总线恢复时间才能接着写入第二个数值
        dat >>= 1;
    }
}
/*******************************************************************************
 * 函 数 名         : Ds18b20ReadByte
 * 函数功能		   : 读取一个字节
 * 输    入         : com
 * 输    出         : 无
 *******************************************************************************/

uint8_t Ds18b20ReadByte()
{

    uint8_t byte = 0, bi;
    int i, j;
    for (j = 8; j > 0; j--)
    {
        gpio_set_direction(DSPORT, GPIO_MODE_OUTPUT);
        gpio_set_level(DSPORT, 0); //先将总线拉低1us
        i++;
        gpio_set_level(DSPORT, 1); //然后释放总线
        i++;
        i++;
        gpio_set_direction(DSPORT, GPIO_MODE_INPUT);
        //延时6us等待数据稳定
        bi = gpio_get_level(DSPORT); //读取数据，从最低位开始读取
        /*将byte左移一位，然后与上右移7位后的bi，注意移动之后移掉那位补0。*/
        byte = (byte >> 1) | (bi << 7);
        os_delay_us(48); //读取完之后等待48us再接着读取下一个数
    }
    return byte;
}
/*******************************************************************************
 * 函 数 名         : Ds18b20ChangTemp
 * 函数功能		   : 让18b20开始转换温度
 * 输    入         : com
 * 输    出         : 无
 *******************************************************************************/

void Ds18b20ChangTemp()
{
    Ds18b20Init();
    Delay1ms(1);
    Ds18b20WriteByte(0xcc); //跳过ROM操作命令
    Ds18b20WriteByte(0x44); //温度转换命令
    //	Delay1ms(100);	//等待转换成功，而如果你是一直刷着的话，就不用这个延时了
}
/*******************************************************************************
 * 函 数 名         : Ds18b20ReadTempCom
 * 函数功能		   : 发送读取温度命令
 * 输    入         : com
 * 输    出         : 无
 *******************************************************************************/

void Ds18b20ReadTempCom()
{

    Ds18b20Init();
    Delay1ms(1);
    Ds18b20WriteByte(0xcc); //跳过ROM操作命令
    Ds18b20WriteByte(0xbe); //发送读取温度命令
}
/*******************************************************************************
 * 函 数 名         : Ds18b20ReadTemp
 * 函数功能		   : 读取温度
 * 输    入         : com
 * 输    出         : 无
 *******************************************************************************/

int Ds18b20ReadTemp()
{
    int temp = 0;
    uint8_t tmh, tml;
    Ds18b20ChangTemp();      //先写入转换命令
    Ds18b20ReadTempCom();    //然后等待转换完后发送读取温度命令
    tml = Ds18b20ReadByte(); //读取温度值共16位，先读低字节
    tmh = Ds18b20ReadByte(); //再读高字节
    temp = tmh;
    temp <<= 8;
    temp |= tml;
    return temp;
}
