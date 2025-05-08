//-----------------------------------------------------------------
// 程序描述:
// 　　故障检测驱动程序
// 作　　者: 凌智电子
// 开始日期: 2019-11-15
// 完成日期: 2019-12-18
// 修改日期: 2019-12-26
// 版　　本: V1.0
// 历史版本:
// 　- V1.0: 
// 调试工具: STM32F103、2.8寸液晶屏
// 说　　明: 
//				
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// 头文件包含
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
// 变量定义
//-----------------------------------------------------------------
char buf[25];
u32 ad2,DC,DC_LOAD;
u32 gain,ri;	
float ad1,vs,RS_v=0;
extern u32 Gain;
extern uint16_t       Data_Rms[2082];					//采集点数 
extern uint16_t       Data_Rms1[198];					//采集点数 
extern volatile u16 Pulse_Val;
extern double freq_high,freq_low,Au_db;
extern double freq_high1,freq_low1;
extern double freq_high2,freq_low2,Au_db1;
extern u32 Ri,AC_ref,AC_ref1,AC_ref2,AC_ref3,AC_FH,AC_FL,RS_FL,RS_ref;//RS_ref表示1KHz正常工作时串接电阻两端放大后的电压值
extern float RS_ref1,RS_FL1; 

//-----------------------------------------------------------------
// 函数声明
//-----------------------------------------------------------------
u8 ERR_c(void);
u8 ERR_r2r3(void);
//-----------------------------------------------------------------
// u8 check_err (void)
//-----------------------------------------------------------------
//
// 函数功能: 故障检测
// 入口参数: 无
// 返回参数: 故障类型
// 注意事项: 无
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
// 函数功能: 故障检测
// 入口参数: 无
// 返回参数: 故障类型
// 注意事项: 1.参数测量
//           2.故障检测
//-----------------------------------------------------------------
u8 check_err1(void)
{	
	u8 i;	
	u8 err ;
	u16 AD0_data;	
	err = Check_OK;
	freq_high = 150;
	freq_low = 0.420;	
	Gate_Set;         											// 空载
	Delay_ms(30);
	AD9833_SetFrequency(AD9833_REG_FREQ0, 1000 , AD9833_OUT_SINUS, 0);//AD9833_DDS产生1KHz正弦信号
	
	// 测量输入端串入的采样电阻两端的电压 
	Delay_ms(5);
	ADC_TIM3_Init(100,1);										// 定时1.39us采集一次数据
	AD0_data=0;															// 初始ADC采集值为0
	DMA_Cmd(DMA1_Channel1, ENABLE);					// 使能DMA 
	ADC_Cmd(ADC1, ENABLE);				 					// 启动ADC1
	while(!adc_finish_fg);												// 等待ADC1采集完成
	AD0_data=ADC_MATH();										// 获取峰峰值														
	adc_finish_fg=0;															// 采集完成标志位清零
	vs=1.0*AD0_data/(495);						     	// 实际放大倍数不是理想的495倍, 而是带有校准系数
	ad1=495*vs;   													// 检错时Rs两端交流信号
	RS_v=ad1;   														// RS_v表示检查故障时串接电阻两端放大后的电压值

	// 测量输出端空载时的输出交流电压
	ad2=0;																	
	for(i=0;i<20;i++)
	{
		ADC_Sampling ();											// ADC2批量数据采集并获取峰峰值
		ad2+=Adc_data[1];    					  // 共采集20次
	}
	ad2/=20;
  ad2=ad2*1;														 // 空载交流电压适当校准
	gain=ad2/(12-vs);										 	 // 计算增益值, vs是输入电阻两端的电压（电压值很小）
	
	// 测量输出端空载时的输出直流电压
	DC=0;
	for(i=0;i<20;i++)
	{
		ADC3_Sampling();											// ADC3批量数据采集并获取输出直流值
		DC+=Adc_data[2];    							// 共采集20次
	}
	DC/=20;
	//VCC = 12183;
	DC = DC*4.57;                					  // 对直流的进行电压变换, 因为直流输入被衰减了
//*******************************************//
//	以下显示便于直观观察测量值
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
//	具体故障检测
//*******************************************//	
	
	if (DC <= 52)    																	// 测得输出直流接近0V时
		err = ERR_R4_Short;															// 故障: R4短路

	// 电容故障判断
	else if((DC >= DC_ref-100)&&(DC <= DC_ref+100))   // 直流电压在正常静态工作点DC_ref左右时，通过点频进行故障判断
	{
		err=ERR_c();
	}
	
  //R2开路
	else if((DC >=VCC/4)&&(DC < DC_ref-100))   				//输出直流大小在3V到5.2V
	{
		err = ERR_R2_Open;
	}
	
	//R1短路
	else if((DC >= VCC-1300)&&(DC < VCC-500))   			//输出直流在11.3V左右，但比R2短路小几百mV
	{
		err = ERR_R1_Short;
	}
	
	//R3开路
	else if((DC>VCC/230)&&(DC<=VCC/6.6))							// 直流在52mv到1.8V之间为R3开路
		err = ERR_R3_Open;
	
	//R2,R3短路 R1,R4开路
	else if((DC <= VCC+500)&&(DC >= VCC-500))  				//直流等于电源电压VCC(12V)左右所要判断的区域(判断输入阻抗的变化)
	{
		err = ERR_r2r3();
	}
	return err;
}

//-----------------------------------------------------------------
// u8 ERR_c(void)
//-----------------------------------------------------------------
//
// 函数功能: 电容故障检测
// 入口参数: 无
// 返回参数: 电容故障类型
// 注意事项: 无
//-----------------------------------------------------------------
u8 ERR_c(void)
{
	u8  errc ;
  freq_high = 150;
	freq_low = 0.420;	
  PoinTFreTest();													// 调用点频测试函数, 得到各个频率点采集值, 以便与正常值比较得出结果
  TJCPrintf( "n0.val=%d", (int)RS_ref1);
	//sprintf((char *)buf,"RS_ref1:%5f, ",);
	TJCPrintf( "n5.val=%d", (int)RS_FL1);
	//LCD_WriteString(140,220,YELLOW,BLACK,(u8 *)buf);
	//sprintf((char *)buf,"RS_FL1:%5f, ",RS_FL1);
	//LCD_WriteString(140,200,YELLOW,BLACK,(u8 *)buf);
	// 2C1/2C2/2C3/C3开路这四种故障不影响三极管放大倍数, 即和正常工作时输出交流大小基本相等
		if(ad2 >= AC_ref-100 ) 									// 电容故障时测得的输出交流ad2与正常工作时输出交流AC_ref进行比较
		{
			freq_high1 = 0;
			freq_low1 = 0;
			Delay_ms(10);
			if(AC_FH > AC_ref2+50)								// AC_FH为检查故障时160kHz频率输入的输出交流电压; AC_ref2为正常工作时160kHz频率输入的输出交流电压 								
				errc = ERR_C3_Open;									// 故障: C3开路
			else if(AC_FH < AC_ref2-50) 
			{
				errc = ERR_C3_Double;								// 故障: C3→2C3
			}
			else 																	// RS_FL1->4Hz信号点频测试时Rs两端电压, RS_ref1->4Hz信号正常工作时Rs两端电压
				if((RS_FL1 >= RS_ref1+20)&&(RS_FL1 <= RS_ref1+100))	// C1与2倍C1的电压值相差26mvpp左右
				{
					errc = ERR_C1_Double;							// 故障: C1→2C1
				}			
			if(AC_FL > AC_ref1+40)								// AC_FL表示故障时400Hz低频信号的输出交流,AC_ref1为与AC_FL同频率时正常工作的输出交流
					errc = ERR_C2_Double;							// 故障: C2→2C2, 调试时，将AC_FL和AC_ref1的值显示出来，改变电路参数，根据实际值来做调整
		}
		// C1开路/C2开路这两种故障极大影响三极管放大倍数, 即比正常工作时输出交流大小要小很多
		else 
			if(ad2<AC_ref-100)
			{
				if((ad1>=RS_ref/4)&&(ad1<2000))    	// 故障时Rs两端电压ad1与正常时Rs两端电压RS_ref进行比较(四分之一RS_ref刚好可做临界值),且要小于2v左右
					 errc = ERR_C2_Open;								// 故障: C2开路
				else 
					if(ad1<RS_ref/11)									// Rs电压(ad1)小于100接近0时为C1开路
					   errc = ERR_C1_Open;							// 故障: C1开路
			}
  return errc;  
}
//-----------------------------------------------------------------
// u8 ERR_r2r3(void)
//-----------------------------------------------------------------
//
// 函数功能: R2,R3短路R1,R4开路
// 入口参数: 无
// 返回参数: 故障类型
// 注意事项: 无
//-----------------------------------------------------------------
u8 ERR_r2r3(void)
{
	 u8  errc;
   ri=1.0*(2000*(US-vs))/vs;    									// 根据分压公式
//*******************************************//
//	测量不准时可以取消下列注释后进行调试修改
//*******************************************//		
//		sprintf((char *)buf,"ri:%5d, ",ri);								//输入阻抗故障值
//		LCD_WriteString(140,140,YELLOW,BLACK,(u8 *)buf);
//		sprintf((char *)buf,"Ri:%5d, ",Ri);								//输入阻抗正常值
//		LCD_WriteString(140,160,YELLOW,BLACK,(u8 *)buf);
//		sprintf((char *)buf,"RS_ref:%5d, ",RS_ref);				//采样电阻正常值
//		LCD_WriteString(140,100,YELLOW,BLACK,(u8 *)buf);
//		sprintf((char *)buf,"RS_v:%5f, ",RS_v);						//采样电阻故障值
//		LCD_WriteString(140,120,YELLOW,BLACK,(u8 *)buf);
	Gate_Set;   																	    	// 空载
	if(ri < Ri-400)     																// 有错时测得的输入电阻ri与正常时的输入电阻Ri比较
		errc = ERR_R2_Short; 
	else 
		if(ri > Ri+600)
		{
			if((int)RS_v < (RS_ref/3.33))								    // RS_v表示检查故障时串接电阻两端放大后的电压值
			errc = ERR_R1_Open;
			else  if((int)RS_v >= (RS_ref/3.22)&&(RS_v<=RS_ref-100)) // 因为RS端电压小于正常工作RS_ref的有R1_Open和R4_Open
			errc = ERR_R4_Open;	
		}
		else  
		{			
			Gate_Clr;																						// 带载测输出直流
			Delay_ms(50);
			ADC3_Sampling();
			DC_LOAD=Adc_data[2];   												// 带载时的直流输出电压
			DC_LOAD = DC_LOAD*4.57;															// 硬件电路设置有衰减！！！
			if(DC_LOAD >= VCC-500)														// 带载直流大于等于VCC便是R3短路
			{errc = ERR_R3_Short;}
			TJCPrintf( "n6.val=%d", (int)DC_LOAD);
			//sprintf((char *)buf,"DC_LOAD:%d, ",DC_LOAD);			//采样电阻故障值
	   // LCD_WriteString(140,120,YELLOW,BLACK,(u8 *)buf);
		}
		return errc;
 }

//-----------------------------------------------------------------
// void display_err_info(u8 err)
//-----------------------------------------------------------------
//
// 函数功能: 故障类型显示
// 入口参数: 故障类型
// 返回参数: 无
// 注意事项: 无
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
