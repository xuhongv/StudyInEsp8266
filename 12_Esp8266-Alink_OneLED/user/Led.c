/******************************************************************************
 * Copyright 2013-2014 
 *
 * FileName:Led.c
 *
 * Description: 
 *
 * Modification history:
 *     2016/5/7, v1.0 create this file.
*******************************************************************************/

#include "esp_common.h"
#include "gpio.h"

/**
    * @brief  no .    
    * @note   no.
    * @param  no.
    * @retval no.
    */
void led_init( void )
{
	GPIO_ConfigTypeDef  GPIOConfig;

	GPIOConfig.GPIO_Pin = GPIO_Pin_15;
	GPIOConfig.GPIO_Mode = GPIO_Mode_Output;
	GPIOConfig.GPIO_Pullup = GPIO_PullUp_EN;
	GPIOConfig.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
	gpio_config( &GPIOConfig );	
}
/**
    * @brief  no .    
    * @note   no.
    * @param  no.
    * @retval no.
    */
void led_on( void )
{
	GPIO_OUTPUT( GPIO_Pin_15 , 0 );
}
/**
    * @brief  no .    
    * @note   no.
    * @param  no.
    * @retval no.
    */
void led_off( void )
{
	GPIO_OUTPUT( GPIO_Pin_15 , 1 );
}


/**
    * @brief  no .    
    * @note   no.
    * @param  no.
    * @retval no.
    */
void led_toggle( void )
{
	uint32_t bit;
	
	bit = GPIO_INPUT_GET( 15 );
	
	GPIO_OUTPUT( GPIO_Pin_15 , bit ^ 0x00000001 );
}

