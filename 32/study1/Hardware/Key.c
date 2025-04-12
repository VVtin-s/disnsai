#include "stm32f10x.h"                  // Device header
#include "Key.h"
#include "Delay.h"

void InitKey()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_Key;
    GPIO_Key.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Key.GPIO_Pin = GPIO_Pin_10 ;//按照实际更改
    GPIO_Key.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&GPIO_Key);
}

uint8_t Key_GetNum()
{
    uint8_t KeyNum = 0;
    if ( GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)//需要更改
    {
        Delay_ms(20);
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1)==1)
        {
        }
        Delay_ms(20);
        KeyNum = 1;
    }
    if ( GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)//需要更改
    {
        Delay_ms(20);
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11)==1)
        {
        }
        Delay_ms(20);
        KeyNum = 2;
    }
   
    return KeyNum;
}
