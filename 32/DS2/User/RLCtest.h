//-----------------------------------------------------------------
// ����Ƶ�����Բ����ǹ��ܳ���ͷ�ļ�
// ͷ�ļ���: RLCtest.h
// ��    ��: ���ǵ���
// ��дʱ��: 2019-11-15
// �޸�����: 
// ��ǰ�汾: V1.0
// ��ʷ�汾:
//-----------------------------------------------------------------

#ifndef _RLCtest_H_
#define _RLCtest_H_

#include <stm32f10x.h>

//					 CD4051 CD1��CD2: PB8��PC4����>A,PB9��PC5����>B
//					 CD4051 CD3: PB10��PC6����>EN(B)
#define Gate_Set (GPIO_SetBits(GPIOC,GPIO_Pin_6))     // ���ؿ��ص�ʹ������B    
#define Gate_Clr (GPIO_ResetBits(GPIOC,GPIO_Pin_6))

#define Gate_Set_A (GPIO_SetBits(GPIOC,GPIO_Pin_4))    // ���������Rs��ģ�⿪�ص�ʹ�ܽ�A     
#define Gate_Clr_A (GPIO_ResetBits(GPIOC,GPIO_Pin_4))

#define Gate_Set_B (GPIO_SetBits(GPIOC,GPIO_Pin_5))    // ���������Rs��ģ�⿪�ص�ʹ�ܽ�B        
#define Gate_Clr_B (GPIO_ResetBits(GPIOC,GPIO_Pin_5))

#define US 11.2																				 // �ź�Դ��ѹ
#define g1 1.0         														 	 // �������У׼

#define fre_count  473
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
