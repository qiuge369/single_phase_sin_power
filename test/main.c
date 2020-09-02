/*2020��9��2�� 08:51:53��һ�γ���
 *
 *Ҫ�����Ƶ�ʷ�Χ20-100Hz����Ƶ�ʿɲ�������������ֵ������5Hz
 *��ô����ֵ��1Hz��
 * */

#include <msp430f6638.h>
#include "oled.h"
#include "bmp.h"
#include "key_button.h"
#include "setclock.h"
#include "pid_delta.h"
#include "q_ADS1118.h"


//��������
void initPWM(void);
void initPara();
float getVoltage();
void pidAdjust(float in_voltage);
void changePWM(int duty_value);
void DispFloatat(unsigned char x,unsigned char y,float dat,unsigned char len1,unsigned char len2 );
void my_key();
void suprotect(float vol);

//��������
int duty=0;//ռ�ձ�
PID_DELTA pid;        //����pid�ṹ�����
int dealtV=0;  //pid�����
int key_value;
double num=0;//����������ֵ


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
//    UCSCTL5|=DIVS_2;//ʹ��USCͳһʱ��ϵͳ����Ԥ��Ƶ����SMCLK����4��Ƶ������������1M

   initPWM();
   initPara();//��ʼֵ
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

/****************************PWM��ʼ�����*********************************/
void initPWM(void)//��
{
  P1DIR |= BIT2;
  P1SEL |= BIT2;        //ѡ��TA.1����

  P1DIR |= BIT3;
  P1SEL |= BIT3;        //ѡ��TA.1����

  TA0CTL |=TASSEL_2 + MC_3 + TACLR;//����A0������,ʱ��ԴSMCLK������ģʽ��ͬʱ���������//*���ü�����
  //TASSEL_2ѡ����SMCLK��MC_1����ģʽ�����������TACLR
  TA0CCTL0 = /*OUTMOD_7+*/  CCIE;//����ȽϼĴ���0��������ģʽΪ2��ͬʱʹ�ܶ�ʱ���жϣ�CCR0��Դ�жϣ���CCIE����ȽϼĴ�����ʹ������
  TA0CCR0 = 480;//����ȽϼĴ���,���ö�ʱ���ж�Ƶ��25K
  TA0CCTL1 |= OUTMOD_2; // TD0CCR1, Reset/Set
  TA0CCR1 = 240;             //ռ�ձ�CCR1/CCR0

  TA0CCTL2 |= OUTMOD_6; // TD0CCR2, Reset/Set
  TA0CCR2 = 240;             //ռ�ձ�CCR2/CCR0
}

/****************************���ó�ʼֵ*********************************/
void initPara()
{
  duty = 200;    //����ֵ����ȷ��
  pid.setPoint = 36;   ////�趨ֵ����ȷ��
  adjust_pid(&pid, 0.69, 0.029, 0);//����PIDϵ��
  adjust_pid_limit(&pid, -10, 10);//�趨PID������������Ʒ�Χ
  ADS1118_GPIO_Init();  //���ùܽţ�ģ��SPI������Vcc��GND��Ҫ6���ߣ���ȥ������Ҫ4���ߣ�����Ҫ�ܽ����ã�
  P8DIR |= BIT4;    //���������ܽ�
}

/****************************��������********************************/
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
/****************************��������ʾ����********************************/
//dat:����    len1:������λ��    len2:С����λ��
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
