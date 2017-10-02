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
/** User area The current device state structure */
dataPoint_t currentDataPoint;

//布尔值，定时器开关的状态
bool isTimer ;

uint16_t timer_timers;

/** 定时器结构体 */
static os_timer_t os_timer;



/**
 * 定时任务函数
 */
void Led_Task_Run(void){

 GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
 //执行完毕，我们要把定时时间设置0 ,定时使能为false
 timer_timers=0;
 isTimer=false;

}


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
        case EVENT_lightOnOff :
            currentDataPoint.valuelightOnOff = dataPointPtr->valuelightOnOff;
            uart1_sendStr_no_wait("收到的指令  %d\n ",currentDataPoint.valuelightOnOff);
            if(0x01 == currentDataPoint.valuelightOnOff)
            {
            	 GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
            }
            else
            {
            	 GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);
            }
            break;

        case EVENT_isTimerOpen :
            currentDataPoint.valueisTimerOpen = dataPointPtr->valueisTimerOpen;

            if(0x01 == currentDataPoint.valueisTimerOpen)
            {
            	isTimer=true;

              }
            else
            {
               /** 关闭该定时器 */
            os_timer_disarm( &os_timer );
            isTimer=false;
            }
            break;


        case EVENT_timerOpen:
        	  currentDataPoint.valuetimerOpen= dataPointPtr->valuetimerOpen;
              if(isTimer){
            	            /** 关闭该定时器 */
            	            os_timer_disarm( &os_timer );

            	            /** 配置该定时器回调函数 */
            	            os_timer_setfn( &os_timer, (ETSTimerFunc *) ( Led_Task_Run ), NULL );

            	            /** 开启该定时器 ：下发的是秒数，这里的单位是毫秒，要乘1000* ，后面false表示仅仅执行一次**/
            	            os_timer_arm( &os_timer, currentDataPoint.valuetimerOpen*1000, false );

            	            /**赋值给timer_timers，方便会调用 */
            	            timer_timers=currentDataPoint.valuetimerOpen;
                          }
               break;

        case WIFI_SOFTAP:
            break;
        case WIFI_AIRLINK:
            break;
        case WIFI_STATION:
            break;
        case WIFI_CON_ROUTER:
            GIZWITS_LOG("@@@@ connected router\n");
 
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


	        //获取GPIO12的高低电平，因为高电平是触发开灯，所以ture就是开灯
	        currentDataPoint.valuelightOnOff = GPIO_INPUT_GET(12)  ;
	        //是否开启定时器的回调
	   		currentDataPoint.valueisTimerOpen =isTimer ;
	   		//定时时间回调,如果用戶設置了定時器关闭，那么我们返回0，否则返回定时时间，注意这个定时时间是固定的。不是倒计时时间
	   		if(isTimer){
	   			currentDataPoint.valuetimerOpen =timer_timers ;
	        }else{
	   			currentDataPoint.valuetimerOpen =0;
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
   		currentDataPoint.valuelightOnOff = ;
   		currentDataPoint.valueisTimerOpen = ;
   		currentDataPoint.valuetimerOpen = ;
    */
}


