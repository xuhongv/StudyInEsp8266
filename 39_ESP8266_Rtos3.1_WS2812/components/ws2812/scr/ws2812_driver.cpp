/*
  This example code is created by gengyuchao used for Latest ESP8266 
  SDK based on FreeRTOS, esp-idf style.  (update:2019-06-22).
  Unless required by applicable law or agreed to in writing, this
  software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
  CONDITIONS OF ANY KIND, either express or implied.
 */
#include "ws2812_driver.hpp"

#include "esp8266/gpio_struct.h"
#include "driver/gpio.h"
#include "driver/spi.h"

//user can change
#define PIXEL_MAX 12 //the total numbers of LEDs you are used in your project

#define GPIO_OUTPUT_SET(gpio_no, bit_value) \
  gpio_output_set(bit_value << gpio_no, ((~bit_value) & 0x01) << gpio_no, 1 << gpio_no, 0)

static void IRAM_ATTR spi_event_callback(int event, void *arg)
{
}

/**
 * @description: spi驱动初始化，必须是使用 gpio13
 * @param {type} 
 * @return: 
 */
void ws2812_spi_mode_init(void)
{
  uint8_t x = 0;

  spi_config_t spi_config;
  // Load default interface parameters
  // CS_EN:1, MISO_EN:1, MOSI_EN:1, BYTE_TX_ORDER:1, BYTE_TX_ORDER:1, BIT_RX_ORDER:0, BIT_TX_ORDER:0, CPHA:0, CPOL:0
  spi_config.interface.val = SPI_DEFAULT_INTERFACE;
  // Load default interrupt enable
  // TRANS_DONE: true, WRITE_STATUS: false, READ_STATUS: false, WRITE_BUFFER: false, READ_BUFFER: false
  spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE;
  // Cancel hardware cs
  spi_config.interface.cs_en = 0;
  // MISO pin is used for DC
  spi_config.interface.miso_en = 0;
  // CPOL: 1, CPHA: 1
  spi_config.interface.cpol = 1;
  spi_config.interface.cpha = 1;
  // Set SPI to master mode
  // 8266 Only support half-duplex
  spi_config.mode = SPI_MASTER_MODE;
  // Set the SPI clock frequency division factor
  spi_config.clk_div = SPI_8MHz_DIV;
  // 注册回调函数，这里没什么用处！
  spi_config.event_cb = spi_event_callback;
  spi_init(HSPI_HOST, &spi_config);
}

//驱动WS2812 LED灯  单引脚初始化
void WS2812_Init(void)
{
  //初始化 HSPI 作为数据输出引脚
  ws2812_spi_mode_init();
}

//GRB format,MSB firsr.
//未提供按字节写数据函数，因为函数切换重启HSPI会影响时序
void WS2812BSend_24bit(uint8_t R, uint8_t G, uint8_t B)
{
  uint32_t GRB = G << 16 | R << 8 | B;
  uint8_t data_buf[24];
  uint8_t *p_data = data_buf;

  //能用　等待优化!
  uint8_t mask = 0x80;
  uint8_t byte = G;

  while (mask)
  {
    if (byte & mask)
    {
      *p_data = 0xFC; /*11111100b;*/
    }
    else
    {
      *p_data = 0XC0; /*11000000b;*/
    }
    mask >>= 1;
    p_data++;
  }

  mask = 0x80;
  byte = R;

  while (mask)
  {
    if (byte & mask)
    {
      *p_data = 0xFC; /*11111100b;*/
    }
    else
    {
      *p_data = 0XC0; /*11000000b;*/
    }
    mask >>= 1;
    p_data++;
  }

  mask = 0x80;
  byte = B;

  while (mask)
  {
    if (byte & mask)
    {
      *p_data = 0xFC; /*11111100b;*/
    }
    else
    {
      *p_data = 0XC0; /*11000000b;*/
    }
    mask >>= 1;
    p_data++;
  }

  uint8_t *p_8_data;
  for (int i = 0; i < 6; i++)
  {
    p_8_data = (data_buf + (i * 4));
    uint8_t temp;
    for (int j = 0; j < 2; j++)
    {
      temp = p_8_data[j];
      p_8_data[j] = p_8_data[3 - j];
      p_8_data[3 - j] = temp;
    }
  }

  uint32_t *spi_buf = (uint32_t *)data_buf;
  spi_trans_t trans = {0};
  trans.mosi = spi_buf;
  trans.bits.mosi = 24 * 8;
  spi_trans(HSPI_HOST,&trans);
}

//The following reference from the network

//delay for millisecond
void HAL_Delay(int time)
{
  // os_delay_us(time*1000);
  vTaskDelay(time / portTICK_RATE_MS);
}

uint8_t rBuffer[PIXEL_MAX] = {0};
uint8_t gBuffer[PIXEL_MAX] = {0};
uint8_t bBuffer[PIXEL_MAX] = {0};

void setAllPixelColor(uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t i = 0;
  for (i = 0; i < PIXEL_MAX; i++)
  {
    rBuffer[i] = 0;
    gBuffer[i] = 0;
    bBuffer[i] = 0;
  }
  for (i = 0; i < PIXEL_MAX; i++)
  {
    rBuffer[i] = r;
    gBuffer[i] = g;
    bBuffer[i] = b;
  }

  for (i = 0; i < PIXEL_MAX; i++)
  {
    WS2812BSend_24bit(rBuffer[i], gBuffer[i], bBuffer[i]);
  }
}

void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t i = 0;

  for (i = 0; i < PIXEL_MAX; i++)
  {
    rBuffer[i] = 0;
    gBuffer[i] = 0;
    bBuffer[i] = 0;
  }
  rBuffer[n] = r;
  gBuffer[n] = g;
  bBuffer[n] = b;
  for (i = 0; i < PIXEL_MAX; i++)
  {
    WS2812BSend_24bit(rBuffer[i], gBuffer[i], bBuffer[i]);
  }
}

void SetPixelColor(uint16_t n, uint32_t c)
{
  uint8_t i = 0;

  rBuffer[n] = (uint8_t)(c >> 16);
  gBuffer[n] = (uint8_t)(c >> 8);
  bBuffer[n] = (uint8_t)c;

  for (i = 0; i < PIXEL_MAX; i++)
  {
    WS2812BSend_24bit(rBuffer[i], gBuffer[i], bBuffer[i]);
  }
}

uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

uint32_t Wheel(uint8_t WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//彩虹效果
void rainbow(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256; j++)
  {
    for (i = 0; i < PIXEL_MAX; i++)
    {
      SetPixelColor(i, Wheel((i + j) & 255));
    }
    HAL_Delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++)
  { // 5 cycles of all colors on wheel
    for (i = 0; i < PIXEL_MAX; i++)
    {
      SetPixelColor(i, Wheel(((i * 256 / PIXEL_MAX) + j) & 255));
    }
    HAL_Delay(wait);
  }
}

//Theatre-style crawling lights.o??¨¹¦Ì?
void theaterChase(uint32_t c, uint8_t wait)
{
  int i, j, q;
  for (j = 0; j < 10; j++)
  { //do 10 cycles of chasing
    for (q = 0; q < 3; q++)
    {
      for (i = 0; i < PIXEL_MAX; i = i + 1) //turn every one pixel on
      {
        SetPixelColor(i + q, c);
      }
      HAL_Delay(wait);

      for (i = 0; i < PIXEL_MAX; i = i + 1) //turn every one pixel off
      {
        SetPixelColor(i + q, 0);
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait)
{
  int i, j, q;
  for (j = 0; j < 256; j++)
  { // cycle all 256 colors in the wheel
    for (q = 0; q < 3; q++)
    {
      for (i = 0; i < PIXEL_MAX; i = i + 1) //turn every one pixel on
      {
        SetPixelColor(i + q, Wheel((i + j) % 255));
      }

      HAL_Delay(wait);

      for (i = 0; i < PIXEL_MAX; i = i + 1) //turn every one pixel off
      {
        SetPixelColor(i + q, 0);
      }
    }
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
  uint16_t i = 0;
  for (i = 0; i < PIXEL_MAX; i++)
  {
    SetPixelColor(i, c);
    HAL_Delay(wait);
  }
}

void WS2812B_Init(void)
{
  WS2812_Init();
  colorWipe(Color(0, 0, 255), 1);
  setAllPixelColor(0, 0, 0);
}

/**
 * @description: 设置颜色
 * @param {type} 此发送格式为rgb顺序
 * @return: 
 */
void ws2812_setColor(uint8_t r, uint8_t g, uint8_t b)
{
  setAllPixelColor(r, g, b);
}

/**
 * @description: 设置颜色
 * @param {type} 此发送格式为grb顺序
 * @return: 
 */
void ws2812_setColor_grb(uint8_t g, uint8_t r, uint8_t b)
{
  setAllPixelColor(r, g, b);
}

void WS2812B_Test(void)
{
  colorWipe(Color(255, 0, 0), 5); // Red
  colorWipe(Color(0, 255, 0), 5); // Green
  colorWipe(Color(0, 0, 255), 5); // Blue
  //colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
  // // Send a theater pixel chase in...
  // theaterChase(Color(127, 127, 127), 50); // White
  // theaterChase(Color(127, 0, 0), 50); // Red
  // theaterChase(Color(0, 0, 127), 50); // Blue

  // rainbow(20);
  // rainbowCycle(20);
  // theaterChaseRainbow(50);

  // setAllPixelColor(255, 0, 0);
  // HAL_Delay(100);
  // setAllPixelColor(0, 255, 0);
  // HAL_Delay(100);
  // setAllPixelColor(0, 0, 255);
  // HAL_Delay(100);
  // setAllPixelColor(0, 0, 0);
  // HAL_Delay(100);
  // setPixelColor(0, 0, 255, 0);
  // HAL_Delay(100);
  // setPixelColor(2, 0, 0, 255);
  // HAL_Delay(100);
  // setPixelColor(4, 255, 0, 0);
  // HAL_Delay(100);
  // setPixelColor(6, 125, 125, 125);
  // HAL_Delay(100);
  // setPixelColor(5, 0, 255, 0);
  // HAL_Delay(100);
  // setPixelColor(3, 0, 0, 255);
  // HAL_Delay(100);
  // setPixelColor(1, 255, 0, 0);
  // HAL_Delay(100);
  // setAllPixelColor(0, 0, 0);
}
