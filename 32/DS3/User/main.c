#include "main.h"                  // Device header

#include "main.h"             // ���Ĺ�����ͷ�ļ�
#include "stm32f10x.h"        // STM32F10x ����ͷ�ļ�
#include "Delay.h"            // ��ʱ���� (ȷ��Delay_init()�����Ҫ����sys.c��Delay.c��)
#include "sys.h"              // ϵͳ���ú��� (���ܰ���ʱ�ӳ�ʼ���� SysTick ��ʼ��)
#include "AD9833.h"           // AD9833����
#include "tjc_usart_hmi.h"    // HMI����ͨ�ż�ҳ�����
#include "Timing Sampling.h"  // ADC��������ع���
#include "RLCtest.h"          // RLC�����߼�
#include "check_err.h"        // ���ϼ�⹦��
#include <stdio.h>            // ���� printf ���� (ͨ�� uart1_init)
#include <math.h>             // ���� log10 ����ѧ����

// --- ���ó��� (�������һ�� config.h �ļ���) ---
#define UART2_BAUD_RATE 9600  // HMI���ڲ����ʣ�����ʵ���������
#define AD9833_DEFAULT_FREQ 1000.0f // AD9833 Ĭ�����Ƶ�� (Hz)



int main(void)
{
	
    Delay_ms(100);         

    
    uart1_init(115200);
    //printf("System Booting... OK\r\n"); // \r\nȷ������

    // 3. ��ʼ��HMIͨ�Ŵ��� (UART2) �ͻ��λ�����
    uart2_init(UART2_BAUD_RATE);
    initRingBuff(); // tjc_usart_hmi.h �к� code_c() ӳ�䵽��
   // printf("HMI UART Initialized (USART2, Baud: %lu)\r\n", (unsigned long)UART2_BAUD_RATE);

    // 4. ��ʼ��AD9833ģ��
    AD9833_Init(PE9833); // ʹ�� AD9833.h �ж���� PE9833 (��Ӧ choise = 0)
    AD9833_Init(PG9833); // ʹ�� AD9833.h �ж���� PG9833 (��Ӧ choise = 1)
    // ����AD9833�ĳ�ʼƵ�ʺͲ���
    AD9833_SetFrequencyQuick(AD9833_DEFAULT_FREQ, AD9833_OUT_SINUS, PE9833);
    AD9833_SetFrequencyQuick(AD9833_DEFAULT_FREQ, AD9833_OUT_SINUS, PG9833);
    //printf("AD9833 Modules Initialized. Default Freq: %.1f Hz\r\n", AD9833_DEFAULT_FREQ);

    // 5. ��ʼ��ADC������� (ȷ����Щ������ͷ�ļ�������)
    ADC_GPIO_Init();                  // ��ʼ��ADC���� (PA1 for ADC1, PC2 for ADC2, PC3 for ADC3)
    ADC_RegularChannel_Configuration(); // ����ADC2��ADC3����ͨ����У׼
    //myADC1_Init();                    // ����ADC1 (��DMA��TIM3����)
		ADC_DMA1_Init();
		ADC_Init_Init();
    //printf("ADC Modules Initialized.\r\n");

    // 6. ��ʼ��ģ�⿪�ؿ���GPIO (RLCtest.h ������)
    GPIO_Gate();    // ��ʼ��PC
    standard();     // ����ģ�⿪�ص���׼/Ĭ��״̬ (Gate_Set_A; Gate_Clr_B;)
                    // ���� RLCtest.h: Gate_Set_A (PC4=1), Gate_Clr_B (PC5=0)
                    // Gate_Set/Gate_Clr (PC6) ���Ƹ��ؿ��أ���ʼ�Ͽ����أ�
    // Gate_Set;    // (GPIO_SetBits(GPIOC,GPIO_Pin_6)) ����ߵ�ƽ�Ͽ����ػ�ʹ��ģ�⿪�ص�ĳ��״̬
                    // ����RLCtest.h�ж��� Gate_Set (GPIO_SetBits(GPIOC,GPIO_Pin_6))
                    // ����ע����˵ CD4051 CD3: PB10����>EN(B)��CD405x��ENͨ���ǵ͵�ƽ��Ч��
                    // ���� Gate_Set (�ø�PC6) �����ǽ�ֹģ�⿪�أ�Gate_Clr (�õ�PC6) ��ʹ�ܡ�
                    // ���������CD4051/2�����ֲ�͵�·����ȷ��EN����Ч��ƽ��
                    // ���� EN �͵�ƽ��Ч��ʹ��ģ�⿪���л�ͨ��:
    GPIO_ResetBits(GPIOB, GPIO_Pin_10); // ʹ��ģ�⿪�� (����PB10��EN�ҵ���Ч)
   // printf("Analog Switch Control GPIOs Initialized. Default state set.\r\n");

    // 7. ��ѡ������ָ�HMI��ʹ����ת����ʼҳ�� (���� page 0)
    TJCPrintf("page main");
		//TJCPrintf( "t%s.txt=\"%.3f\"", "3", 3.0);
//    printf("Initial HMI page command sent (page 0).\r\n");

//    printf("Initialization Complete. Entering Main Loop.\r\n");
//    printf("----------------------------------------\r\n");

	//TJCPrintf( "t%s.txt=\"C1 is double\"", "3");
    // --- ��ѭ�� ---
    while (1)
    {
        // A. ��������HMI�Ĵ������ݣ����ݴ˸��� CurrentPage
        TJC_PageControl();  // �˺����ڲ������ringBuff������ȫ�ֵ�CurrentPage
				
				//printf("%d",CurrentPage);
        // B. ���� CurrentPage ��ֵ��ִ��RLC�����ǵ���Ӧ����ģ��
        // RLC_test() �����ڲ�Ӧ���� CurrentPage ��ֵ��ѡ��ִ���ĸ�ģ��
        // ���Ƴ����ڲ��� CurrentPage ��ǿ�Ƹ�ֵ��
        RLC_test();

        // C. �������������һЩ��ʱ������ȼ����񣬵�Ҫȷ�������������HMI��Ӧ
        // Delay_ms(1); // ���磬һ���ǳ��̵���ʱ
    }
}

// --- ȷ��printf�ض���UART1 ---
// (�ⲿ��ͨ���� tjc_usart_hmi.c �� serial.c �У������ṩ)
// int fputc(int ch, FILE *f)
// {
//     USART_SendData(USART1, (uint8_t)ch);
//     while( RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE) ){}
//     return ch;
// }

// --- SysTick�жϴ����� (���Delay_ms�ǻ���SysTick�ж϶��ǲ�ѯ) ---
// (ͨ���� stm32f10x_it.c �� sys.c ��)
// void SysTick_Handler(void)
// {
//   // �������Delay�ǻ���SysTick�жϵݼ��������Ļ���������Ҫ��Ӧ����
//   // ���磬�����ʹ�������� TimingDelay_Decrement() �Ļ���
// }
