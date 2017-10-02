/**
************************************************************
* @file         user_main.c
* @brief        The program entry file
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
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "gagent_soc.h"
#include "user_devicefind.h"
#include "user_webserver.h"
#include "gizwits_product.h"
#include "driver/hal_key.h"

#include "driver/hal_infrared.h"

#include "driver/hal_motor.h"

#include "driver/hal_rgb_led.h"

#include "driver/hal_temp_hum.h"




#if ESP_PLATFORM
#include "user_esp_platform.h"
#endif

#ifdef SERVER_SSL_ENABLE
#include "ssl/cert.h"
#include "ssl/private_key.h"
#else
#ifdef CLIENT_SSL_ENABLE
unsigned char *default_certificate;
unsigned int default_certificate_len = 0;
unsigned char *default_private_key;
unsigned int default_private_key_len = 0;
#endif
#endif

/**@name User timers related parameters
* @{
*/
#define USER_TIME_MS 1000                                                   ///< Timing time in milliseconds
LOCAL os_timer_t userTimer;                                                 ///< User timer structure
/**@} */ 

/**@name Key related definitions 
* @{
*/
#define GPIO_KEY_NUM                            2                           ///< Defines the total number of key members
#define KEY_0_IO_MUX                            PERIPHS_IO_MUX_GPIO0_U      ///< ESP8266 GPIO function
#define KEY_0_IO_NUM                            0                           ///< ESP8266 GPIO number
#define KEY_0_IO_FUNC                           FUNC_GPIO0                  ///< ESP8266 GPIO name
#define KEY_1_IO_MUX                            PERIPHS_IO_MUX_MTMS_U       ///< ESP8266 GPIO function
#define KEY_1_IO_NUM                            14                          ///< ESP8266 GPIO number
#define KEY_1_IO_FUNC                           FUNC_GPIO14                 ///< ESP8266 GPIO name
LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM];                              ///< Defines a single key member array pointer
LOCAL keys_typedef_t keys;                                                  ///< Defines the overall key module structure pointer    
/**@} */

/**
* Key1 key short press processing
* @param none
* @return none
*/
LOCAL void ICACHE_FLASH_ATTR key1ShortPress(void)
{
    GIZWITS_LOG("#### KEY1 short press ,Production Mode\n");
    
    gizwitsSetMode(WIFI_PRODUCTION_TEST);
}

/**
* Key1 key presses a long press
* @param none
* @return none
*/
LOCAL void ICACHE_FLASH_ATTR key1LongPress(void)
{
    GIZWITS_LOG("#### key1 long press, default setup\n");
    
    gizwitsSetMode(WIFI_RESET_MODE);
}

/**
* Key2 key to short press processing
* @param none
* @return none
*/
LOCAL void ICACHE_FLASH_ATTR key2ShortPress(void)
{
    GIZWITS_LOG("#### key2 short press, soft ap mode \n");

    rgbControl(250,0,0);
    gizwitsSetMode(WIFI_SOFTAP_MODE);
}

/**
* Key2 button long press
* @param none
* @return none
*/
LOCAL void ICACHE_FLASH_ATTR key2LongPress(void)
{
    GIZWITS_LOG("#### key2 long press, airlink mode\n");
    rgbControl(0,250,0);
    gizwitsSetMode(WIFI_AIRLINK_MODE);
}

/**
* Key to initialize
* @param none
* @return none
*/
LOCAL void ICACHE_FLASH_ATTR keyInit(void)
{
    singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
                                key1LongPress, key1ShortPress);
    singleKey[1] = keyInitOne(KEY_1_IO_NUM, KEY_1_IO_MUX, KEY_1_IO_FUNC,
                                key2LongPress, key2ShortPress);
    keys.singleKey = singleKey;
    keyParaInit(&keys);
}

/**
* @brief user_rf_cal_sector_set

* Use the 636 sector (2544k ~ 2548k) in flash to store the RF_CAL parameter
* @param none
* @return none
*/
uint32_t ICACHE_FLASH_ATTR user_rf_cal_sector_set()
{
    return 636;
}

/**
* @brief program entry function

* In the function to complete the user-related initialization
* @param none
* @return none
*/
void ICACHE_FLASH_ATTR user_init(void)
{
    uint32_t system_free_size = 0;

    wifi_station_set_auto_connect(1);
    wifi_set_sleep_type(NONE_SLEEP_T);//set none sleep mode
    espconn_tcp_set_max_con(10);
    uart_init_3(9600,115200);
    UART_SetPrintPort(1);
    GIZWITS_LOG( "---------------SDK version:%s--------------\n", system_get_sdk_version());
    GIZWITS_LOG( "system_get_free_heap_size=%d\n",system_get_free_heap_size());

    struct rst_info *rtc_info = system_get_rst_info();
    GIZWITS_LOG( "reset reason: %x\n", rtc_info->reason);
    if (rtc_info->reason == REASON_WDT_RST ||
        rtc_info->reason == REASON_EXCEPTION_RST ||
        rtc_info->reason == REASON_SOFT_WDT_RST)
    {
        if (rtc_info->reason == REASON_EXCEPTION_RST)
        {
            GIZWITS_LOG("Fatal exception (%d):\n", rtc_info->exccause);
        }
        GIZWITS_LOG( "epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\n",
                rtc_info->epc1, rtc_info->epc2, rtc_info->epc3, rtc_info->excvaddr, rtc_info->depc);
    }

    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
    {
        GIZWITS_LOG( "---UPGRADE_FW_BIN1---\n");
    }
    else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
    {
        GIZWITS_LOG( "---UPGRADE_FW_BIN2---\n");
    }

    os_memset((uint8_t*)&currentDataPoint,0,sizeof(dataPoint_t));

     rgbGpioInit();
     rgbLedInit();


     motorInit();
     motorControl(0);

     dh11Init();

     irInit();


     keyInit();
    
    userInit();

    gizwitsInit();  

    //user timer 
    os_timer_disarm(&userTimer);
    os_timer_setfn(&userTimer, (os_timer_func_t *)userHandle, NULL);
    os_timer_arm(&userTimer, USER_TIME_MS, 1);

    GIZWITS_LOG("--- system_free_size = %d ---\n", system_get_free_heap_size());
}
