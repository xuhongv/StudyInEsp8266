/**
************************************************************
* @file         common.c
* @brief        Generic tools
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
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

/**
* @brief Checksum calculation
*
* @param [in] buf   : data 
* @param [in] len   : data len
*
* @return sum : Checksum
*/
uint8_t ICACHE_FLASH_ATTR gizProtocolSum(uint8_t *buf, uint32_t len)
{
    uint8_t     sum = 0;
    uint32_t    i = 0;

    if(buf == NULL || len <= 0)
    {
        return 0;
    }

    for(i=2; i<len-1; i++)
    {
        sum += buf[i];
    }

    return sum;
}

/**
 * htons unsigned short -> Network byte order
 * ntohs Network byte order -> unsigned short
 */
uint16_t ICACHE_FLASH_ATTR exchangeBytes(uint16_t value)
{
    uint16_t tmp_value;
    uint8_t *index_1, *index_2;

    index_1 = (uint8_t *)&tmp_value;
    index_2 = (uint8_t *)&value;

    *index_1 = *(index_2+1);
    *(index_1+1) = *index_2;

    return tmp_value;
}

/**
 * htonl unsigned long  -> Network byte order
 * ntohl Network byte order -> unsigned long
 */
uint32_t ICACHE_FLASH_ATTR exchangeWord(uint32_t value)
{
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0xFF000000) >> 24) ;
}

/**
* @brief Character to binary
* @param[in]   A  : Character A
* @param[out]  B  : Character B
* @return     : Accounting for one byte of binary data
*/
uint8_t ICACHE_FLASH_ATTR char2hex(char A , char B)
{
    uint8_t a ,b = 0;
    if(A>='0'&&A<='9')
    {
        a=A-'0';
    }
    else if(A>='A' && A<='F')
    {
        a=A-'A'+10;
    }
    else if(A>='a' && A<='f')
    {
        a=A-'a'+10;
    }

    if(B>='0'&&B<='9')
    {
        b=B-'0';
    }
    else if(B>='A' && B<='F')
    {
        b=B-'A'+10;
    }
    else if( B>='a' && B<='f')
    {
        b=B-'a'+10;
    }

    return (a<<4)+b;
}

/**
* @brief checkStringIsNotZero

* Check String Is Zero ,Or Not

* @param[in]    : String
* @param[out]   :
* @return       : 0,String Not All Zero
*               -1,Input Illegal
*               -2,String All Zero
*/
int8_t ICACHE_FLASH_ATTR checkStringIsNotZero(const char *data)
{
    uint32_t i = 0;
    uint32_t dataLen = 0;

    dataLen = gizStrlen(data);

    if(NULL == data)
    {
        return -1;
    }

    for(i=0; i<dataLen; i++)
    {
        if(data[i] != '0')
        {
            return 0;
        }
    }

    return -2;
}


/**
* @brief checkStringIsZero

* Check String Is Zero

* @param[in]    : String
* @param[out]   :
* @return       : 0,String Is All Zero
*               -1,Input Illegal
*               -2,String Not All Zero
*/
int8_t ICACHE_FLASH_ATTR checkStringIsZero(const char *data)
{
    uint32_t i = 0;
    uint32_t dataLen = 0;

    dataLen = gizStrlen(data);

    if(NULL == data)
    {
        return -1;
    }

    for(i=0; i<dataLen; i++)
    {
        if(data[i] != '0')
        {
            return -2;
        }
    }

    return 0;
}

/**
* @brief Calculate week date

* Calculate the day of the week based on the date

* @param [in] iYear   
* @param [in] iMonth   
* @param [in] iDay  
*
* @return sum : Checksum
*/
int ICACHE_FLASH_ATTR returnWeekDay(unsigned int iYear, unsigned int iMonth, unsigned int iDay)
{
    int iWeek = 0;
    unsigned int y = 0, c = 0, m = 0, d = 0;

    if (iMonth == 1 || iMonth == 2)
    {
        c = (iYear - 1) / 100;
        y = (iYear - 1) % 100;
        m = iMonth + 12;
        d = iDay;
    }
    else
    {
        c = iYear / 100;
        y = iYear % 100;
        m = iMonth;
        d = iDay;
    }

    iWeek = y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + d - 1;
    iWeek = iWeek >= 0 ? (iWeek % 7) : (iWeek % 7 + 7);
    if (iWeek == 0)
    {
        iWeek = 7;
    }

    return iWeek;
}

static int ICACHE_FLASH_ATTR isleap(int year)
{
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

static int ICACHE_FLASH_ATTR get_yeardays(int year)
{
    if (isleap(year))
        return 366;
    return 365;
}

/**
* @brief Calculate time zone time

* Will be the time of the current time zone of Greenwich time

* @param [in] Ntp: time of Greenwich
*
* @return sum : Back to the wall time structure
*/
gizTime_t ICACHE_FLASH_ATTR returnNtp2Wt(uint32_t ntp)
{
    gizTime_t tm;
    int x;
    int i=1970, mons[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    uint32_t utc = ntp;

    /* utc + timezone */
    utc += TIMEZONE_DEFAULT * ONE_HOUR;

    for(i=1970; utc>0;)
    {
        x=get_yeardays(i);
        if(utc >= x*DAY_SEC)
        {
            utc -= x*DAY_SEC;
            i++;
        }
        else
        {
            break;
        }
    }
    tm.year = i;

    for(i=0; utc>0;)
    {
        if (isleap(tm.year))
            mons[1]=29;
        if(utc >= mons[i]*DAY_SEC)
        {
            utc -= mons[i]*DAY_SEC;
            i++;
        }
        else
        {
            break;
        }
    }
    mons[1]=28;
    tm.month = i+1;

    for(i=1; utc>0;)
    {
        if(utc >= DAY_SEC)
        {
            utc -= DAY_SEC;
            i++;
        }
        else
        {
            break;
        }
    }
    tm.day=i;

    tm.hour = utc/(60*60);
    tm.minute = utc%(60*60)/60;
    tm.second = utc%60;
    tm.ntp = ntp;

    return tm;
}

/**
* @brief String conversion binary
* @param[in]   pbSrc    : String 
* @param[in]   nLen    : String length 
* @param[out]  pbDest   : binary 
*/
void ICACHE_FLASH_ATTR str2Hex(char *pbDest, char *pbSrc, int nLen)
{
    char h1,h2;
    char s1,s2;
    int i;

    for (i=0; i<nLen; i++)
    {
        h1 = pbSrc[2*i];
        h2 = pbSrc[2*i+1];

        s1 = toupper(h1) - 0x30;
        if (s1 > 9)
            s1 -= 7;

        s2 = toupper(h2) - 0x30;
        if (s2 > 9)
            s2 -= 7;

        pbDest[i] = s1*16 + s2;
    }
}

/**
* @brief Binary conversion string
* @param[in]   pbSrc    : Binary data 
* @param[in]   nLen    : Binary length 
* @param[out]  pbDest   : Character 
*/
void ICACHE_FLASH_ATTR hex2Str(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
{
    char ddl,ddh;
    int i;

    for (i=0; i<nLen; i++) {
        ddh = 0x30 + pbSrc[i] / 16;
        ddl = 0x30 + pbSrc[i] % 16;
        if (ddh > 0x39) ddh = ddh + 7;
        if (ddl > 0x39) ddl = ddl + 7;
        pbDest[i*2] = ddh;
        pbDest[i*2+1] = ddl;
    }

    pbDest[nLen*2] = '\0';
}

#ifdef __cplusplus
}
#endif