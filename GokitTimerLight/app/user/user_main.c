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
#define GPIO_KEY_NUM                            1                           ///< Defines the total number of key members
#define KEY_0_IO_MUX                            PERIPHS_IO_MUX_GPIO4_U      ///< ESP8266 GPIO function
#define KEY_0_IO_NUM                            4                           ///< ESP8266 GPIO number
#define KEY_0_IO_FUNC                           FUNC_GPIO4                  ///< ESP8266 GPIO name

LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM];                              ///< Defines a single key member array pointer
LOCAL keys_typedef_t keys;                                                  ///< Defines the overall key module structure pointer    
/**@} */

/**
* Key1 key short press processing
* @param none
* @return none
*/
LOCAL void ICACHE_FLASH_ATTR keyShortPress(void)
{

	   uart1_sendStr_no_wait("WIFI_SOFTAP_MODE");
       gizwitsSetMode(WIFI_SOFTAP_MODE);


}

LOCAL void ICACHE_FLASH_ATTR keyLongPress(void)
{

    uart1_sendStr_no_wait("WIFI_AIRLINK_MODE ");
    gizwitsSetMode(WIFI_AIRLINK_MODE);


}

LOCAL void ICACHE_FLASH_ATTR keyInit(void)
{

	 os_printf(" keyInit 方法 ");
    singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
                                keyLongPress, keyShortPress); //key1LongPress方法回调对应上面的，key1shortPress同样也是
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
    uart0_sendStr("init 1...");
    os_printf("init 2...");
    UART_SetPrintPort(1);

    struct rst_info *rtc_info = system_get_rst_info();

    if (rtc_info->reason == REASON_WDT_RST ||
        rtc_info->reason == REASON_EXCEPTION_RST ||
        rtc_info->reason == REASON_SOFT_WDT_RST)
    {
        if (rtc_info->reason == REASON_EXCEPTION_RST)
        {
            GIZWITS_LOG("Fatal exception (%d):\n", rtc_info->exccause);
        }
   }

    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
    {
        GIZWITS_LOG( "---UPGRADE_FW_BIN1---\n");
    }
    else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
    {
        GIZWITS_LOG( "---UPGRADE_FW_BIN2---\n");
    }



    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); //GPIO12初始化
    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);//GPIO12 低电平输出

    keyInit();
    
    userInit();

    gizwitsInit();  

    //user timer 
    os_timer_disarm(&userTimer);
    os_timer_setfn(&userTimer, (os_timer_func_t *)userHandle, NULL);
    os_timer_arm(&userTimer, USER_TIME_MS, 1);

    uart1_sendStr_no_wait("init end1...");
    os_printf("init end33...");
    uart0_sendStr("init 34...");

}
