#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "config.h"
#include "debug.h"
#include "user_interface.h"
#include "user_config.h"
#include "mem.h"
#include "cJSON.h"


os_timer_t checkTimer;


void initGetMacAdress(void);


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



uint8 tempMessage[6];
void ICACHE_FLASH_ATTR initGetMacAdress(void) {


	volatile uint32_t chip_ID = system_get_chip_id();

	//获取station下的mac地址
	uint8 macAdress[6];
	if (!wifi_get_macaddr(STATION_IF, macAdress)) {
		INFO("Failed to get mac... \r\n");
	} else {
		INFO("succeed to get mac...\r\n");
	}
	os_sprintf(tempMessage, "%02x:%02x:%02x:%02x:%02x:%02x", macAdress[0],
			macAdress[1], macAdress[2], macAdress[3], macAdress[4],
			macAdress[5]);
	INFO(" MacAdress: %s\r\n", tempMessage);
}



void ICACHE_FLASH_ATTR parseJson() {

	/*
	 *
//解析以下一段json数据
{
	 "mac":"84:f3:eb:b3:a7:05",
	 "number":2,
	 "value":{"name":"半颗心脏",
              "age":18 ,
	          "blog":"https://blog.csdn.net/xh870189248"
	          },
	 "hex":[51,15,63,22,96]
}

     */
	u8* jsonRoot =
			"{\"mac\":\"84:f3:eb:b3:a7:05\", \"number\":2,\"value\":{\"name\":\"xuhongv\",\"age\":18,\"blog\":\"https://blog.csdn.net/xh870189248\"},\"hexArry\":[51,15,63,22,96]}";

	//首先整体判断是否为一个json格式的数据
	cJSON *pJsonRoot = cJSON_Parse(jsonRoot);
	//如果是否json格式数据
	if (pJsonRoot !=NULL) {


		char *s = cJSON_Print(pJsonRoot);
		os_printf("pJsonRoot: %s\r\n", s);
		cJSON_free((void *) s);

		//解析mac字段字符串内容
		cJSON *pMacAdress = cJSON_GetObjectItem(pJsonRoot, "mac");
		//判断mac字段是否json格式
		if (pMacAdress) {
			//判断mac字段是否string类型
			if (cJSON_IsString(pMacAdress))
				os_printf("get MacAdress:%s \n", pMacAdress->valuestring);

		} else
			os_printf("get MacAdress failed \n");



		//解析number字段int内容
		cJSON *pNumber = cJSON_GetObjectItem(pJsonRoot, "number");
		//判断number字段是否存在
		if (pNumber){
			if (cJSON_IsNumber(pNumber))
			os_printf("get Number:%d \n", pNumber->valueint);
		}
		else
			os_printf("get Number failed \n");

		//解析value字段内容，判断是否为json
		cJSON *pValue = cJSON_GetObjectItem(pJsonRoot, "value");
		if (pValue) {
			//进一步剖析里面的name字段:注意这个根节点是 pValue
			cJSON *pName = cJSON_GetObjectItem(pValue, "name");
			if (pName)
				if (cJSON_IsString(pName))
					os_printf("get value->Name : %s \n", pName->valuestring);


			//进一步剖析里面的age字段:注意这个根节点是 pValue
			cJSON *pAge = cJSON_GetObjectItem(pValue, "age");
			if (pAge)
				if (cJSON_IsNumber(pAge))
					os_printf("get value->Age : %d \n", pAge->valueint);


			//进一步剖析里面的blog字段:注意这个根节点是 pValue
			cJSON *pBlog= cJSON_GetObjectItem(pValue, "blog");
			if (pBlog)
				if (cJSON_IsString(pBlog))
				os_printf("get value->pBlog	 : %s \n", pBlog->valuestring);
		}

		//剖析
		cJSON *pArry = cJSON_GetObjectItem(pJsonRoot, "hexArry");
		if (pArry) {
			//获取数组长度
			int arryLength = cJSON_GetArraySize(pArry);
			os_printf("get arryLength : %d \n", arryLength);
			//逐个打印
			int i ;
			for (i = 0; i < arryLength; i++)
				os_printf("cJSON_GetArrayItem(pArry, %d)= %d \n",i,cJSON_GetArrayItem(pArry, i)->valueint);
		}


	} else {
		os_printf("this is not a json data ... \n");
	}


	cJSON_Delete(pJsonRoot);
	os_printf("get freeHeap: %d \n\n", system_get_free_heap_size());
}


void creatJson(){

/*
  {
	 "mac":"84:f3:eb:b3:a7:05",
	 "number":2,
	 "value":{"name":"xuhongv",
              "age":18 ,
	          "blog":"https://blog.csdn.net/xh870189248"
	          },
	 "hex":[51,15,63,22,96]
}
 */


	//取一下本地的station的mac地址
	initGetMacAdress();

	cJSON *pRoot = cJSON_CreateObject();
	cJSON *pValue = cJSON_CreateObject();


	cJSON_AddStringToObject(pRoot,"mac",tempMessage);
	cJSON_AddNumberToObject(pRoot,"number",2);

	cJSON_AddStringToObject(pValue,"mac","xuhongv");
	cJSON_AddNumberToObject(pValue,"age",18);
	cJSON_AddStringToObject(pValue,"mac","https://blog.csdn.net/xh870189248");

	cJSON_AddItemToObject(pRoot, "value",pValue);

    int hex[5]={51,15,63,22,96};
	cJSON *pHex = cJSON_CreateIntArray(hex,5);
	cJSON_AddItemToObject(pRoot,"hex",pHex);

	char *s = cJSON_Print(pRoot);
	os_printf("\r\n creatJson : %s\r\n", s);
	cJSON_free((void *) s);

	cJSON_Delete(pRoot);
}


void user_init(void) {
	os_printf("get freeHeap: %d\n", system_get_free_heap_size());
	parseJson();
	creatJson();

}
