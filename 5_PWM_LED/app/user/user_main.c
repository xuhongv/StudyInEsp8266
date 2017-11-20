
#include "driver/uart.h"  //串口0需要的头文件
#include "osapi.h"  //串口1需要的头文件
#include "user_interface.h" //WIFI连接需要的头文件
#include "gpio.h"  //端口控制需要的头文件
#include "pwm.h"  //pwm控制需要的头文件


os_timer_t timer;
uint8 type,x,y,z;
uint32 duty=0;
void ICACHE_FLASH_ATTR display(void *arg)
{
	for(y=0;y<3;y++)
	{
		for(z=0;z<12;z++)
		{
			duty=z*2000;
			type=y;
			pwm_set_duty(duty,type);//设置 PWM 某个通道信号的占空比, duty 占空比的值, type当前要设置的 PWM 通道
			pwm_start();//设置完成后，需要调用 pwm_start,PWM 开始
		}
	}



}
void user_init()
{
	uint32 pwm_duty_init[3]={0};
	uint32 io_info[][3]={{PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12,12},{PERIPHS_IO_MUX_MTCK_U,FUNC_GPIO13,13},{PERIPHS_IO_MUX_MTDO_U,FUNC_GPIO15,15},};
	pwm_init(1000,pwm_duty_init,3,io_info);//初始化 PWM，1000周期,pwm_duty_init占空比,3通道数,io_info各通道的 GPIO 硬件参数
	for(x=0;x<3;x++)
	{
		 type=x;
	     pwm_set_duty(duty,type);//设置 PWM 某个通道信号的占空比, duty 占空比的值, type当前要设置的 PWM 通道
		 pwm_start();//设置完成后，需要调用 pwm_start,PWM 开始
	}
	os_timer_disarm(&timer);
	os_timer_setfn(&timer,display,NULL);
	os_timer_arm(&timer,1000,1);
}
void user_rf_pre_init()
{

}
