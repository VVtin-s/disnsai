#include "stm32f10x.h"                  // Device header
#include "main.h"
#include "delay.h"
#include "Led.h"
#include "Key.h"
#include "oled.h"
#include "Key_EXTI.h"
#include "Beep.h"
#include "Timer.h"
#include "PWM.h"
uint8_t KeyNum;
uint16_t Timnum;
uint8_t i;
int main()
{
	PWM_Init();
	
	while(1)
	{
		for(i=0 ; i<=100 ; i++)
		{
			PWM_Setcompare1(i);
			Delay_ms(10);
		}
		for(i=0 ; i<=100 ; i++)
		{
			PWM_Setcompare1(100-i);
			Delay_ms(10);
		}
	}
}
