#ifndef __TIMING_SAMPLING_H
#define __TIMING_SAMPLING_H

void STimer_Init(u16 arr, u16 psc);
void DMA1_Init(void);
void myADC_Init(void);
void DMA1_Channel1_IRQHandeler(void);

#endif
