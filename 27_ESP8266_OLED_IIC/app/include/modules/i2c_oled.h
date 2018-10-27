
#ifndef __I2C_OLED_H
#define	__I2C_OLED_H

#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"

//定义OLED显示屏作为从设备地址，看显示屏模块背面电阻焊接到哪一边，地址就是什么
#define OLED_ADDRESS	0x78  // D/C->GND
//#define OLED_ADDRESS	0x7a // D/C->Vcc

//向显示控制寄存器中写命令
void OLED_writeCmd(unsigned char I2C_Command);
//写数据
void OLED_writeDat(unsigned char I2C_Data);
//Oled初始化相关操作，main文件中开始必须初始化一次
bool OLED_Init(void);
//设置屏幕上的显示位置
void OLED_SetPos(unsigned char x, unsigned char y);
//屏幕填充
void OLED_Fill(unsigned char fill_Data);
//清屏
void OLED_CLS(void);
//打开屏幕
void OLED_ON(void);
//关闭屏幕
void OLED_OFF(void);
//在屏幕上的指定位置显示出指定的字符串，参数为列位置（自0起）、行位置（自0起），待显示字符串，字体大小（1是小字体2是大字体）
//小字体时列最大显示21个字符（128/6）,行最大8行（0-7），大字体时列最大显示16个字符（128/8），行最大4行（占用小字体的两行，也就是说，大字体行后紧接着的下一行显示要空一行，如1行显示了大字体，则3行才能显示别的字符，2行显示则会占用1行大字体的下半部分）
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);

//显示位图，由取模工具得位图数据，x0为起始列的点（0-127），y0为起始行的的点（0-7），x1/y1分别为终止行的点，BMP为位图数据
//可以将汉字设计为大小合适的位图，在合适的位置显示
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,const unsigned char BMP[]);//

#endif
