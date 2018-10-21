
#include "osapi.h"
#include "user_interface.h"

#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000

static const partition_item_t at_partition_table[] = { {
		SYSTEM_PARTITION_BOOTLOADER, 0x0, 0x1000 }, { SYSTEM_PARTITION_OTA_1,
		0x1000, SYSTEM_PARTITION_OTA_SIZE }, { SYSTEM_PARTITION_OTA_2,
		SYSTEM_PARTITION_OTA_2_ADDR, SYSTEM_PARTITION_OTA_SIZE }, {
		SYSTEM_PARTITION_RF_CAL, SYSTEM_PARTITION_RF_CAL_ADDR, 0x1000 }, {
		SYSTEM_PARTITION_PHY_DATA, SYSTEM_PARTITION_PHY_DATA_ADDR, 0x1000 }, {
		SYSTEM_PARTITION_SYSTEM_PARAMETER,
		SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 0x3000 }, };

void ICACHE_FLASH_ATTR user_pre_init(void) {
	if (!system_partition_table_regist(at_partition_table,
			sizeof(at_partition_table) / sizeof(at_partition_table[0]),
			SPI_FLASH_SIZE_MAP)) {
		os_printf("system_partition_table_regist fail\r\n");
		while (1)
			;
	}
}

void ICACHE_FLASH_ATTR user_init(void) {

	os_printf("hello world , studyBoard esp8266 \n\n\n\n\n\n\n\n");
}
