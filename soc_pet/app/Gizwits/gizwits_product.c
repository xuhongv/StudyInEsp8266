/**
************************************************************
* @file         gizwits_product.c
* @brief        Control protocol processing, and platform-related hardware initialization
* @author       Gizwits
* @date         2017-07-19
* @version      V03030000
* @copyright    Gizwits
*
* @note         鏈烘櫤浜�.鍙负鏅鸿兘纭欢鑰岀敓
*               Gizwits Smart Cloud  for Smart Products
*               閾炬帴|澧炲�贾祙寮�鏀緗涓珛|瀹夊叏|鑷湁|鑷敱|鐢熸��
*               www.gizwits.com
*
***********************************************************/
#include <stdio.h>
#include <string.h>
#include "gizwits_product.h"
#include "driver/hal_key.h"


#define USER_TIME_MS     100

#define TH_TIMEOUT (1000/USER_TIME_MS)

#define INF_TIMEOUT (500/USER_TIME_MS)

/** User area The current device state structure */
dataPoint_t currentDataPoint;

int8_t ICACHE_FLASH_ATTR gizwitsEventProcess(eventInfo_t *info, uint8_t *data, uint32_t len)
{
    uint8_t i = 0;
    dataPoint_t * dataPointPtr = (dataPoint_t *)data;
    moduleStatusInfo_t * wifiData = (moduleStatusInfo_t *)data;

    if((NULL == info) || (NULL == data))
    {
        GIZWITS_LOG("!!! gizwitsEventProcess Error \n");
        return -1;
    }

    for(i = 0; i < info->num; i++)
    {
        switch(info->event[i])
        {
        case EVENT_LED_OnOff :
            currentDataPoint.valueLED_OnOff = dataPointPtr->valueLED_OnOff;

            if(0x01 == currentDataPoint.valueLED_OnOff)
            {
            	rgbControl(254,0,0);///< 新添加代码: 对应开启红灯 
            }
            else
            {
            	rgbControl(0,0,0);
            }
            break;

        case EVENT_LED_Color:
            currentDataPoint.valueLED_Color = dataPointPtr->valueLED_Color;
            GIZWITS_LOG("Evt: EVENT_LED_Color %d\n", currentDataPoint.valueLED_Color);
            switch(currentDataPoint.valueLED_Color)
            {
            case LED_Color_VALUE0:
            	rgbControl(currentDataPoint.valueLED_R,currentDataPoint.valueLED_G,currentDataPoint.valueLED_B);
                break;
            case LED_Color_VALUE1:
            	rgbControl(254,254,0);
                break;
            case LED_Color_VALUE2:
            	rgbControl(254,0,70);
                break;
            case LED_Color_VALUE3:
            	rgbControl(238,30,30);
                break;
            default:
                break;
            }
            break;

        case EVENT_LED_R:
            currentDataPoint.valueLED_R= dataPointPtr->valueLED_R;
            GIZWITS_LOG("Evt:EVENT_LED_R %d\n",currentDataPoint.valueLED_R);
            rgbControl(currentDataPoint.valueLED_R,currentDataPoint.valueLED_G,currentDataPoint.valueLED_B);
            break;
        case EVENT_LED_G:
            currentDataPoint.valueLED_G= dataPointPtr->valueLED_G;
            GIZWITS_LOG("Evt:EVENT_LED_G %d\n",currentDataPoint.valueLED_G);
            rgbControl(currentDataPoint.valueLED_R,currentDataPoint.valueLED_G,currentDataPoint.valueLED_B);
            break;
        case EVENT_LED_B:
            currentDataPoint.valueLED_B= dataPointPtr->valueLED_B;
            GIZWITS_LOG("Evt:EVENT_LED_B %d\n",currentDataPoint.valueLED_B);
            rgbControl(currentDataPoint.valueLED_R,currentDataPoint.valueLED_G,currentDataPoint.valueLED_B);
            break;
        case EVENT_Motor_Speed:
            currentDataPoint.valueMotor_Speed= dataPointPtr->valueMotor_Speed;
            GIZWITS_LOG("Evt:EVENT_Motor_Speed %d\n",currentDataPoint.valueMotor_Speed);
            motorControl(currentDataPoint.valueMotor_Speed);
            break;

        case WIFI_SOFTAP:
            break;
        case WIFI_AIRLINK:
            break;
        case WIFI_STATION:
            break;
        case WIFI_CON_ROUTER:
            GIZWITS_LOG("@@@@ connected router\n");
            rgbControl(0,0,0);//成功连接路由后关闭红灯
            break;
        case WIFI_DISCON_ROUTER:
            GIZWITS_LOG("@@@@ disconnected router\n");
 
            break;
        case WIFI_CON_M2M:
            GIZWITS_LOG("@@@@ connected m2m\n");
			setConnectM2MStatus(0x01);
 
            break;
        case WIFI_DISCON_M2M:
            GIZWITS_LOG("@@@@ disconnected m2m\n");
			setConnectM2MStatus(0x00);
 
            break;
        case WIFI_RSSI:
            GIZWITS_LOG("@@@@ RSSI %d\n", wifiData->rssi);
            break;
        case TRANSPARENT_DATA:
            GIZWITS_LOG("TRANSPARENT_DATA \n");
            //user handle , Fetch data from [data] , size is [len]
            break;
        case MODULE_INFO:
            GIZWITS_LOG("MODULE INFO ...\n");
            break;
            
        default:
            break;
        }
    }
    system_os_post(USER_TASK_PRIO_2, SIG_UPGRADE_DATA, 0);
    
    return 0; 
}


/**
* User data acquisition

* Here users need to achieve in addition to data points other than the collection of data collection, can be self-defined acquisition frequency and design data filtering algorithm

* @param none
* @return none
*/
void ICACHE_FLASH_ATTR userHandle(void)
{
    /*
    currentDataPoint.valueInfrared = ;//Add Sensor Data Collection
    currentDataPoint.valueTemperature = ;//Add Sensor Data Collection
    currentDataPoint.valueHumidity = ;//Add Sensor Data Collection
    currentDataPoint.valueAlert_1 = ;//Add Sensor Data Collection
    currentDataPoint.valueAlert_2 = ;//Add Sensor Data Collection
    currentDataPoint.valueFault_LED = ;//Add Sensor Data Collection
    currentDataPoint.valueFault_Motor = ;//Add Sensor Data Collection
    currentDataPoint.valueFault_TemHum = ;//Add Sensor Data Collection
    currentDataPoint.valueFault_IR = ;//Add Sensor Data Collection

    */







uint8_t ret=0;
uint8_t curTemperature=0;
uint8_t curHumidity=0;
uint8_t curIr =0;
static uint8_t thCtime=0;
static uint8_t irCtime=0;

thCtime++;
irCtime++;


 if( 5 <irCtime){
	 irCtime=0;
	 curIr=irUpdateStatus();
     currentDataPoint.valueInfrared=curIr;
 }

 if( 5 <thCtime){

	 thCtime=0;
	 ret=dh11Read(&curTemperature,&curHumidity);

	 if(0==ret){
        currentDataPoint.valueTemperature=curTemperature;
        currentDataPoint.valueHumidity=curHumidity;
	 }



 }
	 
    system_os_post(USER_TASK_PRIO_2, SIG_UPGRADE_DATA, 0);
}


/**
* Data point initialization function

* In the function to complete the initial user-related data
* @param none
* @return none
* @note The developer can add a data point state initialization value within this function
*/
void ICACHE_FLASH_ATTR userInit(void)
{
    gizMemset((uint8_t *)&currentDataPoint, 0, sizeof(dataPoint_t));

 	/** Warning !!! DataPoint Variables Init , Must Within The Data Range **/ 
    /*
   		currentDataPoint.valueLED_OnOff = ;
   		currentDataPoint.valueLED_Color = ;
   		currentDataPoint.valueLED_R = ;
   		currentDataPoint.valueLED_G = ;
   		currentDataPoint.valueLED_B = ;
   		currentDataPoint.valueMotor_Speed = ;
   		currentDataPoint.valueInfrared = ;
   		currentDataPoint.valueTemperature = ;
   		currentDataPoint.valueHumidity = ;
   		currentDataPoint.valueAlert_1 = ;
   		currentDataPoint.valueAlert_2 = ;
   		currentDataPoint.valueFault_LED = ;
   		currentDataPoint.valueFault_Motor = ;
   		currentDataPoint.valueFault_TemHum = ;
   		currentDataPoint.valueFault_IR = ;
    */
}


