#include "stm32f10x.h"    // Device header
#include "Delay.h"
void LedInit()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
    
    GPIO_InitTypeDef GPIO_InitLED;
    GPIO_InitLED.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitLED.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2;//GPIO_Pin_All
    GPIO_InitLED.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (GPIOA , &GPIO_InitLED);
  
    GPIO_SetBits(GPIOA,GPIO_Pin_All);
}

void Led1_on()
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_0,Bit_SET);
    Delay_ms(500);
}
void Led1_off()
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_0,Bit_RESET);
    Delay_ms(500);
}
void Led2_on()
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1,Bit_SET);
    Delay_ms(500);
}
void Led2_off()
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1,Bit_RESET);
    Delay_ms(500);
}
void Led1_turn()
{
    if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_1) == 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_1);
    }
    else
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    }
}
void Led2_turn()
{
    if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2) == 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_2);
    }
    else
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_2);
    }
}
