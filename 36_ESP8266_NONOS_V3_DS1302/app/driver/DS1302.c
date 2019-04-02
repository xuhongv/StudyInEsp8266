
#include "DS1302.h"
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"

void ICACHE_FLASH_ATTR
DS1302_master_gpio_init(void) {
	ETS_GPIO_INTR_DISABLE();
//    ETS_INTR_LOCK();

	PIN_FUNC_SELECT(DS1302_MASTER_IO_MUX, DS1302_MASTER_IO_FUNC);
	PIN_FUNC_SELECT(DS1302_MASTER_SCLK_MUX, DS1302_MASTER_SCLK_FUNC);
	PIN_FUNC_SELECT(DS1302_MASTER_RST_MUX, DS1302_MASTER_RST_FUNC);

	GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(DS1302_MASTER_IO_GPIO)),
			GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(DS1302_MASTER_IO_GPIO))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
	GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS,
			GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << DS1302_MASTER_IO_GPIO));

	GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(DS1302_MASTER_SCLK_GPIO)),
			GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(DS1302_MASTER_SCLK_GPIO))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
	GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS,
			GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << DS1302_MASTER_SCLK_GPIO));

	GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(DS1302_MASTER_RST_GPIO)),
			GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(DS1302_MASTER_RST_GPIO))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
	GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS,
			GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << DS1302_MASTER_RST_GPIO));

	DS1302_MASTER_SCLK_LOW_RST_LOW(); //初始化RST、SCLK

	ETS_GPIO_INTR_ENABLE();

}
void ICACHE_FLASH_ATTR
DS1302_master_writeByte(uint8 addr, uint8 wrdata) //向DS1302写入一字节数据
{
	uint8 i;
	GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_RST_GPIO), 1);        //启动DS1302总线
	os_delay_us(1);
	//写入目标地址：addr
	addr = addr & 0xFE;   //最低位置零，寄存器0位为0时写，为1时读
	for (i = 0; i < 8; i++) {
		if (addr & 0x01) {
			GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_IO_GPIO), 1);
		} else {
			GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_IO_GPIO), 0);
		}
		ets_delay_us(1);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_SCLK_GPIO), 1);   //产生上升沿时钟
		ets_delay_us(1);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_SCLK_GPIO), 0);
		ets_delay_us(1);
		;
		addr = addr >> 1;
	}
	//写入数据：wrdata
	for (i = 0; i < 8; i++) {
		if (wrdata & 0x01) {
			GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_IO_GPIO), 1);
		} else {
			GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_IO_GPIO), 0);
		}
		ets_delay_us(1);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_SCLK_GPIO), 1);   //产生上升沿时钟
		ets_delay_us(1);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_SCLK_GPIO), 0);
		ets_delay_us(1);
		;
		wrdata = wrdata >> 1;
	}
	GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_RST_GPIO), 0);		//停止DS1302总线
	os_delay_us(1);
}

uint8 ICACHE_FLASH_ATTR
DS1302_master_readByte(uint8 addr) //从DS1302读出一字节数据
{
	uint8 i, temp = 0; //注意：temp这里必须初始化，不然读出来的数据是错误的，调试了几天才发现这个bug
	GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_RST_GPIO), 1);        //启动DS1302总线
	os_delay_us(1);
	//写入目标地址：addr
	addr = addr | 0x01;    //最低位置高，寄存器0位为0时写，为1时读
	for (i = 8; i > 0; i--) {
		if (addr & 0x01) {
			GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_IO_GPIO), 1);
		} else {
			GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_IO_GPIO), 0);
		}
		ets_delay_us(1);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_SCLK_GPIO), 1);   //产生上升沿时钟
		ets_delay_us(1);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_SCLK_GPIO), 0);
		ets_delay_us(1);
		addr = addr >> 1;
	}
	//读DS1302数据：temp
	for (i = 8; i > 0; i--) {
		temp = temp >> 1;
		os_delay_us(1);
		if (GPIO_INPUT_GET(GPIO_ID_PIN(DS1302_MASTER_IO_GPIO))) {
			temp |= 0x80;
		} else {
			temp &= 0x7F;
		}
		ets_delay_us(1);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_SCLK_GPIO), 1);   //产生上升沿时钟
		ets_delay_us(1);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_SCLK_GPIO), 0);
		ets_delay_us(1);
	}
	GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_MASTER_RST_GPIO), 0);		//停止DS1302总线
	os_delay_us(1);
	return temp;
}
//向DS302写入时钟数据
void ICACHE_FLASH_ATTR DS1302_Clock_init(unsigned char *pDate)		//初始化时钟数据
{

	int i = 0;
	for (i = 0; i < 7; i++) {
		os_printf("DS1302_Clock_init : %d \n", pDate[i]);
	}
	DS1302_master_writeByte(ds1302_control_add,0x00); //关闭写保护
	DS1302_master_writeByte(ds1302_sec_add,0x80); //暂停
	DS1302_master_writeByte(ds1302_charger_add,0xa9); //涓流充电
	DS1302_master_writeByte(ds1302_year_add,pDate[1]); //年
	DS1302_master_writeByte(ds1302_month_add,pDate[2]); //月
	DS1302_master_writeByte(ds1302_date_add,pDate[3]); //日
	DS1302_master_writeByte(ds1302_day_add,pDate[7]); //周
	DS1302_master_writeByte(ds1302_hr_add,pDate[4]); //时
	DS1302_master_writeByte(ds1302_min_add,pDate[5]); //分
	DS1302_master_writeByte(ds1302_sec_add,pDate[6]); //秒
	DS1302_master_writeByte(ds1302_day_add,pDate[7]); //周
	DS1302_master_writeByte(ds1302_control_add,0x80); //打开写保护
}

