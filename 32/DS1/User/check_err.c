//-----------------------------------------------------------------
// ��������:
// �������ϼ����������
// ��������: ���ǵ���
// ��ʼ����: 2019-11-15
// �������: 2019-12-18
// �޸�����: 2019-12-26
// �桡����: V1.0
// ��ʷ�汾:
// ��- V1.0: 
// ���Թ���: STM32F103��2.8��Һ����
// ˵������: 
//				
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// ͷ�ļ�����
//-----------------------------------------------------------------
#include <stm32f10x.h>
#include "Delay.h"
#include "Timing Sampling.h"
#include "tjc_usart_hmi.h"
#include "RLCtest.h"
#include "check_err.h"
//#include "gpio.h"
#include "stdio.h"
#include "AD9833.h"

//-----------------------------------------------------------------
// ��������
//-----------------------------------------------------------------
char buf[25];
u32 ad2,DC,DC_LOAD;
u32 gain,ri;	
float ad1,vs,RS_v=0;
extern u32 Gain;
extern uint16_t       Data_Rms[2082];					//�ɼ����� 
extern uint16_t       Data_Rms1[198];					//�ɼ����� 
extern volatile u16 Pulse_Val;
extern double freq_high,freq_low,Au_db;
extern double freq_high1,freq_low1;
extern double freq_high2,freq_low2,Au_db1;
extern u32 Ri,AC_ref,AC_ref1,AC_ref2,AC_ref3,AC_FH,AC_FL,RS_FL,RS_ref;//RS_ref��ʾ1KHz��������ʱ���ӵ������˷Ŵ��ĵ�ѹֵ
extern float RS_ref1,RS_FL1; 

//-----------------------------------------------------------------
// ��������
//-----------------------------------------------------------------
u8 ERR_c(void);
u8 ERR_r2r3(void);
//-----------------------------------------------------------------
// u8 check_err (void)
//-----------------------------------------------------------------
//
// ��������: ���ϼ��
// ��ڲ���: ��
// ���ز���: ��������
// ע������: ��
//-----------------------------------------------------------------
u8 check_err(void)
{
	u8 err;
	err = check_err1();
	if(err)
		return err;
	return 0;
}
//-----------------------------------------------------------------
//u8 check_err1(void)
//-----------------------------------------------------------------
//
// ��������: ���ϼ��
// ��ڲ���: ��
// ���ز���: ��������
// ע������: 1.��������
//           2.���ϼ��
//-----------------------------------------------------------------
u8 check_err1(void)
{	
	u8 i;	
	u8 err ;
	u16 AD0_data;	
	err = Check_OK;
	freq_high = 150;
	freq_low = 0.420;	
	Gate_Set;         											// ����
	Delay_ms(30);
	AD9833_SetFrequency(AD9833_REG_FREQ0, 1000 , AD9833_OUT_SINUS, 0);//AD9833_DDS����1KHz�����ź�
	
	// ��������˴���Ĳ����������˵ĵ�ѹ 
	Delay_ms(5);
	ADC_TIM3_Init(100,1);										// ��ʱ1.39us�ɼ�һ������
	AD0_data=0;															// ��ʼADC�ɼ�ֵΪ0
	DMA_Cmd(DMA1_Channel1, ENABLE);					// ʹ��DMA 
	ADC_Cmd(ADC1, ENABLE);				 					// ����ADC1
	while(!adc_finish_fg);												// �ȴ�ADC1�ɼ����
	AD0_data=ADC_MATH();										// ��ȡ���ֵ														
	adc_finish_fg=0;															// �ɼ���ɱ�־λ����
	vs=1.0*AD0_data/(495);						     	// ʵ�ʷŴ������������495��, ���Ǵ���У׼ϵ��
	ad1=495*vs;   													// ���ʱRs���˽����ź�
	RS_v=ad1;   														// RS_v��ʾ������ʱ���ӵ������˷Ŵ��ĵ�ѹֵ

	// ��������˿���ʱ�����������ѹ
	ad2=0;																	
	for(i=0;i<20;i++)
	{
		ADC_Sampling ();											// ADC2�������ݲɼ�����ȡ���ֵ
		ad2+=Adc_data[1];    					  // ���ɼ�20��
	}
	ad2/=20;
  ad2=ad2*1;														 // ���ؽ�����ѹ�ʵ�У׼
	gain=ad2/(12-vs);										 	 // ��������ֵ, vs������������˵ĵ�ѹ����ѹֵ��С��
	
	// ��������˿���ʱ�����ֱ����ѹ
	DC=0;
	for(i=0;i<20;i++)
	{
		ADC3_Sampling();											// ADC3�������ݲɼ�����ȡ���ֱ��ֵ
		DC+=Adc_data[2];    							// ���ɼ�20��
	}
	DC/=20;
	//VCC = 12183;
	DC = DC*4.57;                					  // ��ֱ���Ľ��е�ѹ�任, ��Ϊֱ�����뱻˥����
//*******************************************//
//	������ʾ����ֱ�۹۲����ֵ
//*******************************************//
	TJCPrintf( "n1.val=%d", (int)DC);
	TJCPrintf( "n3.val=%d", (int)ad2);
	TJCPrintf( "n4.val=%d", (int)gain);
	TJCPrintf( "n2.val=%d", (int)ad1);
	//sprintf((char *)buf,"DC:%5dmV",DC);
	//LCD_WriteString(30,80,YELLOW,BLACK,(u8 *)buf);
	//sprintf((char *)buf,"ad2:%5dmV",ad2);
	//LCD_WriteString(30,100,YELLOW,BLACK,(u8 *)buf);
	//sprintf((char *)buf,"gain:%3d",gain);
	//LCD_WriteString(30,120,YELLOW,BLACK,(u8 *)buf);
	//sprintf((char *)buf,"ad1:%3f ",ad1);
	//LCD_WriteString(30,140,YELLOW,BLACK,(u8 *)buf);

//*******************************************//
//	������ϼ��
//*******************************************//	
	
	if (DC <= 52)    																	// ������ֱ���ӽ�0Vʱ
		err = ERR_R4_Short;															// ����: R4��·

	// ���ݹ����ж�
	else if((DC >= DC_ref-100)&&(DC <= DC_ref+100))   // ֱ����ѹ��������̬������DC_ref����ʱ��ͨ����Ƶ���й����ж�
	{
		err=ERR_c();
	}
	
  //R2��·
	else if((DC >=VCC/4)&&(DC < DC_ref-100))   				//���ֱ����С��3V��5.2V
	{
		err = ERR_R2_Open;
	}
	
	//R1��·
	else if((DC >= VCC-1300)&&(DC < VCC-500))   			//���ֱ����11.3V���ң�����R2��·С����mV
	{
		err = ERR_R1_Short;
	}
	
	//R3��·
	else if((DC>VCC/230)&&(DC<=VCC/6.6))							// ֱ����52mv��1.8V֮��ΪR3��·
		err = ERR_R3_Open;
	
	//R2,R3��· R1,R4��·
	else if((DC <= VCC+500)&&(DC >= VCC-500))  				//ֱ�����ڵ�Դ��ѹVCC(12V)������Ҫ�жϵ�����(�ж������迹�ı仯)
	{
		err = ERR_r2r3();
	}
	return err;
}

//-----------------------------------------------------------------
// u8 ERR_c(void)
//-----------------------------------------------------------------
//
// ��������: ���ݹ��ϼ��
// ��ڲ���: ��
// ���ز���: ���ݹ�������
// ע������: ��
//-----------------------------------------------------------------
u8 ERR_c(void)
{
	u8  errc ;
  freq_high = 150;
	freq_low = 0.420;	
  PoinTFreTest();													// ���õ�Ƶ���Ժ���, �õ�����Ƶ�ʵ�ɼ�ֵ, �Ա�������ֵ�Ƚϵó����
  TJCPrintf( "n0.val=%d", (int)RS_ref1);
	//sprintf((char *)buf,"RS_ref1:%5f, ",);
	TJCPrintf( "n5.val=%d", (int)RS_FL1);
	//LCD_WriteString(140,220,YELLOW,BLACK,(u8 *)buf);
	//sprintf((char *)buf,"RS_FL1:%5f, ",RS_FL1);
	//LCD_WriteString(140,200,YELLOW,BLACK,(u8 *)buf);
	// 2C1/2C2/2C3/C3��·�����ֹ��ϲ�Ӱ�������ܷŴ���, ������������ʱ���������С�������
		if(ad2 >= AC_ref-100 ) 									// ���ݹ���ʱ��õ��������ad2����������ʱ�������AC_ref���бȽ�
		{
			freq_high1 = 0;
			freq_low1 = 0;
			Delay_ms(10);
			if(AC_FH > AC_ref2+50)								// AC_FHΪ������ʱ160kHzƵ����������������ѹ; AC_ref2Ϊ��������ʱ160kHzƵ����������������ѹ 								
				errc = ERR_C3_Open;									// ����: C3��·
			else if(AC_FH < AC_ref2-50) 
			{
				errc = ERR_C3_Double;								// ����: C3��2C3
			}
			else 																	// RS_FL1->4Hz�źŵ�Ƶ����ʱRs���˵�ѹ, RS_ref1->4Hz�ź���������ʱRs���˵�ѹ
				if((RS_FL1 >= RS_ref1+20)&&(RS_FL1 <= RS_ref1+100))	// C1��2��C1�ĵ�ѹֵ���26mvpp����
				{
					errc = ERR_C1_Double;							// ����: C1��2C1
				}			
			if(AC_FL > AC_ref1+40)								// AC_FL��ʾ����ʱ400Hz��Ƶ�źŵ��������,AC_ref1Ϊ��AC_FLͬƵ��ʱ�����������������
					errc = ERR_C2_Double;							// ����: C2��2C2, ����ʱ����AC_FL��AC_ref1��ֵ��ʾ�������ı��·����������ʵ��ֵ��������
		}
		// C1��·/C2��·�����ֹ��ϼ���Ӱ�������ܷŴ���, ������������ʱ���������СҪС�ܶ�
		else 
			if(ad2<AC_ref-100)
			{
				if((ad1>=RS_ref/4)&&(ad1<2000))    	// ����ʱRs���˵�ѹad1������ʱRs���˵�ѹRS_ref���бȽ�(�ķ�֮һRS_ref�պÿ����ٽ�ֵ),��ҪС��2v����
					 errc = ERR_C2_Open;								// ����: C2��·
				else 
					if(ad1<RS_ref/11)									// Rs��ѹ(ad1)С��100�ӽ�0ʱΪC1��·
					   errc = ERR_C1_Open;							// ����: C1��·
			}
  return errc;  
}
//-----------------------------------------------------------------
// u8 ERR_r2r3(void)
//-----------------------------------------------------------------
//
// ��������: R2,R3��·R1,R4��·
// ��ڲ���: ��
// ���ز���: ��������
// ע������: ��
//-----------------------------------------------------------------
u8 ERR_r2r3(void)
{
	 u8  errc;
   ri=1.0*(2000*(US-vs))/vs;    									// ���ݷ�ѹ��ʽ
//*******************************************//
//	������׼ʱ����ȡ������ע�ͺ���е����޸�
//*******************************************//		
//		sprintf((char *)buf,"ri:%5d, ",ri);								//�����迹����ֵ
//		LCD_WriteString(140,140,YELLOW,BLACK,(u8 *)buf);
//		sprintf((char *)buf,"Ri:%5d, ",Ri);								//�����迹����ֵ
//		LCD_WriteString(140,160,YELLOW,BLACK,(u8 *)buf);
//		sprintf((char *)buf,"RS_ref:%5d, ",RS_ref);				//������������ֵ
//		LCD_WriteString(140,100,YELLOW,BLACK,(u8 *)buf);
//		sprintf((char *)buf,"RS_v:%5f, ",RS_v);						//�����������ֵ
//		LCD_WriteString(140,120,YELLOW,BLACK,(u8 *)buf);
	Gate_Set;   																	    	// ����
	if(ri < Ri-400)     																// �д�ʱ��õ��������ri������ʱ���������Ri�Ƚ�
		errc = ERR_R2_Short; 
	else 
		if(ri > Ri+600)
		{
			if((int)RS_v < (RS_ref/3.33))								    // RS_v��ʾ������ʱ���ӵ������˷Ŵ��ĵ�ѹֵ
			errc = ERR_R1_Open;
			else  if((int)RS_v >= (RS_ref/3.22)&&(RS_v<=RS_ref-100)) // ��ΪRS�˵�ѹС����������RS_ref����R1_Open��R4_Open
			errc = ERR_R4_Open;	
		}
		else  
		{			
			Gate_Clr;																						// ���ز����ֱ��
			Delay_ms(50);
			ADC3_Sampling();
			DC_LOAD=Adc_data[2];   												// ����ʱ��ֱ�������ѹ
			DC_LOAD = DC_LOAD*4.57;															// Ӳ����·������˥��������
			if(DC_LOAD >= VCC-500)														// ����ֱ�����ڵ���VCC����R3��·
			{errc = ERR_R3_Short;}
			TJCPrintf( "n6.val=%d", (int)DC_LOAD);
			//sprintf((char *)buf,"DC_LOAD:%d, ",DC_LOAD);			//�����������ֵ
	   // LCD_WriteString(140,120,YELLOW,BLACK,(u8 *)buf);
		}
		return errc;
 }

//-----------------------------------------------------------------
// void display_err_info(u8 err)
//-----------------------------------------------------------------
//
// ��������: ����������ʾ
// ��ڲ���: ��������
// ���ز���: ��
// ע������: ��
//-----------------------------------------------------------------
void display_err_info(u8 err)
{
	switch (err)
	{
		case ERR_R1_Short:
			TJCPrintf( "t%s.txt=\"R1 is short\"", "3");
			break;
		case ERR_R2_Short:
			TJCPrintf( "t%s.txt=\"R2 is short\"", "3");
			break;
		case ERR_R1_Open:
			TJCPrintf( "t%s.txt=\"R1 is open\"", "3");
			//Delay_ms(700);			
			break;
		case ERR_R2_Open:
			TJCPrintf( "t%s.txt=\"R2 is open\"", "3");
			break;
		case ERR_R3_Short:
			TJCPrintf( "t%s.txt=\"R3 is short\"", "3");
			break;
		case ERR_R3_Open:
			TJCPrintf( "t%s.txt=\"R3 is open\"", "3");
			break;
		case ERR_R4_Short:
			TJCPrintf( "t%s.txt=\"R4 is short\"", "3");
			break;
		case ERR_R4_Open:
			TJCPrintf( "t%s.txt=\"R4 is open\"", "3");
			//Delay_ms(700);			
			break;
		case ERR_C1_Open:
			TJCPrintf( "t%s.txt=\"C1 is open\"", "3");
			//Delay_ms(1000);		
			break;
		case ERR_C2_Open:
			TJCPrintf( "t%s.txt=\"C2 is open\"", "3");
		//Delay_ms(1000);	
			break;
		case ERR_C3_Open:
			TJCPrintf( "t%s.txt=\"C3 is open\"", "3");
			//Delay_ms(1000);	
			break;
		case ERR_C1_Double:
			TJCPrintf( "t%s.txt=\"C1 is double\"", "3");
			//Delay_ms(2000);	
			break;
		case ERR_C2_Double:
			TJCPrintf( "t%s.txt=\"C2 is double\"", "3");
			//Delay_ms(2000);	
			break;
		case ERR_C3_Double:
			TJCPrintf( "t%s.txt=\"C3 is double\"", "3");
			//Delay_ms(2000);	
			break;
		default:
			TJCPrintf( "t%s.txt=\"OK\"", "3");
		
			break;
	}
}
