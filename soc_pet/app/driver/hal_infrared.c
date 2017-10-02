
/*********************************************************
*
* @file      hal_infrared.c
* @author    Gizwtis
* @version   V3.0
* @date      2016-03-09
*
* @brief     机智云 只为智能硬件而生
*            Gizwits Smart Cloud  for Smart Products
*            链接|增值|开放|中立|安全|自有|自由|生态
*            www.gizwits.com
*
*********************************************************/

#include "driver/hal_infrared.h"
#include "driver/gpio16.h"
#include "osapi.h"

bool ICACHE_FLASH_ATTR irUpdateStatus(void)
{
    if(gpio16InputGet())
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void ICACHE_FLASH_ATTR irInit(void)
{
    /* Migrate your driver code */

    gpio16InputConf();
    
    os_printf("irInit \r\n"); 
}

void ICACHE_FLASH_ATTR irSensorTest(void)
{
    /* Test LOG model */

    os_printf("InfIO : %d", irUpdateStatus());
}
