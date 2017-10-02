
/*********************************************************
*
* @file      hal_rgb_led.c
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

#include "driver/hal_rgb_led.h"
#include "osapi.h"

static void ICACHE_FLASH_ATTR rgbDelay(unsigned int us)
{
    /* Define your delay function */
    volatile unsigned int i=0;
	
    for(i=0; i<us; i++);
}


/************ generation clock *********************/
static void ICACHE_FLASH_ATTR clkProduce(void)
{
    SCL_LOW;    // SCL=0
    rgbDelay(40); 
    SCL_HIGH;     // SCL=1
    rgbDelay(40); 
}


/**********  send 32 zero ********************/
static void ICACHE_FLASH_ATTR send_32zero(void)
{
    uint8_t i;
	
    SDA_LOW;   // SDA=0
    for (i=0; i<32; i++)
	{
		clkProduce();
	}
	
}


/********* invert the grey value of the first two bits ***************/
static uint8_t ICACHE_FLASH_ATTR takeAntiCode(uint8_t dat)
{
    uint8_t tmp = 0;

    tmp=((~dat) & 0xC0)>>6;
    return tmp;
}


/****** send gray data *********/
static void ICACHE_FLASH_ATTR dataSend(uint32 dx)
{
    uint8_t i;

    for (i=0; i<32; i++)
    {
        if ((dx & 0x80000000) != 0)
        {

            SDA_HIGH;     //  SDA=1;
        }
        else
        {
            SDA_LOW;    //  SDA=0;
        }

        dx <<= 1;
        clkProduce();
    }
}


/******* data processing  ********************/
static void ICACHE_FLASH_ATTR dataDealwithSend(uint8_t r, uint8_t g, uint8_t b)
{
    uint32 dx = 0; 

    dx |= (uint32)0x03 << 30;             // The front of the two bits 1 is flag bits
    dx |= (uint32)takeAntiCode(b) << 28; 
    dx |= (uint32)takeAntiCode(g) << 26; 
    dx |= (uint32)takeAntiCode(r) << 24; 

    dx |= (uint32)b << 16; 
    dx |= (uint32)g << 8; 
    dx |= r;

    dataSend(dx);
}


void ICACHE_FLASH_ATTR rgbControl(uint8_t R, uint8_t G, uint8_t B)
{
    //contron power
    
    send_32zero();
    dataDealwithSend(R, G, B);	  // display red
    dataDealwithSend(R, G, B);	  // display red
}


void ICACHE_FLASH_ATTR rgbLedInit(void)
{
    //contron power

    send_32zero();
    dataDealwithSend(0, 0, 0);   // display red
    dataDealwithSend(0, 0, 0);
}


void ICACHE_FLASH_ATTR rgbGpioInit(void)
{
    /* Migrate your driver code */

    // SCL/SDA
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15); 

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4); 

    gpio_output_set(0, 0, GPIO_ID_PIN(GPIO_RGB_SCL) | GPIO_ID_PIN(GPIO_RGB_SDA), 0); //| GPIO_ID_PIN(GPIO_RGB_POW)

    os_printf("rgbGpioInit \r\n");
}

void ICACHE_FLASH_ATTR rgbSensorTest(uint8_t rgbcou)
{
    /* Test LOG model */

    if (0 == rgbcou)
    {
        rgbControl(0, 0, 250);
//      GAgent_Printf(GAGENT_CRITICAL, "RGB : B");
    }
    else if (1 == rgbcou)
    {
        rgbControl(0, 250, 0);
//      GAgent_Printf(GAGENT_CRITICAL, "RGB : G");
    }
    else if (2 == rgbcou)
    {
        rgbControl(250, 0, 0);
//      GAgent_Printf(GAGENT_CRITICAL, "RGB : R");
    }
}

