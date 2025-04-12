#include "stm32f10x.h"                  // Device header

#include "Interrupt.h"
uint16_t CountSensorNum;
void CountSensor_Init()
{
    //时钟启用
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    //GPIO
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;      //pin
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB , &GPIO_InitStructure);
    //AFIO
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14); //设置引脚和固定中断路线
    //EXTI
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line14;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure);
    //NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
    
}

void EXTI15_10_IRQHandler(void)
{
    if(EXTI_GetFlagStatus(EXTI_Line14)== SET)
    {
        CountSensorNum++;
        EXTI_ClearITPendingBit(EXTI_Line14);
    }
}

uint16_t CountNum_get()
{
    return CountSensorNum;
}
