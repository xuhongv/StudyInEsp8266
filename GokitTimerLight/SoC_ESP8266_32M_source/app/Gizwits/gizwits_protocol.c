/**
************************************************************
* @file         gizwits_protocol.c
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

#include "mem.h"
#include "gagent_soc.h"
#include "gizwits_product.h"

#include "dataPointTools.h"

/** Protocol global variable **/
gizwitsProtocol_t gizwitsProtocol;

/**@name Gagent module related system task parameters
* @{
*/
#define TaskQueueLen    200                                                 ///< Total length of message queue
LOCAL  os_event_t   TaskQueue[TaskQueueLen];                                ///< message queue
/**@} */

/** System task related parameters */
#define gizwitsTaskQueueLen    200                              ///< Protocol message queue
LOCAL os_event_t gizwitsTaskQueue[gizwitsTaskQueueLen];         ///< Protocol message queue length

/** Timer related parameters */ 
LOCAL os_timer_t gizTimer; 

void ICACHE_FLASH_ATTR setConnectM2MStatus(uint8_t m2mFlag)
{
    gizwitsProtocol.connectM2MFlag = m2mFlag; 
    
    GIZWITS_LOG("setConnectM2MStatus = %d ###$$$###\n", gizwitsProtocol.connectM2MFlag);
}

uint8_t ICACHE_FLASH_ATTR getConnectM2MStatus(void)
{
    GIZWITS_LOG("getConnectM2MStatus = %d ###$$$###\n",gizwitsProtocol.connectM2MFlag);
    
    return gizwitsProtocol.connectM2MFlag; 
}

void ICACHE_FLASH_ATTR uploadDataCBFunc(int32_t result, void *arg,uint8_t *pszDID)
{
    GIZWITS_LOG("[uploadDataCBFunc]: result %d \n", result);
}

/**
* @brief Read system time millisecond count

* @param none
* @return System time milliseconds
*/
uint32_t ICACHE_FLASH_ATTR gizGetTimerCount(void)
{
    return gizwitsProtocol.timerMsCount;
}

/**
* @brief generates "controlled events" according to protocol

* @param [in] issuedData: Controlled data
* @param [out] info: event queue
* @param [out] dataPoints: data point data
* @return 0, the implementation of success, non-0, failed
*/
static int8_t ICACHE_FLASH_ATTR gizDataPoint2Event(gizwitsIssued_t *issuedData, eventInfo_t *info, dataPoint_t *dataPoints)
{
    if((NULL == issuedData) || (NULL == info) ||(NULL == dataPoints))
    {
        GIZWITS_LOG("gizDataPoint2Event Error , Illegal Param\n");
        return -1;
    }
    
    /** Greater than 1 byte to do bit conversion **/
    if(sizeof(issuedData->attrFlags) > 1)
    {
        if(-1 == gizByteOrderExchange((uint8_t *)&issuedData->attrFlags,sizeof(attrFlags_t)))
        {
            GIZWITS_LOG("gizByteOrderExchange Error\n");
            return -1;
        }
    }
    
    if(0x01 == issuedData->attrFlags.flaglightOnOff)
    {
        info->event[info->num] = EVENT_lightOnOff;
        info->num++;
        dataPoints->valuelightOnOff = gizStandardDecompressionValue(lightOnOff_BYTEOFFSET,lightOnOff_BITOFFSET,lightOnOff_LEN,(uint8_t *)&issuedData->attrVals.wBitBuf,sizeof(issuedData->attrVals.wBitBuf));
    }
        
    if(0x01 == issuedData->attrFlags.flagisTimerOpen)
    {
        info->event[info->num] = EVENT_isTimerOpen;
        info->num++;
        dataPoints->valueisTimerOpen = gizStandardDecompressionValue(isTimerOpen_BYTEOFFSET,isTimerOpen_BITOFFSET,isTimerOpen_LEN,(uint8_t *)&issuedData->attrVals.wBitBuf,sizeof(issuedData->attrVals.wBitBuf));
    }
        
        
    if(0x01 == issuedData->attrFlags.flagtimerOpen)
    {
        info->event[info->num] = EVENT_timerOpen;
        info->num++;
        dataPoints->valuetimerOpen = gizX2Y(timerOpen_RATIO,  timerOpen_ADDITION, exchangeBytes(issuedData->attrVals.valuetimerOpen));
    }
    
    return 0;
}

/**
* @brief contrasts the current data with the last data
*
* @param [in] cur: current data point data
* @param [in] last: last data point data
*
* @return: 0, no change in data; 1, data changes
*/
static int8_t ICACHE_FLASH_ATTR gizCheckReport(dataPoint_t *cur, dataPoint_t *last)
{
    int8_t ret = 0;
    static uint32_t lastReportTime = 0;

    if((NULL == cur) || (NULL == last))
    {
        GIZWITS_LOG("gizCheckReport Error , Illegal Param\n");
        return -1;
    }
    if(last->valuelightOnOff != cur->valuelightOnOff)
    {
        GIZWITS_LOG("valuelightOnOff Changed\n");
        ret = 1;
    }
    if(last->valueisTimerOpen != cur->valueisTimerOpen)
    {
        GIZWITS_LOG("valueisTimerOpen Changed\n");
        ret = 1;
    }
    if(last->valuetimerOpen != cur->valuetimerOpen)
    {
        GIZWITS_LOG("valuetimerOpen Changed\n");
        ret = 1;
    }


    return ret;
}

/**
* @brief User data point data is converted to wit the cloud to report data point data
*
* @param [in] dataPoints: user data point data address
* @param [out] devStatusPtr: wit the cloud data point data address
*
* @return 0, the correct return; -1, the error returned
*/
static int8_t ICACHE_FLASH_ATTR gizDataPoints2ReportData(dataPoint_t *dataPoints , devStatus_t *devStatusPtr)
{
    if((NULL == dataPoints) || (NULL == devStatusPtr))
    {
        GIZWITS_LOG("gizDataPoints2ReportData Error , Illegal Param\n");
        return -1;
    }

    gizMemset((uint8_t *)devStatusPtr->wBitBuf,0,sizeof(devStatusPtr->wBitBuf));

    gizStandardCompressValue(lightOnOff_BYTEOFFSET,lightOnOff_BITOFFSET,lightOnOff_LEN,(uint8_t *)devStatusPtr,dataPoints->valuelightOnOff);
    gizStandardCompressValue(isTimerOpen_BYTEOFFSET,isTimerOpen_BITOFFSET,isTimerOpen_LEN,(uint8_t *)devStatusPtr,dataPoints->valueisTimerOpen);
    gizByteOrderExchange((uint8_t *)devStatusPtr->wBitBuf,sizeof(devStatusPtr->wBitBuf));


    devStatusPtr->valuetimerOpen = exchangeBytes(gizY2X(timerOpen_RATIO,  timerOpen_ADDITION, dataPoints->valuetimerOpen)); 



    return 0;
}

/**
* @brief In this function is called by the Gagent module, when the connection status changes will be passed to the corresponding value
* @param [in] value WiFi connection status value
* @return none
*/
void ICACHE_FLASH_ATTR gizWiFiStatus(uint16_t value)
{
    uint8_t rssiValue = 0;
    wifi_status_t status;
    static wifi_status_t lastStatus;

    if(0 != value)
    {
        status.value = exchangeBytes(value); 

        GIZWITS_LOG("@@@@ GAgentStatus[hex]:%02x | [Bin]:%d,%d,%d,%d,%d,%d \r\n", status.value, status.types.con_m2m, status.types.con_route, status.types.binding, status.types.onboarding, status.types.station, status.types.softap);

        //OnBoarding mode status
        if(1 == status.types.onboarding)
        {
            if(1 == status.types.softap)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_SOFTAP;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("OnBoarding: SoftAP or Web mode\r\n");
            }
            else
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_AIRLINK;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("OnBoarding: AirLink mode\r\n");
            }
        }
        else
        {
            if(1 == status.types.softap)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_SOFTAP;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("OnBoarding: SoftAP or Web mode\r\n");
            }

            if(1 == status.types.station)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_STATION;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("OnBoarding: Station mode\r\n");
            }
        }

        //binding mode status
        if(lastStatus.types.binding != status.types.binding)
        {
            lastStatus.types.binding = status.types.binding;
            if(1 == status.types.binding)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_OPEN_BINDING;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("WiFi status: in binding mode\r\n");
            }
            else
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_CLOSE_BINDING;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("WiFi status: out binding mode\r\n");
            }
        }

        //router status
        if(lastStatus.types.con_route != status.types.con_route)
        {
            lastStatus.types.con_route = status.types.con_route;
            if(1 == status.types.con_route)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_CON_ROUTER;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("WiFi status: connected router\r\n");
            }
            else
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_DISCON_ROUTER;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("WiFi status: disconnected router\r\n");
            }
        }

        //M2M server status
        if(lastStatus.types.con_m2m != status.types.con_m2m)
        {
            lastStatus.types.con_m2m = status.types.con_m2m;
            if(1 == status.types.con_m2m)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_CON_M2M;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("WiFi status: connected m2m\r\n");
            }
            else
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_DISCON_M2M;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("WiFi status: disconnected m2m\r\n");
            }
        }

        //APP status
        if(lastStatus.types.app != status.types.app)
        {
            lastStatus.types.app = status.types.app;
            if(1 == status.types.app)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_CON_APP;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("WiFi status: app connect\r\n");
            }
            else
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_DISCON_APP;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("WiFi status: no app connect\r\n");
            }
        }

        //test mode status
        if(lastStatus.types.test != status.types.test)
        {
            lastStatus.types.test = status.types.test;
            if(1 == status.types.test)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_OPEN_TESTMODE;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("WiFi status: in test mode\r\n");
            }
            else
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_CLOSE_TESTMODE;
                gizwitsProtocol.issuedProcessEvent.num++;
                GIZWITS_LOG("WiFi status: out test mode\r\n");
            }
        }

        rssiValue = status.types.rssi;
        gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_RSSI;
        gizwitsProtocol.issuedProcessEvent.num++;

        gizwitsEventProcess(&gizwitsProtocol.issuedProcessEvent, (uint8_t *)&rssiValue, sizeof(rssiValue));
        gizMemset((uint8_t *)&gizwitsProtocol.issuedProcessEvent, 0, sizeof(eventInfo_t));

        lastStatus = status;
    }
}

/**
* @brief This function is called by the Gagent module to receive the relevant protocol data from the cloud or APP
* @param [in] inData The protocol data entered
* @param [in] inLen Enter the length of the data
* @param [out] outData The output of the protocol data
* @param [out] outLen The length of the output data
* @return 0, the implementation of success, non-0, failed
*/
int32_t ICACHE_FLASH_ATTR gizIssuedProcess(uint8_t *didPtr, uint8_t *inData, uint32_t inLen,uint8_t *outData,int32_t *outLen)
{
	uint8_t i = 0;


    if((NULL == inData) || (NULL == outData) || (NULL == outLen)) 
    {
        GIZWITS_LOG("!!! IssuedProcess Error \n"); 
        return -1;
    }

    if(NULL == didPtr)
    {
        GIZWITS_LOG("~~~gizIssuedProcess: did is NULL .\n");
    }
    else
    {
        GIZWITS_LOG("~~~gizIssuedProcess: did %s\n", didPtr);
    }

    GIZWITS_LOG("%s: ", "~~~issued data"); 
    //printf_bufs((uint8_t *)inData,inLen);

    if(NULL == didPtr) 
    { 
        switch(inData[0]) 
        {
            case ACTION_CONTROL_DEVICE:
            	gizDataPoint2Event((gizwitsIssued_t *)&inData[1], &gizwitsProtocol.issuedProcessEvent,&gizwitsProtocol.gizCurrentDataPoint);           
         
                system_os_post(USER_TASK_PRIO_2, SIG_ISSUED_DATA, 0); 
                *outLen = 0; 
                break;
                
            case ACTION_READ_DEV_STATUS:
                gizDataPoints2ReportData(&gizwitsProtocol.gizLastDataPoint,&gizwitsProtocol.reportData.devStatus);
                gizwitsProtocol.reportData.action = ACTION_READ_DEV_STATUS_ACK;
                gizMemcpy(outData, (uint8_t *)&gizwitsProtocol.reportData, sizeof(gizwitsReport_t));
                *outLen = sizeof(gizwitsReport_t);
                
                GIZWITS_LOG("%s: ", "~~~ReadReport \n");
                //printf_bufs((uint8_t *)outData,*outLen);
                break;
                
            case ACTION_W2D_TRANSPARENT_DATA: 
                gizMemcpy(gizwitsProtocol.transparentBuff, &inData[1], inLen-1);
                gizwitsProtocol.transparentLen = inLen-1;

                system_os_post(USER_TASK_PRIO_2, SIG_PASSTHROUGH, 0);
                *outLen = 0;

                break;
                
            default:
 
                break;
        }
    }
    else
    { 
        GIZWITS_LOG(" Error : didPtr  \n");
    }

    return 0;
}

/**
* @brief Gizwits protocol related system event callback function
* @param [in] events Event data pointer
* @return none
* @note in the function to complete the Gizwits protocol related to the handling of system events
*/
void ICACHE_FLASH_ATTR gizwitsTask(os_event_t * events)
{
    uint8_t i = 0;
    uint8_t vchar = 0;

    if(NULL == events)
    {
        GIZWITS_LOG("!!! gizwitsTask Error \n");
    }

    vchar = (uint8)(events->par);

    switch(events->sig)
    {
    case SIG_ISSUED_DATA:
        GIZWITS_LOG("[SIG] :SIG_ISSUED_DATA \n");
        gizwitsEventProcess(&gizwitsProtocol.issuedProcessEvent, (uint8_t *)&gizwitsProtocol.gizCurrentDataPoint, sizeof(dataPoint_t));
        //clean event info
        gizMemset((uint8_t *)&gizwitsProtocol.issuedProcessEvent, 0, sizeof(eventInfo_t));
        break;
    case SIG_PASSTHROUGH:
        GIZWITS_LOG("[SIG] :SIG_PASSTHROUGH \n");
        gizwitsProtocol.issuedProcessEvent.event[0] = TRANSPARENT_DATA;
        gizwitsProtocol.issuedProcessEvent.num = 1;
        gizwitsEventProcess(&gizwitsProtocol.issuedProcessEvent, (uint8_t *)gizwitsProtocol.transparentBuff, gizwitsProtocol.transparentLen);

        gizMemset((uint8_t *)&gizwitsProtocol.issuedProcessEvent, 0, sizeof(eventInfo_t));
        gizMemset((uint8_t *)gizwitsProtocol.transparentBuff, 0, BUFFER_LEN_MAX);
        gizwitsProtocol.transparentLen = 0;
        break;
    case SIG_IMM_REPORT:
        GIZWITS_LOG("[SIG] :SIG_IMM_REPORT \n");
        gizDataPoints2ReportData(&gizwitsProtocol.gizLastDataPoint, (devStatus_t *)&gizwitsProtocol.reportData.devStatus);
        gizwitsProtocol.reportData.action = ACTION_REPORT_DEV_STATUS;
        gagentUploadData(NULL, (uint8_t *)&gizwitsProtocol.reportData, sizeof(gizwitsReport_t),getConnectM2MStatus(),gizwitsProtocol.mac, uploadDataCBFunc);
 
        GIZWITS_LOG("[SIG] :reportData: \n"); 
        //printf_bufs((uint8_t *)&gizwitsProtocol.reportData, sizeof(gizwitsReport_t)); 
        break;
 
    case SIG_UPGRADE_DATA:
        gizwitsHandle((dataPoint_t *)&currentDataPoint);
        break;
    default:
        GIZWITS_LOG("---error sig! ---\n");
        break;
    }
}

 

/**
* @brief gizwits data point update report processing

* The user calls the interface to complete the device data changes reported

* @param [in] dataPoint User device data points
* @return none
*/
int8_t ICACHE_FLASH_ATTR gizwitsHandle(dataPoint_t *dataPoint)
{
    if(NULL == dataPoint)
    {
        GIZWITS_LOG("!!! gizReportData Error \n");

        return (-1);
    }

    //Regularly report conditional
    if((1 == gizCheckReport(dataPoint, (dataPoint_t *)&gizwitsProtocol.gizLastDataPoint)))
    {
        gizDataPoints2ReportData(dataPoint, &gizwitsProtocol.reportData.devStatus);

        gizwitsProtocol.reportData.action = ACTION_REPORT_DEV_STATUS;

        gagentUploadData(NULL, (uint8_t *)&gizwitsProtocol.reportData, sizeof(gizwitsReport_t),getConnectM2MStatus(),gizwitsProtocol.mac, uploadDataCBFunc);
 

        GIZWITS_LOG("~~~reportData \n"); 
        //printf_bufs((uint8_t *)&gizwitsProtocol.reportData, sizeof(gizwitsReport_t));

        gizMemcpy((uint8_t *)&gizwitsProtocol.gizLastDataPoint, (uint8_t *)dataPoint, sizeof(dataPoint_t));
    }

    return 0;
}
/**@} */



/**
* @brief Timer callback function, in the function to complete 10 minutes of regular reporting
* @param none
* @return none
*/
void ICACHE_FLASH_ATTR gizTimerFunc(void)
{
    static uint32_t repCtime = 0;

    //600s Regularly report
    if(TIM_REP_TIMOUT < repCtime)
    {
        repCtime = 0;

        GIZWITS_LOG("@@@@ gokit_timing report! \n");

        system_os_post(USER_TASK_PRIO_2, SIG_IMM_REPORT, 0);
    }
    gizwitsProtocol.timerMsCount++;
    repCtime++;
}

/**@name User API interface
* @{
*/

/**
* @brief WiFi configuration interface

*  The user can call the interface to bring the WiFi module into the corresponding configuration mode or reset the module

* @param[in] mode: 0x0, module reset; 0x01, SoftAp mode; 0x02, AirLink mode; 0x03, module into production test; 0x04: allow users to bind equipment
* @return Error command code
*/
void ICACHE_FLASH_ATTR gizwitsSetMode(uint8_t mode)
{
    switch(mode)
    {
        case WIFI_RESET_MODE:
            gagentReset();
            break;
        case WIFI_SOFTAP_MODE:
        case WIFI_AIRLINK_MODE:
        case WIFI_PRODUCTION_TEST:
            gagentConfig(mode);
            break;
        case WIFI_NINABLE_MODE:
            GAgentEnableBind();
            break;
        default :
            break;
    }
}

/**
* @brief Obtain a network timestamp interface
* @param none
* @return _tm
*/
protocolTime_t ICACHE_FLASH_ATTR gizwitsGetNTPTime(void)
{
    protocolTime_t tmValue;

    gagentGetNTP((_tm *)&tmValue);

    return tmValue;
}

/**
* @brief Obtain a network timestamp interface
* @param none
* @return ntp
*/
uint32_t ICACHE_FLASH_ATTR gizwitsGetTimeStamp(void)
{
    _tm tmValue;

    gagentGetNTP(&tmValue);

    return tmValue.ntp;
}

/**
* @brief gizwits protocol initialization interface

* The user can call the interface to complete the Gizwits protocol-related initialization (including protocol-related timer, serial port initial)

* The user can complete the initialization status of the data points in this interface

* @param none
* @return none
*/
void ICACHE_FLASH_ATTR gizwitsInit(void)
{
    int16_t value = 0;
    struct devAttrs attrs;
    gizMemset((uint8_t *)&gizwitsProtocol, 0, sizeof(gizwitsProtocol_t));

    if(false == wifi_get_macaddr(STATION_IF, gizwitsProtocol.mac))
    {
        GIZWITS_LOG("Failed to get mac addr \r\n");
    }
    
    system_os_task(gizwitsTask, USER_TASK_PRIO_2, gizwitsTaskQueue, gizwitsTaskQueueLen);

    //gokit timer start
    os_timer_disarm(&gizTimer);
    os_timer_setfn(&gizTimer, (os_timer_func_t *)gizTimerFunc, NULL);
    os_timer_arm(&gizTimer, MAX_SOC_TIMOUT, 1);


    //Gagent
    system_os_task(gagentProcessRun, USER_TASK_PRIO_1, TaskQueue, TaskQueueLen);

    attrs.mBindEnableTime = exchangeBytes(NINABLETIME);
    gizMemset((uint8_t *)attrs.mDevAttr, 0, 8);
    attrs.mDevAttr[7] |= DEV_IS_GATEWAY<<0;
    attrs.mDevAttr[7] |= (0x01<<1);
    gizMemcpy(attrs.mstrDevHV, HARDWARE_VERSION, os_strlen(HARDWARE_VERSION));
    gizMemcpy(attrs.mstrDevSV, SOFTWARE_VERSION, os_strlen(SOFTWARE_VERSION));
    gizMemcpy(attrs.mstrP0Ver, P0_VERSION, os_strlen(P0_VERSION));
    gizMemcpy(attrs.mstrProductKey, PRODUCT_KEY, os_strlen(PRODUCT_KEY));
    gizMemcpy(attrs.mstrPKSecret, PRODUCT_SECRET, os_strlen(PRODUCT_SECRET));
    gizMemcpy(attrs.mstrProtocolVer, PROTOCOL_VERSION, os_strlen(PROTOCOL_VERSION));
    gizMemcpy(attrs.mstrSdkVerLow, SDK_VERSION, os_strlen(SDK_VERSION));
    
    gagentInit(attrs);

    GIZWITS_LOG("gizwitsInit OK \r\n");
}

/**
* @brief gizwits report transparent transmission data interface

* The user can call the interface to complete the reporting of private protocol data

* @param [in] data entered private protocol data
* @param [in] len Enter the length of the private protocol data
* @return -1, error returned; 0, returned correctly
*/
int32_t ICACHE_FLASH_ATTR gizwitsPassthroughData(uint8_t * data, uint32_t len)
{
    uint8_t *passReportBuf = NULL;
    uint32_t passReportBufLen = len + sizeof(uint8_t);

    if(NULL == data)
    {
        GIZWITS_LOG("!!! gizReportData Error \n");

        return (-1);
    }

    passReportBuf = (uint8 * )gizMalloc(passReportBufLen);
    if(NULL == passReportBuf)
    {
        GIZWITS_LOG("Failed to malloc buffer\n");
        return (-1);
    }

    passReportBuf[0] = ACTION_D2W_TRANSPARENT_DATA;
    gizMemcpy((uint8_t *)&passReportBuf[1], data, len);

    gagentUploadData(NULL,passReportBuf, passReportBufLen,getConnectM2MStatus(),gizwitsProtocol.mac, uploadDataCBFunc);
 

    gizFree(passReportBuf);
    passReportBuf = NULL;

    return 0;
}
/**@} */