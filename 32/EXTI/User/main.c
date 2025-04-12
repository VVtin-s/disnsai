#include "stm32f10x.h"                  // Device header
#include "main.h"
#include "delay.h"
#include "Led.h"
#include "Key.h"
#include  "oled.h"
#include "Key_EXTI.h"
#include "Beep.h"

uint8_t KeyNum;
int main()
{
	LED_Init();
	OLED_Init();
	Key_Init();
	KeyExti_Init();
	Beep_Init();
	LED1_On();
	
	while(1)
	{
		OLED_ShowNum(1,1,Get_EXTI_Count(),2);
		
	}
}
