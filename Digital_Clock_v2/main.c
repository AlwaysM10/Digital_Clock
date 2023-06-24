#include <msp430.h> 


/**
 * main.c
 */
#define CALADC12_15V_30C  *((unsigned int *)0x1A1A)   // Temperature Sensor Calibration-30 C
                                                      //See device datasheet for TLV table memory mapping
#define CALADC12_15V_85C  *((unsigned int *)0x1A1C)   // Temperature Sensor Calibration-85 C

unsigned int temp;
volatile float temperatureDegC;
volatile float temperatureDegF;
int temtemp = 0;
int hour,minute,second=0;
int year=2022,month=2,day=14;
int timer_hour,timer_minute,timer_second=0;//��ʱ����ʱ����
int tmp_hour;//12h��
int work_pattern=0;//����ģʽ
int last_work_pattern=0;//��һ������ģʽ
int flash_data[6]={2022,2,19,0,0,0};
int set_time[14]={0};//����ʱ��
int alarm_clock_time[18]={0};//��������
int timer_time[6]={0};//���ü�ʱ��
int judge;/*�жϼ��������ֵ*/
int i=1;/*1-����1��2-����2��3-����3*/
int j1,j2,j3=0;/*j1-������������1��ʱ/��/�룬j2-������������2��ʱ/��/�룬j3-������������3��ʱ/��/��*/
int on_off=0;//���ƹرձ�������
int on_off1,on_off2,on_off3;/*����1��2��3�Ŀ���*/
int alarm_clock_light=0;//����ָʾ��
int k,l,m,n=0;/*k-24h����ʱ�䣬l-12h����AMʱ�䣬m-12h����PMʱ�䣬n-���ü�ʱ��ʱ��*/
int timer_pattern=0;/*0-����ʱ��&���㣬1-������2-�رգ�3-��ͣ*/
int set_time_pattern=0;/*0-24Сʱ������ʱ�䣬1-12Сʱ������AMʱ�䣬2-12Сʱ������PMʱ��*/

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	read_flash_int(0x001800, flash_data, 6);
	year=flash_data[0];
	month=flash_data[1];
	day=flash_data[2];
	hour=flash_data[3];
	minute=flash_data[4];
	second=flash_data[5];
	P3DIR |= BIT4;//��ʼ��LED
    P5DIR |= BIT0;
    P5DIR |= BIT1;
    P3DIR |= BIT3;
    P3DIR |= BIT5;
    P3DIR |= BIT7;
    P3DIR |= BIT6;
    P3DIR |= BIT2;
    P3REN |= BIT4;//����LED
    P5REN |= BIT0;
    P5REN |= BIT1;
    P3REN |= BIT3;
    P3REN |= BIT5;
    P3REN |= BIT7;
    P3REN |= BIT6;
    P3REN |= BIT2;
    OLED_Init();/*init OLED*/
    OLED_Clear(); /*clear OLED screen*/
    init_key();/*init keyboard*/
//    SetClock_MCLK12MHZ_SMCLK12MHZ_ACLK32_768K();
    SMCLK_XT2_4Mhz();
    UCSCTL5|=DIVS__32;//ʹ��USCͳһʱ��ϵͳ����Ԥ��Ƶ����SMCLK����32��Ƶ

    TA0CTL |= ID__4 + TASSEL_2 + MC_1 + TACLR;//4��Ƶ
    TA0CCTL1 = OUTMOD_2 ;
    TA0CCTL0 = CCIE;
    TA0CCR0=3125;
    OLED_Clear();
    __enable_interrupt();//����ȫ���ж�
    __bis_SR_register(GIE);//ʹ��ȫ���ж�
    int key_value=0;
    while(1){
        key_value=key();
        if(key_value != 0)
        {
        _delay_cycles(200000);
        }
        Clock_Carry();
        flash_data[0]=year;
        flash_data[1]=month;
        flash_data[2]=day;
        flash_data[3]=hour;
        flash_data[4]=minute;
        flash_data[5]=second;
        write_flash_int(0x001800, flash_data, 6);
        judge=Judge_I_Want_Key_Value(key_value);//��ȡ����Ҫ�ļ���ֵ
        Choose_Work_Pattern(key_value);//ѡ����ģʽ
        if(last_work_pattern!=work_pattern){
            OLED_Clear();//�л�����ģʽʱ����
            Turn_Off_Week_Indicator_Light();//�л�����ģʽʱ�ر�����ָʾ��
            j1=0;j2=0;j3=0;    //�л�����ģʽʱ����������1��2��3����ʱ����ʱ��Ľ�λ��������
            k=0;l=0;m=0;n=0;
        }
        last_work_pattern=work_pattern;
        /*ʱ��ģʽ*/
        if(work_pattern==0){                          //24Сʱ�ƹ���ģʽ
            Clock_24h();
            if(on_off==1)Alarm_Clock_Indicator_Light();
            Control_Alarm_Clock_Indicator_Light();
        }
        if(work_pattern==1){                          //12Сʱ�ƹ���ģʽ
            Clock_12h();
            REFCTL0 &= ~REFMSTR;                      // Reset REFMSTR to hand over control to
                                                          // ADC12_A ref control registers
            ADC12CTL0 = ADC12SHT0_8 + ADC12REFON + ADC12ON;
                                                          // ���òο���ѹΪ1.5V����AD
            ADC12CTL1 = ADC12SHP;                     // ����������������Ϊ�ڲ���ʱ��
            ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10;  // AD��A10ͨ���������¶ȴ��������
            ADC12IE = 0x001;                          // ADC_IFG upon conv result-ADCMEMO
            __delay_cycles(100);                      // Allow ~100us (at default UCS settings)
                                                          // for REF to settle
            ADC12CTL0 |= ADC12ENC;                    // ʹ��AD
            ADC12CTL0 &= ~ADC12SC;
            ADC12CTL0 |= ADC12SC;                   // ��ʼ����

            __bis_SR_register(LPM4_bits + GIE);     // LPM0 with interrupts enabled
            __no_operation();

            // Temperature in Celsius. See the Device Descriptor Table section in the
                         // System Resets, Interrupts, and Operating Modes, System Control Module
                         // chapter in the device user's guide for background information on the
                         // used formula.
            temperatureDegC = (float)(((long)temp - CALADC12_15V_30C) * (85 - 30)) /
                               (CALADC12_15V_85C - CALADC12_15V_30C) + 30.0f; //���϶Ȼ���

            temtemp = temperatureDegC*100;
            OLED_ShowString(57,0,".");
            OLED_ShowNum(41,0,temtemp/100,2,16);
            OLED_ShowNum(65,0,temtemp%100,2,16);    //�¶���ʾ
            OLED_ShowCHinese(81,0,21);
            OLED_ShowCHinese(97,0,22);
            OLED_ShowCHinese(113,0,23);
            // Temperature in Fahrenheit Tf = (9/5)*Tc + 32
            temperatureDegF = temperatureDegC * 9.0f / 5.0f + 32.0f;//���϶Ȼ���
            _delay_cycles(10000);
            __no_operation();                       // SET BREAKPOINT HERE
            if(on_off==1)Alarm_Clock_Indicator_Light();
            Control_Alarm_Clock_Indicator_Light();
        }
        /*����ģʽ*/
        if(work_pattern==2){
            Alarm_Clock();
            on_off=1;
            if(i==1){                                //����1
                Current_Alarm_Clock1_Display();
                Alarm_Clock1_Operate();
            }
            if(i==2){                                //����2
                Current_Alarm_Clock2_Display();
                Alarm_Clock2_Operate();
            }
            if(i==3){                                //����3
                Current_Alarm_Clock3_Display();
                Alarm_Clock3_Operate();
            }
        }
        /*��ʱ��ģʽ*/
        if(work_pattern==3){
            Timer_Display();
            Timer_Carry();
            if(timer_pattern==0){                    //��ʼģʽ&����
                Timer_Operate();
                Moment_Display(timer_time[0]*10+timer_time[1],timer_time[2]*10+timer_time[3],timer_time[4]*10+timer_time[5]);
            }else
                Moment_Display(timer_hour,timer_minute,timer_second);
        }
        /*����ʱ��ģʽ*/
        if(work_pattern==4){
            if(set_time_pattern==0)Set_Time_24h();
            if(set_time_pattern==1)Set_Time_12h_AM();
            if(set_time_pattern==2)Set_Time_12h_PM();
        }
    }
	return 0;
}

#pragma vector = TIMER0_A0_VECTOR//TA0CCR0�жϷ�����
__interrupt void TIMER0_A0_ISR(void)   //
{
    static int cnt=0;
    cnt++;
    if(cnt==10){
        cnt=0;
        second++;//ʱ��
        if(alarm_clock_light==1)P3OUT^=BIT2;
        if(timer_pattern==1&&!(timer_second==0&&timer_minute==0&timer_hour==0))timer_second--;//��ʱ������ģʽ
        else if(timer_pattern==2){                                                            //��ʱ���ر�ģʽ
            timer_second=timer_time[4]*10+timer_time[5];
            timer_minute=timer_time[2]*10+timer_time[3];
            timer_hour=timer_time[0]*10+timer_time[1];
        }
        else if(timer_pattern==4){                                                            //��ʱ������ģʽ
            timer_hour=0;
            timer_minute=0;
            timer_second=0;
        }
        else;                                                                                 //��ʱ����ͣģʽ
    }

}

#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void)
{
  switch(__even_in_range(ADC12IV,34))
  {
  case  0: break;                           // Vector  0:  No interrupt
  case  2: break;                           // Vector  2:  ADC overflow
  case  4: break;                           // Vector  4:  ADC timing overflow
  case  6:                                  // Vector  6:  ADC12IFG0
    temp = ADC12MEM0;                       // ��ȡ������жϱ�־�ѱ����
    __bic_SR_register_on_exit(LPM4_bits);   // Exit active CPU
  case  8: break;                           // Vector  8:  ADC12IFG1
  case 10: break;                           // Vector 10:  ADC12IFG2
  case 12: break;                           // Vector 12:  ADC12IFG3
  case 14: break;                           // Vector 14:  ADC12IFG4
  case 16: break;                           // Vector 16:  ADC12IFG5
  case 18: break;                           // Vector 18:  ADC12IFG6
  case 20: break;                           // Vector 20:  ADC12IFG7
  case 22: break;                           // Vector 22:  ADC12IFG8
  case 24: break;                           // Vector 24:  ADC12IFG9
  case 26: break;                           // Vector 26:  ADC12IFG10
  case 28: break;                           // Vector 28:  ADC12IFG11
  case 30: break;                           // Vector 30:  ADC12IFG12
  case 32: break;                           // Vector 32:  ADC12IFG13
  case 34: break;                           // Vector 34:  ADC12IFG14
  default: break;
  }
}

/*function ����ģʽѡ����*/
void Choose_Work_Pattern(int key_value1){
    if(key_value1!=0&&key_value1==4){
        if(work_pattern==0){         //24Сʱ��ģʽת12Сʱ��ģʽ
            work_pattern=1;
        }
        else {                       //24Сʱ��ģʽת12Сʱ��ģʽ
            work_pattern=0;
        }
    }

    if(key_value1!=0&&key_value1==15&&(work_pattern==0||work_pattern==1)){    //�رձ�������
        if(work_pattern==0){
            on_off=0;
            alarm_clock_light=0;
        }
        else {
            on_off=0;
            alarm_clock_light=0;
        }
    }
    if(key_value1!=0&&key_value1==8){
        if(work_pattern==2&&i==1)i=2;     //����2
        else if(work_pattern==2&&i==2)i=3;//����3
        else {
            work_pattern=2;               //����1
            i=1;
        }
    }
    if(key_value1!=0&&key_value1==12){
        if(work_pattern=3&&timer_pattern==1){work_pattern=3;timer_pattern=3;}//��ʱ����ͣģʽ
        else if(work_pattern=3&&timer_pattern==3){work_pattern=3;timer_pattern=1;}//��ʱ������ģʽ
        else {
            work_pattern=3;//��ʱ����ʼģʽ&����
            timer_pattern=0;
        }
    }
    //��ʱ������
    if(key_value1!=0&&key_value1==14&&work_pattern==3&&(timer_pattern==1||timer_pattern==2||timer_pattern==3)){
        timer_time[0]=0;
        timer_time[1]=0;
        timer_time[2]=0;
        timer_time[3]=0;
        timer_time[4]=0;
        timer_time[5]=0;
        n=0;
        timer_pattern=0;
        work_pattern==3;
    }
    //��ʱ���ر�
    if(key_value1!=0&&key_value1==13&&work_pattern==3&&(timer_pattern==1||timer_pattern==3)){
        timer_pattern=2;
        work_pattern==3;
    }
    if(key_value1!=0&&key_value1==16){
        if(work_pattern==4&&set_time_pattern==0)set_time_pattern=1;     //12Сʱ������AMʱ��
        else if(work_pattern==4&&set_time_pattern==1)set_time_pattern=2;//12Сʱ������PMʱ��
        else{                                                           //24Сʱ������ʱ��
            work_pattern=4;
            set_time_pattern=0;
        }
    }
}

void write_flash_int (unsigned int addr, int *array, int count)
{
    unsigned int *Flash_ptr;            // ����Flashָ��
    int i;

    // Flash�Ĳ���
    Flash_ptr = (unsigned int*)addr;    // ��ʼ��Flashָ��
    FCTL1 = FWKEY + ERASE;              // �����Flash�ν��в���
    FCTL3 = FWKEY;                      // ����(Lock = 0)
    *Flash_ptr = 0;                     // ����һ�ο�д����������

    // дFlash
    FCTL1 = FWKEY + WRT;                // ����д
    for(i = 0; i<count; i++)            // ��count��int��������д��ָ����ַ��
    {
        *Flash_ptr++ = array[i];
    }
    FCTL1 = FWKEY;                      // ���д����λ
    FCTL3 = FWKEY + LOCK;               // ����(Lock = 1)
}

void read_flash_int(unsigned int addr, int *array, int count)
{
    int *address = (int *)addr;
    int i;
    for(i = 0; i<count; i++)
    {
        array[i] = *address++;          // address+=2
    }
}
/*function ʱ�Ӻ�����ʾ����*/
void Clock_Display(){
    OLED_ShowCHinese(0,0,2);    /*2 ->ʱ*/
    OLED_ShowCHinese(16,0,0);   /*0 ->��*/
}
/*function ���Ӻ�����ʾ����*/
void Alarm_Clock_Display(){
    OLED_ShowCHinese(0,0,1);    /*1 ->��*/
    OLED_ShowCHinese(16,0,0);   /*0 ->��*/
}
/*function ��ʱ��������ʾ����*/
void Timer_Display(){
    OLED_ShowCHinese(0,0,3);    /*3 ->��*/
    OLED_ShowCHinese(16,0,2);   /*2 ->ʱ*/
    OLED_ShowCHinese(32,0,4);   /*4 ->��*/
}
/*function ��������ʾ����*/
void Date_Display(int _year,int _month,int _day){
    OLED_ShowNum(0,4,_year,4,16);
    if(_year/1000==0)OLED_ShowChar(0,2,' ');
    if(_year/100==0)OLED_ShowChar(0,2,'  ');
    if(_year/10==0)OLED_ShowChar(0,2,'  ');
    OLED_ShowCHinese(32,4,5);
    if(!((_month==1||_month==3||_month==5||_month==7||_month==8||_month==10||_month==12)&&(_day>0&&_day<32)||(_month==4||_month==6||_month==9||_month==11)&&(_day>0&&_day<31)||(_year%100!=0&&_year%4==0||_year%100==0&&_year%400==0)&&_month==2&&(_day>0&&_day<30)||!(_year%100!=0&&_year%4==0||_year%100==0&&_year%400==0)&&_month==2&&(_day>0&&_day<29)))
        return;                    /*function �ж������Ƿ�����*/
     else if(_month>9&&_day>9){
        OLED_ShowNum(48,4,_month,2,16);
        OLED_ShowNum(80,4,_day,2,16);
        OLED_ShowCHinese(64,4,6);
        OLED_ShowCHinese(96,4,7);
    }else if(_month>9&&_day<10){
        OLED_ShowNum(48,4,_month,2,16);
        OLED_ShowNum(80,4,_day,1,16);
        OLED_ShowCHinese(64,4,6);
        OLED_ShowCHinese(88,4,7);
    }else if(_month<10&&_day>9){
        OLED_ShowNum(48,4,_month,1,16);
        OLED_ShowNum(72,4,_day,2,16);
        OLED_ShowCHinese(56,4,6);
        OLED_ShowCHinese(88,4,7);
    }else{
        OLED_ShowNum(48,4,_month,1,16);
        OLED_ShowNum(72,4,_day,1,16);
        OLED_ShowCHinese(56,4,6);
        OLED_ShowCHinese(80,4,7);
    }
}
/*function ����ʱ��ʱ��������ʾ����*/
void Set_Date_Display(int _year,int _month,int _day){
    OLED_ShowNum(0,4,_year,4,16);
    OLED_ShowCHinese(32,4,5);
    OLED_ShowCHinese(64,4,6);
    OLED_ShowCHinese(96,4,7);
    OLED_ShowNum(48,4,_month,2,16);
    OLED_ShowNum(80,4,_day,2,16);
    OLED_ShowCHinese(64,4,6);
    OLED_ShowCHinese(96,4,7);
}
/*function ������ʾ����*/
void Week_Display(int _week){
    if(_week<0||_week>6)return;
    else{
    OLED_ShowCHinese(0,6,8);    /*8 ->��*/
    OLED_ShowCHinese(16,6,9);   /*9 ->��*/
    if(_week!=0){OLED_ShowCHinese(32,6,_week+9);}/*week+9 ->һ~����*/
    else{OLED_ShowCHinese(32,6,7);}/*7 ->��*/
    switch(_week){                /*function LEDָʾ���ڼ�*/
    case 0: P3OUT &= ~BIT4;break;
    case 1: P5OUT &= ~BIT0;break;
    case 2: P5OUT &= ~BIT1;break;
    case 3: P3OUT &= ~BIT3;break;
    case 4: P3OUT &= ~BIT5;break;
    case 5: P3OUT &= ~BIT7;break;
    case 6: P3OUT &= ~BIT6;break;
    default: break;
    }
    }
}
/*function ���չ�ʽ����*/
int Zeller(){
    int year1,month1;
    if(month==1||month==2){year1=year-1;month1=month+12;}
    else {year1=year;month1=month;}
    int week=(year1%100+year1%100/4+year1%100/100/4-2*year1%100/100+26*(month1+1)/10+day-1)%7;
    return week;
}

/*function ʱ������ʾ����*/
void Moment_Display(int _hour,int _minute,int _second){
    OLED_ShowNum(0,2,_hour,2,16);
    OLED_ShowNum(24,2,_minute,2,16);
    OLED_ShowNum(48,2,_second,2,16);
    if(_hour<10)OLED_ShowNum(0,2,0,1,16);
    if(_minute<10)OLED_ShowNum(24,2,0,1,16);
    if(_second<10)OLED_ShowNum(48,2,0,1,16);
    OLED_ShowChar(16,2,':');
    OLED_ShowChar(40,2,':');
}

/*function AM&PM��ʾ����*/
void Ante_Post_Meridiem(int _hour){
    if(_hour<12){
        OLED_ShowString(96,2,"AM");
    }else {
        OLED_ShowString(96,2,"PM");
    }
}
/*function 24Сʱ��ģʽ���AM&PM��ʾ*/
void Ante_Post_Meridiem_Remove(){
    OLED_ShowChar(96,2,' ');
    OLED_ShowChar(104,2,' ');
}
/*function 24-12Сʱ��ת������*/
int Ante_Post_Meridiem_Transform(int _hour){
    int tmp=0;
    if(_hour>0&&_hour<13)tmp=_hour ;
    else if(_hour==0)tmp=_hour+12;
    else tmp=_hour-12;
    return tmp;
}

/*function �صƺ���*/
void Turn_Off_Week_Indicator_Light(){              //�л�����ģʽʱ�ر�����ָʾ��
    P3OUT |=BIT4;
    P5OUT |=BIT0;
    P5OUT |=BIT1;
    P3OUT |=BIT3;
    P3OUT |=BIT5;
    P3OUT |=BIT7;
    P3OUT |=BIT6;
}
/*function ����1,2,3��ʾ����*/
void Alarm_Clock_Time_Display(){
    /*����1*/
    OLED_ShowNum(0,2,alarm_clock_time[0]*10+alarm_clock_time[1],2,16);
    OLED_ShowNum(24,2,alarm_clock_time[2]*10+alarm_clock_time[3],2,16);
    OLED_ShowNum(48,2,alarm_clock_time[4]*10+alarm_clock_time[5],2,16);
    OLED_ShowChar(16,2,':');
    OLED_ShowChar(40,2,':');
    /*����2*/
    OLED_ShowNum(0,4,alarm_clock_time[6]*10+alarm_clock_time[7],2,16);
    OLED_ShowNum(24,4,alarm_clock_time[8]*10+alarm_clock_time[9],2,16);
    OLED_ShowNum(48,4,alarm_clock_time[10]*10+alarm_clock_time[11],2,16);
    OLED_ShowChar(16,4,':');
    OLED_ShowChar(40,4,':');
    /*����3*/
    OLED_ShowNum(0,6,alarm_clock_time[12]*10+alarm_clock_time[13],2,16);
    OLED_ShowNum(24,6,alarm_clock_time[14]*10+alarm_clock_time[15],2,16);
    OLED_ShowNum(48,6,alarm_clock_time[16]*10+alarm_clock_time[17],2,16);
    OLED_ShowChar(16,6,':');
    OLED_ShowChar(40,6,':');
}

/*function ��ǰ����ָʾ����*/
void Current_Alarm_Clock1_Display(){
    OLED_ShowChar(64,2,'^');
    OLED_ShowChar(64,4,' ');
    OLED_ShowChar(64,6,' ');
}
void Current_Alarm_Clock2_Display(){
    OLED_ShowChar(64,2,' ');
    OLED_ShowChar(64,4,'^');
    OLED_ShowChar(64,6,' ');
}
void Current_Alarm_Clock3_Display(){
    OLED_ShowChar(64,2,' ');
    OLED_ShowChar(64,4,' ');
    OLED_ShowChar(64,6,'^');
}

/*function ����ָʾ�ƺ���*/          //�ж�ʱ���Ƿ����е���������
void Alarm_Clock_Indicator_Light(){
    if(on_off1==1&&second<=20&&(hour==alarm_clock_time[0]*10+alarm_clock_time[1]&&minute==alarm_clock_time[2]*10+alarm_clock_time[3]))
        alarm_clock_light=1;
    else if(on_off2==1&&second<=20&&(hour==alarm_clock_time[6]*10+alarm_clock_time[7]&&minute==alarm_clock_time[8]*10+alarm_clock_time[9]))
        alarm_clock_light=1;
    else if(on_off3==1&&second<=20&&(hour==alarm_clock_time[12]*10+alarm_clock_time[13]&&minute==alarm_clock_time[14]*10+alarm_clock_time[15]))
        alarm_clock_light=1;
    else alarm_clock_light=0;
}
/* function ����&�ر�����ָʾ��*/
void Control_Alarm_Clock_Indicator_Light(){
    if(alarm_clock_light==1)P3OUT&=~BIT2;
    else P3OUT|=BIT2;
}

/*function ���ӿ�&����ʾ����*/
void Alarm_Clock1_On_Off_Display(){
    if(on_off1)OLED_ShowCHinese(96,2,16);   /*16 ->��*/
    else OLED_ShowCHinese(96,2,17);         /*17 ->��*/
}
void Alarm_Clock2_On_Off_Display(){
    if(on_off2)OLED_ShowCHinese(96,4,16);
    else OLED_ShowCHinese(96,4,17);
}
void Alarm_Clock3_On_Off_Display(){
    if(on_off3)OLED_ShowCHinese(96,6,16);
    else OLED_ShowCHinese(96,6,17);
}

/*function ���õ�һ������*/
void Alarm_Clock1_Operate(){
    if(judge>=0&&judge<=2&&j1==0){
        alarm_clock_time[0]=judge;
    }
    if(((judge>=0&&judge<=9&&(alarm_clock_time[0]==0||alarm_clock_time[0]==1))||(judge>=0&&judge<=3&&alarm_clock_time[0]==2))&&j1==1){
        alarm_clock_time[1]=judge;
    }
    if(judge>=0&&judge<=5&&j1==2){
        alarm_clock_time[2]=judge;
    }
    if(judge>=0&&judge<=9&&j1==3){
        alarm_clock_time[3]=judge;
    }
    if(judge==13)j1++;
    if(judge==14&&on_off1){on_off1=0;return;}
    if(judge==14&&!on_off1){on_off1=1;return;}
}

/*function ���õڶ�������*/
void Alarm_Clock2_Operate(){
    if(judge>=0&&judge<=2&&j2==0){
        alarm_clock_time[6]=judge;
    }
    if(((judge>=0&&judge<=9&&(alarm_clock_time[6]==0||alarm_clock_time[6]==1))||(judge>=0&&judge<=3&&alarm_clock_time[6]==2))&&j2==1){
        alarm_clock_time[7]=judge;
    }
    if(judge>=0&&judge<=5&&j2==2){
        alarm_clock_time[8]=judge;
    }
    if(judge>=0&&judge<=9&&j2==3){
        alarm_clock_time[9]=judge;
    }
    if(judge==13)j2++;
    if(judge==14&&on_off2){on_off2=0;return;}
    if(judge==14&&!on_off2){on_off2=1;return;}
}

/*function ���õ���������*/
void Alarm_Clock3_Operate(){
    if(judge>=0&&judge<=2&&j3==0){
        alarm_clock_time[12]=judge;
    }
    if(((judge>=0&&judge<=9&&(alarm_clock_time[12]==0||alarm_clock_time[12]==1))||(judge>=0&&judge<=3&&alarm_clock_time[12]==2))&&j3==1){
        alarm_clock_time[13]=judge;
    }
    if(judge>=0&&judge<=5&&j3==2){
        alarm_clock_time[14]=judge;
    }
    if(judge>=0&&judge<=9&&j3==3){
        alarm_clock_time[15]=judge;
    }
    if(judge==13)j3++;
    if(judge==14&&on_off3){on_off3=0;return;}
    if(judge==14&&!on_off3){on_off3=1;return;}
}

/*function ���ü�ʱ��*/
void Timer_Operate(){
    if(judge>=0&&judge<=9&&n==0){
        timer_time[0]=judge;
    }
    if(judge>=0&&judge<=9&&n==1){
        timer_time[1]=judge;
    }
    if(judge>=0&&judge<=5&&n==2){
        timer_time[2]=judge;
    }
    if(judge>=0&&judge<=9&&n==3){
        timer_time[3]=judge;
    }
    if(judge>=0&&judge<=5&&n==4){
        timer_time[4]=judge;
    }
    if(judge>=0&&judge<=9&&n==5){
        timer_time[5]=judge;
    }
    if(judge==13){n++;}
    if(judge==14){
        timer_hour=timer_time[0]*10+timer_time[1];
        timer_minute=timer_time[2]*10+timer_time[3];
        timer_second=timer_time[4]*10+timer_time[5];
        n=0;
        timer_pattern=1;
    }
}

/*function ����ʱ�人����ʾ����*/
void Set_Time_Display(){
    OLED_ShowCHinese(0,0,18);    /*18 ->��*/
    OLED_ShowCHinese(16,0,19);   /*19 ->��*/
    OLED_ShowCHinese(32,0,2);   /*2 ->ʱ*/
    OLED_ShowCHinese(48,0,20);   /*20 ->��*/
}

/*function 24Сʱ��������ʱ�亯��*/
void Set_Time_24h_Operate(){
    if(judge>=0&&judge<=9&&k==0){
        set_time[0]=judge;
    }
    if(judge>=0&&judge<=9&&k==1){
        set_time[1]=judge;
    }
    if(judge>=0&&judge<=9&&k==2){
        set_time[2]=judge;
    }
    if(judge>=0&&judge<=9&&k==3){
        set_time[3]=judge;
    }
    if(judge>=0&&judge<=1&&k==4){
        set_time[4]=judge;
    }
    if(((judge>=1&&judge<=9&&set_time[4]==0)||(judge>=0&&judge<=2&&set_time[4]==1))&&k==5){
        set_time[5]=judge;
    }
    if(judge>=0&&judge<=3&&k==6){
        set_time[6]=judge;
    }
    if(judge>=0&&judge<=9&&k==7){
        set_time[7]=judge;
    }
    if(judge>=0&&judge<=2&&k==8){
        set_time[8]=judge;
    }
    if(((judge>=0&&judge<=9&&(set_time[8]==0||set_time[8]==1))||(judge>=0&&judge<=3&&set_time[8]==2))&&k==9){
        set_time[9]=judge;
    }
    if(judge>=0&&judge<=5&&k==10){
        set_time[10]=judge;
    }
    if(judge>=0&&judge<=9&&k==11){
        set_time[11]=judge;
    }
    if(judge>=0&&judge<=5&&k==12){
        set_time[12]=judge;
    }
    if(judge>=0&&judge<=9&&k==13){
        set_time[13]=judge;
    }
    if(judge==13)k++;
    if(judge==14){
        year=set_time[0]*1000+set_time[1]*100+set_time[2]*10+set_time[3];
        month=set_time[4]*10+set_time[5];
        day=set_time[6]*10+set_time[7];
        hour=set_time[8]*10+set_time[9];
        minute=set_time[10]*10+set_time[11];
        second=set_time[12]*10+set_time[13];
        k=0;
        work_pattern=0;
    }
}

/*function 12Сʱ��������AMʱ�亯��*/
void Set_Time_12h_AM_Operate(){
    if(judge>=0&&judge<=9&&l==0){
        set_time[0]=judge;
    }
    if(judge>=0&&judge<=9&&l==1){
        set_time[1]=judge;
    }
    if(judge>=0&&judge<=9&&l==2){
        set_time[2]=judge;
    }
    if(judge>=0&&judge<=9&&l==3){
        set_time[3]=judge;
    }
    if(judge>=0&&judge<=1&&l==4){
        set_time[4]=judge;
    }
    if(((judge>=1&&judge<=9&&set_time[4]==0)||(judge>=0&&judge<=2&&set_time[4]==1))&&l==5){
        set_time[5]=judge;
    }
    if(judge>=0&&judge<=3&&l==6){
        set_time[6]=judge;
    }
    if(judge>=0&&judge<=9&&l==7){
        set_time[7]=judge;
    }
    if(judge>=0&&judge<=1&&l==8){
        set_time[8]=judge;
    }
    if(((judge>=0&&judge<=9&&set_time[8]==0)||(judge>=0&&judge<=2&&set_time[8]==1))&&l==9){
        set_time[9]=judge;
    }
    if(judge>=0&&judge<=5&&l==10){
        set_time[10]=judge;
    }
    if(judge>=0&&judge<=9&&l==11){
        set_time[11]=judge;
    }
    if(judge>=0&&judge<=5&&l==12){
        set_time[12]=judge;
    }
    if(judge>=0&&judge<=9&&l==13){
        set_time[13]=judge;
    }
    if(judge==13)l++;
    if(judge==14){
        year=set_time[0]*1000+set_time[1]*100+set_time[2]*10+set_time[3];
        month=set_time[4]*10+set_time[5];
        day=set_time[6]*10+set_time[7];
        if(set_time[8]*10+set_time[9]==12)hour=0;
        else hour=set_time[8]*10+set_time[9];
        minute=set_time[10]*10+set_time[11];
        second=set_time[12]*10+set_time[13];
        l=0;
        work_pattern=0;
    }
}

/*function 12Сʱ��������PMʱ�亯��*/
void Set_Time_12h_PM_Operate(){
    if(judge>=0&&judge<=9&&m==0){
        set_time[0]=judge;
    }
    if(judge>=0&&judge<=9&&m==1){
        set_time[1]=judge;
    }
    if(judge>=0&&judge<=9&&m==2){
        set_time[2]=judge;
    }
    if(judge>=0&&judge<=9&&m==3){
        set_time[3]=judge;
    }
    if(judge>=0&&judge<=1&&m==4){
        set_time[4]=judge;
    }
    if(((judge>=1&&judge<=9&&set_time[4]==0)||(judge>=0&&judge<=2&&set_time[4]==1))&&m==5){
        set_time[5]=judge;
    }
    if(judge>=0&&judge<=3&&m==6){
        set_time[6]=judge;
    }
    if(judge>=0&&judge<=9&&m==7){
        set_time[7]=judge;
    }
    if(judge>=0&&judge<=1&&m==8){
        set_time[8]=judge;
    }
    if(((judge>=0&&judge<=9&&set_time[8]==0)||(judge>=0&&judge<=2&&set_time[8]==1))&&m==9){
        set_time[9]=judge;
    }
    if(judge>=0&&judge<=5&&m==10){
        set_time[10]=judge;
    }
    if(judge>=0&&judge<=9&&m==11){
        set_time[11]=judge;
    }
    if(judge>=0&&judge<=5&&m==12){
        set_time[12]=judge;
    }
    if(judge>=0&&judge<=9&&m==13){
        set_time[13]=judge;
    }
    if(judge==13)m++;
    if(judge==14){
        year=set_time[0]*1000+set_time[1]*100+set_time[2]*10+set_time[3];
        month=set_time[4]*10+set_time[5];
        day=set_time[6]*10+set_time[7];
        if(set_time[8]*10+set_time[9]!=12)hour=set_time[8]*10+set_time[9]+12;
        else hour=12;
        minute=set_time[10]*10+set_time[11];
        second=set_time[12]*10+set_time[13];
        m=0;
        work_pattern=0;
    }
}

/*function ������ʱ�����λ����*/
void Clock_Carry(){
    if(second>=60){second=0;minute++;}
    if(minute>=60){minute=0;hour++;}
    if(hour>=24){minute=0;hour=0;day++;}
    if((month==1||month==3||month==5||month==7||month==8||month==10||month==12)&&day>=32){month++;day=1;}//����31��
    if(month==13)month=1;
    if((month==4||month==6||month==9||month==11)&&day>=31){month++;day=1;}//С��30��
    if(!(year%100!=0&&year%4==0||year%100==0&&year%400==0)&&month==2&&day>=29){month++;day=1;}//ƽ��2��28��
    if((year%100!=0&&year%4==0||year%100==0&&year%400==0)&&month==2&&day>=30){month++;day=1;}//����2��29��
}

/*function ��ʱ��ʱ�����λ����*/
void Timer_Carry(){
    if(timer_second==-1&&(timer_minute!=0||timer_hour!=0)){timer_second=59;timer_minute-=1;}
    if(timer_minute==-1&&timer_hour!=0){timer_minute=59;timer_hour-=1;}
    if(timer_hour==-1&&timer_minute==-1&&timer_second==-1){timer_hour=0;timer_minute=0;timer_second=0;}//��ʱ������
}

/*function 24Сʱ��ʱ�ӹ���ģʽ*/
void Clock_24h(){
    Clock_Display();
    Date_Display(year,month,day);
    Week_Display(Zeller());
    Moment_Display(hour,minute,second);
    Ante_Post_Meridiem_Remove();
}

/*function 12Сʱ��ʱ�ӹ���ģʽ*/
void Clock_12h(){
    Clock_Display();
    Date_Display(year,month,day);
    Week_Display(Zeller());
    Ante_Post_Meridiem(hour);
    tmp_hour=Ante_Post_Meridiem_Transform(hour);
    Moment_Display(tmp_hour,minute,second);
}

/*function ���ӹ���ģʽ*/
void Alarm_Clock(){
    Alarm_Clock_Display();
    Alarm_Clock_Time_Display();
    Alarm_Clock1_On_Off_Display();
    Alarm_Clock2_On_Off_Display();
    Alarm_Clock3_On_Off_Display();
}

/*function 24Сʱ��������ʱ�乤��ģʽ*/
void Set_Time_24h(){
    Set_Time_Display();
    Moment_Display(set_time[8]*10+set_time[9],set_time[10]*10+set_time[11],set_time[12]*10+set_time[13]);
    Set_Date_Display(set_time[0]*1000+set_time[1]*100+set_time[2]*10+set_time[3],set_time[4]*10+set_time[5],set_time[6]*10+set_time[7]);
    Ante_Post_Meridiem_Remove();
    Set_Time_24h_Operate();
}

/*function 12Сʱ��������AMʱ�乤��ģʽ*/
void Set_Time_12h_AM(){
    Set_Time_Display();
    Moment_Display(set_time[8]*10+set_time[9],set_time[10]*10+set_time[11],set_time[12]*10+set_time[13]);
    Set_Date_Display(set_time[0]*1000+set_time[1]*100+set_time[2]*10+set_time[3],set_time[4]*10+set_time[5],set_time[6]*10+set_time[7]);
    OLED_ShowString(96,2,"AM");
    Set_Time_12h_AM_Operate();
}

/*function 12Сʱ��������PMʱ�乤��ģʽ*/
void Set_Time_12h_PM(){
    Set_Time_Display();
    Moment_Display(set_time[8]*10+set_time[9],set_time[10]*10+set_time[11],set_time[12]*10+set_time[13]);
    Set_Date_Display(set_time[0]*1000+set_time[1]*100+set_time[2]*10+set_time[3],set_time[4]*10+set_time[5],set_time[6]*10+set_time[7]);
    OLED_ShowString(96,2,"PM");
    Set_Time_12h_PM_Operate();
}
/*function ��ȡ����Ҫ�ļ���ֵ*/
int Judge_I_Want_Key_Value(int key_value2){
    int judge=-1;
    switch(key_value2)
    {
    case 1:judge=1;break;
    case 2:judge=2;break;
    case 3:judge=3;break;
    case 4:judge=10;break;
    case 5:judge=4;break;
    case 6:judge=5;break;
    case 7:judge=6;break;
    case 8:judge=11;break;
    case 9:judge=7;break;
    case 10:judge=8;break;
    case 11:judge=9;break;
    case 12:judge=12;break;
    case 13:judge=13;break;
    case 14:judge=0;break;
    case 15:judge=14;break;
    case 16:judge=15;break;
    default:break;
    }
    return judge;
}

void SMCLK_XT2_4Mhz(void)
{

    P7SEL |= BIT2+BIT3;                       // Port select XT2
    UCSCTL6 &= ~XT2OFF;          // Enable XT2
    UCSCTL6 &= ~XT2OFF + XT2DRIVE_1;          // Enable XT2
    UCSCTL3 |= SELREF_2;                      // FLLref = REFO
                                              // Since LFXT1 is not used,
                                              // sourcing FLL with LFXT1 can cause
                                              // XT1OFFG flag to set
    UCSCTL4 |= SELA_2;                        // ACLK=REFO,SMCLK=DCO,MCLK=DCO

    // Loop until XT1,XT2 & DCO stabilizes - in this case loop until XT2 settles
    do
    {
      UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
                                              // Clear XT2,XT1,DCO fault flags
      SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

    UCSCTL6 &= ~XT2DRIVE0;                    // Decrease XT2 Drive according to
                                              // expected frequency
    UCSCTL4 |= SELS_5 + SELM_5;               // SMCLK=MCLK=XT2
}
