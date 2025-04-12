#ifndef __SERIALPACKRT_H
#define __SERIALPACKRT_H
#include "stdio.h"

extern uint16_t Serial_TxPacket[4];
extern uint16_t Serial_RxPacket[4];

void USART1_IRQHandler(void);

void Serial_SendPacket(void);


#endif
