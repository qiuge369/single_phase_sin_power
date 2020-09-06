/*单相正弦逆变电源双极性2020年9月5日 15:20:36第一次尝试
 *继电器P5.2
 *ADS换到另一行了
 * */

#include <msp430f6638.h>
#include "oled.h"
#include "bmp.h"
#include "key_button.h"
#include "setclock.h"
#include "pid_delta.h"
#include "q_ADS1118.h"

void my_key();
void DispFloatat(unsigned char x,unsigned char y,double dat,unsigned char len1,unsigned char len2 );
void initSPWM(void);
void init();
void getVoltage();
void SPWM_Set_Freq(unsigned int freq);

unsigned char sin[] = {0, 2 ,4 ,6 ,8,10, 12 ,14 ,16 ,18,20, 22 ,24 ,26 ,28,30,
                       32 ,34 ,36 ,38,40, 42 ,44 ,46 ,48,50, 52 ,54 ,56 ,58,60,
                       62 ,64 ,66 ,68,70, 72 ,74 ,76 ,78,80, 82 ,84 ,86 ,88,90,
                       92 ,94 ,96 ,98,100, 102 ,104 ,106 ,108,110, 112 ,114 ,116 ,118,120,
                       122 ,124 ,126 ,128,130, 132 ,134 ,136 ,138,140, 142 ,144 ,146 ,148,150,
                       152 ,154 ,156 ,158,160, 162 ,164 ,166 ,168,170, 172 ,174 ,176 ,178,180,
                       182 ,184 ,186 ,188,190, 192 ,194 ,196 ,198,200, 202 ,204 ,206 ,208,210,
                       212 ,214 ,216 ,218,220, 222 ,224 ,226 ,228,230, 232 ,234 ,236 ,238,240,
                       242 ,244 ,246 ,248,250, 252 ,252, 250 ,248,246 ,244, 242, 240 ,238 ,236,234,
                       232, 230 ,228 ,226 ,224,222, 220 ,218 ,216 ,214,212, 210 ,208 ,206 ,204,202,
                       200 ,198 ,196 ,194,192, 190 ,188 ,186 ,184,182, 180 ,178 ,176 ,174,172, 170 ,
                       168 ,166 ,164,162, 160 ,158 ,156 ,154,152, 150 ,148 ,146 ,144,142, 140 ,138 ,
                       136 ,134,132, 130 ,128 ,126 ,124,122, 120 ,118 ,116 ,114,112, 110 ,108 ,106 ,
                       104,102, 100 ,98 ,96 ,94,92, 90 ,88 ,86 ,84,82, 80 ,78 ,76 ,74,72, 70 ,68 ,66 ,
                       64,62, 60 ,58 ,56 ,54,52, 50 ,48 ,46 ,44,42, 40 ,38 ,36 ,34,32, 30 ,28 ,26 ,24,
                       22, 20 ,18 ,16 ,14,12, 10 ,8 ,6 ,4,2, 0};

#define t 5//死区时间
#define spwm_num 256
int key_value;
int sin_index=0;
int frequence=50;

int Value=0;
double Voltage=0;
double v_sum=0;
double Vrms=0;
double a[256]={0};
int i=0;
double max=0;

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	SetClock_MCLK12MHZ_SMCLK25MHZ_ACLK32_768K();          //系统时钟为25MHZ（25.26MHz）
    initSPWM();
    init();//初始化
    __enable_interrupt();//开启总中断
    SPWM_Set_Freq(100);
    while(1)
    {
        my_key();
//        getVoltage();
        //显示函数
        //读取电压函数
    }
}
void initSPWM(void)
{
    P9DIR |= BIT1;//PWM两路输出
    P9OUT &= ~BIT1;
    P9REN &= ~BIT1;


    P9DIR |= BIT2;
    P9OUT &= ~BIT2;
    P9REN &= ~BIT2;


    //TA1用来输出两路反向带死区的PWM波//
    TA1CCR0 = 1952;//50Hz
    TA1CCR1 = 256;//这个值应该是随意的吧？
    TA1CTL = TASSEL_2 + MC_1 + TACLR;         // SMCLK, up mode, clear TAR
    TA1CCTL0 = CCIE;
    TA1CCTL1 = CCIE;

}

/****************************设置初始值*********************************/
void init()
{
    OLED_Init();/*init OLED*/
    OLED_Clear(); /*clear OLED screen*/
    init_key();

    OLED_ShowString(0,0,"freq:");
    OLED_ShowString(0,2,"Vrms:");
    ADS1118_GPIO_Init();  //配置管脚（模拟SPI，加上Vcc、GND需要6根线，除去这俩需要4根线，故需要管脚配置）

    TA0CCR0 = 12500;//很大即可吧？
    TA0CTL = TASSEL_1 + MC_1 + TACLR;         // SMCLK, up mode, clear TAR
    TA0CCTL0 = CCIE;
}

int count=0;
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    __enable_interrupt();
//    if(count==10)
//    {
        getVoltage();
//    }
    OLED_ShowNum(96,0,frequence,3,16);
    DispFloatat(72,2,Voltage,2,3);//显示电压值
}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR(void)
{
    P9OUT &= ~BIT1;
    __delay_cycles(t);
    P9OUT |= BIT2;
    TA1CCTL1 &= ~CCIFG;
}

#pragma vector=TIMER1_A0_VECTOR//25.6K的频率，每中断256次就输出一个完整的正弦波，则输出正弦波的频率即为100Hz
__interrupt void TIMER1_A0_ISR(void)
{
    P9OUT &= ~BIT2;
    __delay_cycles(t);
    P9OUT |= BIT1;
    TA1CCTL0 &= ~CCIFG;
    TA1CCR1=sin[sin_index++];
    if(sin_index>256)
        sin_index=0;
}

/****************************设置频率*********************************/
void SPWM_Set_Freq(unsigned int freq)
{
    unsigned long freq_num;
    freq_num=25000000/(freq*spwm_num)-1;
    TA1CCR0 =freq_num;
}
/****************************读取电压值函数********************************/

void getVoltage()
{
        Value = Write_SIP(0xf38b);           //AD数值     Conversion Register
        Voltage=change_voltage(Value,4.096);
//        DispFloatat(72,2,Voltage,2,3);//显示电压值
        if(i<200)
        {
            a[i++]=Voltage;
            if(Voltage>max)
            max=Voltage;
        }
        else
            i=0;

}

/****************************按键函数********************************/
//A,B步进加减，步进值加减5Hz
void my_key()
{
    key_value= key();   /*scan Array_button, get the key value*/
            if(key_value!=0)
            {
                    switch(key_value)
                    {
                      case(1):
                                key_value=0;
                            break;
                      case(2):
                                key_value=0;
                            break;
                      case(3):
                                key_value=0;
                            break;
                      case(4)://A
                              if(frequence<100)
                                  frequence=frequence+5;
                              SPWM_Set_Freq(frequence);
                                key_value=0;
                            break;
                      case(5):
                                key_value=0;
                            break;
                      case(6):
                                key_value=0;
                            break;
                      case(7):
                                key_value=0;
                            break;
                        case(8)://B
                                if(frequence>5)
                                    frequence=frequence-5;
                                SPWM_Set_Freq(frequence);
                                key_value=0;
                            break;
                        case(9):
                                key_value=0;
                            break;
                        case(10):
                                key_value=0;
                            break;
                        case(11):
                                key_value=0;
                            break;
                        case(12)://C
                                key_value=0;
                            break;
                        case(13):
                                key_value=0;
                            break;
                        case(14):
                                key_value=0;
                            break;
                        case(15):
                                key_value=0;
                            break;
                        case(16)://D
                                key_value=0;
                            break;
                      default:break;
                    }
            }
}

/****************************浮点数显示函数********************************/
//dat:数据    len1:整数的位数    len2:小数的位数
const long numtab[]={
  1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000,10000000000};
void DispFloatat(unsigned char x,unsigned char y,double dat,unsigned char len1,unsigned char len2 )
{
    int dat1,dat2;
    dat1=(int)dat;
    dat2=(int)((dat-dat1)*numtab[len2]);
    OLED_ShowNum(x,y,dat1,len1,16);
    OLED_ShowString(x+8*len1,y, ".");
    if(dat2/numtab[len2-1]==0)
        {
            if(len2>2)
            {
                if(dat2/numtab[len2-2]==0){
                    OLED_ShowString(x+8*len1+8,y,"0");
                    OLED_ShowString(x+8*len1+16,y,"0");
                    OLED_ShowNum(x+8*len1+24,y,dat2,len2-2,16);
                }else{
                    OLED_ShowString(x+8*len1+8,y,"0");
                    OLED_ShowNum(x+8*len1+16,y,dat2,len2-1,16);
                }

            }  else{
                    OLED_ShowString(x+8*len1+8,y,"0");
                    OLED_ShowNum(x+8*len1+16,y,dat2,len2-1,16);
                }
        }
    else
        OLED_ShowNum(x+8*len1+8,y,dat2,len2,16);
}
