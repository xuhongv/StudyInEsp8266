#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "sntp.h"
#include "user_config.h"
#include "DS1302.h"
#include "xSntp.h"

sntp_data data;

#define SYSTEM_PARTITION_OTA_SIZE 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR 0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR 0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR 0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0x3fd000

static const partition_item_t at_partition_table[] = { {
		SYSTEM_PARTITION_BOOTLOADER, 0x0, 0x1000 }, { SYSTEM_PARTITION_OTA_1,
		0x1000, SYSTEM_PARTITION_OTA_SIZE }, { SYSTEM_PARTITION_OTA_2,
SYSTEM_PARTITION_OTA_2_ADDR, SYSTEM_PARTITION_OTA_SIZE }, {
		SYSTEM_PARTITION_RF_CAL,
		SYSTEM_PARTITION_RF_CAL_ADDR, 0x1000 }, { SYSTEM_PARTITION_PHY_DATA,
SYSTEM_PARTITION_PHY_DATA_ADDR, 0x1000 }, { SYSTEM_PARTITION_SYSTEM_PARAMETER,
		SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 0x3000 }, };

void ICACHE_FLASH_ATTR user_pre_init(void) {
	if (!system_partition_table_regist(at_partition_table,
			sizeof(at_partition_table) / sizeof(at_partition_table[0]), 6)) {
		os_printf("system_partition_table_regist fail\r\n");
		while (1)
			;
	}
}


LOCAL os_timer_t second_timer, getNetSyncTimer;
LOCAL unsigned char time_buf1[8] = { 20, 19, 3, 29, 16, 20, 00, 6 }; //20空年3月13日18时50分00秒6周
LOCAL unsigned char time_buf[8]; //空年月日时分秒周
LOCAL void ICACHE_FLASH_ATTR second_timer_Callback(void) {

    unsigned char i, tmp;
	time_buf[1] = DS1302_master_readByte(ds1302_year_add);  //年
	time_buf[2] = DS1302_master_readByte(ds1302_month_add);  //月
	time_buf[3] = DS1302_master_readByte(ds1302_date_add);  //日
	time_buf[4] = DS1302_master_readByte(ds1302_hr_add);  //时
	time_buf[5] = DS1302_master_readByte(ds1302_min_add);  //分
	time_buf[6] = (DS1302_master_readByte(ds1302_sec_add)) & 0x7F;  //秒
	time_buf[7] = DS1302_master_readByte(ds1302_day_add);  //周

	for (i = 0; i < 8; i++) {           //BCD处理
		tmp = time_buf[i] / 16;
		time_buf1[i] = time_buf[i] % 16;
		time_buf1[i] = time_buf1[i] + tmp * 10;
	}

	os_printf("20%x-%d-%d  %d:%d:%d \n", time_buf[1], time_buf[2], time_buf[3],
			time_buf1[4], time_buf1[5], time_buf1[6]);
}

LOCAL void ICACHE_FLASH_ATTR second_timer_init(void) {
	os_timer_disarm(&second_timer); //关闭second_timer
	os_timer_setfn(&second_timer, (os_timer_func_t *) second_timer_Callback,
	NULL); //设置定时器回调函数
	os_delay_us(60000); //延时等待稳定
	os_timer_arm(&second_timer, 1000, 1); //使能毫秒定时器
}

LOCAL void ICACHE_FLASH_ATTR SyncNetTimeCallBack(void) {

    //获取时间戳
	uint32 ts = sntp_get_current_timestamp();

	if (ts != 0) {
		os_timer_disarm(&getNetSyncTimer);
		os_delay_us(60000); //延时等待稳定
		os_delay_us(60000); //延时等待稳定
        //处理时间戳，返回具体的时间
		char *pDate = (void *) sntp_get_real_time(ts);
		data = sntp_get_time_change(pDate);
		os_printf("20%x_%x_%x_%x:%x:%x_%x\n",data.year,data.month,data.day,data.hour,data.minute,data.second,data.week);

		time_buf1[1] = data.year;
		time_buf1[2] = data.month;
		time_buf1[3] = data.day;
		time_buf1[4] = data.hour;
		time_buf1[5] = data.minute;
		time_buf1[6] = data.second;
        //写入ds1302
		DS1302_Clock_init(time_buf1);

		os_timer_arm(&second_timer, 1000, 1); //使能毫秒定时器
	}
}

void ICACHE_FLASH_ATTR wifi_event_handler_cb(System_Event_t *event) {

	if (event == NULL) {
		return;
	}

	switch (event->event) {

	case EVENT_STAMODE_GOT_IP:
		//开始网络授时
		sntp_setservername(0, "time1.aliyun.com");
		sntp_setservername(1, "time2.aliyun.com");
		sntp_setservername(2, "time3.aliyun.com"); // set sntp server after got ip address
		sntp_init();
		os_timer_disarm(&getNetSyncTimer);
		os_timer_setfn(&getNetSyncTimer, (os_timer_func_t *) SyncNetTimeCallBack,NULL);
		os_timer_arm(&getNetSyncTimer, 1000, 1);
		break;

	case EVENT_STAMODE_DISCONNECTED:
		wifi_station_connect();
		break;
	}
}



void ICACHE_FLASH_ATTR user_init(void) {


    //连接指定的路由器
	struct station_config stationConf;
	wifi_set_opmode_current(STATION_MODE);
	os_memset(&stationConf, 0, sizeof(struct station_config));
	os_sprintf(stationConf.ssid,  "HUAWEI-APPT");
	os_sprintf(stationConf.password,  "xlinyun#123456");
	wifi_station_set_config_current(&stationConf);
	wifi_set_event_handler_cb(wifi_event_handler_cb);
	wifi_station_connect();

    //初始化ds1302
	DS1302_master_gpio_init();

	//读取时间的定时器开始
	second_timer_init();

}

