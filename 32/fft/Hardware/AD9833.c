/**********************************************************
                       康威电子
功能：stm32f103rct6控制AD9833模块
接口：控制引脚接口请参照AD9833.h
时间：2023/06/08
版本：2.1
作者：康威电子
其他：本程序只供学习使用，盗版必究。

更多电子需求，请到淘宝店，康威电子竭诚为您服务 ^_^
https://kvdz.taobao.com/ 
**********************************************************/
#include "AD9833.h"		
#include "delay.h"	
#include "stm32f10x.h"                  // Device header
#include "Serial.h"
//时钟速率为25 MHz时， 可以实现0.1 Hz的分辨率；而时钟速率为1 MHz时，则可以实现0.004 Hz的分辨率。
//调整参考时钟修改此处即可。
#define FCLK 25000000	//设置参考时钟25MHz，板默认板载晶振频率25Mhz。

#define RealFreDat    268435456.0/FCLK//总的公式为 Fout=（Fclk/2的28次方）*28位寄存器的值

/************************************************************
** 函数名称 ：void AD983_GPIO_Init(void)  
** 函数功能 ：初始化控制AD9833需要用到的IO口
** 入口参数 ：无
** 出口参数 ：无
** 函数说明 ：无
**************************************************************/

void AD983_GPIO_Init(uint8_t choise)
{
	GPIO_InitTypeDef GPIO_InitStructure ;
	switch(choise){
		case 0:			
     
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 //使能PA端口时钟

			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10| GPIO_Pin_11| GPIO_Pin_12; 

			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz ; 

			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ; 

			GPIO_Init(GPIOE ,&GPIO_InitStructure) ; 
		
			break;
			
			
		case 1:
		
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);	 //使能PG端口时钟

			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2| GPIO_Pin_3| GPIO_Pin_4; 

			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz ; 

			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ; 

			GPIO_Init(GPIOG ,&GPIO_InitStructure) ; 
		
			break;	
		default:
			break;
	}
} 

unsigned long freq_trl_word (unsigned long freq)
{
	u32 a;
	a=freq * 10.73742;
	return (unsigned long )a;				
	// 输出频率为Fmclk/（2^28）*freq    freq计算频率控制字，Fmclk采用25MHZ晶振
}

/**********************************************************************************************
** 函数名称 ：unsigned char AD9833_SPI_Write(unsigned char* data,unsigned char bytesNumber)
** 函数功能 ：使用模拟SPI向AD9833写数据
** 入口参数 ：* data:写入数据缓冲区,第一个字节是寄存器地址；第二个字节开始要写入的数据。
						bytesNumber: 要写入的字节数
** 出口参数 ：无
** 函数说明 ：无
************************************************************************************************/
unsigned char AD9833_SPI_Write(unsigned char* data,unsigned char bytesNumber, uint8_t choise)
{
	Serial_Printf("SPI Start");
  unsigned char i,j; 
	unsigned char writeData[5]	= {0,0, 0, 0, 0};
	
    AD9833_SCLKE = 1; 
    AD9833_FSYNCE = 0 ; 
		AD9833_SCLKG = 1; 
    AD9833_FSYNCG = 0 ; 
	switch(choise){
		case 0:
		
				for(i = 0;i < bytesNumber;i ++)
				{
					writeData[i] = data[i + 1];
				}
				
				for(i=0 ;i<bytesNumber ;i++) 
				{
					for(j=0 ;j<8 ;j++)      
					{ 
							if(writeData[i] & 0x80) 
							{
								AD9833_SDATAE = 1; 
							Serial_Printf("sdata1");
							}
								else 
								AD9833_SDATAE = 0; 

							AD9833_SCLKE = 0; 
							writeData[i] <<= 1; 
							AD9833_SCLKE = 1; 
					}  
				}
					AD9833_SDATAE = 1; 
					AD9833_FSYNCE = 1; 
			break;
		
		case 1:
			
				for(i = 0;i < bytesNumber;i ++)
				{
					writeData[i] = data[i + 1];
				}
				
				for(i=0 ;i<bytesNumber ;i++) 
				{
					for(j=0 ;j<8 ;j++)      
					{ 
							if(writeData[i] & 0x80) 
								AD9833_SDATAG = 1; 
							else 
								AD9833_SDATAG = 0; 

							AD9833_SCLKG = 0; 
							writeData[i] <<= 1; 
							AD9833_SCLKG = 1; 
					}  
				}
					AD9833_SDATAG = 1; 
					AD9833_FSYNCG = 1; 
				break;
				default:
					break;
	}
	return i;
}

/************************************************************
** 函数名称 ：void AD9833_Init(void)  
** 函数功能 ：初始化控制AD9833需要用到的IO口及寄存器
** 入口参数 ：无
** 出口参数 ：无
** 函数说明 ：无
**************************************************************/
void AD9833_Init(uint8_t choise)
{
    AD983_GPIO_Init(choise);
    AD9833_SetRegisterValue(AD9833_REG_CMD | AD9833_RESET, choise);
}

/*****************************************************************************************
** 函数名称 ：void AD9833_Reset(void)  
** 函数功能 ：设置AD9833的复位位
** 入口参数 ：无
** 出口参数 ：无
** 函数说明 ：无
*******************************************************************************************/
void AD9833_Reset(uint8_t choise)
{
	switch(choise){
		case 0:
			
			AD9833_SetRegisterValue(AD9833_REG_CMD | AD9833_RESET, choise);
			Delay_ms(10);
			break;
		case 1:
		
			AD9833_SetRegisterValue(AD9833_REG_CMD | AD9833_RESET, choise);
			Delay_ms(10);
			break;
		default:
			break;
	}
}

/*****************************************************************************************
** 函数名称 ：void AD9833_ClearReset(void)  
** 函数功能 ：清除AD9833的复位位。
** 入口参数 ：无
** 出口参数 ：无
** 函数说明 ：无
*******************************************************************************************/
void AD9833_ClearReset(uint8_t choise)
{
	AD9833_SetRegisterValue(AD9833_REG_CMD, choise);
}

/*****************************************************************************************
** 函数名称 ：void AD9833_SetRegisterValue(unsigned short regValue)
** 函数功能 ：将值写入寄存器
** 入口参数 ：regValue：要写入寄存器的值。
** 出口参数 ：无
** 函数说明 ：无
*******************************************************************************************/
void AD9833_SetRegisterValue(unsigned short regValue, uint8_t choise)
{
	unsigned char data[5] = {0x03, 0x00, 0x00};	
	
	data[1] = (unsigned char)((regValue & 0xFF00) >> 8);
	data[2] = (unsigned char)((regValue & 0x00FF) >> 0);
	AD9833_SPI_Write(data,2, choise);
}

/*****************************************************************************************
** 函数名称 ：void AD9833_SetFrequencyQuick(float fout,unsigned short type)
** 函数功能 ：写入频率寄存器
** 入口参数 ：val：要写入的频率值。
**						type：波形类型；AD9833_OUT_SINUS正弦波、AD9833_OUT_TRIANGLE三角波、AD9833_OUT_MSB方波
** 出口参数 ：无
** 函数说明 ：时钟速率为25 MHz时， 可以实现0.1 Hz的分辨率；而时钟速率为1 MHz时，则可以实现0.004 Hz的分辨率。
*******************************************************************************************/
void AD9833_SetFrequencyQuick(float fout,unsigned short type, uint8_t choise)
{
	AD9833_SetFrequency(AD9833_REG_FREQ0, fout,type, choise);
}

/*****************************************************************************************
** 函数名称 ：void AD9833_SetFrequency(unsigned short reg, float fout,unsigned short type)
** 函数功能 ：写入频率寄存器
** 入口参数 ：reg：要写入的频率寄存器。
**						val：要写入的值。
**						type：波形类型；AD9833_OUT_SINUS正弦波、AD9833_OUT_TRIANGLE三角波、AD9833_OUT_MSB方波
** 出口参数 ：无
** 函数说明 ：无
*******************************************************************************************/
void AD9833_SetFrequency(unsigned short reg, float fout,unsigned short type, uint8_t choise)
{
	unsigned short freqHi = reg;
	unsigned short freqLo = reg;
	unsigned long val=RealFreDat*fout;
	freqHi |= (val & 0xFFFC000) >> 14 ;
	freqLo |= (val & 0x3FFF);
	AD9833_SetRegisterValue(AD9833_B28|type, choise);
	AD9833_SetRegisterValue(freqLo, choise);
	AD9833_SetRegisterValue(freqHi, choise);
}

/*****************************************************************************************
** 函数名称 ：void AD9833_SetPhase(unsigned short reg, unsigned short val)
** 函数功能 ：写入相位寄存器。
** 入口参数 ：reg：要写入的相位寄存器。
**						val：要写入的值。
** 出口参数 ：无
** 函数说明 ：无
*******************************************************************************************/
void AD9833_SetPhase(unsigned short reg, unsigned short val, uint8_t choise)
{
	unsigned short phase = reg;
	phase |= val;
	AD9833_SetRegisterValue(phase, choise);
}

/*****************************************************************************************
** 函数名称 ：void AD9833_Setup(unsigned short freq, unsigned short phase,unsigned short type)
** 函数功能 ：写入相位寄存器。
** 入口参数 ：freq：使用的频率寄存器。
							phase：使用的相位寄存器。
							type：要输出的波形类型。
** 出口参数 ：无
** 函数说明 ：无
*******************************************************************************************/
void AD9833_Setup(unsigned short freq, unsigned short phase,unsigned short type, uint8_t choise)
{
	unsigned short val = 0;
	
	val = freq | phase | type;
	AD9833_SetRegisterValue(val , choise);
}

/*****************************************************************************************
** 函数名称 ：void AD9833_SetWave(unsigned short type)
** 函数功能 ：设置要输出的波形类型。
** 入口参数 ：type：要输出的波形类型。
** 出口参数 ：无
** 函数说明 ：无
*******************************************************************************************/
void AD9833_SetWave(unsigned short type, uint8_t choise)
{
	AD9833_SetRegisterValue(type, choise);
}








