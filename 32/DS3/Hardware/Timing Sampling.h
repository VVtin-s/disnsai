#include "stm32f10x.h"                  // Device header

#ifndef __TIMING_SAMPLING_H
#define __TIMING_SAMPLING_H
#define sample_num 1024
#define Channel 3
#define CodeSize 1024
#define	VCC    	11800// 12181
#define VREF 3300
extern uint32_t FFT_SourceData[sample_num];
extern uint32_t FFT_OutData[sample_num/2];	  //fft输出序列
extern uint32_t FFT_Mag[sample_num/2] ;	
extern uint16_t Adc_data[sample_num];
extern uint16_t Adc_data1[CodeSize*Channel];
extern uint8_t adc_finish_fg;//adc获取数据标志位

void ADC_GPIO_Init(void);
void ADC_TIM3_Init(u16 arr, u16 psc);
void ADC_DMA1_Init(void);
void ADC_Init_Init(void);
void myADC1_Init(void);
void Get_FFT_Source_Data(void);
void GetPowerMag(void);
void FFT_test(void);
void ResetADC(void);
void ADC_Calibration(ADC_TypeDef * ADCx);
void ADC_RegularChannel_Configuration(void);
void ADC_Initialization(ADC_TypeDef* ADCx);
float ADC_MATH(void);
void ADC_Samp_Sweep (void);
void ADC_Sampling(void) ;
void ADC3_Sampling(void) ;

#endif
