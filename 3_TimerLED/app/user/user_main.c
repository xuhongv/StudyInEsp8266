/*********************************************************************************************
程序名： 　LED闪烁（控制WIFI模块的GPIO口）
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
#include "gpio.h"  //端口控制需要的头文件
#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "gpio.h"



/** 定时器结构体 */
static os_timer_t os_timer;


/** LED操作命令 */
void Led_Cmd(bool status ){
    if (status == true ) {
    	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);
    } else {
    	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
    }
}



void Led_Task_Run(void){

	static bool status = false;
      if ( status == true ) {
	        status = false;
	    } else  {
	        status = true;
	    }

        os_printf("执行操作 ,%d",status);
        os_printf("\r\n\r\n");
	    Led_Cmd( status );
}

void  user_init(void)//初始化
{

   /** 设置串口0和串口1的波特率*/
        uart_init(57600, 57600);


   	    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
   	    Led_Cmd(false);


       /** 关闭该定时器 */
       os_timer_disarm( &os_timer );
       /** 配置该定时器回调函数 */
       os_timer_setfn( &os_timer, (ETSTimerFunc *) ( Led_Task_Run ), NULL );
       /** 启动该定时器 */
       os_timer_arm( &os_timer, 500, true );
}

void user_rf_pre_init()
{

}




