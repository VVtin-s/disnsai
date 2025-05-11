#ifndef _AD9833_H_
#define _AD9833_H_

#include "sys.h"

#define PE9833 					(0)
#define PG9833					(1)

#define AD9833_FSYNCE 	PEout(10)
#define AD9833_SCLKE 		PEout(11)
#define AD9833_SDATAE 	PEout(12)
#define AD9833_FSYNCG 	PGout(2)
#define AD9833_SCLKG 		PGout(3)
#define AD9833_SDATAG 	PGout(4)
/******************************************************************************/
/* AD9833                                                                    */
/******************************************************************************/
/* �Ĵ��� */

#define AD9833_REG_CMD		(0 << 14)
#define AD9833_REG_FREQ0	(1 << 14)
#define AD9833_REG_FREQ1	(2 << 14)
#define AD9833_REG_PHASE0	(6 << 13)
#define AD9833_REG_PHASE1	(7 << 13)

/* �������λ */

#define AD9833_B28				(1 << 13)
#define AD9833_HLB				(1 << 12)
#define AD9833_FSEL0			(0 << 11)
#define AD9833_FSEL1			(1 << 11)
#define AD9833_PSEL0			(0 << 10)
#define AD9833_PSEL1			(1 << 10)
#define AD9833_PIN_SW			(1 << 9)
#define AD9833_RESET			(1 << 8)
#define AD9833_SLEEP1			(1 << 7)
#define AD9833_SLEEP12		(1 << 6)
#define AD9833_OPBITEN		(1 << 5)
#define AD9833_SIGN_PIB		(1 << 4)
#define AD9833_DIV2				(1 << 3)
#define AD9833_MODE				(1 << 1)

#define AD9833_OUT_SINUS		((0 << 5) | (0 << 1) | (0 << 3))//���Ҳ� 
#define AD9833_OUT_TRIANGLE	((0 << 5) | (1 << 1) | (0 << 3))//���ǲ�
#define AD9833_OUT_MSB			((1 << 5) | (0 << 1) | (1 << 3)) //����
#define AD9833_OUT_MSB2			((1 << 5) | (0 << 1) | (0 << 3))

void AD983_GPIO_Init(uint8_t chose);//��ʼ��IO��
void AD9833_Init(uint8_t choise);//��ʼ��IO�ڼ��Ĵ���

void AD9833_Reset(uint8_t choise);			//��λAD9833�ĸ�λλ
void AD9833_ClearReset(uint8_t choise);	//���AD9833�ĸ�λλ

void AD9833_SetRegisterValue(unsigned short regValue, uint8_t choise);												//��ֵд��Ĵ���
void AD9833_SetFrequency(unsigned short reg, float fout,unsigned short type, uint8_t choise);	//д��Ƶ�ʼĴ���
void AD9833_SetPhase(unsigned short reg, unsigned short val, uint8_t choise);									//д����λ�Ĵ���

void AD9833_Setup(unsigned short freq,unsigned short phase,unsigned short type, uint8_t choise);//ѡ��Ƶ�ʡ���λ�Ͳ�������
void AD9833_SetFrequencyQuick(float fout,unsigned short type, uint8_t choise);//����Ƶ�ʼ���������
unsigned long freq_trl_word (unsigned long freq);
#endif 
