
#ifndef _IOT_DHT11_H_
#define _IOT_DHT11_H_

#include <stdio.h>
#include "esp_system.h"
#include <stdlib.h>
#include <string.h>

/* Define your drive pin */
#define DHT11_GPIO_PIN 5

#define DHT11_OUT_HIGH gpio_set_level(DHT11_GPIO_PIN, 1);
#define DHT11_OUT_LOW gpio_set_level(DHT11_GPIO_PIN, 0);
#define DHT11_IN gpio_get_level(DHT11_GPIO_PIN)

#define MEAN_NUM 10

typedef struct
{
	uint8_t th_num;
	uint8_t th_amount;
	uint8_t th_bufs[10][2];
} th_typedef_t;

/* Function declaration */
uint8_t dh11Read(uint8_t *temperature, uint8_t *humidity);
uint8_t dh11Init(void); //Init DHT11
void dh11SensorTest(void);

#endif
