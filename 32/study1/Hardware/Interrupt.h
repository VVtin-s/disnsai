#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__
#include "stm32f10x.h"                  // Device header

void CountSensor_Init(void);
uint16_t CountNum_get(void);

#endif
