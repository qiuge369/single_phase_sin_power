#include <msp430f6638.h>
#include "oled.h"
#include "bmp.h"
#include "setclock.h"
#include "Half_Bridge.h"
#include "ads.h"

#define t 5
int a1=0,a=0,b=0,c=50,f=0,x[2]={5,0},h0=0,h1=0,h2=0;
char ax=0;
int f1,f2,f3,f0,g1;
float e=0,d=0;
long int d1=0;
static unsigned short count_sin = 0;
static unsigned char Index_sin = 0;
static unsigned int step_sin = 280;
void INIT(void);
void KeyHead(void);
void showf(void);
void getI(void);
void getV(void);
void showI(void);

int main(void)
 {

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    INIT();
    SetClock_MCLK12MHZ_SMCLK12MHZ_ACLK32_768K();          //系统时钟为25MHZ
    SPWM_Initial();
    __enable_interrupt();
    while(1)
    {
        showf();
        showI();
        getV();
    }

}
void INIT(void)
{
    P4REN |= BIT0 + BIT1 + BIT2 + BIT3;
    P4OUT |= BIT0 + BIT1 + BIT2 + BIT3;
    OLED_Init();
    P4DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3);
    P4DIR |= BIT4 + BIT5 + BIT6 + BIT7;//初始化OLED显示屏和按键的接口
    OLED_Clear(0,7);//清屏
    OLED_ShowString(10,4,"A");
    OLED_ShowString(20,4,":");
    OLED_ShowString(10,2,"V");
    OLED_ShowString(20,2,":");
    OLED_ShowString(10,0,"F");
    OLED_ShowString(20,0,":");

    P8DIR |= BIT2;
    P8OUT &= ~BIT2;

    ADS1118_init();

    TA0CCR0 = 10800;
    TA0CTL = TASSEL_1 + MC_1 + TACLR;         // SMCLK, up mode, clear TAR
    TA0CCTL0 = CCIE;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    __enable_interrupt();
    getI();
}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR(void)
{
    P5OUT &= ~BIT2;
    __delay_cycles(t);
    P5OUT |= BIT3;
    TA1CCTL1 &= ~CCIFG;
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
    P5OUT &= ~BIT3;
    __delay_cycles(t);
    P5OUT |= BIT2;
    TA1CCTL0 &= ~CCIFG;
    count_sin += step_sin;
    Index_sin = count_sin >> 8;
    if(b==0)
        TA1CCR1 = 8*sin[Index_sin]+75;
    else
        TA1CCR1 = 8*tri[Index_sin]+80;
}

void showI(void)
{
    OLED_ShowNum(40,4,f0,1,5);
    OLED_ShowString(50,4,".");
    OLED_ShowNum(60,4,f1,1,5);
    OLED_ShowNum(70,4,g1,1,5);
    OLED_ShowNum(80,4,f3,1,5);
    if(f>1500)
    {
        P8OUT |= BIT2;
        h0=1;
    }
    if(h0!=0)
    {
        if(h2==1700)
        {
            h2=0;
            if(h1==4)
            {
                h1=0;
                h0=0;
                P8OUT &= ~BIT2;
            }
            else
                h1++;
        }
        else
        {
            h2++;
        }
    }
}

void showf(void)
{
    KeyHead();
    OLED_ShowNum(40,0,x[0],1,5);
    OLED_ShowNum(50,0,x[1],1,5);
    OLED_ShowNum(80,0,a,1,5);
    if(x[0]==0&x[1]==0)
        c=100;
    else
        c=10*x[0]+x[1];
    step_sin=(c*280)/50;
}

void getV()
{
    int c1,c2,c3,c0,c00;
    d1=(long int)(10008.3-273.6*d);
    c3=d1%10;
    c2=((d1-c3)/10)%10;
    c1=((d1-c3-c2*10)/100)%10;
    c0=((d1-c3-c2*10-c1*100)/1000)%10;
    c00=(d1-c3-c2*10-c1*100-c0*1000)/10000;
    OLED_ShowNum(30,2,c00,1,5);
    OLED_ShowNum(40,2,c0,1,5);
    OLED_ShowString(50,2,".");
    OLED_ShowNum(60,2,c1,1,5);
    OLED_ShowNum(70,2,c2,1,5);
    OLED_ShowNum(80,2,c3,1,5);
}

void getI()
{
    e=ADS1118_Get_Current();
    d=e;
    e=e*1206.7+21.4;
    f=(int)e;
    f3=f%10;
    f2=((f-f3)/10)%10;
    f1=((f-f3-f2*10)/100)%10;
    f0=(f-f3-f2*10-f1*100)/1000;
    g1=f2;
    if(f<295)
    {
        TA1CCR0=2160;
    }
    if(f<327)
    {
        TA1CCR0=2155;
    }
    else if(f<390)
    {
        TA1CCR0=2150;
    }
    else if(f<430)
    {
        TA1CCR0=2145;
    }
    else
    {
        TA1CCR0=2140;
    }
}
//值是一样的但列标不一样来区分不同列的键)
volatile unsigned char KeyVal; //键值
volatile unsigned char CF[4], Cont[4];
/*
 **描述：新型4X4按键扫描程序 放在1ms-10ms中断内使用（十分稳定不需要再写消抖程序）
 **备注：按键弹起时 keyVal = 0 单键按下 keyVal 有16个值，你自己程序可以针对不同值
 **进行不同程序操作 keyVal单键值分别为
 **0x11,0x12,0x14,0x18,
 **0x21,0x22,0x24,0x28,
 **0x31,0x32,0x34,0x38,
 **0x41,0x42,0x44,0x48,
 */
const unsigned char KeyOut[4] = { 0xef, 0xdf, 0xbf, 0x7f }; //4X4按输出端控制

void KeyHead() {
    static unsigned int ReadData[4];
    int i;
    for (i = 0; i < 16; i=i+4) {
        P4OUT = KeyOut[i/4] | 0x0f; //忽略低4位
        ReadData[i] = (P4IN | 0xf0) ^ 0xff;
        // CF[i] = ReadData[i] & (ReadData[i] ^ Cont[i]);
        // Cont[i] = ReadData[i];
//输出键值
        switch (ReadData[i]) //第i列
        {
        case 0x08:
            KeyVal = (i << 4 + 8);
            if(i<=4)
                x[a]=3+i;
            if(i==8)
                a=1;
            break;
        case 0x04:
            KeyVal = (i << 4 + 4);
            if(i<=4)
                x[a]=2+i;
            if(i==8)
                a=0;
            break;
        case 0x02:
            KeyVal = (i << 4 + 2);
            if(i<=8)
                x[a]=1+i;
            if(i==12)
                b=1;
            break;
        case 0x01:
            KeyVal = (i << 4 + 1);
            if(i<=8)
                x[a]=i;
            if(i==12)
                b=0;
            break;
        default:
            KeyVal = 0;
            break;
        }
    }
}
