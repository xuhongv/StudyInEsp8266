#ifndef _HAL_INFRARED_H
#define _HAL_INFRARED_H
#include <stdio.h>
#include <c_types.h>
#include <gpio.h>
#include <eagle_soc.h>

/* Define your drive pin */
#define Infrared_GPIO_PIN       16

/* Set GPIO Direction */
#define GET_INF                 GPIO_INPUT_GET(GPIO_ID_PIN(Infrared_GPIO_PIN))

/* Function declaration */
void irInit(void);
bool irUpdateStatus(void);
void irSensorTest(void);

#endif /*_HAL_INFRARED_H*/


