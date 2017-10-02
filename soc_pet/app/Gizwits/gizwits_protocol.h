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
#define PRODUCT_KEY "de9e8d18d9394cce9081b25a531e552b"
/**@} */
/**@name Product Secret (Product key)
* @{
*/
#define PRODUCT_SECRET "b6642a6a5a784c898286d3edf77f99e0"

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
#define LED_OnOff_BYTEOFFSET                    0
#define LED_OnOff_BITOFFSET                     0
#define LED_OnOff_LEN                           1
#define LED_Color_BYTEOFFSET                    0
#define LED_Color_BITOFFSET                     1
#define LED_Color_LEN                           2
#define Infrared_BYTEOFFSET                    5
#define Infrared_BITOFFSET                     0
#define Infrared_LEN                           1
#define Alert_1_BYTEOFFSET                    8
#define Alert_1_BITOFFSET                     0
#define Alert_1_LEN                           1
#define Alert_2_BYTEOFFSET                    8
#define Alert_2_BITOFFSET                     1
#define Alert_2_LEN                           1
#define Fault_LED_BYTEOFFSET                    9
#define Fault_LED_BITOFFSET                     0
#define Fault_LED_LEN                           1
#define Fault_Motor_BYTEOFFSET                    9
#define Fault_Motor_BITOFFSET                     1
#define Fault_Motor_LEN                           1
#define Fault_TemHum_BYTEOFFSET                    9
#define Fault_TemHum_BITOFFSET                     2
#define Fault_TemHum_LEN                           1
#define Fault_IR_BYTEOFFSET                    9
#define Fault_IR_BITOFFSET                     3
#define Fault_IR_LEN                           1

#define LED_R_RATIO                         1
#define LED_R_ADDITION                      0
#define LED_R_MIN                           0
#define LED_R_MAX                           254
#define LED_G_RATIO                         1
#define LED_G_ADDITION                      0
#define LED_G_MIN                           0
#define LED_G_MAX                           254
#define LED_B_RATIO                         1
#define LED_B_ADDITION                      0
#define LED_B_MIN                           0
#define LED_B_MAX                           254
#define Motor_Speed_RATIO                         1
#define Motor_Speed_ADDITION                      -5
#define Motor_Speed_MIN                           0
#define Motor_Speed_MAX                           10
#define Temperature_RATIO                         1
#define Temperature_ADDITION                      -13
#define Temperature_MIN                           0
#define Temperature_MAX                           200
#define Humidity_RATIO                         1
#define Humidity_ADDITION                      0
#define Humidity_MIN                           0
#define Humidity_MAX                           100
/**@} */

/** Writable data points Boolean and enumerated variables occupy byte size */
#define COUNT_W_BIT 1

/** Read-only data points Boolean and enumerated variables occupy byte size */
#define COUNT_R_BIT 1


/** Read-only data points Boolean and enumerated variables occupy byte size */
#define COUNT_ALERT_BIT 1


/** Read-only data points Boolean and enumerated variables occupy byte size */
#define COUNT_FAULT_BIT 1

typedef enum
{
    LED_Color_VALUE0 = 0,//自定义
    LED_Color_VALUE1 = 1,//黄色
    LED_Color_VALUE2 = 2,//紫色
    LED_Color_VALUE3 = 3,//粉色
    LED_Color_VALUE_MAX,
} LED_Color_ENUM_T;

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
  EVENT_LED_OnOff,
  EVENT_LED_Color,
  EVENT_LED_R,
  EVENT_LED_G,
  EVENT_LED_B,
  EVENT_Motor_Speed,
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
  bool valueLED_OnOff;
  uint32_t valueLED_Color;
  uint32_t valueLED_R;
  uint32_t valueLED_G;
  uint32_t valueLED_B;
  int32_t valueMotor_Speed;
  bool valueInfrared;
  int32_t valueTemperature;
  uint32_t valueHumidity;
  bool valueAlert_1;
  bool valueAlert_2;
  bool valueFault_LED;
  bool valueFault_Motor;
  bool valueFault_TemHum;
  bool valueFault_IR;
} dataPoint_t;


/** Corresponding to the protocol "4.10 WiFi module control device" in the flag " attr_flags" */ 
typedef struct {
  uint8_t flagLED_OnOff:1;
  uint8_t flagLED_Color:1;
  uint8_t flagLED_R:1;
  uint8_t flagLED_G:1;
  uint8_t flagLED_B:1;
  uint8_t flagMotor_Speed:1;
} attrFlags_t;


/** Corresponding protocol "4.10 WiFi module control device" in the data value "attr_vals" */

typedef struct {
  uint8_t wBitBuf[COUNT_W_BIT];
  uint8_t valueLED_R;
  uint8_t valueLED_G;
  uint8_t valueLED_B;
  uint8_t valueMotor_Speed;
} attrVals_t;

/** The flag "attr_flags (1B)" + data value "P0 protocol area" in the corresponding protocol "4.10 WiFi module control device"attr_vals(6B)" */ 
typedef struct {
    attrFlags_t attrFlags;
    attrVals_t  attrVals;
}gizwitsIssued_t;

/** Corresponding protocol "4.9 Device MCU to the WiFi module to actively report the current state" in the device status "dev_status(11B)" */ 

typedef struct {
  uint8_t wBitBuf[COUNT_W_BIT];
  uint8_t valueLED_R;
  uint8_t valueLED_G;
  uint8_t valueLED_B;
  uint8_t valueMotor_Speed;
  uint8_t rBitBuf[COUNT_R_BIT];
  uint8_t valueTemperature;
  uint8_t valueHumidity;
  uint8_t alertBitBuf[COUNT_ALERT_BIT];
  uint8_t faultBitBuf[COUNT_FAULT_BIT];
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