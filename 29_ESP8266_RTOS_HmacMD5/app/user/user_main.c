#include "esp_common.h"
#include "user_config.h"

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
 *******************************************************************************/
uint32 user_rf_cal_sector_set(void) {
	flash_size_map size_map = system_get_flash_size_map();
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

void TaskGetDeviceInfo(void *pvParameters) {

	//举个靶子，要加密的信息是  https://github.com/xuhongv
	uint8 info[30] = { "https://github.com/xuhongv" }, output16[32],
			output32[32], keyOutput[32];

	//md5 16位加密
	DeBugLog("before encrypt info: %s ", info);
	XH_MD5StartDigest(info, strlen((char *) info), XH_MD5_16, output16);
	DeBugLog("after encrypt Md5 %dbit: %s ", XH_MD5_16, output16);

	//md5 32位加密
	XH_MD5StartDigest(info, strlen((char *) info), XH_MD5_32, output32);
	DeBugLog("after encrypt Md5 %dbit: %s ", XH_MD5_32, output32);

	//Hmac md5加密，而且加密密码是 xuhong
	HMAC_XH_MD5(info, strlen((char *) info), "xuhong", keyOutput);
	DeBugLog("after encrypt HMAC_XH_MD5 : %s ", keyOutput);

	vTaskDelete(NULL);
}

void user_init(void) {

	DeBugLog("SDK version:%s %d  ", system_get_sdk_version(),
			system_get_free_heap_size());

	xTaskCreate(TaskGetDeviceInfo, "TaskGetDeviceInfo", 1024, NULL, 2, NULL);

}

