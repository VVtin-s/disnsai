#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Led.h"
#include "Key.h"
#include "OLED.h"
#include "Interrupt.h"
uint8_t KeyNum;

void lightaLed()//exp1
    {
        GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
        Delay_ms(500);
        GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
        Delay_ms(500);
    }
    
void runninglight()//exp2
    {
        GPIO_Write(GPIOA, ~0x0001);
        Delay_ms(500);
        GPIO_Write(GPIOA, ~0x0002);
        Delay_ms(500);
        GPIO_Write(GPIOA, ~0x0004);
        Delay_ms(500);
        GPIO_Write(GPIOA, ~0x0008);
        Delay_ms(500);
        GPIO_Write(GPIOA, ~0x0010);
        Delay_ms(500);
        GPIO_Write(GPIOA, ~0x0020);
        Delay_ms(500);
        GPIO_Write(GPIOA, ~0x0040);
        Delay_ms(500);
        GPIO_Write(GPIOA, ~0x0080);
        Delay_ms(500);
    }
    
void Ledlight2()
{
    Led1_on();
    Led2_off();
    Delay_ms(500);
    Led1_off();
    Led2_on();
    Delay_ms(500);
}

void Led_Key()
{
    KeyNum = Key_GetNum();
    if(KeyNum == 1)
    {   
        Led1_on();
    }
    if(KeyNum == 2)
    {   
        Led1_off();
    }
}
void Led_Kturn()
{
    KeyNum = Key_GetNum();
    if(KeyNum == 1)
    {   
        Led1_turn();
    }
    if(KeyNum == 2)
    {   
        Led2_turn();
    }
    
}
void Oled1()
{
    OLED_Init();
    
    OLED_ShowChar(1,1,'A');
    OLED_ShowString(1,3,"Hello world!");
    OLED_ShowNum(2,1,123456,6);
    OLED_ShowSignedNum(2,7,-66,2);
    OLED_ShowHexNum(3,1,0xAA55,4);
    OLED_ShowBinNum(4,1,0xAA55,16);
    //OLED_Clear();
}

int main()
{
    LedInit();
    InitKey();
    CountSensor_Init();
    //choose one function bellow to test.
   // GPIO_ResetBits(GPIOA,GPIO_Pin_0);
   // GPIO_SetBits(GPIOA, GPIO_Pin_0);
    //GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
    //中断实验
    //OLED_ShowString(1,1,"Count:")
    
    while(1)
    {
        lightaLed();
        //中断实验
        //OLED_ShowNum(1,7,CountNum_get(),5);
    }
    
}
