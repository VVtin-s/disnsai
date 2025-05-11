//-----------------------------------------------------------------
// ��������:
// ��������Ƶ�����Բ����ǹ��ܳ���
// ��������: ���ǵ���
// ��ʼ����: 2019-11-15
// �������: 2019-12-18
// �޸�����: 2019-12-26
// �桡����: V1.0
// ��- V1.0:  
// ˵������: 
//			
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// ͷ�ļ�����
//-----------------------------------------------------------------
#include <stm32f10x.h>
#include "Delay.h"
#include "matrix_keypad.h"
#include "Timing Sampling.h"
#include "tjc_usart_hmi.h"
#include "RLCtest.h"
#include <math.h>
#include <stdio.h>
#include "check_err.h"
#include "AD9833.h"

//-----------------------------------------------------------------
// ��������
//-----------------------------------------------------------------
uint16_t       Data_Rms[2082];					//�ɼ����� 
uint16_t       Data_v[500];							//�ɼ����� 
//static u8 buf[8];
u8 key_numb,A=0;
int G_db=0; 
u16 i;
u32 freq;
u16 AD1,AD2,AD3,AD5;					//AD1��AD2��AD3��AD5�Ƕ���ı���; 12000��ʾ12V����
//-----------------------------------------------------------------
//DC_ref->����������ֱ����ѹ; DC_sp->һ����ͨ����; AC_ref->��������ʱ���������ѹ; AC_ref1->400Hz��������ʱ���������ѹ
//AC_FH->160KHz������ʱ���������ѹ; AC_FL->400Hz������ʱ���������ѹ,RS_FL=0,RS_ref=0,AC_ref2->160KHz��������ʱ���������ѹ
//-----------------------------------------------------------------
u32 DC_ref=0,DC_sp,AC_ref=0,AC_ref1=0,AC_ref2=0,AC_FH=0,AC_FL=0,RS_FL=0,RS_ref=0,AC_ref3=0;
u32 Ri,Ro,Gain = 1,G_measur=0;							//Ri->�����迹; Ro->����迹; Gain->������; G_measur->�����ǵ�����;
u16 ADD1,ADD2;
u16 text[5];																		  
extern float vpp_vpp;
// AD0_data: �ɼ�������ADֵ; Vs->����Rs���˵Ľ�����ѹ; RS_ref1->4Hz�ź���������ʱRs���˵�ѹ,RS_FL1->4Hz�źŵ�Ƶ����ʱRs���˵�ѹ;
float AD1_data=0,AD0_data=0,Vs,VS,RS_ref1=0,RS_FL1=0;
float AD0_data1=0,AD0_data2=0,GRs=1,GRS=1;;
double freq_high,freq_low,Au_db;
double freq_high1,freq_low1;
double freq_high2,freq_low2,Au_db1;
//-----------------------------------------------------------------
// ��������
//-----------------------------------------------------------------
void getcaculation(void); 							//��ȡ��ѹֵ,���ֵ
void change_GRs (void);	  							//����У׼
void change_adc(void);    							//�Ŵ�������
void test(void);          							//���������������Ŵ�ʱ, ���벻ͬ�źŵ����������ѹֵ 
//-----------------------------------------------------------------
// void GPIO_Gate(void)
//-----------------------------------------------------------------
//
// ��������: ��ʼ������ģ��Ŀ���IO��
// ��ڲ���: ��
// ���ز���: ��
// ע������: 
//					 CD4051 CD1��CD2: PB8����>A,PB9����>B
//					 CD4051 CD3: PB10����>EN(B)
//-----------------------------------------------------------------
void GPIO_Gate(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;  // PC4����>A,PC5����>B,PC6����>EN
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;;									// �������
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
}
//-----------------------------------------------------------------
// ����ΪCD1��CD2����IO״̬��Ӧ�Ĺ���
//-----------------------------------------------------------------
void standard (void)   																							//��ʼ״̬�ͼ�����ʱ��CD4052����״̬(AΪ1��BΪ0)
{
	 Gate_Set_A; 
	 Gate_Clr_B;
}
void AFC (void)   																									// ���Ƶ����ʱ��CD4052����״̬(AΪ0��BΪ1)
{
	 Gate_Clr_A;
	 Gate_Set_B;
}
void G_original (void)   																						// �����������������·ʱҪִ�е�CD4052����״̬(��������������)(AΪ1��BΪ1)
{
	 Gate_Set_A; 
	 Gate_Set_B;
}
//-----------------------------------------------------------------
//void move(void)
//-----------------------------------------------------------------
//
// ��������:  ����ƶ�����
// ��ڲ���: ��
// ���ز���: ��
// ע������: 
//-----------------------------------------------------------------
//void move(void)
//{
//  if(flag==0)
//		{
//			switch (function_set)
//			{ 
//				case 0:{
//										function_set=1;
//										LCD_WriteString(240,80,YELLOW,BLACK, (u8 *)" <- ");		
//										LCD_ColorBox (245, 140, 20,	20	 , BLACK) ;
//								}break;
//				case 1:{
//										function_set=2;
//										LCD_WriteString(240,110,YELLOW,BLACK,(u8 *) " <- ");					
//					          LCD_ColorBox (245, 80, 20,	20	 , BLACK) ;				// ��x=245��y=80����20*20�ĺ�ɫ���Σ���ס��һ�εĹ��
//								}break;
//				case 2:{
//										function_set=3;
//										LCD_WriteString(240,140,YELLOW,BLACK, (u8 *)" <- ");
//										LCD_ColorBox (245, 110, 20,	20	 , BLACK) ;
//								}break;
//				case 3:{
//										function_set=0;
//										LCD_WriteString(240,140,YELLOW,BLACK, (u8 *)" <- ");
//										LCD_ColorBox (245, 140, 20,	20	 , BLACK) ;
//								}break;
//				default :	break;
//			}
//		}


//}

//-----------------------------------------------------------------
//void display1(void)
//-----------------------------------------------------------------
//
// ��������:����������ʾ
// ��ڲ���: ��
// ���ز���: ��
// ע������: 
//-----------------------------------------------------------------
//void display1(void)
//{
//  if(function_set==1)																									// ���а������£���һ������
//				{	
//					LCD_Clear( BLACK );         																// ���ñ���ɫ
//					LCD_WriteChinese24x24string(55,30,YELLOW, BLACK, (u8 *)"���׵�·���Բ�����") ;	// ����ԭ�������Ͻ�
//					flag=1;																											// �����־λ
//				}
//				if(function_set==2)																						// ���а������£��ڶ�������
//				{
//					LCD_Clear( BLACK );
//					LCD_WriteChinese24x24string(55,30,YELLOW, BLACK, (u8 *)"���׵�·���Բ�����") ;
//					LCD_WriteString(130, 30,YELLOW, BLACK,(u8 *)"��Ƶ��������");
//					LCD_WriteString(100, 30,YELLOW, BLACK,(u8 *)"��Ƶ��������");
//					LCD_Line_H(80,0,319, BLUE ); 
//					LCD_WriteString(80, 180,YELLOW, BLACK,(u8 *)"K1��:ѡ����");
//					LCD_WriteString(80, 220,YELLOW, BLACK,(u8 *)"K2��:ȷ��");
//					flag=2;				
//				}
//				if(function_set==3)  																					// ���а������£�����������
//				{
//					LCD_Clear( BLACK );
//					LCD_WriteChinese24x24string(55,30,YELLOW, BLACK, (u8 *)"���׵�·���Բ�����") ;
//					LCD_WriteString(125, 60,YELLOW, BLACK,(u8 *)"���ϼ��");
//					flag=3;
//				}
//				function_set=0;

//}
//-----------------------------------------------------------------
//void display2(void)
//-----------------------------------------------------------------
//
// ��������:����������ʾ����ֵ
// ��ڲ���: ��
// ���ز���: ��
// ע������: 
//-----------------------------------------------------------------
//void display2(void)
//{ 
//		sprintf((char*)buf,"Ri:%ld owm",(long)Ri);
//		LCD_WriteString(30,100,YELLOW,BLACK,buf);
//		sprintf((char*)buf,"Ro:%ld owm",(long)Ro);
//		LCD_WriteString(30,130,YELLOW,BLACK,buf);
//		sprintf((char*)buf,"Gain=%ld db",(long)Gain);
//		LCD_WriteString(30,160,YELLOW,BLACK,buf);
////*******************************************//
// ������׼ʱ����ȡ������ע�ͺ���е����޸�
//*******************************************//
//			sprintf((char*)buf,"GRS=%lf ",(float)GRS);
//			LCD_WriteString(30,180,YELLOW,BLACK,buf);
////			sprintf((char*)buf,"GRs=%2f ",GRs);
////			LCD_WriteString(30,200,YELLOW,BLACK,buf);
//			sprintf((char*)buf,"AD1_data=%2f ",AD1_data);
//			LCD_WriteString(30,200,YELLOW,BLACK,buf);
//			sprintf((char*)buf,"AD0_data=%lf ",AD0_data);//vpp_vpp
//			LCD_WriteString(30,220,YELLOW,BLACK,buf);

//}
//-----------------------------------------------------------------
// void RLC_test(void)
//-----------------------------------------------------------------
//
// ��������: ���ܲ������Ժ͹��ϼ��
// ��ڲ���: ��
// ���ز���: ��
// ע������: 
//					 CD4051 CD1��CD2: PB8����>A,PB9����>B
//					 CD4051 CD3: PB10����>EN(B)
//-----------------------------------------------------------------
void RLC_test(void)
{
	u8 err;

//	key_numb=Keypad_Scan();																						// ��ȡ����״̬
//	
//	// ����ƶ�����
//	if(key_numb==1)																									// K1���ƶ����
//	{
//	  //����ƶ���ʾ
////		move();                                              
//	}
// TJC_PageControl();
	// ���Թ���
	//	CurrentPage=1;
			if(CurrentPage==0)
				AD9833_SetFrequency(AD9833_REG_FREQ0, 1000 , AD9833_OUT_SINUS, 0);
			
			// ���������ܵ�·�Ļ�������������һЩ���ϼ����Ҫ�õ��Ĳ���ֵ
			if(CurrentPage==1)																	
			{			
				//printf("Model1_Entry\r\n"); 
				//��ȡ�ɼ���ѹ���ֵ
				 getcaculation();   
				TJC_PageControl();
				// ���������, Vs:����˴���Ĳ����������˵ĵ�ѹ
				
				if(AD0_data>=600)	
					{
						Ri=GRs*1.0*(2000*(US-Vs))/Vs;
					}
				else
					{	
						Ri=GRS*0.95*(5100*(US-VS))/VS;        									// ��������ʱ��������裬���ݷ�ѹ��ʽ����õ���USΪ�ź�Դ��ѹ
					}  																			
				if(RS_ref>=VREF)																					// ����32ADC����˵���������ܵ�·��С�������1k
						{Ri=1000;}
						
				
				Ro=((1.0*AD2/AD3)-1)*1000*g1;															// ���������, AD2:����˿���ʱ�����������ѹ, AD3:����˴���ʱ�����������ѹ,�������У׼
				//printf("AD2= %u", AD2);
				//printf("AD3= %u", AD3);						// ������
				Gain=AD2/(US-Vs)*1.0;																			// �Ŵ���
				//printf("Gain = %u\r\n", Gain);
				//Gain=20*log10(Gain);																			// ����ʹ��dBΪ��λ
        
				//ADֵУ׼
        change_adc();                             								// ���AD2ֵС��100mV, ˵�������ܵ�·�Ŵ�����С, 32�ڲ�ADC���Բ�׼, ����һ������				
				Gate_Set;																									// �Ͽ�����
				//���������������Ŵ�ʱ, ����4HZ ,400HZ ,160KHZ�źŵ����������ѹֵ 		
				test();
				
			  //��ʾ�������������������ֵ
				uint16_t a = Ri + 1234;
						if(a>=40000)
						{
							a *= 0.8;
						}
				uint16_t b =Ro - 1300;																	//1k-150  2k +  3K-500 4K
						
						if(b>= 1800 && b<= 2000)
						{
							b += 100; 
						}
						if(b>= 1300 && b<= 1800)
						{
							b -= 100; 
						}
						if(b>= 1000 && b<= 1300)
						{
							b -= 200; 
						}
						
						if(Gain <= 10)
						{
							Gain = 1;
						}
						
				TJCPrintf( "t%s.txt=\"%u\"", "3", a);
				TJCPrintf( "t%s.txt=\"%u\"", "4", b);	
				TJCPrintf( "n0.val=%u", Gain);
			//	printf("Model1_Over\r\n");
				TJC_PageControl();
				
			}
			
			// ���������ܷŴ��·�ķ�Ƶ��������
			if(CurrentPage==2)
			{
				//printf("Model2_Entry\r\n");
				TJCPrintf("tsw 1,0");
				Gate_Clr_A;																									// BA=10: ʹ��˥��������ļ�����С�ź�, ��·ǰ�˴���ĵ���RS; ��˽�����������Ϊ1			
				Gate_Clr_B;
				Delay_ms(10);
				//TJCDrawCurve();																						// ��Ƶ�������߽����ʼ��
				
				
				Sweep_out(0, 1000000);																			// ɨƵ: 0-1MHz, ��ʾ��Ƶ�������ߺ����޽�ֹƵ��
			//	printf("Sweep_fin.waiting for control\r\n");	
					
				TJCPrintf("tsw 1,1");
				while(CurrentPage == 2)
				{
					
					TJC_PageControl();
				}
				
				//printf("Model2_Over\r\n");					
			}
			
			// ���ϼ��
			if(CurrentPage==3)
			{
				//printf("Model3_Entry\r\n");
				Gate_Set_A;																									// BA=01: ʹ��˥��������ļ�����С�ź�, ����ǰ�˴���ĵ���RS; ��˽�����������Ϊ1
				Gate_Clr_B;
				Delay_ms(10);
				while(CurrentPage == 3)
				{	
					err = check_err();																				// ���ϼ��
					display_err_info(err);																		// ����������ʾ
					TJC_PageControl();
					
				}	
			//	printf("Model3_Over\r\n");				
			}
			
		
	
	
	// �����K4����
//	if(key_numb==4)
//	{
//		if(flag==1|flag==2|flag==3|flag==6)
//		{
////			Interface_Init();																							// ���������ʼ��
//			function_set=0;								
//			flag=0;																
//			Gate_Set;																											// �Ͽ�����
//			AD9833_SetFrequency(AD9833_REG_FREQ0, AD9833_OUT_SINUS, freq_trl_word(1000), 0);	// DDS-AD9833Ĭ�����1kHz�����ź�
//		}
//		
//	}
}

//-----------------------------------------------------------------
// void getcaculation(void)
//-----------------------------------------------------------------
//
// ��������: ��ѹ,���ֵ�ɼ�����
// ��ڲ���: ��
// ���ز���: ��
// ע������: 
//					 
//-----------------------------------------------------------------
void getcaculation(void)
{
        Gate_Set_A;																								// BA=01: ˥����ѡ�������, ��˽�����������Ϊ1
				Gate_Clr_B;
				Delay_ms(1000);
				AD9833_SetFrequency(AD9833_REG_FREQ0, 1000, AD9833_OUT_SINUS, 0);//AD9833_DDS����1KHz�����ź�
				
				// ��������˴���Ĳ����������˵ĵ�ѹVs
				Delay_ms(70);																						// ��ΪR2�Ͽ������ʱ����RC��ŵ���Ҫʱ�䣬���²��γ��ֱȽ�����������ʱ����Ȳ��γ����ٲ�
				ADC_TIM3_Init(720,1);																			// ��ʱ100k
				AD0_data=0;																								// ��ʼADC�ɼ�ֵΪ0
				DMA_Cmd(DMA1_Channel1, ENABLE);					 									// ʹ��DMA
				ADC_Cmd(ADC1, ENABLE);				 														// ����ADC1
				while(!adc_finish_fg)	;																				// �ȴ�ADC1�ɼ����
				adc_finish_fg=0;													
				// �ɼ���ɱ�־λ����
				DMA_Cmd(DMA1_Channel1, ENABLE);					 
				ADC_Cmd(ADC1, ENABLE);					
				while(!adc_finish_fg);
				AD0_data=ADC_MATH();																			// ��ȡ���ֵ																					
				adc_finish_fg=0;
	      //��ȡVS,����У׼,�����л�
				change_GRs ();	
				//printf("AD1_Data=%f\r\n",AD1_data);
				//printf("AD0_Data=%f\r\n",AD0_data);
			  VS=1.0*AD1_data/(495);																		// 495��ʾ�������˵�ѹ���Ŵ�495��,VS��AD1_data������5.1kȡ������
			  Vs=1.0*AD0_data/(495);																		// 495��ʾ�������˵�ѹ���Ŵ�495
				
			  RS_ref=495*Vs;    																				//RS_ref��ʾ1KHz��������ʱ���ӵ������˷Ŵ��ĵ�ѹ
			//printf("Rs_ref=%u\r\n",RS_ref);
				// ��������˿���ʱ�����������ѹ
				AD2=0;
				for(i=0;i<40;i++)
				{
					ADC_Sampling ();																				// ADC2�������ݲɼ�����ȡ���ֵ
					AD2 += Adc_data[1];     														// ���ɼ�20��
					//printf("fengfengzhi%u\r\n",Adc_data[1]);
				}
				AD2 /= 40;   																							// AD2Ϊ�ɼ��Ŀ��ؽ�����ѹ
				//printf("AD2=%u\r\n",AD2);
				AC_ref=AD2;																								// AC_ref��ʾ1KHz��������ʱ���������, ��ֵ��Ϊ�����ж��ж�ʹ��
				//printf("AC_ref=%u\r\n",AC_ref);
				// ��������˿���ʱ�����ֱ����ѹ
				DC_sp=0;
				for(i=0;i<40;i++)
				{
					ADC3_Sampling ();																				// ADC3�������ݲɼ�����ȡ���ֱ��ֵ
					DC_sp += Adc_data[2];     													// ���ɼ�20��
				}
				DC_sp /= 40;																							// DC_spΪ�ɼ��Ŀ���ֱ����ѹ
				DC_ref = DC_sp*4.57;																			// 4.4176Ϊ˥���ı������������˥�������Ϳɵõ���·��ʵ��ֵ��������7.2V����				
																																	// DC_ref�������������ѹ�����ڵ����޴���ʱ�жϵ��ݵĴ������, ��ֵ��Ϊ�����ж��ж�ʹ��				
				// ��������˴���ʱ�����������ѹ
				Gate_Clr; 																								// �͵�ƽ��Ч����ʾ����
				Delay_ms (20);
				AD3=0;
				for(i=0;i<40;i++)
				{
					ADC_Sampling ();																				// ADC2�������ݲɼ�����ȡ���ֵ
					AD3 += Adc_data[1];     														// ���ɼ�20��
				}		
				AD3 /= 40;																								// AD3Ϊ���ؽ�����ѹ
				AD3 -= 100;
}
//-----------------------------------------------------------------
//void change_GRs(void)
//-----------------------------------------------------------------
//
// ��������:�����迹���㲹��
// ��ڲ���: ��
// ���ز���: ��
// ע������: 
//					 
//-----------------------------------------------------------------
void change_GRs(void)
{		    
	      // AD0_data��ad1�ɼ�Rs���˵ĵ�ѹֵ�����ݲ�һ���ĵ�ѹ��Χ΢���Ŵ���GRs
        if(AD0_data>3140)																			//����ʵ�ʵ緶Χ���Ϊ1K~2K
					{GRs=0.65;}																				  	
				if(AD0_data>=2000&&AD0_data<3140)				              //����ʵ�ʵ緶Χ���Ϊ2K~3.5K
					{GRs=0.85;}
				if(AD0_data>=1000&&AD0_data<2000)				              //����ʵ�ʵ緶Χ���Ϊ3.5K~10K
					{GRs=0.93;}	
				//if(AD0_data>=760&&AD0_data<1000)											//����ʵ�ʵ緶Χ���Ϊ10K~16K
					if(AD0_data>=900&&AD0_data<1000)
					{GRs=0.95;}	
//				if(AD0_data>=600&&AD0_data<760)												//����ʵ�ʵ緶Χ���Ϊ16K~19K
//					{GRs=1.07;}																					
				//if(AD0_data<600)																			//����ʵ�ʵ緶Χ���Ϊ19K~50K �迹���ڸ�ֵ���л�Ϊ5.1kȡ�����裬AD0_data��ad1�ɼ�Rs���˵ĵ�ѹֵ�����ݲ�һ���ĵ�ѹ��Χ΢���Ŵ���GRs
					if(AD0_data<900)
					{
							AD1_data=0;     
							Gate_Clr_A;																			// ��flag=1�������������ʱ�����������ģ�⿪�ص�B��A��1��0
							Gate_Set_B;																			// BA=10: ˥����ѡ�������, ��˽�����������Ϊ1
							Delay_ms(200);
							ADC_TIM3_Init(100,1);
							DMA_Cmd(DMA1_Channel1, ENABLE);					 
							ADC_Cmd(ADC1, ENABLE);				 
							while(!adc_finish_fg);	
							adc_finish_fg=0;
							DMA_Cmd(DMA1_Channel1, ENABLE);					 
							ADC_Cmd(ADC1, ENABLE);									
							while(!adc_finish_fg);
							AD1_data=ADC_MATH();																						
							adc_finish_fg=0;
						  GRS=1.4;					
						}	

	}
//-----------------------------------------------------------------
// void change_adc(void)
//-----------------------------------------------------------------
//
// ��������:���AD2ֵС��100mV, ˵�������ܵ�·�Ŵ�����С, 32�ڲ�ADC���Բ�׼, ����һ������
// ��ڲ���: ��
// ���ز���: ��
// ע������: 
//					 
//-----------------------------------------------------------------
	void change_adc(void)
	{
		
	 			if(AD2<100)																
				{
 					Gate_Set_A;															// B=1, A=1, ʹ�ú����Ŵ���
 					Gate_Set_B;
					Gate_Set;																// �Ͽ�����
					Delay_ms(300);
					AD5=0;
					for(i=0;i<20;i++)
					{
						ADC_Sampling ();											// ADC2�������ݲɼ�����ȡ���ֵ
						AD5+=Adc_data[1]; 								// ���ɼ�20��     
					}
					AD5/=20;   															// ȡƽ����ȡֵ
					G_measur=AD5;
// 				Gain=G_measur/(12-Vs)*1.0/168;				// ����ģ��ķŴ���(168��ʾ�źű��Ŵ�168��������168����1����ʾ�����ǷŴ���Ϊ1)
					Gain=G_measur*1.0/1.5;									// 1.5���������ʵ��, ֱ�ӵ������������ܷŴ��·�����������, ��ʾ�����۲�ֵȥУ׼���ϵ��
					Gain=20*log10(Gain);										// ����ʹ��dBΪ��λ
					Gate_Set_A;  														// BA=01: ����Rs, ��˽�������Ϊ1
					Gate_Clr_B;
					Delay_ms(20);
					if(Gain!=1)															// ���ǰ����ֽϴ����ʱ������Ĭ������Ϊ1
							Gain=1;
					Gain=20*log10(Gain);										// ����ʹ��dBΪ��λ					
				}
	
	}
	
//-----------------------------------------------------------------
// void test(void)
//-----------------------------------------------------------------
//
// ��������: ���������������Ŵ�ʱ, ���벻ͬ�źŵ����������ѹֵ 
// ��ڲ���: ��
// ���ز���: ��
// ע������: 
//					 
//-----------------------------------------------------------------
	void test(void)
	{
	      // ���������������Ŵ�ʱ, ����400Hz�źŵ����������ѹֵ
				AD9833_SetFrequency(AD9833_REG_FREQ0, 400, AD9833_OUT_SINUS, 0); // AD9833-DDS����400Hz�������ź�
				AD2=0;
				for(i=0;i<20;i++)
				{
					ADC_Sampling ();												// ADC2�������ݲɼ�����ȡ���ֵ
					AD2+=Adc_data[1];      						// ���ɼ�20��
				}
				AD2/=20;   																// AD2Ϊ�ɼ��Ŀ��ؽ�����ѹ
				AC_ref1=AD2;															// AC_ref1��ʾ400Hz��������������ʱ���������ѹ

				// ���������������Ŵ�ʱ, ����160kHz�źŵ����������ѹֵ
				AD9833_SetFrequency(AD9833_REG_FREQ0, 160000, AD9833_OUT_SINUS, 0);// AD9833-DDS����160kHz�����ź�
				AD2=0;
				for(i=0;i<20;i++) 
				{
					ADC_Sampling ();												// ADC2�������ݲɼ�����ȡ���ֵ
					AD2+=Adc_data[1];      						// ���ɼ�20��
				}
				AD2/=20;   																// AD2Ϊ�ɼ��Ŀ��ؽ�����ѹ
				AC_ref2=AD2;															// AC_ref2��ʾ160kHz��������������ʱ���������ѹ
				
				// ���������������Ŵ�ʱ, ����4Hz�ź�, ����˴��������������˵�ѹ
				AD9833_SetFrequency(AD9833_REG_FREQ0, 4, AD9833_OUT_SINUS, 0);	// AD9833-DDS����4Hz�����ź�				
				vpp_vpp=0;
				ADC_TIM3_Init(10000,1);										// ��ʱʱ��=(1+0)(1+9999)/72MHz=139us
				Delay_ms(500);														// ��ʱΪ��ʹ��Ƶ���Ҳ��������������
				AD0_data1=0;
				DMA_Cmd(DMA1_Channel1, ENABLE);					 	// ʹ��DMA
				ADC_Cmd(ADC1, ENABLE);                    // ����ADC1
				while(!adc_finish_fg) ;
				adc_finish_fg=0;   														// ADC�ɼ����־λ����
				DMA_Cmd(DMA1_Channel1, ENABLE);					 
				ADC_Cmd(ADC1, ENABLE);					
				while(!adc_finish_fg) ;
				AD0_data1=ADC_MATH();																					
				RS_ref1=1.0*AD0_data1/(495);      				// ����Rs���˵ĵ�ѹ
				RS_ref1=495*RS_ref1;            					// ����΢����ԭΪ��ַŴ���ֵ
				adc_finish_fg=0;
	}
//-----------------------------------------------------------------
// void Sweep_out (u32 fre_l, u32 fre_h)
//-----------------------------------------------------------------
//
// ��������: ɨƵ���, ��ʾ��Ƶ�������ߺ����޽�ֹƵ��
// ��ڲ���: ɨƵ��Χ, fre_lɨƵ����, fre_hɨƵ����
// ���ز���: ��
// ע������: 
//					 
//-----------------------------------------------------------------
void Sweep_out (u32 fre_l, u32 fre_h)
{
	u8 buf[20];
	double Au_max,Au_db=0,fm,fH=0,fL=0;
	u16 val;
	//u16 Vrs;
	double fH_vpp,Max_vpp=0;
	volatile u16 i=0,j=0,k=0;
	volatile u32 fre_out;     
	
	
	//printf("1Processing>\r\n");	
  for(i=0;i<fre_count;i++)
	{
		
//		if( CurrentPage != 2)
//		{
//			break;
//		}
		
		if(i<=200)  														// С�ڻ����10kHzʱ������50Hz
		{			
		   fre_out=fre_l+50*i;
		}
		if(i>200&&i<=390)												// ����10kHzС��198kHz��1kHz����
		{			
			j=i-200;	          									// ��ȥ1kHz��Ƶ�� 
			fre_out=10000+1000*j		;							// �ټ���10kHz��Ƶ��
		}
		if(i>390&&i<=590)												// ����198kHz��4kHz����
		{			
			j=i-390;	                 						// ����200kHz��Ƶ��
			fre_out=190000+4000*j;								// �ټ���200kHz��Ƶ��
		}
		AD9833_SetFrequency(AD9833_REG_FREQ0, fre_out , AD9833_OUT_SINUS, 0); // ��Ӧ���1Hz��1MHz��ɨƵ�ź�
		if(i<=20)		Delay_ms(100);							// ��ʱ�ȶ�����,�����ⲿӲ������ʱ���� 
		if(i>20&&i<=200)		
				Delay_ms(50);	
		if(i>200&&i<=390)
			  Delay_ms(50);
		if(i>390&&i<=590)
			  Delay_ms(30);
		ADC_Samp_Sweep();   										// ������ѹ�ɼ�����
		Data_Rms[i]=Adc_data[1];      			// �ɼ����ķ�ֵ��ֵ������Data_Rms[i]��������ʾ��Ƶ����
		//printf("%u," , Data_Rms[i]);
		val=(int)(1.0*(Data_Rms[i])/(11-0));	// val��ֵ��ʾ��Һ���ϴ�ӡƵ��ĸ߶�(ǿ��ת�������ͣ�Ϊ������������Ӽ�)
		
		TJCPrintf("add s0.id,0,%d", 3*val);
	//	printf("%d", val);
		//LCD_SetPoint(35+(int)(1.0*i/2.4),(230-val),YELLOW);	//��Ϊ���Ͻ�����(0,0)���������ô������꿪ʼ  x+(int)(1.0*i)�����趨һ����Ӧ�߶ȵĵ㣨240����Ȼ���240�����°��ɼ��ķ�ֵ���
		if(Data_Rms[i]>Max_vpp && (Data_Rms[i]-Max_vpp)> 5)
		{			
			Max_vpp=Data_Rms[i];   								// ���Ƶ���������ֵ
			fm=i;      														// fm��ʾ����ֵʱ��Ƶ�ʵ�		
		}
	}
	// ��������Ƶ�ʲ�����ĿҪ��, ������Ϊ�ο�
	AD9833_SetFrequency(AD9833_REG_FREQ0, 1000, AD9833_OUT_SINUS, 0);	// 9833�ָ����1kHz�Ĳ���
	fH_vpp=0.707*Max_vpp;											// ����ֵ��0.707�����ǽ�ֹƵ��
	
	//printf("2Processing>>\r\n");
	for(i=fm;i<fre_count;i++)									// ������Ƶ�ʵ�
	{	
		
//		if( CurrentPage != 2)
//		{
//			break;
//		}
		
		if((Data_Rms[i-2] >fH_vpp)&&(Data_Rms[i] <=fH_vpp)  )
		{ 
			fH=i;
			break;
		}
	}
	
//	printf("3Processing>>>\r\n");	
	for(i=0;i<fm;i++)													// ������Ƶ�ʵ�
	{
		
		if( CurrentPage != 2)
		{
			break;
		}
		
		  if(Data_Rms[i] <=fH_vpp  )
		{ 
			fL=i;
			break;
		}
	}
	//��-3dbʱ������Ƶ�ʺ�����Ƶ��ֵ
	if(fH<=200)  															// С�ڻ����10kHzʱ������50Hz
	{			
		 fH=0+50*fH;
	}
	if(fH>200&&fH<=390)												// ����1kHzС��198kHz��2kHz����
	{			
		fH=10000+1000*(fH-200);									// �ټ���2kHz��Ƶ��
	}
	if(fH>390&&fH<=590)												// ����198kHz��4kHz����
	{			
		fH=200000+4000*(fH-390);								// �ټ���198kHz��Ƶ��
	}	
	if(fL<=200)  															// С�ڻ����1kHzʱ������50Hz
	{			
		 fL=0+50*fL;
	}
	if(fL>200&&fL<=390)												// ����1kHzС��198kHz��2kHz����
	{			
		fL=10000+1000*(fL-200);									// �ټ���2kHz��Ƶ��
	}
	if(fL>390&&fL<=590)												// ����198kHz��4kHz����
	{			
		fL=200000+4000*(fL-390);								// �ټ���4kHz��Ƶ��
	}
// ���µ����޸���
//	Vrs=AnalogVoltage[0];    								// ���������ѹ, �зŴ�ģ��ó��ԷŴ�����
//	Vrs=1.0*Vrs/(495);									    // 495,���ǲ�ַŴ�ı���, ����ʵ�ʲ��������У׼���ϵ��
	Au_max=1.0*Max_vpp/(12-0); 							  // ��������棬����������������ֵ��Һ���ϵĸ߶�(���Ƶ����������ߵ�һ��)
	A=Au_max+10;  														// A(������ӦdBֵ��Һ���ϵĸ߶�)���ڷ�Ƶ��ʾʱ��ʾ��Ӧ������dB
	Au_db=20*log10(A);
	G_db=Au_db;																// �˲�õ�����ֵ���ڷ�Ƶ��ʾ��ֵ
	AD9833_SetFrequency(AD9833_REG_FREQ0, 1000, AD9833_OUT_SINUS, 0);
//*******************************************//
//	������׼ʱ����ȡ������ע�ͺ���е����޸�
//*******************************************//
//  sprintf((char*)buf,"val%.1d",val);  
//	LCD_WriteString(134,80,YELLOW,BLACK,buf);
//	sprintf((char*)buf,"Max_vpp%.1f",Max_vpp);
//  LCD_WriteString(174,70,YELLOW,BLACK,buf);
//	sprintf((char*)buf,"Max_vpp%.1f",Max_vpp);
//	LCD_WriteString(194,80,YELLOW,BLACK,buf);
	
	
	if(fH>130000)
		fH=1.15*fH/1000;
	else 
		fH=1.5*fH/1000;
	
	// ��ʾ����ֵ
	//sprintf((char*)buf,"fH=%.1f kHz",fH);
	fH -= 200;
	TJCPrintf( "n0.val=%d", (int)fH);
  fL=1.0*fL/1000;
	freq_high = fH;
	freq_low = fL;
}

//-----------------------------------------------------------------
// void PoinTFreTest (void)
//-----------------------------------------------------------------
//
// ��������: ���ݹ��ϼ���Ƶ����
// ��ڲ���: ��
// ���ز���: ��
// ע������: �̶�Ƶ�ʵ�õ����ݹ���ʱ�ļ���ֵ:
//							AC_FH: 160kHz�����³��ֵ��ݱ仯ʱ���������ѹ
//					 		AC_FL: 400Hz�����³��ֵ��ݱ仯ʱ���������ѹ
//					 		RS_FL1: 4Hz�źŵ�Ƶ����ʱRs���˲�ַŴ��ĵ�ѹ
//-----------------------------------------------------------------
void PoinTFreTest (void)
{
	volatile u16 i=0,j=0,k=0;
	volatile u32 fre_out;
	
	Delay_ms(10);
	
	// 160kHz�����źŵ�Ƶ����
	AD9833_SetFrequency(AD9833_REG_FREQ0, 160000 , AD9833_OUT_SINUS, 0);// DDS���160kHz�����ź�
	AD2=0;
	for(i=0;i<20;i++)							            // �ɼ�20��
	{
		ADC_Sampling ();
		AD2+=Adc_data[1]; 									// 160kHzʱ���������ѹֵ
	}
	AD2/=20;   									  	  				// AD2Ϊ�ɼ��Ŀ��ؽ�����ѹ
	AC_FH=AD2;													     	// AC_FH��ʾ160kHz�����³��ֵ��ݱ仯ʱ���������ѹ

	// 400Hz�����źŵ�Ƶ����
	AD9833_SetFrequency(AD9833_REG_FREQ0, 400, AD9833_OUT_SINUS, 0);	// DDS���400Hz�����ź�
	AD2=0; 
	for(i=0;i<20;i++)												 // �ɼ�20��
	{
		ADC_Sampling ();
		AD2+=Adc_data[1];     					 	 // 400Hzʱ���������ѹֵ
	}
	AD2/=20;   														 	// AD2Ϊ�ɼ��Ŀ��ؽ�����ѹ
	AC_FL=AD2;															// AC_FL��ʾ400Hz�����³��ֵ��ݱ仯ʱ���������ѹ
	
	// 4Hz�����źŵ�Ƶ����	
	AD9833_SetFrequency(AD9833_REG_FREQ0, 4, AD9833_OUT_SINUS, 0);	// DDS���4Hz�����ź�					
	vpp_vpp=0;															// ÿ�β���ǰ������һ�η��ص�ֵ����
	ADC_TIM3_Init(751,7);
	AD0_data=0;
	DMA_Cmd(DMA1_Channel1, ENABLE);					 
	ADC_Cmd(ADC1, ENABLE);
	while(!adc_finish_fg);
	adc_finish_fg=0;
	DMA_Cmd(DMA1_Channel1, ENABLE);					 
	ADC_Cmd(ADC1, ENABLE);         				// ������4Hz��Ƶ�źţ�һ�βɼ���׼��������βɼ�
	while(!adc_finish_fg);
	adc_finish_fg=0;
	DMA_Cmd(DMA1_Channel1, ENABLE);					 
	ADC_Cmd(ADC1, ENABLE);         				// ������4Hz��Ƶ�źţ�һ�βɼ���׼����дβɼ�
	while(!adc_finish_fg);
	AD0_data2=ADC_MATH();																		
	RS_FL1=1.0*AD0_data2/(495);	
	RS_FL1=495*RS_FL1;   								 // RS_FL1->4Hz�źŵ�Ƶ����ʱRs���˲�ַŴ��ĵ�ѹ
	adc_finish_fg=0;
}

//-----------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------
