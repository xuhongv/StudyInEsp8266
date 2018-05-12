#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "gpio.h"

int DSPORT = 14; //(为gpio14接口)

void Delay1ms(int y) {
	os_delay_us(y * 1000);
}
/*******************************************************************************
 * 函 数 名         : Ds18b20Init
 * 函数功能		   : 初始化
 * 输    入         : 无
 * 输    出         : 初始化成功返回1，失败返回0
 *******************************************************************************/

uint8 Ds18b20Init() {
	int i;

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	GPIO_OUTPUT_SET(DSPORT, 0);		 //将总线拉低480us~960us
	os_delay_us(642);		 //延时642us
	GPIO_OUTPUT_SET(DSPORT, 1);	//然后拉高总线，如果DS18B20做出反应会将在15us~60us后总线拉低
	while (GPIO_INPUT_GET(DSPORT))	//等待DS18B20拉低总线
	{
		Delay1ms(1);
		i++;
		if (i > 5)	//等待>5MS
				{
			return 0;	//初始化失败
		}

	}
	return 1;	//初始化成功
}

/*******************************************************************************
 * 函 数 名         : Ds18b20WriteByte
 * 函数功能		   : 向18B20写入一个字节
 * 输    入         : com
 * 输    出         : 无
 *******************************************************************************/

void Ds18b20WriteByte(uint8 dat) {
	int i, j;

	for (j = 0; j < 8; j++) {
		GPIO_OUTPUT_SET(DSPORT, 0);	     	  //每写入一位数据之前先把总线拉低1us
		i++;
		GPIO_OUTPUT_SET(DSPORT, dat & 0x01);	     	  //然后写入一个数据，从最低位开始
		os_delay_us(70); //延时68us，持续时间最少60us
		GPIO_OUTPUT_SET(DSPORT, 1);	//然后释放总线，至少1us给总线恢复时间才能接着写入第二个数值
		dat >>= 1;
	}
}
/*******************************************************************************
 * 函 数 名         : Ds18b20ReadByte
 * 函数功能		   : 读取一个字节
 * 输    入         : com
 * 输    出         : 无
 *******************************************************************************/

uint8 Ds18b20ReadByte() {
	uint8 byte, bi;
	int i, j;
	for (j = 8; j > 0; j--) {
		GPIO_OUTPUT_SET(DSPORT, 0);	//先将总线拉低1us
		i++;
		GPIO_OUTPUT_SET(DSPORT, 1);	  //然后释放总线
		i++;
		i++;	  //延时6us等待数据稳定
		bi = GPIO_INPUT_GET(DSPORT);	 //读取数据，从最低位开始读取
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

void Ds18b20ChangTemp() {
	Ds18b20Init();
	Delay1ms(1);
	Ds18b20WriteByte(0xcc);		//跳过ROM操作命令		 
	Ds18b20WriteByte(0x44);	    //温度转换命令
//	Delay1ms(100);	//等待转换成功，而如果你是一直刷着的话，就不用这个延时了

}
/*******************************************************************************
 * 函 数 名         : Ds18b20ReadTempCom
 * 函数功能		   : 发送读取温度命令
 * 输    入         : com
 * 输    出         : 无
 *******************************************************************************/

void Ds18b20ReadTempCom() {

	Ds18b20Init();
	Delay1ms(1);
	Ds18b20WriteByte(0xcc);	 //跳过ROM操作命令
	Ds18b20WriteByte(0xbe);	 //发送读取温度命令
}
/*******************************************************************************
 * 函 数 名         : Ds18b20ReadTemp
 * 函数功能		   : 读取温度
 * 输    入         : com
 * 输    出         : 无
 *******************************************************************************/

int Ds18b20ReadTemp() {
	int temp = 0;
	uint8 tmh, tml;
	Ds18b20ChangTemp();			 	//先写入转换命令
	Ds18b20ReadTempCom();			//然后等待转换完后发送读取温度命令
	tml = Ds18b20ReadByte();		//读取温度值共16位，先读低字节
	tmh = Ds18b20ReadByte();		//再读高字节
	temp = tmh;
	temp <<= 8;
	temp |= tml;
	return temp;
}
