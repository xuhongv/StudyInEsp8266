/**
************************************************************
* @file         dataPointTools.h
* @brief        Data point processing tool set
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
#ifndef _DATAPOINT_TOOLS_H_
#define _DATAPOINT_TOOLS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int8_t gizByteOrderExchange(uint8_t *buf,uint32_t dataLen);
uint32_t gizY2X(uint32_t ratio, int32_t addition, int32_t preValue);
int32_t gizX2Y(uint32_t ratio, int32_t addition, uint32_t preValue);
uint32_t gizY2XFloat(float ratio, float addition, float preValue);
float gizX2YFloat(float ratio, float addition, uint32_t preValue);
uint8_t gizAcrossByteJudge(uint32_t bitOffset,uint32_t bitLen);
int32_t  gizVarlenCompressValue(uint32_t bitOffset,uint32_t bitLen,uint8_t *bufAddr,uint32_t srcData);
int32_t gizVarlenDecompressionValue(uint32_t bitOffset,uint32_t bitLen,uint8_t *arrayAddr,uint32_t arrayLen);
int32_t gizStandardDecompressionValue(uint32_t byteOffset,uint32_t bitOffset,uint32_t bitLen,uint8_t *arrayAddr,uint32_t arrayLen);
int32_t gizStandardCompressValue(uint32_t byteOffset,uint32_t bitOffset,uint32_t bitLen,uint8_t *bufAddr,uint32_t srcData);

#endif
