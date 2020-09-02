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
void initSPWM(void);//初始化SPWM
void initPara();
void getVoltage();
void pidAdjust(double in_voltage);
void DispFloatat(unsigned char x,unsigned char y,float dat,unsigned char len1,unsigned char len2 );
void my_key();
void suprotect(float vol);

//变量声明
int duty=0;//占空比
PID_DELTA pid;        //声明pid结构体变量
int dealtV=0;  //pid误差量
int key_value;
double num=0;//按键所得数值

int spwm_1,spwm_2;
int frequence=100;
double K=1;//调制度，初始值假设电压值为准确输出1V
double dealtK=0;

int spwm[256]=
{
 488,500,512,524,536,548,560,571,583,595,607,618,630,641,652,664,675,686,697,
 707,718,729,739,749,759,769,779,788,798,807,816,824,833,841,850,858,865,873,
 880,887,894,900,907,913,918,924,929,934,939,943,947,951,955,958,961,964,967,
 969,971,972,974,975,975,976,976,976,975,975,974,972,971,969,967,964,961,958,
 955,951,947,943,939,934,929,924,918,913,907,900,894,887,880,873,865,858,850,
 841,833,824,816,807,798,788,779,769,759,749,739,729,718,707,697,686,675,664,
 652,641,630,618,607,595,583,571,560,548,536,524,512,500,488,476,464,452,440,
 428,416,405,393,381,369,358,346,335,324,312,301,290,279,269,258,247,237,227,
 217,207,197,188,178,169,160,152,143,135,126,118,111,103,96,89,82,76,69,63,58,
 52,47,42,37,33,29,25,21,18,15,12,9,7,5,4,2,1,1,0,0,0,1,1,2,4,5,7,9,12,15,18,21,
 25,29,33,37,42,47,52,58,63,69,76,82,89,96,103,111,118,126,135,143,152,160,169,
 178,188,197,207,217,227,237,247,258,269,279,290,301,312,324,335,346,358,369,381,
 393,405,416,428,440,452,464,476
};

void main()
{
   WDTCTL=WDTPW+WDTHOLD;
   SetClock_MCLK12MHZ_SMCLK25MHZ_ACLK32_768K();//12MHz
//    UCSCTL5|=DIVS_2;//使用USC统一时钟系统进行预分频，将SMCLK进行4分频——————1M

   initSPWM();
   initPara();//初始值
   OLED_Init();/*init OLED*/
   OLED_Clear(); /*clear OLED screen*/
   init_key();

   OLED_ShowString(0,0,"frequence:");
   OLED_ShowString(0,2,"voltage:");
   __enable_interrupt();//开启总中断

//   while(1)while进不来，只能用中断
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
    TA0CCR1 =K*spwm[spwm_1++];//第一路
    if(spwm_1==256)
        spwm_1=0;
    TA0CCR2 =K*spwm[spwm_2++];//第二路
    if(spwm_2==256)
        spwm_2=0;

    my_key();
//       getVoltage();
    OLED_ShowNum(96,0,frequence,3,16);
}

/****************************SPWM初始化输出*********************************
*功能：输出两路SPWM波，相位可调
* 输入：无
* 输出：P1.2,P1.3
* 注意：定时器也为TA1.1;载波频率为126.26K；；默认正弦频率为100HZ，幅度未知
*/
void initSPWM(void)
{
  P1DIR |= BIT2;//CCR2
  P1SEL |= BIT2;

  P1DIR |= BIT3;//CCR3
  P1SEL |= BIT3;
  spwm_1=0;
  spwm_2=1;//相位相差
  TA0CTL |=TASSEL_2 + MC_3 + TACLR;//配置A0计数器,时钟源SMCLK，上升模式，同时清除计数器//*配置计数器
  //TASSEL_2选择了SMCLK，MC_1计数模式，，最后清零TACLR
//  TA0CCTL0 = CCIE;//使能定时器中断（CCR0单源中断），CCIE捕获比较寄存器的使能配置
  TA0CCR0 = 198;//载波500K
  TA0CCTL1 |= OUTMOD_2;
  TA0CCR1 = spwm[spwm_1];

  TA0CCTL2 |= OUTMOD_6;
  TA0CCR2 = spwm[spwm_2];

  TA1CCR0 =976;////25000000/(256*100)=976。100Hz，256个点：25.6KHZ
  TA1CTL =TASSEL_2+MC_3+TACLR;//选择时钟为SMCLK，UP模式
  TA1CTL  |= TAIE;//开启中断
  TA1CCTL0 = CCIE;//使能定时器中断（CCR0单源中断），CCIE捕获比较寄存器的使能配置
}
/****************************设置频率*********************************/
void SPWM_Set_Freq(unsigned int freq)
{
    unsigned long freq_num;
    freq_num=25000000/(freq*256)-1;
    TA1CCR0 =freq_num;
}
/****************************设置初始值*********************************/
void initPara()
{
  pid.setPoint = 36;   ////设定值，不确定
  adjust_pid(&pid, 0, 0, 0);//调整PID系数
  adjust_pid_limit(&pid, -10, 10);//设定PID误差增量的限制范围
  ADS1118_GPIO_Init();  //配置管脚（模拟SPI，加上Vcc、GND需要6根线，除去这俩需要4根线，故需要管脚配置）

  P8DIR |= BIT4;    //过流保护管脚
}
/*****************************PID控制器*********************************/
void pidAdjust(double in_voltage)
{
  dealtK = PidDeltaCal(&pid,in_voltage);  //返回误差增量
  K=K+dealtK;
}
/****************************读取电压值函数********************************/
int Value=0;
double Voltage=0;
void getVoltage()
{
        Value = Write_SIP(0xe38b);           //AD数值     Conversion Register
        Voltage=change_voltage(Value,4.096);
        Voltage=Voltage*11.98;//-(1.519*current-0.1115)
        pidAdjust(Voltage);
        DispFloatat(72,2,Voltage,2,3);//显示电压值
}

/*****************************过流保护*********************************/
int c_i=0;
void suprotect(float vol)
{
    if(vol>1.625)
        {
            c_i++;
            if(c_i>10)
            {
                P8OUT |= BIT4;        //置高
                __delay_cycles(120000000);//延时5S？
                P8OUT &= ~BIT4;        //置高
            }
        }
    else
        c_i=0;
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
