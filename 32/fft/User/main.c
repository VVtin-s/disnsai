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
#include "Serial.h"
#include "SerialPackRT.h"
#include "math.h"
#include "arm_math.h"

uint16_t Rx_Data;

int main()
{
		Serial_Init();
		float data;//????float fefe
		while(1)
		{
			data=arm_sin_f32(3.1415926/6);//sin(30�),????0.5
			Serial_Printf("%f",data);
			Delay_ms(1000);
		}

	
}
