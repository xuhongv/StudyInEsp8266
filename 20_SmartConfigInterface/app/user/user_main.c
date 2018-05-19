#include "../include/debug.h"
#include "hal_key.h"
#include "../include/uart.h"
#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "xSmartConfig.h"

//按键定义
#define GPIO_KEY_NUM                            1
#define KEY_0_IO_MUX                            PERIPHS_IO_MUX_MTDI_U
#define KEY_0_IO_NUM                            12
#define KEY_0_IO_FUNC                           FUNC_GPIO12

LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM];
LOCAL keys_typedef_t keys;

LOCAL void ICACHE_FLASH_ATTR funXSmartConfigCallBack(
		xSmartConfig_Status_Code code) {
	os_printf("funXSmartConfigCallBack: %d\n \n\n", code);
}

//按键初始化
LOCAL void ICACHE_FLASH_ATTR keyLongPress(void) {

}

LOCAL void ICACHE_FLASH_ATTR keyShortPress(void) {
	os_printf("----------开始进去SmartConfig配网模式...\n\n\n-----");
	xSmartConfig_Start();
}

LOCAL void ICACHE_FLASH_ATTR keyInit(void) {
	singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
			keyLongPress, keyShortPress);
	keys.singleKey = singleKey;
	keyParaInit(&keys);
}

void ICACHE_FLASH_ATTR user_init(void) {
	uart_init(115200, 115200);
	os_printf("----------BIT_RATE_115200\n\n\n-----");
	keyInit();

	register_xSmartConfigCallBack(funXSmartConfigCallBack);
}

//用户⾃自定义 RF_CAL 参数存放在 Flash 的扇区号
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

	default:
		rf_cal_sec = 0;
		break;
	}

	return rf_cal_sec;
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void) {
}

