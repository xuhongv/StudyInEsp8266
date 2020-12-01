/*
 * @Author: your name
 * @Date: 2020-11-23 16:53:05
 * @LastEditTime: 2020-11-26 10:27:03
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \ESP8266_RTOS_SDK_Tencent\ESP8266_RTOS_SDK\examples\peripherals\spi_oled\components\iot_hal_oled\include\iot_hal_oled.h
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

  /**
  * OLED模式设置
	* 0:4线串行模式
	* 1:并行8080模式
  */

#define OLED_MODE 0
#define SIZE 16
#define XLevelL 0x00
#define XLevelH 0x10
#define Max_Column 128
#define Max_Row 64
#define Brightness 0xFF
#define X_WIDTH 128
#define Y_WIDTH 64

#define OLED_SLAVE_ADDR 0x78

#define OLED_CMD 0  //写命令
#define OLED_DATA 1 //写数据

  /* OLED控制用函数 */
  void OLED_Clear(void);                                 /* OLED清屏 */
  void OLED_Display_On(void);                            /* OLED开 */
  void OLED_ShowPosture(void);                           /* 提示信息 */
  void OLED_Display_Off(void);                           /* OLED关 */
  void GPIO_OLED_InitConfig(void);                       /* OLED初始化 */
  void OLED_WR_Byte(uint8_t dat, uint8_t cmd);           /* 写字节 */
  void OLED_Set_Pos(unsigned char x, unsigned char y);   /* 设置坐标 */
  void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr); /* 显示字符 */
  void OLED_ShowString(uint8_t x, uint8_t y, char *p);   /* 显示字符串 */
  void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no);                                                        /* 显示中文 */
  void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);                               /* 显示数字 */
  void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[]); /* 显示图片 */
  void Board_MPU_Angle_Show(void);
  void Platform_MPU_Angle_Show(void);
  void Lipo_Voltage_Display(void);
  void DHT11_TemHum_Display(void);
  void USB_ON_Dispaly(void);
  void USB_OFF_Dispaly(void);

#ifdef __cplusplus
}
#endif