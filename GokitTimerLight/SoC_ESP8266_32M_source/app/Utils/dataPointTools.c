/**
************************************************************
* @file         dataPointTools.c
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
#include "dataPointTools.h"
#include "common.h"

/**
* @brief array buffer network byte order conversion
*
* @param [in] buf: buf address
* @param [in] dataLen: byte length
*
* @return   correct: 0
            Failed: -1
*/
int8_t ICACHE_FLASH_ATTR gizByteOrderExchange(uint8_t *buf,uint32_t dataLen)
{
    uint32_t i = 0;
    uint8_t preData = 0;
    uint8_t aftData = 0;

    if(NULL == buf)
    {
        return -1;
    }

    for(i = 0;i<dataLen/2;i++)
    {
        preData = buf[i];
        aftData = buf[dataLen - i - 1];
        buf[i] = aftData;
        buf[dataLen - i - 1] = preData;
    }
    return 0;
}


/**
* @brief is converted to the value of x in the protocol and the value of the actual communication transmission
*
* @param [in] ratio: correction coefficient k
* @param [in] addition: Increment m
* @param [in] preValue: The y value in the protocol, which is the display value of the App UI interface
*
* @return aft_value: The value of the actual communication as a value in the protocol
*/
uint32_t ICACHE_FLASH_ATTR gizY2X(uint32_t ratio, int32_t addition, int32_t preValue)
{
    uint32_t aftValue = 0;

    //x=(y - m)/k
    aftValue = ((preValue - addition) / ratio);

    return aftValue;
}

/**
* @brief into the y value of the agreement and App UI interface display value
*
* @param [in] ratio: correction coefficient k
* @param [in] addition: Increment m
* @param [in] preValue: The value of the actual communication as an x value in the protocol
*
* @return aftValue: as the y value of the agreement, App UI interface is the display value
*/
int32_t ICACHE_FLASH_ATTR gizX2Y(uint32_t ratio, int32_t addition, uint32_t preValue)
{
    int32_t aftValue = 0;

    //y=k * x + m
    aftValue = (preValue * ratio + addition);

    return aftValue;
}

/**
* @brief into the value of the agreement and the actual value of the communication, only for floating-point data to do
*
* @param [in] ratio: correction coefficient k
* @param [in] addition: Increment m
* @param [in] preValue: The y value in the protocol, which is the display value of the App UI interface
*
* @return aft_value: The value of the actual communication as a value in the protocol
*/
uint32_t ICACHE_FLASH_ATTR gizY2XFloat(float ratio, float addition, float preValue)
{
    uint32_t aftValue = 0;

    //x=(y - m)/k
    aftValue = ((preValue - addition) / ratio);

    return aftValue;
}

/**
* @brief into the y value of the agreement and App UI interface display value, only for the floating-point data to do
*
* @param [in] ratio: correction coefficient k
* @param [in] addition: Increment m
* @param [in] preValue: The value of the actual communication as an x value in the protocol
*
* @return: as the y value of the agreement, App UI interface is the display value
*/
float ICACHE_FLASH_ATTR gizX2YFloat(float ratio, float addition, uint32_t preValue)
{
    float aftValue = 0;

    //y=k * x + m
    aftValue = (preValue * ratio + addition);

    return aftValue;
}

/**
* @brief data points cross-byte judgment
*
* @param [in] bitOffset: Bit offset
* @param [in] bitLen: occupies the bit length
*
* @return does not cross bytes: 0
             Cross byte: 1
*/
uint8_t ICACHE_FLASH_ATTR gizAcrossByteJudge(uint32_t bitOffset,uint32_t bitLen)
{
    if((0 == bitOffset)||(0 == bitOffset%8))
    {
        if(bitLen <= 8)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        if(8 - bitOffset%8 >= bitLen)
        {
            return 0;
        }
        else
        {
            return 1; 
        }
    }
}

/**
* @brief bool and enum type data point data compression
*
* @param [in] byteOffset: byte offset
* @param [in] bitOffset: Bit offset
* @param [in] bitLen: occupies the bit length
* @param [out] arrayAddr: array address
* @param [in] srcData: raw data
*
* @return: 0, the correct return; -1, the error returned
*/
int32_t ICACHE_FLASH_ATTR gizVarlenCompressValue(uint32_t bitOffset,uint32_t bitLen,uint8_t *bufAddr,uint32_t srcData)
{
    uint8_t highBit = 0;
    uint8_t lowBit = 0;
    uint8_t ret = 0;
    uint32_t byteOffset = 0;
    if(NULL == bufAddr)
    {
        return -1;
    }       
    /* ??bitOffset?? byteOffset*/
    byteOffset = bitOffset/8;
    ret = gizAcrossByteJudge(bitOffset,bitLen);
    if(0 == ret)
    {
        bufAddr[byteOffset] |= (((uint8_t)srcData)<<(bitOffset%8));
    }
    else if(1 == ret)
    {
        /* ????????????? */ 
        highBit = ((uint8_t)srcData)>>(8-bitOffset%8);
        lowBit = (uint8_t)srcData & (0xFF >> (8-bitOffset%8));
        bufAddr[byteOffset + 1] |=  highBit;
        bufAddr[byteOffset] |= (lowBit<<(bitOffset%8));
    }

    return 0;
}

/**
* @brief bool and enum type data point data decompression
*
* @param [in] byteOffset: byte offset
* @param [in] bitOffset: Bit offset
* @param [in] bitLen: occupies the bit length
* @param [in] arrayAddr: array address
* @param [in] arrayLen: array length
*
* @return destValue: decompression after the results of data, -1 said the error returned
*/
int32_t ICACHE_FLASH_ATTR gizVarlenDecompressionValue(uint32_t bitOffset, uint32_t bitLen, uint8_t * arrayAddr, uint32_t arrayLen)
{
    int32_t destValue = 0;
    uint8_t ret = 0;
    uint32_t byteOffset = 0;
    uint8_t highBit = 0 ,lowBit = 0;
    uint8_t destBufTemp[50];//TERRY WARNING
    
    
    if(NULL == arrayAddr || 0 == arrayLen)
    {
        return -1;
    }
    /* ??bitOffset?? byteOffset*/
    byteOffset = bitOffset/8;
    memcpy(destBufTemp,arrayAddr,arrayLen);
    if(arrayLen > 1)// Judge Byte order conversion
    {
        if(-1 == gizByteOrderExchange(destBufTemp,arrayLen))
        {
            return -1;
        }
    }
    ret = gizAcrossByteJudge(bitOffset,bitLen);//Judge Byte Step
    if(0 == ret)
    {
        destValue |= ((destBufTemp[byteOffset] >> (bitOffset%8)) & (0xff >> (8 - bitLen)));
    }
    else if(1 == ret)
    {
        /* ????????2?? */
        highBit = destBufTemp[byteOffset + 1]& (0xFF >> (8-(bitLen-(8-bitOffset%8))));
        lowBit = destBufTemp[byteOffset]>> (bitOffset%8);
        destValue |=  (highBit << (8-bitOffset%8));
        destValue |= lowBit;
    }
    return destValue;
}

/**
* @brief bool and enum type data point data decompression
*
* @param [in] byteOffset: byte offset
* @param [in] bitOffset: Bit offset
* @param [in] bitLen: occupies the bit length
* @param [in] arrayAddr: array address
* @param [in] arrayLen: array length
*
* @return destValue: decompression after the results of data, -1 said the error returned
*/
int32_t ICACHE_FLASH_ATTR gizStandardDecompressionValue(uint32_t byteOffset,uint32_t bitOffset,uint32_t bitLen,uint8_t *arrayAddr,uint32_t arrayLen)
{
    uint8_t ret = 0;
    uint8_t highBit = 0 ,lowBit = 0;
    uint8_t destBufTemp[50];
    int32_t destValue = 0;
    
    if(NULL == arrayAddr || 0 == arrayLen)
    {
        return -1;
    }

    memcpy(destBufTemp,arrayAddr,arrayLen);
    if(arrayLen > 1)// Judge Byte order conversion
    {
        if(-1 == gizByteOrderExchange(destBufTemp,arrayLen))
        {
            return -1;
        }
    }
    ret = gizAcrossByteJudge(bitOffset,bitLen);//Judge Byte Step
    if(0 == ret)
    {
        destValue |= ((destBufTemp[byteOffset] >> (bitOffset%8)) & (0xff >> (8 - bitLen)));
    }
    else if(1 == ret)
    {
        /* Temporarily supports only up to 2 bytes */
        highBit = destBufTemp[byteOffset + 1]& (0xFF >> (8-(bitLen-(8-bitOffset%8))));
        lowBit = destBufTemp[byteOffset]>> (bitOffset%8);
        destValue |=  (highBit << (8-bitOffset%8));
        destValue |= lowBit;
    }
    return destValue;
}

/**
* @brief bool and enum type data point data compression
*
* @param [in] byteOffset: byte offset
* @param [in] bitOffset: Bit offset
* @param [in] bitLen: occupies the bit length
* @param [out] arrayAddr: array address
* @param [in] srcData: raw data
*
* @return: 0, the correct return; -1, the error returned
*/
int32_t ICACHE_FLASH_ATTR gizStandardCompressValue(uint32_t byteOffset,uint32_t bitOffset,uint32_t bitLen,uint8_t *bufAddr,uint32_t srcData)
{
    uint8_t highBit = 0;
    uint8_t lowBit = 0;
    uint8_t ret = 0;

    if(NULL == bufAddr)
    {
        return -1;
    }

    ret = gizAcrossByteJudge(bitOffset,bitLen);
    if(0 == ret)
    {
        bufAddr[byteOffset] |= (((uint8_t)srcData)<<(bitOffset%8));
    }
    else if(1 == ret)
    {
        /* Temporarily support up to two bytes of compression */ 
        highBit = ((uint8_t)srcData)>>(8-bitOffset%8);
        lowBit = (uint8_t)srcData & (0xFF >> (8-bitOffset%8));
        bufAddr[byteOffset + 1] |=  highBit;
        bufAddr[byteOffset] |= (lowBit<<(bitOffset%8));
    }

    return 0;
}
