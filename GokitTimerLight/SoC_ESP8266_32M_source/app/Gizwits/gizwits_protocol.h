/**
************************************************************
* @file         gizwits_protocol.h
* @brief        Gizwits protocol related files (SDK API interface function definition)
* @author       Gizwits
* @date         2017-07-19
* @version      V03030000
* @copyright    Gizwits
*
* @note         机智云.只为智能硬件而生
*               Gizwits Smart Cloud  for Smart Products
*               链接|增值ֵ|开放|中立|安全|自有|自由|生态
*               www.gizwits.com
*
***********************************************************/

#ifndef _GIZWITS_PROTOCOL_H
#define _GIZWITS_PROTOCOL_H
#include <stdint.h>
#include "osapi.h"
#include "user_interface.h"
#include "user_webserver.h"
#include "common.h"

/**
* @name Protocol version number
* @{
*/
#define PROTOCOL_VERSION              "00000004"
/**@} */

/**@name P0 protocol version number
* @{
*/
#define P0_VERSION                    "00000002"
/**@} */

/**@name Product Key (Product identification code)
* @{
*/
#define PRODUCT_KEY "02ace125034540bfa64dc77bff1d45a8"
/**@} */
/**@name Product Secret (Product key)
* @{
*/
#define PRODUCT_SECRET "cb27cef9d72d41cf84174e1e14dd330e"

#define MAC_LEN_8266                  6
#define BUFFER_LEN_MAX                900
#define REPORT_TIME_MAX               6000          ///< 6s

/**@name Timer correlation status
* @{
*/
#define MAX_SOC_TIMOUT                1             ///< 1ms
#define TIM_REP_TIMOUT                600000        ///< 600s Regularly report
/**@} */

/**@name Task related definitions
* @{
*/
#define SIG_ISSUED_DATA               0x01          ///< Protocol processing tasks
#define SIG_PASSTHROUGH               0x02          ///< Protocol transparent task
#define SIG_IMM_REPORT                0x03          ///< The agreement immediately reports the task
#define SIG_UPGRADE_DATA              0x04
/**@} */



/**@name Whether the device is in the control class, 0 means no, 1 means yes
* @{
*/
#define DEV_IS_GATEWAY   0                    
/**@} */

/**@name Binding time
* @{
*/
#define NINABLETIME  0
/**@} */



#define MAX_PACKAGE_LEN    (sizeof(devStatus_t)+sizeof(attrFlags_t)+20)                 ///< Data buffer maximum length
#define RB_MAX_LEN          (MAX_PACKAGE_LEN*2)     ///< Maximum length of ring buffer

/**@name Data point related definition
* @{
*/
#define lightOnOff_BYTEOFFSET                    0
#define lightOnOff_BITOFFSET                     0
#define lightOnOff_LEN                           1
#define isTimerOpen_BYTEOFFSET                    0
#define isTimerOpen_BITOFFSET                     1
#define isTimerOpen_LEN                           1

#define timerOpen_RATIO                         1
#define timerOpen_ADDITION                      0
#define timerOpen_MIN                           0
#define timerOpen_MAX                           3600
/**@} */

/** Writable data points Boolean and enumerated variables occupy byte size */
#define COUNT_W_BIT 1







/** Event enumeration */
typedef enum
{
  WIFI_SOFTAP = 0x00,                               ///< WiFi SOFTAP configuration event
  WIFI_AIRLINK,                                     ///< WiFi module AIRLINK configuration event
  WIFI_STATION,                                     ///< WiFi module STATION configuration event
  WIFI_OPEN_BINDING,                                ///< The WiFi module opens the binding event
  WIFI_CLOSE_BINDING,                               ///< The WiFi module closes the binding event
  WIFI_CON_ROUTER,                                  ///< The WiFi module is connected to a routing event
  WIFI_DISCON_ROUTER,                               ///< The WiFi module has been disconnected from the routing event
  WIFI_CON_M2M,                                     ///< The WiFi module has a server M2M event
  WIFI_DISCON_M2M,                                  ///< The WiFi module has been disconnected from the server M2M event
  WIFI_OPEN_TESTMODE,                               ///< The WiFi module turns on the test mode event
  WIFI_CLOSE_TESTMODE,                              ///< The WiFi module turns off the test mode event
  WIFI_CON_APP,                                     ///< The WiFi module connects to the APP event
  WIFI_DISCON_APP,                                  ///< The WiFi module disconnects the APP event
  WIFI_RSSI,                                        ///< WiFi module RSSI event
  WIFI_NTP,                                         ///< Network time event
  MODULE_INFO,                                      ///< Module information event
  TRANSPARENT_DATA,                                 ///< Transparency events
  EVENT_lightOnOff,
  EVENT_isTimerOpen,
  EVENT_timerOpen,
  EVENT_TYPE_MAX                                    ///< Enumerate the number of members to calculate (user accidentally deleted)
} EVENT_TYPE_T;

/** P0 Command code */
typedef enum
{
    ACTION_CONTROL_DEVICE       = 0x01,             ///< Protocol 4.10 WiFi Module Control Device WiFi Module Send
    ACTION_READ_DEV_STATUS      = 0x02,             ///< Protocol 4.8 WiFi Module Reads the current status of the device WiFi module sent
    ACTION_READ_DEV_STATUS_ACK  = 0x03,             ///< Protocol 4.8 WiFi Module Read Device Current Status Device MCU Reply
    ACTION_REPORT_DEV_STATUS    = 0x04,             ///< Protocol 4.9 device MCU to the WiFi module to actively report the current status of the device to send the MCU
    ACTION_W2D_TRANSPARENT_DATA = 0x05,             ///< WiFi to device MCU transparent
    ACTION_D2W_TRANSPARENT_DATA = 0x06,             ///< Device MCU to WiFi
} actionType_t;   

/** Protocol network time structure */
typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint32_t ntp;
}protocolTime_t;
      

/** WiFi module configuration parameters */
typedef enum
{
  WIFI_RESET_MODE = 0x00,                           ///< WIFI module reset
  WIFI_SOFTAP_MODE,                                 ///< WIFI module softAP mode
  WIFI_AIRLINK_MODE,                                ///< WIFI module AirLink mode
  WIFI_PRODUCTION_TEST,                             ///< The MCU requests the WiFi module to enter the production mode
  WIFI_NINABLE_MODE,                                ///< The MCU requests the module to enter binding mode
}WIFI_MODE_TYPE_T;

/** WiFi module working status */
typedef union
{
    uint16_t value;
    struct
    {
        uint16_t            softap:1;               ///< Indicates the state of the SOFTAP mode in which the WiFi module is located, with the type bool
        uint16_t            station:1;              ///< Indicates the STATION mode status of the WiFi module, type bool
        uint16_t            onboarding:1;           ///< Indicates the configuration status of the WiFi module, type bool
        uint16_t            binding:1;              ///< Indicates the binding status of the WiFi module, type bool
        uint16_t            con_route:1;            ///< Indicates the connection status of the WiFi module to the router, type bool
        uint16_t            con_m2m:1;              ///< Indicates the state of the WiFi module and the cloud m2m, type bool
        uint16_t            reserve1:2;             ///< Data bits are filled
        uint16_t            rssi:3;                 ///< Indicates the signal strength of the route, the type is a value
        uint16_t            app:1;                  ///< Indicates the connection status of the WiFi module to the APP side, type bool
        uint16_t            test:1;                 ///< Indicates the state of the field measurement mode in which the WiFi module is located, with the type bool
        uint16_t            reserve2:3;             ///< Data bits are filled
    }types;
} wifi_status_t;

#pragma pack(1)

/** User Area Device State Structure */
typedef struct {
  bool valuelightOnOff;
  bool valueisTimerOpen;
  uint32_t valuetimerOpen;
} dataPoint_t;


/** Corresponding to the protocol "4.10 WiFi module control device" in the flag " attr_flags" */ 
typedef struct {
  uint8_t flaglightOnOff:1;
  uint8_t flagisTimerOpen:1;
  uint8_t flagtimerOpen:1;
} attrFlags_t;


/** Corresponding protocol "4.10 WiFi module control device" in the data value "attr_vals" */

typedef struct {
  uint8_t wBitBuf[COUNT_W_BIT];
  uint16_t valuetimerOpen;
} attrVals_t;

/** The flag "attr_flags (1B)" + data value "P0 protocol area" in the corresponding protocol "4.10 WiFi module control device"attr_vals(6B)" */ 
typedef struct {
    attrFlags_t attrFlags;
    attrVals_t  attrVals;
}gizwitsIssued_t;

/** Corresponding protocol "4.9 Device MCU to the WiFi module to actively report the current state" in the device status "dev_status(11B)" */ 

typedef struct {
  uint8_t wBitBuf[COUNT_W_BIT];
  uint16_t valuetimerOpen;
} devStatus_t; 

/** Wifi signal strength numerical structure */
typedef struct {
    uint8_t rssi;                                   ///< WIFI signal strength
}moduleStatusInfo_t;


                                                                         
/** Event queue structure */                               
typedef struct {                           
    uint8_t num;                                    ///< Number of queue members
    uint8_t event[EVENT_TYPE_MAX];                  ///< Queue member event content
}eventInfo_t;                                


/** Corresponding protocol "4.9 Device status bit in the active status of the device MCU to the WiFi module" dev_status (11B) " */
typedef struct 
{   
    uint8_t action;                        
    devStatus_t devStatus;                          ///< Function: Used to store the device status data to be reported
}gizwitsReport_t;

/** Protocol processing related structure */
typedef struct
{
    uint8_t mac[MAC_LEN_8266];
    uint8_t connectM2MFlag;
    uint32_t timerMsCount;                          ///< Time Count (Ms)
    uint8_t transparentBuff[BUFFER_LEN_MAX];        ///< Transparent data storage area
    uint32_t transparentLen;                        ///< Transparent data length
    dataPoint_t gizCurrentDataPoint;                ///< Current device status data
    dataPoint_t gizLastDataPoint;                   ///< The last reported device status data
    gizwitsReport_t reportData;                     ///< Protocol to report actual data
    eventInfo_t issuedProcessEvent;                 ///< Control events
}gizwitsProtocol_t;

#pragma pack()

/**@name User API interface
* @{
*/

void setConnectM2MStatus(uint8_t m2mFlag);
uint8_t getConnectM2MStatus(void);
void gizwitsSetMode(uint8_t mode);
uint32_t gizwitsGetTimeStamp(void);
protocolTime_t gizwitsGetNTPTime(void);
void gizwitsInit(void);
int8_t gizwitsHandle(dataPoint_t *dataPoint);
int32_t gizwitsPassthroughData(uint8_t * data, uint32_t len);
int32_t gizwitsUploadData(uint8_t *did,uint8_t *data, uint32_t dataLen);

/**@} */

#endif