#include "main.h"                  // Device header

int main(void)
{
	Delay_ms(100);
	AD9833_Init(0);
	AD9833_Init(1);
	uart1_init(115200);
	uart2_init(9600);
	ADC_GPIO_Init();
	ADC_RegularChannel_Configuration();
	GPIO_Gate();
	ADC_DMA1_Init();
	ADC_Init_Init();
	Gate_Set;															// ��ʼ״̬Ϊ����
	Gate_Set_A;    												// Ĭ��״̬�����봮��2K�����裬ģ�⿪��1��B��AΪ0��1  
	Gate_Clr_B;
	AD9833_SetFrequencyQuick(1000,AD9833_OUT_SINUS, 0);
	AD9833_SetFrequencyQuick(1000,AD9833_OUT_SINUS, 1);
	
    while (1)
    { 

			RLC_test();
					

		}
}
