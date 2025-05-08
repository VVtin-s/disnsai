#ifndef _AD9959_H
#define _AD9959_H

#include "sys.h"

#define uchar unsigned char
#define uint unsigned int 


//AD998 I/O控制总线

//端口C宏定义
#define SCLK_1 (GPIO_SetBits(GPIOC,GPIO_Pin_0)) 
#define SCLK_0 (GPIO_ResetBits(GPIOC,GPIO_Pin_0)) 

#define CS_1 (GPIO_SetBits(GPIOC,GPIO_Pin_1))
#define CS_0 (GPIO_ResetBits(GPIOC,GPIO_Pin_1))

#define IO_update_1 (GPIO_SetBits(GPIOC,GPIO_Pin_2)) 
#define IO_update_0 (GPIO_ResetBits(GPIOC,GPIO_Pin_2)) 

#define SDIO0_1 (GPIO_SetBits(GPIOC,GPIO_Pin_3))
#define SDIO0_0 (GPIO_ResetBits(GPIOC,GPIO_Pin_3))

#define PS0_1 (GPIO_SetBits(GPIOC,GPIO_Pin_4)) 
#define PS0_0 (GPIO_ResetBits(GPIOC,GPIO_Pin_4)) 

#define PS1_1 (GPIO_SetBits(GPIOC,GPIO_Pin_5))
#define PS1_0 (GPIO_ResetBits(GPIOC,GPIO_Pin_5))

#define PS2_1 (GPIO_SetBits(GPIOC,GPIO_Pin_6))	
#define PS2_0 (GPIO_ResetBits(GPIOC,GPIO_Pin_6))	

#define PS3_1 (GPIO_SetBits(GPIOC,GPIO_Pin_7))
#define PS3_0 (GPIO_ResetBits(GPIOC,GPIO_Pin_7))

//端口A宏定义
#define SDIO1_1 (GPIO_SetBits(GPIOD,GPIO_Pin_3))
#define SDIO1_0 (GPIO_ResetBits(GPIOD,GPIO_Pin_3))

#define SDIO2_1  (GPIO_SetBits(GPIOD,GPIO_Pin_2))
#define SDIO2_0 (GPIO_ResetBits(GPIOD,GPIO_Pin_2))

#define SDIO3_1  (GPIO_SetBits(GPIOD,GPIO_Pin_1))		
#define SDIO3_0 (GPIO_ResetBits(GPIOD,GPIO_Pin_1))	

#define PWR_1  (GPIO_SetBits(GPIOA,GPIO_Pin_11))
#define PWR_0 (GPIO_ResetBits(GPIOA,GPIO_Pin_11))

#define Reset_1  (GPIO_SetBits(GPIOA,GPIO_Pin_12))
#define Reset_0 (GPIO_ResetBits(GPIOA,GPIO_Pin_12))


//-----------------------------------------------------------------
// 外部函数声明
//-----------------------------------------------------------------
extern void GPIO_AD9959_Configuration(void)  ;//I/O端口配置
extern void AD9959_Init(void);  //复位
extern void WrFrequencyTuningWorddata(double f,uchar *ChannelFrequencyTuningWorddata);
extern void IO_update(void)  ;
extern void WriteToAD9959ViaSpi(uchar RegisterAddress, uchar NumberofRegisters, uchar *RegisterData,uchar temp) ;
extern void WrPhaseOffsetTuningWorddata(double f,uchar *ChannelPhaseOffsetTuningWorddata);
extern void WrAmplitudeTuningWorddata(double f,uchar *ChannelAmplitudeTuningWorddata);//计算过程注意与计算频率区别
extern void WrAmplitudeTuningWorddata1(double f,uchar *ChannelAmplitudeTuningWorddata,uchar *ASRAmplituteWordata);

#endif
