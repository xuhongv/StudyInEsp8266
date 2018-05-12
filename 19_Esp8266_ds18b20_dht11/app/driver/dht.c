/*
 * DHT模块数据读取
 */

#include "ets_sys.h"
#include "osapi.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "gpio.h"
#include "dht.h"

uint8 wendu, shidu;

//定义循环操作的最高上限
#define MAXTIMINGS 10000

//定义循环操作的最高上限
#define DHT_MAXCOUNT 32000

//读取DHT电平时长的分界值，低于此时长其响应数值为为0，高于此时长为1，参见DHT模块说明
#define BREAKTIME 32

//定义DHT模块接入Esp8266的哪个引脚，本例以gpio0为例，大家可以自行更改与硬件连接一致。
#define DHT_PIN 5

//定义枚举类型的传感器变量
enum sensor_type SENSOR;

//根据从DHT模块上获取的脉冲值，计算湿度值的函数，DHT11与DHT22的脉冲值与湿度值的关系参阅各自的模块说明，这里即是根据各自模块的固有属性，给出计算方法
//DHT11的湿度值为前八位脉冲，记录在data[0]中
static inline float scale_humidity(int *data) {
	if (SENSOR == SENSOR_DHT11) {
		return data[0];
	} else {
		float humidity = data[0] * 256 + data[1];
		return humidity /= 10;
	}
}

//根据从DHT模块上获取的脉冲值，计算温度值的函数，DHT11与DHT22的脉冲值与温度值的关系参阅各自的模块说明，这里即是根据各自模块的固有属性，给出计算方法
//DHT11的温度值为第三组八位脉冲，记录在data[3]中
static inline float scale_temperature(int *data) {
	if (SENSOR == SENSOR_DHT11) {
		return data[2];
	} else {
		float temperature = data[2] & 0x7f;
		temperature *= 256;
		temperature += data[3];
		temperature /= 10;
		if (data[2] & 0x80)
			temperature *= -1;
		return temperature;
	}
}

//毫秒延时函数，即系统微妙延时函数的1000倍
static inline void delay_ms(int sleep) {
	os_delay_us(1000 * sleep);
}

//定义一个读取DHT模块各项值的变量，并初始化为DHT11模块和读取成功与否标志为假
static struct sensor_reading reading = { .source = "DHT11", .success = 0 };

//读取DHT模块数据  static
void ICACHE_FLASH_ATTR pollDHTCb(void * arg) {
	//计数变量
	int counter = 0;
	//输入状态记录变量
	int laststate = 1;
	//循环控制变量
	int i = 0;
	//获取到的数据位的数量
	int bits_in = 0;

	//用于记录获取脉冲数据的数组
	int data[100];
	//初始化前5位为0，分别记录湿度整数、湿度小数、温度整数、温度小数和校验和
	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	//禁止中断操作
	ets_intr_lock();

	//构建DHT的读取时序，具体时序参阅DHT模块说明
	// 设置连接DHT11的引脚输出为高，持续20毫秒
	GPIO_OUTPUT_SET(DHT_PIN, 1);
	delay_ms(20);

	// 设置连接DHT11的引脚输出为低，持续20毫秒
	GPIO_OUTPUT_SET(DHT_PIN, 0);
	delay_ms(20);

	// 设置连接DHT11的引脚设置为输入，并延时40微秒
	GPIO_DIS_OUTPUT(DHT_PIN);
	os_delay_us(40);

	// 限时等待连接DHT11的引脚输入状态变成0，即等待获取DHT响应，如长时间未变为0则转为读取失败状态
	while (GPIO_INPUT_GET(DHT_PIN) == 1 && i < DHT_MAXCOUNT) {
		if (i >= DHT_MAXCOUNT) {
			goto fail;
		}
		i++;
	}

	// 通过循环读取DHT返回的40位0、1数据
	for (i = 0; i < MAXTIMINGS; i++) {
		// 每次循环计数归零
		counter = 0;
		//如果输入引脚状态没有改变，则计数加1并延时1微妙，如果计数超过1000，也就是说同一输入状态持续1000微妙以上，或者输入状态改变，则结束while循环
		while (GPIO_INPUT_GET(DHT_PIN) == laststate) {
			counter++;
			os_delay_us(1);
			if (counter == 1000)
				break;
		}
		//记录最新的输入状态
		laststate = GPIO_INPUT_GET(DHT_PIN);
		//如果计数超过1000，结束for循环
		if (counter == 1000)
			break;

		// 根据读取时序，DHT相应开始后，第三次脉冲跳变后的高电平时间长短则为0或者1的值，尔后每次先一个50微妙的开始发送数据信号后为具体数据，据此规律进行脉冲读取
		//如果为代表0、1数据的脉冲，则读取值，少于模块定义的时长为0，大于为1，通过位操作将收到的5组8位数据分别存入data0至4中，分别代表湿度整数、湿度小数、温度整数、温度小数及校验和的值
		if ((i > 3) && (i % 2 == 0)) {
			//默认为0，则左移动一位，最低一位补0
			data[bits_in / 8] <<= 1;
			//如果大于32us，则数据为1，和1相与，最低位为1
			if (counter > BREAKTIME) {
				data[bits_in / 8] |= 1;
			}
			bits_in++;
		}
	}

	//退出禁止中断操作
	ets_intr_unlock();
	//读取的数据位少于40位，则表示读取失败
	if (bits_in < 40) {
		goto fail;
	}

	//根据data前4位计算校验和，计算方法参阅DHT模块说明
	int checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;

	//如果计算得到的校验和与读取到的不一致，则表示读取失败
	if (data[4] != checksum) {
		goto fail;
	}
	//记录得到的温度信息与湿度信息，并标记为读取成功，结束pollDHTCb函数
	reading.temperature = scale_temperature(data);
	reading.humidity = scale_humidity(data);

	wendu = (uint8) (reading.temperature);
	shidu = (uint8) (reading.humidity);

	reading.success = 1;
	return;

	//读取失败后，标记为读取失败，结束pollDHTCb函数
	fail:

	reading.success = 0;
}

//DHT模块初始化。第一个参数为模块类型，也就是DHT11或者22，第二个参数为读取温湿度值的间隔时间，单位为毫秒，根据模块说明采样周期不得低于1秒
void DHTInit(enum sensor_type sensor_type) {
	SENSOR = sensor_type;
	// 设置GPIO0为标准的输入输出接口，如果大家使用别的GPIO接口，此语句也要修改为接口的接口
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);

}
