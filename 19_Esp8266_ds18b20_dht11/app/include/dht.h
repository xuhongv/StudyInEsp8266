//定义一个枚举类型，即DHT11还是DHT22
enum sensor_type{
	SENSOR_DHT11,SENSOR_DHT22
};
//定义一个读取DHT模块数据的结构，依次为温度、湿度、某型模块、传感器编号及是否读取成功标识
struct sensor_reading{
	float temperature;
	float humidity;
	const char* source;
	uint8_t sensor_id[16];
	BOOL success;
};

extern uint8 wendu,shidu;

//DHT模块初始化并启动
void DHTInit(enum sensor_type);

