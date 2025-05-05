#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "stdarg.h"

uint16_t Serial_RxData;
uint16_t Serial_RxFlag;

uint16_t Serial_TxPacket[4];
uint16_t Serial_RxPacket[4];


//void Serial_Init(void)
//{
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
//	
//	GPIO_InitTypeDef GPIO_InitStucture;
//	GPIO_InitStucture.GPIO_Mode = GPIO_Mode_AF_PP;    //TX_GPIO
//	GPIO_InitStucture.GPIO_Pin = GPIO_Pin_9;
//	GPIO_InitStucture.GPIO_Speed  = GPIO_Speed_50MHz;
//	GPIO_Init( GPIOA, &GPIO_InitStucture );
//	
//	GPIO_InitStucture.GPIO_Mode = GPIO_Mode_IPU;			//RX_GPIO
//	GPIO_InitStucture.GPIO_Pin = GPIO_Pin_10;
//	GPIO_InitStucture.GPIO_Speed  = GPIO_Speed_50MHz;
//	GPIO_Init( GPIOA, &GPIO_InitStucture );
//	
//	USART_InitTypeDef USART_InitStructure;
//	USART_InitStructure.USART_BaudRate = 115200;
//	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;					//USART Init
//	USART_InitStructure.USART_Parity = USART_Parity_No;
//	USART_InitStructure.USART_StopBits = USART_StopBits_1;
//	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//	USART_Init(USART1 , &USART_InitStructure);
//	
//	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);									//NVIC IT Init
//	
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//	
//	NVIC_InitTypeDef NVIC_InitStructure;
//	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_Init(&NVIC_InitStructure);
//	
//	USART_Cmd(USART1 , ENABLE);
//}

//发送字节
void Serial_SendByte(uint16_t Byte)
{
	USART_SendData(USART1 , Byte);
	while(USART_GetFlagStatus(USART1 , USART_FLAG_TXE) == RESET)
	{
		;
	}
}

//发送数组
void Serial_SendArray(uint16_t *Array , uint16_t Length)
{
	uint16_t i;
	for(i = 0 ; i<Length ; i++)
	{
		Serial_SendByte(Array[i]);
	}
}
void Serial_SendString(char *String)
{
	uint16_t i;
	for(i = 0; String[i] != '\0' ; i++)
	{
		Serial_SendByte(String[i]);
	}
}

//乘方计算
uint32_t Serial_Pow(uint32_t X , uint32_t Y)
{
	uint32_t Result = 1;
	while(Y--)
	{
		Result *= X;
	}
	return X;
}

//发送数字
void Serial_SendNumber(uint32_t Number , uint16_t Length)
{
	uint16_t i;
	for( i = 0 ; i < Length ; i++)
	{
		Serial_SendByte( Number / Serial_Pow(10 , Length - i - 1) %10 + '0');
	}
}

//重定义fputc
//int fputc(int ch, FILE *f)
//{
//	Serial_SendByte(ch);
//	return ch;
//}
//重写prinrf
void Serial_Printf(char *format , ...)
{
	char Srting[100];
	va_list arg;
	va_start(arg , format);
	vsprintf(Srting , format , arg);
	va_end(arg);
	Serial_SendString(Srting);
}

//中断函数
void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		Serial_RxData = USART_ReceiveData(USART1);
		Serial_RxFlag = 1;
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

//获取接受标志
uint16_t Serial_GetRxFlag(void)
{
	if(Serial_RxFlag == 1)
	{
		Serial_RxFlag = 0;
		return 1;
	}
	return 0;
}

//获取接受信息
uint16_t Serial_GetRxData(void)
{
	return Serial_RxData;
}


void Serial_SendPacket(void)
{
	Serial_SendByte(0xFF);
	Serial_SendArray(Serial_TxPacket, 4);
	Serial_SendByte(0xFE);
}

