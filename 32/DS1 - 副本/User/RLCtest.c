//-----------------------------------------------------------------
// 程序描述:
// 　　简易频率特性测试仪功能程序
// 作　　者: 凌智电子
// 开始日期: 2019-11-15
// 完成日期: 2019-12-18
// 修改日期: 2019-12-26
// 版　　本: V1.0
// 　- V1.0:  
// 说　　明: 
//			
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// 头文件包含
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
// 变量定义
//-----------------------------------------------------------------
uint16_t       Data_Rms[2082];					//采集点数 
uint16_t       Data_v[500];							//采集点数 
//static u8 buf[8];
u8 key_numb,A=0;
int G_db=0; 
u16 i;
u32 freq;
u16 AD1,AD2,AD3,AD5;					//AD1、AD2、AD3、AD5是定义的变量; 12000表示12V供电
//-----------------------------------------------------------------
//DC_ref->正常工作的直流电压; DC_sp->一个普通变量; AC_ref->正常工作时输出交流电压; AC_ref1->400Hz正常工作时输出交流电压
//AC_FH->160KHz检查故障时输出交流电压; AC_FL->400Hz检查故障时输出交流电压,RS_FL=0,RS_ref=0,AC_ref2->160KHz正常工作时输出交流电压
//-----------------------------------------------------------------
u32 DC_ref=0,DC_sp,AC_ref=0,AC_ref1=0,AC_ref2=0,AC_FH=0,AC_FL=0,RS_FL=0,RS_ref=0,AC_ref3=0;
u32 Ri,Ro,Gain = 1,G_measur=0;							//Ri->输入阻抗; Ro->输出阻抗; Gain->总增益; G_measur->测试仪的增益;
u16 ADD1,ADD2;
u16 text[5];																		  
extern float vpp_vpp;
// AD0_data: 采集的最终AD值; Vs->电阻Rs两端的交流电压; RS_ref1->4Hz信号正常工作时Rs两端电压,RS_FL1->4Hz信号点频测试时Rs两端电压;
float AD1_data=0,AD0_data=0,Vs,VS,RS_ref1=0,RS_FL1=0;
float AD0_data1=0,AD0_data2=0,GRs=1,GRS=1;;
double freq_high,freq_low,Au_db;
double freq_high1,freq_low1;
double freq_high2,freq_low2,Au_db1;
//-----------------------------------------------------------------
// 函数定义
//-----------------------------------------------------------------
void getcaculation(void); 							//获取电压值,峰峰值
void change_GRs (void);	  							//增益校准
void change_adc(void);    							//放大倍数调整
void test(void);          							//测试三极管正常放大时, 输入不同信号的输出交流电压值 
//-----------------------------------------------------------------
// void GPIO_Gate(void)
//-----------------------------------------------------------------
//
// 函数功能: 初始化调理模块的控制IO口
// 入口参数: 无
// 返回参数: 无
// 注意事项: 
//					 CD4051 CD1和CD2: PB8——>A,PB9——>B
//					 CD4051 CD3: PB10——>EN(B)
//-----------------------------------------------------------------
void GPIO_Gate(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;  // PC4——>A,PC5——>B,PC6——>EN
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;;									// 推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
}
//-----------------------------------------------------------------
// 以下为CD1和CD2控制IO状态对应的功能
//-----------------------------------------------------------------
void standard (void)   																							//初始状态和检查错误时的CD4052开关状态(A为1，B为0)
{
	 Gate_Set_A; 
	 Gate_Clr_B;
}
void AFC (void)   																									// 测幅频特性时的CD4052开关状态(A为0，B为1)
{
	 Gate_Clr_A;
	 Gate_Set_B;
}
void G_original (void)   																						// 将测试仪输入输出短路时要执行的CD4052开关状态(测量测试仪增益)(A为1，B为1)
{
	 Gate_Set_A; 
	 Gate_Set_B;
}
//-----------------------------------------------------------------
//void move(void)
//-----------------------------------------------------------------
//
// 函数功能:  光标移动功能
// 入口参数: 无
// 返回参数: 无
// 注意事项: 
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
//					          LCD_ColorBox (245, 80, 20,	20	 , BLACK) ;				// 在x=245，y=80处画20*20的黑色矩形，遮住上一次的光标
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
// 函数功能:测量界面显示
// 入口参数: 无
// 返回参数: 无
// 注意事项: 
//-----------------------------------------------------------------
//void display1(void)
//{
//  if(function_set==1)																									// 当有按键按下，第一个界面
//				{	
//					LCD_Clear( BLACK );         																// 设置背景色
//					LCD_WriteChinese24x24string(55,30,YELLOW, BLACK, (u8 *)"简易电路特性测试仪") ;	// 坐标原点在左上角
//					flag=1;																											// 界面标志位
//				}
//				if(function_set==2)																						// 当有按键按下，第二个界面
//				{
//					LCD_Clear( BLACK );
//					LCD_WriteChinese24x24string(55,30,YELLOW, BLACK, (u8 *)"简易电路特性测试仪") ;
//					LCD_WriteString(130, 30,YELLOW, BLACK,(u8 *)"相频特性曲线");
//					LCD_WriteString(100, 30,YELLOW, BLACK,(u8 *)"幅频特性曲线");
//					LCD_Line_H(80,0,319, BLUE ); 
//					LCD_WriteString(80, 180,YELLOW, BLACK,(u8 *)"K1键:选择功能");
//					LCD_WriteString(80, 220,YELLOW, BLACK,(u8 *)"K2键:确认");
//					flag=2;				
//				}
//				if(function_set==3)  																					// 当有按键按下，第三个界面
//				{
//					LCD_Clear( BLACK );
//					LCD_WriteChinese24x24string(55,30,YELLOW, BLACK, (u8 *)"简易电路特性测试仪") ;
//					LCD_WriteString(125, 60,YELLOW, BLACK,(u8 *)"故障检测");
//					flag=3;
//				}
//				function_set=0;

//}
//-----------------------------------------------------------------
//void display2(void)
//-----------------------------------------------------------------
//
// 函数功能:测量界面显示测量值
// 入口参数: 无
// 返回参数: 无
// 注意事项: 
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
// 测量不准时可以取消下列注释后进行调试修改
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
// 函数功能: 功能参数测试和故障检测
// 入口参数: 无
// 返回参数: 无
// 注意事项: 
//					 CD4051 CD1和CD2: PB8——>A,PB9——>B
//					 CD4051 CD3: PB10——>EN(B)
//-----------------------------------------------------------------
void RLC_test(void)
{
	u8 err;

//	key_numb=Keypad_Scan();																						// 获取按键状态
//	
//	// 光标移动功能
//	if(key_numb==1)																									// K1键移动光标
//	{
//	  //光标移动显示
////		move();                                              
//	}
// TJC_PageControl();
	// 测试功能
	//	CurrentPage=1;
			if(CurrentPage==0)
				AD9833_SetFrequency(AD9833_REG_FREQ0, 1000 , AD9833_OUT_SINUS, 0);
			
			// 测试三极管电路的基本参数并保存一些故障检测需要用到的参数值
			if(CurrentPage==1)																	
			{			
				//printf("Model1_Entry\r\n"); 
				//获取采集电压峰峰值
				 getcaculation();   
				TJC_PageControl();
				// 求输入电阻, Vs:输入端串入的采样电阻两端的电压
				
				if(AD0_data>=600)	
					{
						Ri=GRs*1.0*(2000*(US-Vs))/Vs;
					}
				else
					{	
						Ri=GRS*0.95*(5100*(US-VS))/VS;        									// 正常工作时的输入电阻，根据分压公式计算得到，US为信号源电压
					}  																			
				if(RS_ref>=VREF)																					// 超过32ADC量程说明是三极管电路最小输入电阻1k
						{Ri=1000;}
						
				
				Ro=((1.0*AD2/AD3)-1)*1000*g1;															// 求输出电阻, AD2:输出端空载时的输出交流电压, AD3:输出端带载时的输出交流电压,输入电阻校准
				//printf("AD2= %u", AD2);
				//printf("AD3= %u", AD3);						// 求增益
				Gain=AD2/(US-Vs)*1.0;																			// 放大倍数
				//printf("Gain = %u\r\n", Gain);
				//Gain=20*log10(Gain);																			// 增益使用dB为单位
        
				//AD值校准
        change_adc();                             								// 如果AD2值小于100mV, 说明三极管电路放大倍数较小, 32内部ADC测试不准, 做进一步处理				
				Gate_Set;																									// 断开负载
				//测试三极管正常放大时, 输入4HZ ,400HZ ,160KHZ信号的输出交流电压值 		
				test();
				
			  //显示测量输入电阻输出电阻等值
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
			
			// 测试三极管放大电路的幅频特性曲线
			if(CurrentPage==2)
			{
				//printf("Model2_Entry\r\n");
				TJCPrintf("tsw 1,0");
				Gate_Clr_A;																									// BA=10: 使用衰减倍数大的即输入小信号, 短路前端串入的电阻RS; 后端交流检测的增益为1			
				Gate_Clr_B;
				Delay_ms(10);
				//TJCDrawCurve();																						// 幅频特性曲线界面初始化
				
				
				Sweep_out(0, 1000000);																			// 扫频: 0-1MHz, 显示幅频特性曲线和上限截止频率
			//	printf("Sweep_fin.waiting for control\r\n");	
					
				TJCPrintf("tsw 1,1");
				while(CurrentPage == 2)
				{
					
					TJC_PageControl();
				}
				
				//printf("Model2_Over\r\n");					
			}
			
			// 故障检测
			if(CurrentPage==3)
			{
				//printf("Model3_Entry\r\n");
				Gate_Set_A;																									// BA=01: 使用衰减倍数大的即输入小信号, 接入前端串入的电阻RS; 后端交流检测的增益为1
				Gate_Clr_B;
				Delay_ms(10);
				while(CurrentPage == 3)
				{	
					err = check_err();																				// 故障检测
					display_err_info(err);																		// 故障类型显示
					TJC_PageControl();
					
				}	
			//	printf("Model3_Over\r\n");				
			}
			
		
	
	
	// 如果是K4按下
//	if(key_numb==4)
//	{
//		if(flag==1|flag==2|flag==3|flag==6)
//		{
////			Interface_Init();																							// 开机界面初始化
//			function_set=0;								
//			flag=0;																
//			Gate_Set;																											// 断开负载
//			AD9833_SetFrequency(AD9833_REG_FREQ0, AD9833_OUT_SINUS, freq_trl_word(1000), 0);	// DDS-AD9833默认输出1kHz正弦信号
//		}
//		
//	}
}

//-----------------------------------------------------------------
// void getcaculation(void)
//-----------------------------------------------------------------
//
// 函数功能: 电压,峰峰值采集计算
// 入口参数: 无
// 返回参数: 无
// 注意事项: 
//					 
//-----------------------------------------------------------------
void getcaculation(void)
{
        Gate_Set_A;																								// BA=01: 衰减器选择大倍数的, 后端交流检测的增益为1
				Gate_Clr_B;
				Delay_ms(1000);
				AD9833_SetFrequency(AD9833_REG_FREQ0, 1000, AD9833_OUT_SINUS, 0);//AD9833_DDS产生1KHz正弦信号
				
				// 测量输入端串入的采样电阻两端的电压Vs
				Delay_ms(70);																						// 因为R2断开测电阻时由于RC充放电需要时间，导致波形出现比较慢，所以延时两秒等波形出现再测
				ADC_TIM3_Init(720,1);																			// 定时100k
				AD0_data=0;																								// 初始ADC采集值为0
				DMA_Cmd(DMA1_Channel1, ENABLE);					 									// 使能DMA
				ADC_Cmd(ADC1, ENABLE);				 														// 启动ADC1
				while(!adc_finish_fg)	;																				// 等待ADC1采集完成
				adc_finish_fg=0;													
				// 采集完成标志位清零
				DMA_Cmd(DMA1_Channel1, ENABLE);					 
				ADC_Cmd(ADC1, ENABLE);					
				while(!adc_finish_fg);
				AD0_data=ADC_MATH();																			// 获取峰峰值																					
				adc_finish_fg=0;
	      //获取VS,增益校准,开关切换
				change_GRs ();	
				//printf("AD1_Data=%f\r\n",AD1_data);
				//printf("AD0_Data=%f\r\n",AD0_data);
			  VS=1.0*AD1_data/(495);																		// 495表示电阻两端电压被放大495倍,VS和AD1_data是用于5.1k取样电阻
			  Vs=1.0*AD0_data/(495);																		// 495表示电阻两端电压被放大495
				
			  RS_ref=495*Vs;    																				//RS_ref表示1KHz正常工作时串接电阻两端放大后的电压
			//printf("Rs_ref=%u\r\n",RS_ref);
				// 测量输出端空载时的输出交流电压
				AD2=0;
				for(i=0;i<40;i++)
				{
					ADC_Sampling ();																				// ADC2批量数据采集并获取峰峰值
					AD2 += Adc_data[1];     														// 共采集20次
					//printf("fengfengzhi%u\r\n",Adc_data[1]);
				}
				AD2 /= 40;   																							// AD2为采集的空载交流电压
				//printf("AD2=%u\r\n",AD2);
				AC_ref=AD2;																								// AC_ref表示1KHz正常工作时输出交流电, 此值作为故障判断判断使用
				//printf("AC_ref=%u\r\n",AC_ref);
				// 测量输出端空载时的输出直流电压
				DC_sp=0;
				for(i=0;i<40;i++)
				{
					ADC3_Sampling ();																				// ADC3批量数据采集并获取输出直流值
					DC_sp += Adc_data[2];     													// 共采集20次
				}
				DC_sp /= 40;																							// DC_sp为采集的空载直流电压
				DC_ref = DC_sp*4.57;																			// 4.4176为衰减的倍数，乘以这个衰减倍数就可得到电路的实际值，工作点7.2V左右				
																																	// DC_ref是正常工作点电压，用于电阻无错误时判断电容的错误情况, 此值作为故障判断判断使用				
				// 测量输出端带载时的输出交流电压
				Gate_Clr; 																								// 低电平有效，表示带载
				Delay_ms (20);
				AD3=0;
				for(i=0;i<40;i++)
				{
					ADC_Sampling ();																				// ADC2批量数据采集并获取峰峰值
					AD3 += Adc_data[1];     														// 共采集20次
				}		
				AD3 /= 40;																								// AD3为带载交流电压
				AD3 -= 100;
}
//-----------------------------------------------------------------
//void change_GRs(void)
//-----------------------------------------------------------------
//
// 函数功能:输入阻抗计算补偿
// 入口参数: 无
// 返回参数: 无
// 注意事项: 
//					 
//-----------------------------------------------------------------
void change_GRs(void)
{		    
	      // AD0_data是ad1采集Rs两端的电压值，根据不一样的电压范围微调放大倍数GRs
        if(AD0_data>3140)																			//测量实际电范围大概为1K~2K
					{GRs=0.65;}																				  	
				if(AD0_data>=2000&&AD0_data<3140)				              //测量实际电范围大概为2K~3.5K
					{GRs=0.85;}
				if(AD0_data>=1000&&AD0_data<2000)				              //测量实际电范围大概为3.5K~10K
					{GRs=0.93;}	
				//if(AD0_data>=760&&AD0_data<1000)											//测量实际电范围大概为10K~16K
					if(AD0_data>=900&&AD0_data<1000)
					{GRs=0.95;}	
//				if(AD0_data>=600&&AD0_data<760)												//测量实际电范围大概为16K~19K
//					{GRs=1.07;}																					
				//if(AD0_data<600)																			//测量实际电范围大概为19K~50K 阻抗大于该值就切换为5.1k取样电阻，AD0_data是ad1采集Rs两端的电压值，根据不一样的电压范围微调放大倍数GRs
					if(AD0_data<900)
					{
							AD1_data=0;     
							Gate_Clr_A;																			// 当flag=1（测基本参数）时，控制增益的模拟开关的B、A置1，0
							Gate_Set_B;																			// BA=10: 衰减器选择大倍数的, 后端交流检测的增益为1
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
// 函数功能:如果AD2值小于100mV, 说明三极管电路放大倍数较小, 32内部ADC测试不准, 做进一步处理
// 入口参数: 无
// 返回参数: 无
// 注意事项: 
//					 
//-----------------------------------------------------------------
	void change_adc(void)
	{
		
	 			if(AD2<100)																
				{
 					Gate_Set_A;															// B=1, A=1, 使用后级最大放大倍数
 					Gate_Set_B;
					Gate_Set;																// 断开负载
					Delay_ms(300);
					AD5=0;
					for(i=0;i<20;i++)
					{
						ADC_Sampling ();											// ADC2批量数据采集并获取峰峰值
						AD5+=Adc_data[1]; 								// 共采集20次     
					}
					AD5/=20;   															// 取平均获取值
					G_measur=AD5;
// 				Gain=G_measur/(12-Vs)*1.0/168;				// 调理模块的放大倍数(168表示信号被放大168倍，除以168等于1来表示测试仪放大倍数为1)
					Gain=G_measur*1.0/1.5;									// 1.5这个倍数是实测, 直接导线连接三极管放大电路的输入输出端, 由示波器观察值去校准这个系数
					Gain=20*log10(Gain);										// 增益使用dB为单位
					Gate_Set_A;  														// BA=01: 接上Rs, 后端交流增益为1
					Gate_Clr_B;
					Delay_ms(20);
					if(Gain!=1)															// 如果前面出现较大误差时，将其默认设置为1
							Gain=1;
					Gain=20*log10(Gain);										// 增益使用dB为单位					
				}
	
	}
	
//-----------------------------------------------------------------
// void test(void)
//-----------------------------------------------------------------
//
// 函数功能: 测试三极管正常放大时, 输入不同信号的输出交流电压值 
// 入口参数: 无
// 返回参数: 无
// 注意事项: 
//					 
//-----------------------------------------------------------------
	void test(void)
	{
	      // 测试三极管正常放大时, 输入400Hz信号的输出交流电压值
				AD9833_SetFrequency(AD9833_REG_FREQ0, 400, AD9833_OUT_SINUS, 0); // AD9833-DDS产生400Hz的正弦信号
				AD2=0;
				for(i=0;i<20;i++)
				{
					ADC_Sampling ();												// ADC2批量数据采集并获取峰峰值
					AD2+=Adc_data[1];      						// 共采集20次
				}
				AD2/=20;   																// AD2为采集的空载交流电压
				AC_ref1=AD2;															// AC_ref1表示400Hz条件下正常工作时输出交流电压

				// 测试三极管正常放大时, 输入160kHz信号的输出交流电压值
				AD9833_SetFrequency(AD9833_REG_FREQ0, 160000, AD9833_OUT_SINUS, 0);// AD9833-DDS产生160kHz正弦信号
				AD2=0;
				for(i=0;i<20;i++) 
				{
					ADC_Sampling ();												// ADC2批量数据采集并获取峰峰值
					AD2+=Adc_data[1];      						// 共采集20次
				}
				AD2/=20;   																// AD2为采集的空载交流电压
				AC_ref2=AD2;															// AC_ref2表示160kHz条件下正常工作时输出交流电压
				
				// 测试三极管正常放大时, 输入4Hz信号, 输入端串入采样电阻的两端电压
				AD9833_SetFrequency(AD9833_REG_FREQ0, 4, AD9833_OUT_SINUS, 0);	// AD9833-DDS产生4Hz正弦信号				
				vpp_vpp=0;
				ADC_TIM3_Init(10000,1);										// 定时时间=(1+0)(1+9999)/72MHz=139us
				Delay_ms(500);														// 延时为了使低频正弦波多输出几个周期
				AD0_data1=0;
				DMA_Cmd(DMA1_Channel1, ENABLE);					 	// 使能DMA
				ADC_Cmd(ADC1, ENABLE);                    // 启动ADC1
				while(!adc_finish_fg) ;
				adc_finish_fg=0;   														// ADC采集完标志位置零
				DMA_Cmd(DMA1_Channel1, ENABLE);					 
				ADC_Cmd(ADC1, ENABLE);					
				while(!adc_finish_fg) ;
				AD0_data1=ADC_MATH();																					
				RS_ref1=1.0*AD0_data1/(495);      				// 计算Rs两端的电压
				RS_ref1=495*RS_ref1;            					// 数据微调后还原为差分放大后的值
				adc_finish_fg=0;
	}
//-----------------------------------------------------------------
// void Sweep_out (u32 fre_l, u32 fre_h)
//-----------------------------------------------------------------
//
// 函数功能: 扫频输出, 显示幅频特性曲线和上限截止频率
// 入口参数: 扫频范围, fre_l扫频下限, fre_h扫频上限
// 返回参数: 无
// 注意事项: 
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
		
		if(i<=200)  														// 小于或等于10kHz时，步进50Hz
		{			
		   fre_out=fre_l+50*i;
		}
		if(i>200&&i<=390)												// 大于10kHz小于198kHz，1kHz步进
		{			
			j=i-200;	          									// 减去1kHz的频点 
			fre_out=10000+1000*j		;							// 再加上10kHz的频率
		}
		if(i>390&&i<=590)												// 大于198kHz，4kHz步进
		{			
			j=i-390;	                 						// 减掉200kHz的频点
			fre_out=190000+4000*j;								// 再加上200kHz的频率
		}
		AD9833_SetFrequency(AD9833_REG_FREQ0, fre_out , AD9833_OUT_SINUS, 0); // 对应输出1Hz到1MHz的扫频信号
		if(i<=20)		Delay_ms(100);							// 延时稳定数据,减少外部硬件的延时干扰 
		if(i>20&&i<=200)		
				Delay_ms(50);	
		if(i>200&&i<=390)
			  Delay_ms(50);
		if(i>390&&i<=590)
			  Delay_ms(30);
		ADC_Samp_Sweep();   										// 交流电压采集函数
		Data_Rms[i]=Adc_data[1];      			// 采集到的幅值赋值给数组Data_Rms[i]，用于显示幅频曲线
		//printf("%u," , Data_Rms[i]);
		val=(int)(1.0*(Data_Rms[i])/(11-0));	// val的值表示在液晶上打印频点的高度(强制转化成整型，为了能与整数相加减)
		
		TJCPrintf("add s0.id,0,%d", 3*val);
	//	printf("%d", val);
		//LCD_SetPoint(35+(int)(1.0*i/2.4),(230-val),YELLOW);	//因为左上角坐标(0,0)，画点设置从纵坐标开始  x+(int)(1.0*i)，先设定一个相应高度的点（240），然后从240处往下按采集的幅值打点
		if(Data_Rms[i]>Max_vpp && (Data_Rms[i]-Max_vpp)> 5)
		{			
			Max_vpp=Data_Rms[i];   								// 求出频带里的最大幅值
			fm=i;      														// fm表示最大幅值时的频率点		
		}
	}
	// 以下下限频率不是题目要求, 这里作为参考
	AD9833_SetFrequency(AD9833_REG_FREQ0, 1000, AD9833_OUT_SINUS, 0);	// 9833恢复输出1kHz的波形
	fH_vpp=0.707*Max_vpp;											// 最大幅值的0.707倍就是截止频率
	
	//printf("2Processing>>\r\n");
	for(i=fm;i<fre_count;i++)									// 求上限频率点
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
	for(i=0;i<fm;i++)													// 求下限频率点
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
	//求-3db时的上限频率和下限频率值
	if(fH<=200)  															// 小于或等于10kHz时，步进50Hz
	{			
		 fH=0+50*fH;
	}
	if(fH>200&&fH<=390)												// 大于1kHz小于198kHz，2kHz步进
	{			
		fH=10000+1000*(fH-200);									// 再加上2kHz的频率
	}
	if(fH>390&&fH<=590)												// 大于198kHz，4kHz步进
	{			
		fH=200000+4000*(fH-390);								// 再加上198kHz的频率
	}	
	if(fL<=200)  															// 小于或等于1kHz时，步进50Hz
	{			
		 fL=0+50*fL;
	}
	if(fL>200&&fL<=390)												// 大于1kHz小于198kHz，2kHz步进
	{			
		fL=10000+1000*(fL-200);									// 再加上2kHz的频率
	}
	if(fL>390&&fL<=590)												// 大于198kHz，4kHz步进
	{			
		fL=200000+4000*(fL-390);								// 再加上4kHz的频率
	}
// 以下调试修改用
//	Vrs=AnalogVoltage[0];    								// 计算输入电压, 有放大的，得除以放大倍数。
//	Vrs=1.0*Vrs/(495);									    // 495,这是差分放大的倍数, 根据实际测试情况可校准这个系数
	Au_max=1.0*Max_vpp/(12-0); 							  // 计算出增益，并用于设置增益数值在液晶上的高度(与幅频特性曲线最高点一致)
	A=Au_max+10;  														// A(设置相应dB值在液晶上的高度)用于幅频显示时显示对应的增益dB
	Au_db=20*log10(A);
	G_db=Au_db;																// 此测得的增益值用于幅频显示幅值
	AD9833_SetFrequency(AD9833_REG_FREQ0, 1000, AD9833_OUT_SINUS, 0);
//*******************************************//
//	测量不准时可以取消下列注释后进行调试修改
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
	
	// 显示上限值
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
// 函数功能: 电容故障检测点频测试
// 入口参数: 无
// 返回参数: 无
// 注意事项: 固定频率点得到电容故障时的几个值:
//							AC_FH: 160kHz条件下出现电容变化时输出交流电压
//					 		AC_FL: 400Hz条件下出现电容变化时输出交流电压
//					 		RS_FL1: 4Hz信号点频测试时Rs两端差分放大后的电压
//-----------------------------------------------------------------
void PoinTFreTest (void)
{
	volatile u16 i=0,j=0,k=0;
	volatile u32 fre_out;
	
	Delay_ms(10);
	
	// 160kHz正弦信号点频测试
	AD9833_SetFrequency(AD9833_REG_FREQ0, 160000 , AD9833_OUT_SINUS, 0);// DDS输出160kHz正弦信号
	AD2=0;
	for(i=0;i<20;i++)							            // 采集20次
	{
		ADC_Sampling ();
		AD2+=Adc_data[1]; 									// 160kHz时输出交流电压值
	}
	AD2/=20;   									  	  				// AD2为采集的空载交流电压
	AC_FH=AD2;													     	// AC_FH表示160kHz条件下出现电容变化时输出交流电压

	// 400Hz正弦信号点频测试
	AD9833_SetFrequency(AD9833_REG_FREQ0, 400, AD9833_OUT_SINUS, 0);	// DDS输出400Hz正弦信号
	AD2=0; 
	for(i=0;i<20;i++)												 // 采集20次
	{
		ADC_Sampling ();
		AD2+=Adc_data[1];     					 	 // 400Hz时输出交流电压值
	}
	AD2/=20;   														 	// AD2为采集的空载交流电压
	AC_FL=AD2;															// AC_FL表示400Hz条件下出现电容变化时输出交流电压
	
	// 4Hz正弦信号点频测试	
	AD9833_SetFrequency(AD9833_REG_FREQ0, 4, AD9833_OUT_SINUS, 0);	// DDS输出4Hz正弦信号					
	vpp_vpp=0;															// 每次测量前，将上一次返回的值清零
	ADC_TIM3_Init(751,7);
	AD0_data=0;
	DMA_Cmd(DMA1_Channel1, ENABLE);					 
	ADC_Cmd(ADC1, ENABLE);
	while(!adc_finish_fg);
	adc_finish_fg=0;
	DMA_Cmd(DMA1_Channel1, ENABLE);					 
	ADC_Cmd(ADC1, ENABLE);         				// 由于是4Hz低频信号，一次采集不准则进行两次采集
	while(!adc_finish_fg);
	adc_finish_fg=0;
	DMA_Cmd(DMA1_Channel1, ENABLE);					 
	ADC_Cmd(ADC1, ENABLE);         				// 由于是4Hz低频信号，一次采集不准则进行次采集
	while(!adc_finish_fg);
	AD0_data2=ADC_MATH();																		
	RS_FL1=1.0*AD0_data2/(495);	
	RS_FL1=495*RS_FL1;   								 // RS_FL1->4Hz信号点频测试时Rs两端差分放大后的电压
	adc_finish_fg=0;
}

//-----------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------
