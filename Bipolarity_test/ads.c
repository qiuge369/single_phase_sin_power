/*
 * ads.c
 *
 *  Created on: 2018年4月25日
 *      Author: ZZX
 */
#include <msp430f6638.h>
#include "ads.h"
void ADS1118_init(void)
{
 ADS1118_CS_OUT;
 ADS1118_CLK_OUT;
 ADS1118_IN_OUT;
 ADS1118_OUT_IN;
 ADS1118x_CS_OUT;
 ADS1118x_CLK_OUT;
 ADS1118x_IN_OUT;
 ADS1118x_OUT_IN;
 CLR_ADS1118_CS;
 _NOP();
 CLR_ADS1118_CLK;
 _NOP();
 CLR_ADS1118_IN;
 _NOP();
 CLR_ADS1118x_CS;
 _NOP();
 CLR_ADS1118x_CLK;
 _NOP();
 CLR_ADS1118x_IN;
 _NOP();
}
void DAC_init(void)
{
    DAC_SYNC_OUT;
    DAC_SCLK_OUT;
    DAC_IN_OUT;
    SET_DAC_SYNC;
    SET_DAC_SCLK;
    CLR_DAC_IN;
}
/*
unsigned char Read_ADS1118_Data(unsigned char Write_Data)   //SPI为全双工通信方式
{
 unsigned char i;
 unsigned char temp;
 unsigned char Read_Data=0x00;
 unsigned char A=0x80,B=0x01;
 temp=Write_Data;
 Read_Data=0x00;
 for(i=0;i<8;i++)
   {
    if(temp&A)           SET_ADS1118_IN;
    else                 CLR_ADS1118_IN;
    _NOP();
    SET_ADS1118_CLK;//The rising edge of CLK send Data,DIN Send the MSB of the Data firstly
    _NOP();
    if(ADS1118_OUT_Val)  Read_Data|=B;
    CLR_ADS1118_CLK;//The falling edge of CLK send Data,DOUT Send the LSB of the Data firstly
    A=(A>>1);
    B=(B<<1);
   }
 A=0x80;B=0x01;
 return Read_Data;
}
*/


unsigned char ADS1118_Read(unsigned char data)   //SPI为全双工通信方式
{
 unsigned char i,temp,Din;
  temp=data;
  for(i=0;i<8;i++)
  {
   Din = Din<<1;
   if(0x80&temp)
    SET_ADS1118_IN;
   else
    CLR_ADS1118_IN;
   _delay_cycles(1);
    SET_ADS1118_CLK;
    _delay_cycles(1);
   if(ADS1118_OUT_Val)
    Din |= 0x01;
    _delay_cycles(1);
    CLR_ADS1118_CLK;
    _delay_cycles(1);
    temp = (temp<<1);
  }
  return Din;
}
unsigned char ADS1118x_Read(unsigned char data)
{
     unsigned char i,temp,Din;
     temp=data;
     for(i=0;i<8;i++)
     {
      Din = Din<<1;
      if(0x80&temp)
       SET_ADS1118x_IN;
      else
       CLR_ADS1118x_IN;
      _delay_cycles(1);
       SET_ADS1118x_CLK;
       _delay_cycles(1);
      if(ADS1118x_OUT_Val)
       Din |= 0x01;
       _delay_cycles(1);
       CLR_ADS1118x_CLK;
       _delay_cycles(1);
       temp = (temp<<1);
     }
     return Din;
}


float ADS1118_Voltage;
float ADS1118_Get_Voltage(void)
{
 unsigned int i=0;
 unsigned char Data_REG_H,Data_REG_L;
 unsigned int Data_REG;
 while((ADS1118x_OUT_Val)&&(i<1000)) i++;
 Data_REG_H=ADS1118x_Read((unsigned char)((Control_Regist_MSB>>8)));
 Data_REG_L=ADS1118x_Read((unsigned char)Control_Regist_LSB);
 Data_REG=(Data_REG_H<<8)+Data_REG_L;
 ADS1118x_Read((unsigned char)((Control_Regist_MSB>>8)));
 ADS1118x_Read((unsigned char)Control_Regist_LSB);

 if(Data_REG>=0x8000)
   {
    Data_REG=0xFFFF-Data_REG;//把0xFFFF改成0x10000
    ADS1118_Voltage=(-1.0)*((Data_REG*FS/0x8000));
   }
 else
     ADS1118_Voltage=(1.0)*((Data_REG*FS/32768));
 return ADS1118_Voltage;
}

float ADS1118_Current;
float ADS1118_Get_Current(void)
{
    unsigned int i=0;
    unsigned char Data_REG_H,Data_REG_L;
    unsigned int Data_REG;
    while((ADS1118_OUT_Val)&&(i<1000)) i++;
    Data_REG_H=ADS1118_Read((unsigned char)((Control_Regist_MSB>>8)));
    Data_REG_L=ADS1118_Read((unsigned char)Control_Regist_LSB);
    Data_REG=(Data_REG_H<<8)+Data_REG_L;
    ADS1118_Read((unsigned char)((Control_Regist_MSB>>8)));
    ADS1118_Read((unsigned char)Control_Regist_LSB);

    if(Data_REG>=0x8000)
      {
       Data_REG=0xFFFF-Data_REG;//把0xFFFF改成0x10000
       ADS1118_Current=(-1.0)*((Data_REG*FS/0x8000));
      }
    else
        ADS1118_Current=(1.0)*((Data_REG*FS/32768));
    return ADS1118_Current;
}


float ADS1118_Temperature=25.9;
void ADS1118_Get_Temperature(void)
{
 unsigned int i=0;
 unsigned char Data_REG_H,Data_REG_L;
 unsigned int Data_REG;
 while((ADS1118_OUT_Val)&&(i<200)) i++;
 Data_REG_H=ADS1118_Read(0x83);
 Data_REG_L=ADS1118_Read(0xFB);
 Data_REG=(Data_REG_H<<8)+Data_REG_L;
 ADS1118_Read(0x83);
 ADS1118_Read(0xFB);
 ADS1118_Temperature=(Data_REG>>2)*0.03125;
}
void ADC2DAC(unsigned char data)
{
    unsigned char i,temp;
     temp=data;
     for(i=0;i<8;i++)
     {
      if(0x80&temp)
       SET_ADS1118_IN;
      else
       CLR_ADS1118_IN;
      _delay_cycles(1);
       SET_ADS1118_CLK;
       _delay_cycles(1);
      if(ADS1118_OUT_Val)
          SET_DAC_IN;
      else
          CLR_DAC_IN;
       _delay_cycles(1);
       CLR_DAC_SCLK;
       CLR_ADS1118_CLK;
       _delay_cycles(1);
       SET_DAC_SCLK;
       temp = (temp<<1);
     }
}
void ADCTODAC(void)
{
    unsigned int i=0;
    while((ADS1118_OUT_Val)&&(i<10000)) i++;
    CLR_DAC_IN;
    _delay_cycles(1);
    CLR_DAC_SYNC;
    _delay_cycles(1);
    for(i=0;i<8;i++)
    {
        CLR_DAC_SCLK;
        _delay_cycles(1);
        SET_DAC_SCLK;
        _delay_cycles(1);
    }
    ADC2DAC((unsigned char)((Control_Regist_MSB>>8)));
    SET_DAC_SYNC;
    ADC2DAC((unsigned char)Control_Regist);
    ADS1118_Read((unsigned char)((Control_Regist_MSB>>8)));
    ADS1118_Read((unsigned char)Control_Regist_LSB);
}
int y[16];
void DAC(long int z2)
{
//    float s= (float)z2;
//    s=(s/4096)*65536;
//    z2=(int)s;
    if(z2<65536)
    {
        int i=0;
        y[0]=z2%2;
        for(i=1;i<16;i++)
        {
            z2=(z2-y[i-1])/2;
            y[i]=z2%2;
        }
        i=0;
        CLR_DAC_IN;
        _delay_cycles(1);
        CLR_DAC_SYNC;
        _delay_cycles(1);
        for(i=0;i<8;i++)
        {
            if(i!=3)
            {
            CLR_DAC_SCLK;
            _delay_cycles(2);
            SET_DAC_SCLK;
            _delay_cycles(2);
            }
            else
            {
                SET_DAC_IN;
                _delay_cycles(2);
                CLR_DAC_SCLK;
                _delay_cycles(2);
                SET_DAC_SCLK;
                _delay_cycles(2);
                CLR_DAC_IN;
                _delay_cycles(2);
            }
        }
        for(i=0;i<16;i++)
        {
            if(y[15-i]==1)
                SET_DAC_IN;
            else
                CLR_DAC_IN;
             _delay_cycles(1);
             CLR_DAC_SCLK;
             _delay_cycles(1);
             SET_DAC_SCLK;
             _delay_cycles(1);
        }
        SET_DAC_SYNC;
        _delay_cycles(1);
//        CLR_DAC_IN;
//        _delay_cycles(1);
//        CLR_DAC_SYNC;
//        _delay_cycles(1);
//        for(i=0;i<24;i++)
//        {
//            CLR_DAC_SCLK;
//            _delay_cycles(2);
//            SET_DAC_SCLK;
//            _delay_cycles(2);
//        }
//        SET_DAC_SYNC;
    }
}

