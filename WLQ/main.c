#include <msp430.h>
#include "setclock.h"
#include "ads1118.h"
#include "oled.h"
#include "bmp.h"
#define Num 512
//#define RATE 2300
#define RATE 2700
//#define RATE 1670

/**
 * main.c 25313K
 */
static unsigned short cnt = 0;
static unsigned int freq_mul = 334, step = 7, freq_sum = 342;//150
unsigned int mode = 1, Config_M = 0x44, Config_L = 0x8B;
unsigned int check1 = 0, check2 = 0, wt = 0, key = 0;
float ADS1118_Voltage = 0, freq = 0, power = 0;
unsigned int Config_Result_M, Config_Result_L;
unsigned int waveform = 0;

const unsigned int sin_wave[Num] = {262, 265, 268, 271, 274, 277, 281, 284, 287, 290, 293, 296, 299, 302, 306, 309, 312, 315, 318, 321, 324, 327, 330, 333, 336, 339, 342, 345, 348, 351, 354, 357, 360, 363, 366, 369, 372, 374, 377, 380, 383, 386, 388, 391, 394, 397, 399, 402, 404, 407, 410, 412, 415, 417, 420, 422, 425, 427, 430, 432, 434, 437, 439, 441, 443, 446, 448, 450, 452, 454, 456, 458, 460, 462, 464, 466, 468, 470, 472, 473, 475, 477, 479, 480, 482, 483, 485, 487, 488, 489, 491, 492, 494, 495, 496, 497, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 510, 511, 512, 512, 513, 514, 514, 515, 515, 516, 516, 516, 517, 517, 517, 517, 517, 518, 518, 518, 518, 518, 518, 517, 517, 517, 517, 517, 516, 516, 516, 515, 515, 514, 514, 513, 512, 512, 511, 510, 510, 509, 508, 507, 506, 505, 504, 503, 502, 501, 500, 499, 497, 496, 495, 494, 492, 491, 489, 488, 487, 485, 483, 482, 480, 479, 477, 475, 473, 472, 470, 468, 466, 464, 462, 460, 458, 456, 454, 452, 450, 448, 446, 443, 441, 439, 437, 434, 432, 430, 427, 425, 422, 420, 417, 415, 412, 410, 407, 404, 402, 399, 397, 394, 391, 388, 386, 383, 380, 377, 374, 372, 369, 366, 363, 360, 357, 354, 351, 348, 345, 342, 339, 336, 333, 330, 327, 324, 321, 318, 315, 312, 309, 306, 302, 299, 296, 293, 290, 287, 284, 281, 277, 274, 271, 268, 265, 262,
        259, 256, 253, 250, 247, 244, 240, 237, 234, 231, 228, 225, 222, 219, 215, 212, 209, 206, 203, 200, 197, 194, 191, 188, 185, 182, 179, 176, 173, 170, 167, 164, 161, 158, 155, 152, 149, 147, 144, 141, 138, 135, 133, 130, 127, 124, 122, 119, 117, 114, 111, 109, 106, 104, 101, 99, 96, 94, 91, 89, 87, 84, 82, 80, 78, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57, 55, 53, 51, 49, 48, 46, 44, 42, 41, 39, 38, 36, 34, 33, 32, 30, 29, 27, 26, 25, 24, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 11, 10, 9, 9, 8, 7, 7, 6, 6, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 8, 9, 9, 10, 11, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 25, 26, 27, 29, 30, 32, 33, 34, 36, 38, 39, 41, 42, 44, 46, 48, 49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71, 73, 75, 78, 80, 82, 84, 87, 89, 91, 94, 96, 99, 101, 104, 106, 109, 111, 114, 117, 119, 122, 124, 127, 130, 133, 135, 138, 141, 144, 147, 149, 152, 155, 158, 161, 164, 167, 170, 173, 176, 179, 182, 185, 188, 191, 194, 197, 200, 203, 206, 209, 212, 215, 219, 222, 225, 228, 231, 234, 237, 240, 244, 247, 250, 253, 256, 259,
};


void ADS1118_init(void)
{
    ADS1118_CS_OUT;
    ADS1118_CLK_OUT;
    ADS1118_IN_OUT;
    ADS1118_OUT_IN;
    CLR_ADS1118_CS;
    _NOP();
    CLR_ADS1118_CLK;
    _NOP();
    CLR_ADS1118_IN;
    _NOP();
}

unsigned char ADS1118_Read(unsigned char data)   //SPI为全双工通信方式
{
    unsigned char i,temp,Din;
    temp=data;
    for(i=0;i<8;i++)
    {
        Din = Din<<1;
        SET_ADS1118_CLK;
        __delay_cycles(1);
        if(ADS1118_OUT_Val)
            Din |= 0x01;
        if(0x80&temp)
            SET_ADS1118_IN;
        else
            CLR_ADS1118_IN;
        CLR_ADS1118_CLK;
        __delay_cycles(1);
        temp = (temp<<1);
    }
    return Din;
}

void Timer_Init()
{
    P9DIR |= BIT6;
    P9OUT |= BIT6;
    P8DIR |= BIT2;
    P8OUT &= ~BIT2;
    TA1CCR0 = RATE;
    TA1CCR1 = 0;
    TA1CTL |= MC_1 + TASSEL_2 + TACLR;
    TA1CCTL0 = CCIE;
    TA1CCTL1 = CCIE;
}

void ADS1118_Get_U(void)
{
    CLR_ADS1118_CS;
    unsigned int i=0;
    unsigned char Data_REG_H,Data_REG_L;
    unsigned int Data_REG;
    while((ADS1118_OUT_Val)&&(i<1000)) i++;
    Data_REG_H=ADS1118_Read(Config_M);
    __delay_cycles(1);
    Data_REG_L=ADS1118_Read(Config_L);
    Data_REG=(Data_REG_H<<8)+Data_REG_L;
    ADS1118_CS_OUT;
//  Config_Result_M = ADS1118_Read(Config_M);
//  __delay_cycles(1);
//  Config_Result_L = ADS1118_Read(Config_L);
    CLR_ADS1118_IN;
    _NOP();
    if(Data_REG>=0x8000)
    {
        Data_REG=0xFFFF-Data_REG;//把0xFFFF改成0x10000
        ADS1118_Voltage = (-1.0)*((Data_REG*FS/0x8000));
    }
    else
    {
        if(Config_M == 0x44)
        {
            power = (1.0)*((Data_REG*FS/32768));                        //浮点数计算耗时，影响波形
            Config_M = 0x54;
        }
        else if(Config_M == 0x54)
        {
            ADS1118_Voltage = (1.0)*((Data_REG*FS/32768));
            Config_M = 0x44;
        }
        //ADS1118_Voltage=(1.0)*((Data_REG*FS/32768));
    }
}

// LookupTable 这绝对是我写过最智障的代码。。。。
const unsigned int TA1[12] = { 2720, 2717, 2715, 2712, 2706, 2704, 2703, 2696, 2690, 2684, 2678, 2677 };
void adjust_PWM()
{
    int v = (int)(ADS1118_Voltage*1000);
    if(v<40)
    {
        TA1CCR0 = TA1[0]+20;
        step = 7;
    }
    else if(v>40 && v<97)
    {
        TA1CCR0 = TA1[0]+14;//2720;
        step = 7;
    }
    else if(v>97 && v<119)
    {
        TA1CCR0 = TA1[1]+11;//2717;
        step = 7;
    }
    else if(v>119 && v<129)
    {
        TA1CCR0 = TA1[3]+16;//2712;
        step = 7;
    }
    else if(v>129 && v<156)
    {
        TA1CCR0 = TA1[3]+16;//2712;
        step = 7;
    }
    else if(v>156 && v<188)
    {
        TA1CCR0 = TA1[4]+16;//2706
        step = 7;
    }
    else if(v>188 && v<200)
    {
        TA1CCR0 = TA1[6]+17;//2703
        step = 7;
    }
    else if(v>200 && v<225)
    {
        TA1CCR0 = TA1[6]+13;//2703
        step = 6;
    }
    else if(v>225 && v<250)
    {
        TA1CCR0 = TA1[7]+20;//2696
        step = 5;
    }
    else if(v>250 && v<270)
    {
        TA1CCR0 = TA1[8]+21;//2690
        step = 5;
    }
    else if(v>290 && v<300)
    {
        TA1CCR0 = TA1[10]+26;//2678
        step = 5;
    }
    else if(v>300 && v<360)
    {
        TA1CCR0 = TA1[10]+26;//2678
        step = 3;
    }
    else if(v>360 && v<420)
    {
        TA1CCR0 = TA1[10]+18;//2678
        step = 3;
    }
    else if(v>420 && v<460)
    {
        TA1CCR0 = TA1[11]+15;//2675
        step = 3;
    }
    else if(v>460 && v<500)
    {
        TA1CCR0 = TA1[11]+5;//2675
        step = 1;
    }
    else if(v>500)
    {
        TA1CCR0 = TA1[11];//2675
        step = 0;
    }
    freq_sum = freq_mul + step;
}

int main(void)

{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    SetClock_MCLK12MHZ_SMCLK12MHZ_ACLK32_768K();
    Timer_Init();
    ADS1118_init();
    OLED_Init();
    OLED_Clear();
    P4REN |= BIT0 + BIT1 + BIT2 + BIT3;
    P4OUT |= BIT0 + BIT1 + BIT2 + BIT3;
    P4DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3);
    P4DIR |= BIT4;
    P4OUT &= ~BIT4;

    __bis_SR_register(GIE);
    OLED_ShowString(37, 6, "f=");
    OLED_ShowString(53+40, 6, "Hz");
    while(1)
    {
        if(wt == 0x4F)//
        {
            ADS1118_Get_U();
            wt = 0;
            freq = 0.146*freq_mul + 1.143;
            OLED_ShowNum(53, 6, ((int)(freq*10))/10, 2, 16);
            OLED_ShowChar(53+16, 6,'.');
            OLED_ShowNum(53+24, 6, ((int)(freq*10))%10, 1, 16);
            adjust_PWM();
            OLED_ShowNum(53, 4, (int)TA1CCR0,5,16);
            OLED_ShowNum(53, 2, power, 5, 16);
        }
        if(ADS1118_Voltage>0.8)
            mode = 0;
        if(power>1.67 || power<1.57)
            mode = 2;                               //mode = 2 为保护模式
        if(key&&(mode == 2))
        {
            P3OUT |= BIT3;
            key = 0;
        }
    }
}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR(void)
{
    if(mode == 1)
    {
        P3OUT &= ~BIT3;
    }
    else if(mode == 0)
    {
        P3OUT |= BIT3;
    }
    P9OUT &= ~BIT6;
    __delay_cycles(10);
    P8OUT |= BIT2;
    TA1CCTL1 &= ~CCIFG;
    if(!(P4IN & BIT0))
    {
        if(freq_mul<680)
            freq_mul += 3;
        __delay_cycles(10000000);
    }
    if(!(P4IN & BIT1))
    {
        if(freq_mul>0)
            freq_mul -= 3;
        __delay_cycles(10000000);
    }
    if(!(P4IN & BIT2))
    {
        key = 1;                            //key = 1启动过欠压保护
        if(mode == 2)
            mode = 1;
        __delay_cycles(10000000);
    }
    if(!(P4IN & BIT3))
    {
        freq_mul = 334;
        mode = 1;
        key = 0;
        __delay_cycles(10000000);
    }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)     //9.746kHZ
{
    P3OUT ^= BIT3;
    static unsigned int tnt = 0, Index = 0;
    if(mode == 1)
    {
        P3OUT &= ~BIT3;
    }
    else if(mode == 0)
    {
        check1++;
        check2 = (check1 == 0xFF)? check2 + 1: check2;
        check1 %= 0xFF;
        mode = (check2 == 60)? 1 : 0;
        check2 %= 60;
        P3OUT |= BIT3;
    }
    P8OUT &= ~BIT2;
    __delay_cycles(10);
    P9OUT |= BIT6;
    TA1CCTL0 &= ~CCIFG;
    cnt += freq_sum;
    Index = cnt >> 8;
    tnt = (int)(Index<<1);
    TA1CCR1 = (int)(5*sin_wave[tnt] + 70 );

    if(tnt >= Num-5)
    {
        tnt = 0;
        wt++;
    }
}
