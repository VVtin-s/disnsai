#include "stm32f10x.h"                  // Device header
#ifndef  __KEY_EXTI
#define	__KEY_EXTI	

void KeyExti_Init(void);
void EXTI4_IRQHandler(void);
uint16_t Get_EXTI_Count(void);
void Keydelay_ms(uint16_t ms);
#endif
