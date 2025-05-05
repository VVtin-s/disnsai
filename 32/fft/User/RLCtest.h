//-----------------------------------------------------------------
// 简易频率特性测试仪功能程序头文件
// 头文件名: RLCtest.h
// 作    者: 凌智电子
// 编写时间: 2019-11-15
// 修改日期: 
// 当前版本: V1.0
// 历史版本:
//-----------------------------------------------------------------

#ifndef _RLCtest_H_
#define _RLCtest_H_

#include <stm32f10x.h>

//					 CD4051 CD1和CD2: PB8——>A,PB9——>B
//					 CD4051 CD3: PB10——>EN(B)
#define Gate_Set (GPIO_SetBits(GPIOB,GPIO_Pin_10))     // 负载开关的使能引脚B    
#define Gate_Clr (GPIO_ResetBits(GPIOB,GPIO_Pin_10))

#define Gate_Set_A (GPIO_SetBits(GPIOB,GPIO_Pin_8))    // 控制增益和Rs的模拟开关的使能脚A     
#define Gate_Clr_A (GPIO_ResetBits(GPIOB,GPIO_Pin_8))

#define Gate_Set_B (GPIO_SetBits(GPIOB,GPIO_Pin_9))    // 控制增益和Rs的模拟开关的使能脚B        
#define Gate_Clr_B (GPIO_ResetBits(GPIOB,GPIO_Pin_9))

#define US 12																					 // 信号源电压
#define g1 1.15          														 	 // 输出电阻校准
extern u32 DC_ref;
extern void RLC_test(void);
extern void Sweep_out(u32 fre_l, u32 fre_h);
extern void PoinTFreTest(void);
extern void GPIO_Gate(void);
extern void Sweep_out2(void);
extern void standard (void);
extern void AFC (void) ;
extern void G_original (void);



#endif
