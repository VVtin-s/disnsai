/**********************************************************
                       ��������
���ܣ�stm32f103rct6����AD9833ģ��
�ӿڣ��������Žӿ������AD9833.h
ʱ�䣺2023/06/08
�汾��2.1
���ߣ���������
������������ֻ��ѧϰʹ�ã�����ؾ���

������������뵽�Ա��꣬�������ӽ߳�Ϊ������ ^_^
https://kvdz.taobao.com/ 
**********************************************************/
#include "AD9833.h"		
#include "delay.h"	
#include "stm32f10x.h"                  // Device header
#include "Serial.h"
//ʱ������Ϊ25 MHzʱ�� ����ʵ��0.1 Hz�ķֱ��ʣ���ʱ������Ϊ1 MHzʱ�������ʵ��0.004 Hz�ķֱ��ʡ�
//�����ο�ʱ���޸Ĵ˴����ɡ�
#define FCLK 25000000	//���òο�ʱ��25MHz����Ĭ�ϰ��ؾ���Ƶ��25Mhz��

#define RealFreDat    268435456.0/FCLK//�ܵĹ�ʽΪ Fout=��Fclk/2��28�η���*28λ�Ĵ�����ֵ

/************************************************************
** �������� ��void AD983_GPIO_Init(void)  
** �������� ����ʼ������AD9833��Ҫ�õ���IO��
** ��ڲ��� ����
** ���ڲ��� ����
** ����˵�� ����
**************************************************************/

void AD983_GPIO_Init(uint8_t choise)
{
	GPIO_InitTypeDef GPIO_InitStructure ;
	switch(choise){
		case 0:			
     
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 //ʹ��PA�˿�ʱ��

			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10| GPIO_Pin_11| GPIO_Pin_12; 

			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz ; 

			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ; 

			GPIO_Init(GPIOE ,&GPIO_InitStructure) ; 
		
			break;
			
			
		case 1:
		
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);	 //ʹ��PG�˿�ʱ��

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
	// ���Ƶ��ΪFmclk/��2^28��*freq    freq����Ƶ�ʿ����֣�Fmclk����25MHZ����
}

/**********************************************************************************************
** �������� ��unsigned char AD9833_SPI_Write(unsigned char* data,unsigned char bytesNumber)
** �������� ��ʹ��ģ��SPI��AD9833д����
** ��ڲ��� ��* data:д�����ݻ�����,��һ���ֽ��ǼĴ�����ַ���ڶ����ֽڿ�ʼҪд������ݡ�
						bytesNumber: Ҫд����ֽ���
** ���ڲ��� ����
** ����˵�� ����
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
** �������� ��void AD9833_Init(void)  
** �������� ����ʼ������AD9833��Ҫ�õ���IO�ڼ��Ĵ���
** ��ڲ��� ����
** ���ڲ��� ����
** ����˵�� ����
**************************************************************/
void AD9833_Init(uint8_t choise)
{
    AD983_GPIO_Init(choise);
    AD9833_SetRegisterValue(AD9833_REG_CMD | AD9833_RESET, choise);
}

/*****************************************************************************************
** �������� ��void AD9833_Reset(void)  
** �������� ������AD9833�ĸ�λλ
** ��ڲ��� ����
** ���ڲ��� ����
** ����˵�� ����
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
** �������� ��void AD9833_ClearReset(void)  
** �������� �����AD9833�ĸ�λλ��
** ��ڲ��� ����
** ���ڲ��� ����
** ����˵�� ����
*******************************************************************************************/
void AD9833_ClearReset(uint8_t choise)
{
	AD9833_SetRegisterValue(AD9833_REG_CMD, choise);
}

/*****************************************************************************************
** �������� ��void AD9833_SetRegisterValue(unsigned short regValue)
** �������� ����ֵд��Ĵ���
** ��ڲ��� ��regValue��Ҫд��Ĵ�����ֵ��
** ���ڲ��� ����
** ����˵�� ����
*******************************************************************************************/
void AD9833_SetRegisterValue(unsigned short regValue, uint8_t choise)
{
	unsigned char data[5] = {0x03, 0x00, 0x00};	
	
	data[1] = (unsigned char)((regValue & 0xFF00) >> 8);
	data[2] = (unsigned char)((regValue & 0x00FF) >> 0);
	AD9833_SPI_Write(data,2, choise);
}

/*****************************************************************************************
** �������� ��void AD9833_SetFrequencyQuick(float fout,unsigned short type)
** �������� ��д��Ƶ�ʼĴ���
** ��ڲ��� ��val��Ҫд���Ƶ��ֵ��
**						type���������ͣ�AD9833_OUT_SINUS���Ҳ���AD9833_OUT_TRIANGLE���ǲ���AD9833_OUT_MSB����
** ���ڲ��� ����
** ����˵�� ��ʱ������Ϊ25 MHzʱ�� ����ʵ��0.1 Hz�ķֱ��ʣ���ʱ������Ϊ1 MHzʱ�������ʵ��0.004 Hz�ķֱ��ʡ�
*******************************************************************************************/
void AD9833_SetFrequencyQuick(float fout,unsigned short type, uint8_t choise)
{
	AD9833_SetFrequency(AD9833_REG_FREQ0, fout,type, choise);
}

/*****************************************************************************************
** �������� ��void AD9833_SetFrequency(unsigned short reg, float fout,unsigned short type)
** �������� ��д��Ƶ�ʼĴ���
** ��ڲ��� ��reg��Ҫд���Ƶ�ʼĴ�����
**						val��Ҫд���ֵ��
**						type���������ͣ�AD9833_OUT_SINUS���Ҳ���AD9833_OUT_TRIANGLE���ǲ���AD9833_OUT_MSB����
** ���ڲ��� ����
** ����˵�� ����
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
** �������� ��void AD9833_SetPhase(unsigned short reg, unsigned short val)
** �������� ��д����λ�Ĵ�����
** ��ڲ��� ��reg��Ҫд�����λ�Ĵ�����
**						val��Ҫд���ֵ��
** ���ڲ��� ����
** ����˵�� ����
*******************************************************************************************/
void AD9833_SetPhase(unsigned short reg, unsigned short val, uint8_t choise)
{
	unsigned short phase = reg;
	phase |= val;
	AD9833_SetRegisterValue(phase, choise);
}

/*****************************************************************************************
** �������� ��void AD9833_Setup(unsigned short freq, unsigned short phase,unsigned short type)
** �������� ��д����λ�Ĵ�����
** ��ڲ��� ��freq��ʹ�õ�Ƶ�ʼĴ�����
							phase��ʹ�õ���λ�Ĵ�����
							type��Ҫ����Ĳ������͡�
** ���ڲ��� ����
** ����˵�� ����
*******************************************************************************************/
void AD9833_Setup(unsigned short freq, unsigned short phase,unsigned short type, uint8_t choise)
{
	unsigned short val = 0;
	
	val = freq | phase | type;
	AD9833_SetRegisterValue(val , choise);
}

/*****************************************************************************************
** �������� ��void AD9833_SetWave(unsigned short type)
** �������� ������Ҫ����Ĳ������͡�
** ��ڲ��� ��type��Ҫ����Ĳ������͡�
** ���ڲ��� ����
** ����˵�� ����
*******************************************************************************************/
void AD9833_SetWave(unsigned short type, uint8_t choise)
{
	AD9833_SetRegisterValue(type, choise);
}








