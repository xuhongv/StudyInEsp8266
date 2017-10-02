/**
************************************************************
* @file         gizwits_product.h
* @brief        Corresponding gizwits_product.c header file (including product hardware and software version definition)
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
#ifndef _GIZWITS_PRODUCT_H_
#define _GIZWITS_PRODUCT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "gizwits_protocol.h"

/**
* MCU software version number
*/
#define SOFTWARE_VERSION                        "03030000"

/**
* MCU hardware version number
*/
#define HARDWARE_VERSION                        "03000001"

/**
* Gagent minor version number for OTA upgrade
* OTA hardware version number: 00ESP826
* OTA software version number: 040206xx // "xx" is version number defaults to "25", consistent with the Gagent library version
*/
#define SDK_VERSION                             "25"    

#ifndef SOFTWARE_VERSION
    #error "no define SOFTWARE_VERSION"
#endif

#ifndef HARDWARE_VERSION
    #error "no define HARDWARE_VERSION"
#endif


/** User area The current device state structure */
extern dataPoint_t currentDataPoint;

/** User area data point initialization */
void userInit(void);

/** User area device status detection */
void userHandle(void);

/** User zone event handling */
int8_t gizwitsEventProcess(eventInfo_t * info, uint8_t * data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
