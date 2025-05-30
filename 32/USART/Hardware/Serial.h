#ifndef __SERIAL_H
#define __SERIAL_H

#include "stdio.h"

extern uint16_t Serial_RxFlag;

void Serial_Init(void);
void Serial_SendByte(uint16_t Byte);
void Serial_SendArray(uint16_t *Array , uint16_t Length);
void Serial_SendString(char *String);
uint32_t Serial_Pow(uint32_t X , uint32_t Y);
void Serial_SendNumber(uint32_t Number , uint16_t Length);
int fputc(int ch, FILE *f);
void Serial_Printf(char *format , ...);
uint16_t Serial_GetRxData(void);
uint16_t Serial_GetRxFlag(void);
//void USART1_IRQHandler(void);

#endif
