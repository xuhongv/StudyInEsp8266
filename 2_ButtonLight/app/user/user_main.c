/*********************************************************************************************
程序名： 　按键控制LED（读取GPIO口的状态）
编写人： 　　 　
编写时间：　　
硬件支持：　　
接口说明：　　
修改日志：　　
　　NO.1-								
/*********************************************************************************************
说明：

/*********************************************************************************************/
#include "driver/uart.h"  //串口0需要的头文件
#include "osapi.h"  //串口1需要的头文件
#include "user_interface.h" //WIFI连接需要的头文件
#include "gpio.h"  //端口控制需要的头文件

void delay_ms(uint16 x)
{
	for(;x>0;x--)
	{
	  os_delay_us(1000);
	}
}

void  user_init()//初始化
{
	uint8 bz=0;

    uart_init(57600, 57600);//设置串口0和串口1的波特率

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);//选择GPIO14
    GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1);//GPIO14为高

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);//选择GPIO2
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(2)) ; // 设置GPIO2为输入

    while(1)
    {
    	system_soft_wdt_feed();//这里我们喂下看门狗  ，不让看门狗复位

	    if(GPIO_INPUT_GET(GPIO_ID_PIN(2))==0x00)//读取GPIO2的值，按键按下为0
	    {
			delay_ms(20); //延时20MS，去抖
			if(GPIO_INPUT_GET(GPIO_ID_PIN(2))==0x00)
			{
				bz++;
				if(bz==2)bz=0;
				if(bz==1)GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0);           //GPIO14为低
				if(bz==0)GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1);			//GPIO14为高

				while(GPIO_INPUT_GET(GPIO_ID_PIN(2))==0x00);           //等待按键释放
			}
		}

    }

}

void user_rf_pre_init()
{

}
