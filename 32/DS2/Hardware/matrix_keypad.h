#ifndef __MATRIX_KEYPAD_H
#define __MATRIX_KEYPAD_H

#include "stm32f10x.h"

extern volatile uint8_t key_pressed;
extern volatile char key_value;

char Keypad_Scan(void);
void Keypad_GPIO_EXTI_Init(void);
void KeyConrolIO_GPIO_init(void);
void GPIO_Turn(uint8_t Key);
void Key_Control(void);

#endif
