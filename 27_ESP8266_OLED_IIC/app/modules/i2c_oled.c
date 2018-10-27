/*
* 0.96 OLED  ��ʾ����
*/
//include�ļ��������Ȼ���ͷ�ļ���߼���ͷ�ļ����Է���ʾ���ֱ���֮��Ķ���δ���������

#include "i2c_oled.h"

#include "i2c_master.h"
#include "i2c_oled_fonts.h"

#define MAX_SMALL_FONTS 21 //С����
#define MAX_BIG_FONTS 16 //������

static bool oledstat = false;//oled״̬��ʼ��ʶΪ��

//��ĳ�Ĵ�����д��������ݣ�����λ�Ĵ���ʵ���ض�����
bool ICACHE_FLASH_ATTR
OLED_writeReg(uint8_t reg_addr,uint8_t val)
{
  //����iicͨ��
  i2c_master_start();
  //дOLED��Ϊ���豸�ĵ�ַ
  i2c_master_writeByte(OLED_ADDRESS);
  //��δ�յ���Ӧ�ź���ֹͣiicͨ�ţ�����ʧ�ܱ�ʶ
  if(!i2c_master_checkAck()) {
	  i2c_master_stop();
    return 0;
  }
  //д�Ĵ�����ַ
  i2c_master_writeByte(reg_addr);
  //��δ�յ���Ӧ�ź���ֹͣiicͨ�ţ�����ʧ�ܱ�ʶ
  if(!i2c_master_checkAck()) {
	  i2c_master_stop();
    return 0;
  }
  //д���ݣ����¸�λ��ֵ
  i2c_master_writeByte(val&0xff);
  //��δ�յ���Ӧ�ź���ֹͣiicͨ�ţ�����ʧ�ܱ�ʶ
  if(!i2c_master_checkAck()) {
	  i2c_master_stop();
    return 0;
  }
  //ֹͣiicͨ��
  i2c_master_stop();

  //����oled״̬Ϊ�棬����ʼ����ϣ�������
  if (reg_addr==0x00)
    oledstat=true;
    
  return 1;
}

//����ʾ���ƼĴ�����д���������ֵ���������0.96OLED��ʾ��_����оƬ�ֲᡱ�ĵ���28ҳ
void ICACHE_FLASH_ATTR
OLED_writeCmd(unsigned char I2C_Command)
{
  OLED_writeReg(0x00,I2C_Command);
}

//д����ʾ�����ݣ�Ҳ���ǵ���Ҫ��ʾ�ķֱ��ʵ�
void ICACHE_FLASH_ATTR
OLED_writeDat(unsigned char I2C_Data)
{
	OLED_writeReg(0x40,I2C_Data);
}

//Oled��ʼ����ز������ο�51���̺��о�԰�ĵ��и����Ĳ��裬����Ϊʲô������ʼ���������ˣ�Ҳ����ģ������涨��
bool ICACHE_FLASH_ATTR
OLED_Init(void)
{
  //��ʱ10��
  os_delay_us(60000);
  os_delay_us(40000);


  OLED_writeCmd(0xAE);   //display off
  OLED_writeCmd(0x20);        //Set Memory Addressing Mode
  OLED_writeCmd(0x10);        //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
  OLED_writeCmd(0xb0);        //Set Page Start Address for Page Addressing Mode,0-7
  OLED_writeCmd(0xc8);        //Set COM Output Scan Direction
  OLED_writeCmd(0x00);//---set low column address
  OLED_writeCmd(0x10);//---set high column address
  OLED_writeCmd(0x40);//--set start line address
  OLED_writeCmd(0x81);//--set contrast control register
  OLED_writeCmd(0x7f);
  OLED_writeCmd(0xa1);//--set segment re-map 0 to 127
  OLED_writeCmd(0xa6);//--set normal display
  OLED_writeCmd(0xa8);//--set multiplex ratio(1 to 64)
  OLED_writeCmd(0x3F);//
  OLED_writeCmd(0xa4);//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
  OLED_writeCmd(0xd3);//-set display offset
  OLED_writeCmd(0x00);//-not offset
  OLED_writeCmd(0xd5);//--set display clock divide ratio/oscillator frequency
  OLED_writeCmd(0xf0);//--set divide ratio
  OLED_writeCmd(0xd9);//--set pre-charge period
  OLED_writeCmd(0x22); //
  OLED_writeCmd(0xda);//--set com pins hardware configuration
  OLED_writeCmd(0x12);
  OLED_writeCmd(0xdb);//--set vcomh
  OLED_writeCmd(0x20);//0x20,0.77xVcc
  OLED_writeCmd(0x8d);//--set DC-DC enable
  OLED_writeCmd(0x14);//
  OLED_writeCmd(0xaf);//--turn on oled panel

  OLED_Fill(0x00);  //����
  
  return oledstat;
}

//������Ļ�ϵ���ʾλ��
void ICACHE_FLASH_ATTR
OLED_SetPos(unsigned char x, unsigned char y)
{ 
	OLED_writeCmd(0xb0+y);
	OLED_writeCmd(((x&0xf0)>>4)|0x10);
	OLED_writeCmd((x&0x0f)|0x01);
}

//��Ļ���
void ICACHE_FLASH_ATTR
OLED_Fill(unsigned char fill_Data)
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		OLED_writeCmd(0xb0+m);		//�ڼ�ҳ,Ҳ���ǵڼ���
		OLED_writeCmd(0x00);		//�е�λ����ʼ��ַ
		OLED_writeCmd(0x10);		//�и�λ����ʼ��ַ
		for(n=0;n<132;n++)
			{
				OLED_writeDat(fill_Data);
			}
	}
}

//����
void ICACHE_FLASH_ATTR
OLED_CLS(void)
{
	OLED_Fill(0x00);
}

//��LED��Ļ
void ICACHE_FLASH_ATTR
OLED_ON(void)
{
	OLED_writeCmd(0X8D);
	OLED_writeCmd(0X14);
	OLED_writeCmd(0XAF);
}

//�ر�LED��Ļ
void ICACHE_FLASH_ATTR
OLED_OFF(void)
{
	OLED_writeCmd(0X8D);
	OLED_writeCmd(0X10);
	OLED_writeCmd(0XAE);
}


//����Ļ�ϵ�ָ��λ����ʾ��ָ�����ַ���������Ϊ��λ�á���λ�ã�����ʾ�ַ����������С��Ҳ���ǵ�����طֱ��ʵ�Ĺ���
void ICACHE_FLASH_ATTR
OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					OLED_writeDat(F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					OLED_writeDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					OLED_writeDat(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}

//��ʾλͼ����ȡģ���ߵ�λͼ���ݣ�x0Ϊ��ʼ�еĵ㣬y0Ϊ��ʼ�еĵĵ㣬x1/y1�ֱ�Ϊ��ֹ�еĵ㣬BMPΪλͼ���ݣ�Ҳ���ǵ�����طֱ��ʵ�Ĺ���
//���Խ��������Ϊ��С���ʵ�λͼ���ں��ʵ�λ����ʾ
void ICACHE_FLASH_ATTR
OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,const unsigned char BMP[])//
{
	unsigned int j=0;
	unsigned char x,y;

	if (x1>128)
		x1=128;
	if (y1>8)
		y1=8;

	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
			OLED_writeDat(BMP[j++]);
		}
	}
}
