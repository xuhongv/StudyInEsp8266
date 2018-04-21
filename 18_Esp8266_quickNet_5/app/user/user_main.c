#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "user_interface.h"
#include "c_types.h"
#include "smartconfig.h"

static os_timer_t os_timer;

LOCAL void ICACHE_FLASH_ATTR setTurnOnOffFlag(bool isReset) {

	if (!isReset) {
		//先读出来再加一
		u8 saveNumber[4];
		spi_flash_read(550 * 4096 + 20, (uint32 *) &saveNumber, 4);

		if (saveNumber[0] > 8 || saveNumber[0] < 0) {
			saveNumber[0] = 1;
		} else {
			saveNumber[0]++;
		}

		//数字转字符串
		spi_flash_erase_sector(550);
		spi_flash_write(550 * 4096 + 20, (uint32 *) &saveNumber, 4);
	} else {
		u8 saveNumber[4];
		saveNumber[0] = 0;
		//数字转字符串
		spi_flash_erase_sector(550);
		spi_flash_write(550 * 4096 + 20, (uint32 *) &saveNumber, 4);
	}

}

LOCAL u8 ICACHE_FLASH_ATTR getTurnOnOffFlag() {
	u8 tempSaveData[4];
	spi_flash_read(550 * 4096 + 20, (uint32 *) &tempSaveData, 4);
	os_printf("current save tempSaveData : %d \n:", tempSaveData[0]);
	//如果读取失败
	if (tempSaveData[0] == -1) {
		tempSaveData[0] = 1;

		spi_flash_erase_sector(550);
		spi_flash_write(550 * 4096 + 20, (uint32 *) &tempSaveData, 4);
	}
	return tempSaveData[0];
}

void Led_Cmd() {
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
	os_delay_us(60000);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);
	os_delay_us(60000);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
	os_delay_us(60000);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);
	os_delay_us(60000);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
	os_delay_us(60000);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);
	os_delay_us(60000);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
	os_delay_us(60000);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);
}

void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata) {
	switch (status) {
	case SC_STATUS_WAIT:
		os_printf("SC_STATUS_WAIT\n");
		break;
	case SC_STATUS_FIND_CHANNEL:
		os_printf("SC_STATUS_FIND_CHANNEL\n");
		break;
	case SC_STATUS_GETTING_SSID_PSWD:
		os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
		sc_type *type = pdata;
		if (*type == SC_TYPE_ESPTOUCH) {
			os_printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
		} else {
			os_printf("SC_TYPE:SC_TYPE_AIRKISS\n");
		}
		break;
	case SC_STATUS_LINK:
		os_printf("SC_STATUS_LINK\n");
		struct station_config *sta_conf = pdata;
		wifi_station_set_config(sta_conf);
		wifi_station_disconnect();
		wifi_station_connect();
		break;
	case SC_STATUS_LINK_OVER:
		os_printf("SC_STATUS_LINK_OVER\n");
		if (pdata != NULL) {
			uint8 phone_ip[4] = { 0 };
			os_memcpy(phone_ip, (uint8*) pdata, 4);
			os_printf("Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1],
					phone_ip[2], phone_ip[3]);
		}
		smartconfig_stop();
		break;
	}

}
void hw_test_timer_cb(void) {
	static u8 statusFlag = 0;
	statusFlag++;
	if (statusFlag == 1) {
		u8 flag = getTurnOnOffFlag();
		os_printf("current save flag : %d \n:", flag);
		if (flag > 4) {
			//LED开始闪烁
			Led_Cmd();
			//保存为0
			setTurnOnOffFlag(true);
			//进去一键配网模式
			smartconfig_set_type(SC_TYPE_ESPTOUCH);
			wifi_set_opmode(STATION_MODE);
			smartconfig_start(smartconfig_done);
			//关闭定时器
			os_timer_disarm(&os_timer);
		}
	} else if (statusFlag == 3) {
		setTurnOnOffFlag(true);
	}

}

void ICACHE_FLASH_ATTR user_init(void) {

	uart_init(57600, 57600);
	os_printf("SDK version:%s\n", system_get_sdk_version());

	setTurnOnOffFlag(false);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); //GPIO选择
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0); //常亮

	/** 关闭该定时器 */
	os_timer_disarm(&os_timer);
	/** 配置该定时器回调函数 */
	os_timer_setfn(&os_timer, (ETSTimerFunc *) (hw_test_timer_cb), NULL);
	/** 启动该定时器 */
	os_timer_arm(&os_timer, 1000, true);

}

uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void) {
	enum flash_size_map size_map = system_get_flash_size_map();
	uint32 rf_cal_sec = 0;

	switch (size_map) {
	case FLASH_SIZE_4M_MAP_256_256:
		rf_cal_sec = 128 - 5;
		break;

	case FLASH_SIZE_8M_MAP_512_512:
		rf_cal_sec = 256 - 5;
		break;

	case FLASH_SIZE_16M_MAP_512_512:
	case FLASH_SIZE_16M_MAP_1024_1024:
		rf_cal_sec = 512 - 5;
		break;

	case FLASH_SIZE_32M_MAP_512_512:
	case FLASH_SIZE_32M_MAP_1024_1024:
		rf_cal_sec = 1024 - 5;
		break;

	case FLASH_SIZE_64M_MAP_1024_1024:
		rf_cal_sec = 2048 - 5;
		break;
	case FLASH_SIZE_128M_MAP_1024_1024:
		rf_cal_sec = 4096 - 5;
		break;
	default:
		rf_cal_sec = 0;
		break;
	}
	return rf_cal_sec;
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void) {
}
