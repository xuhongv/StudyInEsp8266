#include "c_types.h"

#ifndef APP_INCLUDE_DRIVER_DS1302_H_
#define APP_INCLUDE_DRIVER_DS1302_H_

/**********************引脚选择*******************/
#define DS1302_MASTER_IO_MUX PERIPHS_IO_MUX_MTDI_U
#define DS1302_MASTER_SCLK_MUX PERIPHS_IO_MUX_MTMS_U
#define DS1302_MASTER_RST_MUX PERIPHS_IO_MUX_MTCK_U
#define DS1302_MASTER_IO_GPIO 12
#define DS1302_MASTER_SCLK_GPIO 14
#define DS1302_MASTER_RST_GPIO 13
#define DS1302_MASTER_IO_FUNC FUNC_GPIO12
#define DS1302_MASTER_SCLK_FUNC FUNC_GPIO14
#define DS1302_MASTER_RST_FUNC FUNC_GPIO13
/**********************end*******************/

#define DS1302_MASTER_SCLK_LOW_RST_LOW()  \
    gpio_output_set(0, 1<<DS1302_MASTER_SCLK_GPIO | 1<<DS1302_MASTER_RST_GPIO, 1<<DS1302_MASTER_SCLK_GPIO | 1<<DS1302_MASTER_RST_GPIO, 0)



#define ds1302_sec_add 0x80 //秒数据地址
#define ds1302_min_add 0x82 //分数据地址
#define ds1302_hr_add 0x84 //时数据地址
#define ds1302_date_add 0x86 //日数据地址
#define ds1302_month_add 0x88 //月数据地址
#define ds1302_day_add 0x8a //星期数据地址
#define ds1302_year_add 0x8c //年数据地址
#define ds1302_control_add 0x8e //控制数据地址
#define ds1302_charger_add 0x90
#define ds1302_clkburst_add 0xbe

void DS1302_master_gpio_init(void);
//当Vcc=2.0v时，最大时钟频率为0.5MHz；当Vcc=5.0v时，最大时钟频率为2.0MHz.

void DS1302_master_writeByte(uint8 addr,uint8 wrdata); //向DS1302写入一字节数据

uint8 DS1302_master_readByte(uint8 addr);//从DS1302读出一字节数据

void DS1302_Clock_init(unsigned char *pDate);//初始化时钟数据

#endif /* APP_INCLUDE_DRIVER_DS1302_H_ */
