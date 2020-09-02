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
void initSPWM(void);//��ʼ��SPWM
void initPara();
void getVoltage();
void pidAdjust(double in_voltage);
void DispFloatat(unsigned char x,unsigned char y,float dat,unsigned char len1,unsigned char len2 );
void my_key();
void suprotect(float vol);

//��������
int duty=0;//ռ�ձ�
PID_DELTA pid;        //����pid�ṹ�����
int dealtV=0;  //pid�����
int key_value;
double num=0;//����������ֵ

int spwm_1,spwm_2;
int frequence=100;
double K=1;//���ƶȣ���ʼֵ�����ѹֵΪ׼ȷ���1V
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
//    UCSCTL5|=DIVS_2;//ʹ��USCͳһʱ��ϵͳ����Ԥ��Ƶ����SMCLK����4��Ƶ������������1M

   initSPWM();
   initPara();//��ʼֵ
   OLED_Init();/*init OLED*/
   OLED_Clear(); /*clear OLED screen*/
   init_key();

   OLED_ShowString(0,0,"frequence:");
   OLED_ShowString(0,2,"voltage:");
   __enable_interrupt();//�������ж�

//   while(1)while��������ֻ�����ж�
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
    TA0CCR1 =K*spwm[spwm_1++];//��һ·
    if(spwm_1==256)
        spwm_1=0;
    TA0CCR2 =K*spwm[spwm_2++];//�ڶ�·
    if(spwm_2==256)
        spwm_2=0;

    my_key();
//       getVoltage();
    OLED_ShowNum(96,0,frequence,3,16);
}

/****************************SPWM��ʼ�����*********************************
*���ܣ������·SPWM������λ�ɵ�
* ���룺��
* �����P1.2,P1.3
* ע�⣺��ʱ��ҲΪTA1.1;�ز�Ƶ��Ϊ126.26K����Ĭ������Ƶ��Ϊ100HZ������δ֪
*/
void initSPWM(void)
{
  P1DIR |= BIT2;//CCR2
  P1SEL |= BIT2;

  P1DIR |= BIT3;//CCR3
  P1SEL |= BIT3;
  spwm_1=0;
  spwm_2=1;//��λ���
  TA0CTL |=TASSEL_2 + MC_3 + TACLR;//����A0������,ʱ��ԴSMCLK������ģʽ��ͬʱ���������//*���ü�����
  //TASSEL_2ѡ����SMCLK��MC_1����ģʽ�����������TACLR
//  TA0CCTL0 = CCIE;//ʹ�ܶ�ʱ���жϣ�CCR0��Դ�жϣ���CCIE����ȽϼĴ�����ʹ������
  TA0CCR0 = 198;//�ز�500K
  TA0CCTL1 |= OUTMOD_2;
  TA0CCR1 = spwm[spwm_1];

  TA0CCTL2 |= OUTMOD_6;
  TA0CCR2 = spwm[spwm_2];

  TA1CCR0 =976;////25000000/(256*100)=976��100Hz��256���㣺25.6KHZ
  TA1CTL =TASSEL_2+MC_3+TACLR;//ѡ��ʱ��ΪSMCLK��UPģʽ
  TA1CTL  |= TAIE;//�����ж�
  TA1CCTL0 = CCIE;//ʹ�ܶ�ʱ���жϣ�CCR0��Դ�жϣ���CCIE����ȽϼĴ�����ʹ������
}
/****************************����Ƶ��*********************************/
void SPWM_Set_Freq(unsigned int freq)
{
    unsigned long freq_num;
    freq_num=25000000/(freq*256)-1;
    TA1CCR0 =freq_num;
}
/****************************���ó�ʼֵ*********************************/
void initPara()
{
  pid.setPoint = 36;   ////�趨ֵ����ȷ��
  adjust_pid(&pid, 0, 0, 0);//����PIDϵ��
  adjust_pid_limit(&pid, -10, 10);//�趨PID������������Ʒ�Χ
  ADS1118_GPIO_Init();  //���ùܽţ�ģ��SPI������Vcc��GND��Ҫ6���ߣ���ȥ������Ҫ4���ߣ�����Ҫ�ܽ����ã�

  P8DIR |= BIT4;    //���������ܽ�
}
/*****************************PID������*********************************/
void pidAdjust(double in_voltage)
{
  dealtK = PidDeltaCal(&pid,in_voltage);  //�����������
  K=K+dealtK;
}
/****************************��ȡ��ѹֵ����********************************/
int Value=0;
double Voltage=0;
void getVoltage()
{
        Value = Write_SIP(0xe38b);           //AD��ֵ     Conversion Register
        Voltage=change_voltage(Value,4.096);
        Voltage=Voltage*11.98;//-(1.519*current-0.1115)
        pidAdjust(Voltage);
        DispFloatat(72,2,Voltage,2,3);//��ʾ��ѹֵ
}

/*****************************��������*********************************/
int c_i=0;
void suprotect(float vol)
{
    if(vol>1.625)
        {
            c_i++;
            if(c_i>10)
            {
                P8OUT |= BIT4;        //�ø�
                __delay_cycles(120000000);//��ʱ5S��
                P8OUT &= ~BIT4;        //�ø�
            }
        }
    else
        c_i=0;
}
/****************************��������********************************/
//A,B�����Ӽ�������ֵ�Ӽ�5Hz
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
