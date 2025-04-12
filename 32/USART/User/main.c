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
	OLED_Init();
	Serial_Init();
	
	Serial_TxPacket[0] = 0x01;
	Serial_TxPacket[1] = 0x02;
	Serial_TxPacket[2] = 0x03;
	Serial_TxPacket[3] = 0x04;
	
	Serial_SendPacket();
	
	while(1)
	{
		if (Serial_GetRxFlag() == 1)
		{
			OLED_ShowHexNum(1, 1, Serial_RxPacket[0], 2);
			OLED_ShowHexNum(1, 4, Serial_RxPacket[1], 2);
			OLED_ShowHexNum(1, 7, Serial_RxPacket[2], 2);
			OLED_ShowHexNum(1, 10, Serial_RxPacket[3], 2);
		}
	}
}
