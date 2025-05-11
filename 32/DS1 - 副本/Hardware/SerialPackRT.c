#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "stdarg.h"
#include "Serial.h"


//中断函数
//void USART1_IRQHandler(void)
//{
//	static uint16_t Rx_State = 0;
//	static uint16_t RXDataOrder;
//	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
//	{
//		uint16_t Rxdata = USART_ReceiveData(USART1);
//		
//		if (Rx_State == 0)
//		{
//			if(Rxdata == 0xFF)
//			{
//				Rx_State = 1;
//				RXDataOrder = 0;
//			}
//		}
//		else if (Rx_State == 1)
//		{
//			Serial_RxPacket[RXDataOrder] = Rxdata;
//			RXDataOrder ++;
//			if(RXDataOrder>=4)
//			{
//				Rx_State = 2;
//				
//			}
//		}
//		else if (Rx_State == 2)
//		{
//			if(Rxdata == 0xFE)
//			{
//				Rx_State = 0;
//				Serial_RxFlag = 1;
//			}
//		}
//		
//	}
//}


//获取接受信息

