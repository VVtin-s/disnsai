#include "stm32f10x.h"                  // Device header
#include "main.h"
#include "delay.h"
#include "Led.h"
#include "Key.h"
#include  "oled.h"
uint8_t KeyNum;
int main()
{
	LED_Init();
	OLED_Init();
	OLED_ShowString( 1 , 1 , "Loaa");
	OLED_ShowString( 1, 5, "My baby." );
	OLED_ShowNum(2 , 1, 521 , 3);	
	LED1_On();
	while(1)
	{
	
		
	}
}
