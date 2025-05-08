#ifndef  __MAIN_H_
#define __MAIN_H_

#include "main.h"             // 您的工程主头文件
#include "stm32f10x.h"        // STM32F10x 器件头文件
#include "Delay.h"            // 延时函数 (确保Delay_init()如果需要，在sys.c或Delay.c中)
#include "sys.h"              // 系统配置函数 (可能包含时钟初始化和 SysTick 初始化)
#include "AD9833.h"           // AD9833驱动
#include "tjc_usart_hmi.h"    // HMI串口通信及页面控制
#include "Timing Sampling.h"  // ADC采样及相关功能
#include "RLCtest.h"          // RLC测试逻辑
#include "check_err.h"        // 故障检测功能
#include <stdio.h>            // 用于 printf 调试 (通过 uart1_init)
#include <math.h>  


#endif
