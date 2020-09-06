/*
 * ads.h
 *
 *  Created on: 2018Äê4ÔÂ25ÈÕ
 *      Author: ZZX
 */

#ifndef ADS_H_
#define ADS_H_
#include <msp430f6638.h>
#include    <in430.h>
#include    <intrinsics.h>

#define      ADS1118_CS         BIT2//2
#define      ADS1118_CLK        BIT4//1
#define      ADS1118_OUT        BIT0//9
#define      ADS1118_IN         BIT6//10
#define      DAC_SYNC           BIT4
#define      DAC_SCLK           BIT1
#define      DAC_IN             BIT0
#define      ADS1118x_CS        BIT4
#define      ADS1118x_CLK       BIT2
#define      ADS1118x_OUT       BIT5
#define      ADS1118x_IN        BIT3

#define      ADS1118_Port_OUT   P9OUT//A
#define      ADS1118_Port_DIR   P9DIR
#define      ADS1118_Port_IN    P9IN
#define      DAC_Port_OUT       P8OUT
#define      DAC_Port_DIR       P8DIR
#define      DAC_Port_IN        P8IN
#define      ADS1118x_Port_OUT  P7OUT//V
#define      ADS1118x_Port_DIR  P7DIR
#define      ADS1118x_Port_IN   P7IN

#define      DAC_SYNC_OUT       (DAC_Port_DIR|=DAC_SYNC)
#define      SET_DAC_SYNC       (DAC_Port_OUT|=DAC_SYNC)
#define      CLR_DAC_SYNC       (DAC_Port_OUT&=~DAC_SYNC)

#define      DAC_SCLK_OUT       (DAC_Port_DIR|=DAC_SCLK)
#define      SET_DAC_SCLK       (DAC_Port_OUT|=DAC_SCLK)
#define      CLR_DAC_SCLK       (DAC_Port_OUT&=~DAC_SCLK)

#define      DAC_IN_OUT         (DAC_Port_DIR|=DAC_IN)
#define      SET_DAC_IN         (DAC_Port_OUT|=DAC_IN)
#define      CLR_DAC_IN         (DAC_Port_OUT&=~DAC_IN)

#define      ADS1118_CS_OUT     (ADS1118_Port_DIR|=ADS1118_CS)
#define      SET_ADS1118_CS     (ADS1118_Port_OUT|=ADS1118_CS)
#define      CLR_ADS1118_CS     (ADS1118_Port_OUT&=~ADS1118_CS)

#define      ADS1118_CLK_OUT    (ADS1118_Port_DIR|=ADS1118_CLK)
#define      SET_ADS1118_CLK    (ADS1118_Port_OUT|=ADS1118_CLK)
#define      CLR_ADS1118_CLK    (ADS1118_Port_OUT&=~ADS1118_CLK)

#define      ADS1118_OUT_IN     (ADS1118_Port_DIR&=~ADS1118_OUT)
#define      ADS1118_OUT_Val    (ADS1118_Port_IN&ADS1118_OUT)

#define      ADS1118_IN_OUT     (ADS1118_Port_DIR|=ADS1118_IN)
#define      SET_ADS1118_IN     (ADS1118_Port_OUT|=ADS1118_IN)
#define      CLR_ADS1118_IN     (ADS1118_Port_OUT&=~ADS1118_IN)

#define      ADS1118x_CS_OUT     (ADS1118x_Port_DIR|=ADS1118x_CS)
#define      SET_ADS1118x_CS     (ADS1118x_Port_OUT|=ADS1118x_CS)
#define      CLR_ADS1118x_CS     (ADS1118x_Port_OUT&=~ADS1118x_CS)

#define      ADS1118x_CLK_OUT    (ADS1118x_Port_DIR|=ADS1118x_CLK)
#define      SET_ADS1118x_CLK    (ADS1118x_Port_OUT|=ADS1118x_CLK)
#define      CLR_ADS1118x_CLK    (ADS1118x_Port_OUT&=~ADS1118x_CLK)

#define      ADS1118x_OUT_IN     (ADS1118x_Port_DIR&=~ADS1118x_OUT)
#define      ADS1118x_OUT_Val    (ADS1118x_Port_IN&ADS1118x_OUT)

#define      ADS1118x_IN_OUT     (ADS1118x_Port_DIR|=ADS1118x_IN)
#define      SET_ADS1118x_IN     (ADS1118x_Port_OUT|=ADS1118x_IN)
#define      CLR_ADS1118x_IN     (ADS1118x_Port_OUT&=~ADS1118x_IN)


#define      SS          BITF      //    x    Unused in Continuous conversion mode(Always reads back as 0)
#define      MUX2        BITE      //    1
#define      MUX1        BITD      //    1
#define      MUX0        BITC      //    1    111 = AINP is AIN3 and AINN is GND
#define      PGA2        BITB      //    0
#define      PGA1        BITA      //    0
#define      PGA0        BIT9      //    1    001 = FS is ¡À4.096 V
#define      MODE        BIT8      //    0    0 = Continuous conversion mode

#define      FS          4.096

#define      DR2         BIT7      //    1
#define      DR1         BIT6      //    0
#define      DR0         BIT5      //    0    100 = 128 SPS (default)
#define      TS_MODE     BIT4      //    0    0 = ADC mode (default)        (1 = Temperature sensor mode)
#define      PULL_UP_EN  BIT3      //    1    1 = Pull-up resistor enabled on DOUT/DRDY pin (default)
#define      NOP1        BIT2      //    0
#define      NOP0        BIT1      //    1    01 = Valid data, update the Config register (default)
#define      NOT_USED    BIT0      //    x    Always reads '1'

#define      Control_Regist      (MUX2+MUX0+PGA0+DR2+PULL_UP_EN+NOP0)
#define      Control_Regist_MSB  (MUX2+MUX0+PGA0)
//#define      Control_Regist      (PGA0+DR2+PULL_UP_EN+NOP0)
//#define      Control_Regist_MSB  (PGA0)
#define      Control_Regist_LSB  (DR2+PULL_UP_EN+NOP0)
extern float ADS1118_Voltage;
extern float ADS1118_Temperature;
extern float ADS1118_Current;

/*
#define      Control_Regist       0x068B
#define      Control_Regist_MSB   0x06
#define      Control_Regist_LSB   0x8B
extern float Temperature;
*/


extern void ADS1118_init(void);
extern unsigned char ADS1118_Read(unsigned char);
extern unsigned char ADS1118x_Read(unsigned char);
extern float ADS1118_Get_Voltage(void);
extern void ADS1118_Get_Temperature(void);
extern void ADC2DAC(unsigned char);
extern void ADCTODAC(void);
extern void DAC(long int);
extern float ADS1118_Get_Current(void);
extern void DAC_init(void);

//void ADS1118_init(void);
//unsigned char ADS1118_Read(unsigned char data);
//unsigned char Read_ADS1118_Data(unsigned char Write_Data);
//void ADS1118_Get_Voltage(void);
//void ADS1118_Get_Temperature(void);



#endif /* ADS_H_ */
