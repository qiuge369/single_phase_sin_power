/*2020年9月2日 08:51:53第一次尝试
 *
 *要求：输出频率范围20-100Hz，且频率可步进调整，步进值不大于5Hz
 *那么步进值就1Hz吧
 * */

#include <msp430f6638.h>
#include "oled.h"
#include "bmp.h"
#include "key_button.h"
#include "setclock.h"
#include "pid_delta.h"
#include "q_ADS1118.h"


//函数声明
void initPWM(void);
void initPara();
float getVoltage();
void pidAdjust(float in_voltage);
void changePWM(int duty_value);
void DispFloatat(unsigned char x,unsigned char y,float dat,unsigned char len1,unsigned char len2 );
void my_key();
void suprotect(float vol);

//变量声明
int duty=0;//占空比
PID_DELTA pid;        //声明pid结构体变量
int dealtV=0;  //pid误差量
int key_value;
double num=0;//按键所得数值


int spwm[256]=
{
 1875,1912,1949,1985,2022,2059,2095,2131,2168,2204,2239,2275,2310,2346,2380,
 2415,2449,2483,2516,2549,2582,2614,2646,2677,2708,2739,2769,2798,2827,2855,
 2882,2909,2936,2961,2986,3011,3035,3058,3080,3101,3122,3142,3162,3180,3198,
 3215,3231,3246,3261,3274,3287,3299,3310,3321,3330,3339,3346,3353,3359,3364,
 3368,3371,3373,3375,3375,3375,3373,3371,3368,3364,3359,3353,3346,3339,3330,
 3321,3310,3299,3287,3274,3261,3246,3231,3215,3198,3180,3162,3142,3122,3101,
 3080,3058,3035,3011,2986,2961,2936,2909,2882,2855,2827,2798,2769,2739,2708,
 2677,2646,2614,2582,2549,2516,2483,2449,2415,2380,2346,2310,2275,2239,2204,
 2168,2131,2095,2059,2022,1985,1949,1912,1875,1838,1801,1765,1728,1691,1655,
 1619,1582,1546,1511,1475,1440,1404,1370,1335,1301,1267,1234,1201,1168,1136,
 1104,1073,1042,1011,981,952,923,895,868,841,814,789,764,739,715,692,670,649,
 628,608,588,570,552,535,519,504,489,476,463,451,440,429,420,411,404,397,391,
 386,382,379,377,375,375,375,377,379,382,386,391,397,404,411,420,429,440,451,
 463,476,489,504,519,535,552,570,588,608,628,649,670,692,715,739,764,789,814,
 841,868,895,923,952,981,1011,1042,1073,1104,1136,1168,1201,1234,1267,1301,1335,
 1370,1404,1440,1475,1511,1546,1582,1619,1655,1691,1728,1765,1801,1838
};

void main()
{
   WDTCTL=WDTPW+WDTHOLD;
   SetClock_MCLK12MHZ_SMCLK24MHZ_ACLK32_768K();//12MHz
//    UCSCTL5|=DIVS_2;//使用USC统一时钟系统进行预分频，将SMCLK进行4分频——————1M

   initPWM();
   initPara();//初始值
   OLED_Init();/*init OLED*/
   OLED_Clear(); /*clear OLED screen*/
   init_key();

   while(1)
   {

   }
}

#pragma vector=TIMERA0_VECTOR
__interrupt void TimerA(void)
{

}

/****************************PWM初始化输出*********************************/
void initPWM(void)//可
{
  P1DIR |= BIT2;
  P1SEL |= BIT2;        //选择TA.1功能

  P1DIR |= BIT3;
  P1SEL |= BIT3;        //选择TA.1功能

  TA0CTL |=TASSEL_2 + MC_3 + TACLR;//配置A0计数器,时钟源SMCLK，上升模式，同时清除计数器//*配置计数器
  //TASSEL_2选择了SMCLK，MC_1计数模式，，最后清零TACLR
  TA0CCTL0 = /*OUTMOD_7+*/  CCIE;//捕获比较寄存器0输出，输出模式为2，同时使能定时器中断（CCR0单源中断），CCIE捕获比较寄存器的使能配置
  TA0CCR0 = 480;//捕获比较寄存器,设置定时器中断频率25K
  TA0CCTL1 |= OUTMOD_2; // TD0CCR1, Reset/Set
  TA0CCR1 = 240;             //占空比CCR1/CCR0

  TA0CCTL2 |= OUTMOD_6; // TD0CCR2, Reset/Set
  TA0CCR2 = 240;             //占空比CCR2/CCR0
}

/****************************设置初始值*********************************/
void initPara()
{
  duty = 200;    //测试值？不确定
  pid.setPoint = 36;   ////设定值，不确定
  adjust_pid(&pid, 0.69, 0.029, 0);//调整PID系数
  adjust_pid_limit(&pid, -10, 10);//设定PID误差增量的限制范围
  ADS1118_GPIO_Init();  //配置管脚（模拟SPI，加上Vcc、GND需要6根线，除去这俩需要4根线，故需要管脚配置）
  P8DIR |= BIT4;    //过流保护管脚
}

/****************************按键函数********************************/
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
char a;
void DispFloatat(unsigned char x,unsigned char y,float dat,unsigned char len1,unsigned char len2 )
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
