#include "stm32f10x.h"   // Device header
#include "Beep.h"
uint16_t EXTI_Count;

void KeyExti_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE , ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE , &GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE , GPIO_PinSource4);
	
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel  = EXTI4_IRQn ;
	NVIC_InitStructure.NVIC_IRQChannelCmd  = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority  = 1;
	NVIC_Init(&NVIC_InitStructure);
	
}

uint16_t Get_EXTI_Count(void)
{
	return EXTI_Count;
}

void Keydelay_ms(uint16_t ms)
{
	int i = 0;
	while(ms--)
	{
		i = 12000;
		while(i--);
	}
}
//void EXTI4_IRQHandler(void)
//{
//	if(EXTI_GetITStatus(EXTI_Line4) == SET)
//	{
//		Keydelay_ms(30);
//		if(EXTI_GetITStatus(EXTI_Line4) == SET)
//		{
//			EXTI_Count++;
//			Beep_Turn();
//			EXTI_ClearITPendingBit(EXTI_Line4);
//		}
//	}

//}
