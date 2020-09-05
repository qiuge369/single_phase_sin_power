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
#include "Math.h"


//函数声明
void initSPWM(void);//初始化SPWM
void initPara();
void getVoltage();
void pidAdjust(double in_voltage);
void DispFloatat(unsigned char x,unsigned char y,float dat,unsigned char len1,unsigned char len2 );
void my_key();
void suprotect(float vol);
void SPWM_Set_Freq(unsigned int freq);
unsigned char MiddlevalueFilter();//中值滤波函数

#define spwm_num  256
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
int v_i=0;//采样一整个周期电压的计数

int spwm[spwm_num]=
{
 50,51,52,54,55,56,57,59,60,61,62,63,65,66,67,68,
 69,70,71,72,74,75,76,77,78,79,80,81,82,83,84,84,
 85,86,87,88,89,89,90,91,92,92,93,94,94,95,95,96,
 96,97,97,97,98,98,99,99,99,99,99,100,100,100,100,
 100,100,100,100,100,100,100,99,99,99,99,99,98,98,
 97,97,97,96,96,95,95,94,94,93,92,92,91,90,89,89,
 88,87,86,85,84,84,83,82,81,80,79,78,77,76,75,74,
 72,71,70,69,68,67,66,65,63,62,61,60,59,57,56,55,
 54,52,51,50,49,48,46,45,44,43,41,40,39,38,37,35,
 34,33,32,31,30,29,28,26,25,24,23,22,21,20,19,18,
 17,16,16,15,14,13,12,11,11,10,9,8,8,7,6,6,5,5,4,
 4,3,3,3,2,2,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,
 1,1,1,2,2,3,3,3,4,4,5,5,6,6,7,8,8,9,10,11,11,12,
 13,14,15,16,16,17,18,19,20,21,22,23,24,25,26,28,
 29,30,31,32,33,34,35,37,38,39,40,41,43,44,45,46,
 48,49
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
   OLED_ShowString(0,2,"Vrms:");

   __enable_interrupt();//开启总中断

//   while(1)while进不来，只能用中断
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
    TA0CCR1 =K*spwm[spwm_1++];//第一路
    if(spwm_1==spwm_num)
        spwm_1=0;
    TA0CCR2 =K*spwm[spwm_2++];//第二路
    if(spwm_2==spwm_num)
        spwm_2=0;

    my_key();
    getVoltage();
    OLED_ShowNum(96,0,frequence,3,16);
    TA1IV=0;//中断清零
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
  spwm_2=5;//相位相差
  TA0CTL |=TASSEL_2 + MC_3 + TACLR;//配置A0计数器,时钟源SMCLK，上升模式，同时清除计数器//*配置计数器
  //TASSEL_2选择了SMCLK，MC_1计数模式，，最后清零TACLR
  TA0CCR0 = 100;//载波250K
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
    freq_num=25000000/(freq*spwm_num)-1;
    TA1CCR0 =freq_num;
}
/****************************设置初始值*********************************/
void initPara()
{
  pid.setPoint = 1;   ////设定值
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
double v_sum=0;
double Vrms=0;
double a[256]={0};
void getVoltage()
{
        Value = Write_SIP(0xf38b);           //AD数值     Conversion Register
        Voltage=change_voltage(Value,4.096);
        Voltage=Voltage-1;//减去直流偏置
        v_sum+=Voltage*Voltage;
        a[v_i]=Voltage;
        v_i++;
        if(v_i==spwm_num)
        {
            v_i=0;
            Vrms =sqrt(v_sum / spwm_num);
            v_sum=0;
            DispFloatat(72,2,Vrms,2,3);//显示电压值
        }
}

//针对中值滤波的电压采样函数
int Value_2=0;
double Voltage_2=0;
double get_ad()
{
        Value_2 = Write_SIP(0xf38b);           //AD数值     Conversion Register
        Voltage_2=change_voltage(Value,4.096);
        return Voltage_2;
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
                __delay_cycles(125000000);//延时5S
                P8OUT &= ~BIT4;        //置高
            }
        }
    else
        c_i=0;
}

/****************************中值滤波函数********************************/
unsigned char MiddlevalueFilter()

{
  int NUM=11;
  double value_buf[NUM];
  int i,j,k;
  double temp;
  for(i=0;i<NUM;i++)
  {
    value_buf[i] = get_ad();
    //这需要延时吗
  }
  for (j=0;j<NUM-1;j++)
  {
   for (k=0;k<NUM-j;k++)
   {
    if(value_buf[k]>value_buf[k+1])
     {
       temp = value_buf[k];
       value_buf[k] = value_buf[k+1];
       value_buf[k+1] = temp;
     }
   }
  }
  return value_buf[(NUM-1)/2];
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
