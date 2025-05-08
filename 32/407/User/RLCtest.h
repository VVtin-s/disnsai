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

//					 CD4051 CD1��CD2: PB8����>A,PB9����>B
//					 CD4051 CD3: PB10����>EN(B)
#define Gate_Set (GPIO_SetBits(GPIOB,GPIO_Pin_10))     // ���ؿ��ص�ʹ������B    
#define Gate_Clr (GPIO_ResetBits(GPIOB,GPIO_Pin_10))

#define Gate_Set_A (GPIO_SetBits(GPIOB,GPIO_Pin_8))    // ���������Rs��ģ�⿪�ص�ʹ�ܽ�A     
#define Gate_Clr_A (GPIO_ResetBits(GPIOB,GPIO_Pin_8))

#define Gate_Set_B (GPIO_SetBits(GPIOB,GPIO_Pin_9))    // ���������Rs��ģ�⿪�ص�ʹ�ܽ�B        
#define Gate_Clr_B (GPIO_ResetBits(GPIOB,GPIO_Pin_9))

#define US 12																					 // �ź�Դ��ѹ
#define g1 1.15          														 	 // �������У׼
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
