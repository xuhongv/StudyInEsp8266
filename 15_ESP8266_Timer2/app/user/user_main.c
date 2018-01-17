#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "user_interface.h"

#define	REG_READ(_r)					(*(volatile	uint32	*)(_r))
#define	WDEV_NOW()				REG_READ(0x3ff20c00)

void Led_Cmd(bool status) {
	if (status == true) {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);
	} else {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
	}
}

void hw_test_timer_cb(void) {
	static bool status = false;
	if (status == true) {
		status = false;
		os_printf("Led_Cmd false");
	} else {
		status = true;
		os_printf("Led_Cmd true");
	}
	Led_Cmd(status);

}

void ICACHE_FLASH_ATTR user_init(void) {

	uart_init(57600, 57600);
	os_printf("SDK version:%s\n", system_get_sdk_version());

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	Led_Cmd(false);

	hw_timer_init(0, 0);
	hw_timer_set_func(hw_test_timer_cb);
	hw_timer_arm(500);

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
