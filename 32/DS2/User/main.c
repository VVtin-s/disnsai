#include "main.h"                  // Device header

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
#include <math.h>             // 用于 log10 等数学函数

// --- 配置常量 (建议放在一个 config.h 文件中) ---
#define UART2_BAUD_RATE 9600  // HMI串口波特率，根据实际情况调整
#define AD9833_DEFAULT_FREQ 1000.0f // AD9833 默认输出频率 (Hz)



int main(void)
{
	
    Delay_ms(100);         

    
    uart1_init(115200);
    //printf("System Booting... OK\r\n"); // \r\n确保换行

    // 3. 初始化HMI通信串口 (UART2) 和环形缓冲区
    uart2_init(UART2_BAUD_RATE);
    initRingBuff(); // tjc_usart_hmi.h 中宏 code_c() 映射到此
   // printf("HMI UART Initialized (USART2, Baud: %lu)\r\n", (unsigned long)UART2_BAUD_RATE);

    // 4. 初始化AD9833模块
    AD9833_Init(PE9833); // 使用 AD9833.h 中定义的 PE9833 (对应 choise = 0)
    AD9833_Init(PG9833); // 使用 AD9833.h 中定义的 PG9833 (对应 choise = 1)
    // 设置AD9833的初始频率和波形
    AD9833_SetFrequencyQuick(AD9833_DEFAULT_FREQ, AD9833_OUT_SINUS, PE9833);
    AD9833_SetFrequencyQuick(AD9833_DEFAULT_FREQ, AD9833_OUT_SINUS, PG9833);
    //printf("AD9833 Modules Initialized. Default Freq: %.1f Hz\r\n", AD9833_DEFAULT_FREQ);

    // 5. 初始化ADC采样相关 (确保这些函数在头文件中声明)
    ADC_GPIO_Init();                  // 初始化ADC引脚 (PA1 for ADC1, PC2 for ADC2, PC3 for ADC3)
    ADC_RegularChannel_Configuration(); // 配置ADC2和ADC3规则通道并校准
    //myADC1_Init();                    // 配置ADC1 (带DMA和TIM3触发)
		ADC_DMA1_Init();
		ADC_Init_Init();
    //printf("ADC Modules Initialized.\r\n");

    // 6. 初始化模拟开关控制GPIO (RLCtest.h 中声明)
    GPIO_Gate();    // 初始化PC
    standard();     // 设置模拟开关到标准/默认状态 (Gate_Set_A; Gate_Clr_B;)
                    // 根据 RLCtest.h: Gate_Set_A (PC4=1), Gate_Clr_B (PC5=0)
                    // Gate_Set/Gate_Clr (PC6) 控制负载开关，初始断开负载：
    // Gate_Set;    // (GPIO_SetBits(GPIOC,GPIO_Pin_6)) 假设高电平断开负载或使能模拟开关的某个状态
                    // 您的RLCtest.h中定义 Gate_Set (GPIO_SetBits(GPIOC,GPIO_Pin_6))
                    // 并在注释中说 CD4051 CD3: PB10——>EN(B)。CD405x的EN通常是低电平有效。
                    // 所以 Gate_Set (置高PC6) 可能是禁止模拟开关，Gate_Clr (置低PC6) 是使能。
                    // 请根据您的CD4051/2数据手册和电路连接确认EN的有效电平。
                    // 假设 EN 低电平有效来使能模拟开关切换通道:
    GPIO_ResetBits(GPIOB, GPIO_Pin_10); // 使能模拟开关 (假设PB10是EN且低有效)
   // printf("Analog Switch Control GPIOs Initialized. Default state set.\r\n");

    // 7. 可选：发送指令到HMI，使其跳转到初始页面 (例如 page 0)
    TJCPrintf("page main");
		//TJCPrintf( "t%s.txt=\"%.3f\"", "3", 3.0);
//    printf("Initial HMI page command sent (page 0).\r\n");

//    printf("Initialization Complete. Entering Main Loop.\r\n");
//    printf("----------------------------------------\r\n");

	//TJCPrintf( "t%s.txt=\"C1 is double\"", "3");
    // --- 主循环 ---
    while (1)
    {
        // A. 处理来自HMI的串口数据，并据此更新 CurrentPage
        TJC_PageControl();  // 此函数内部会解析ringBuff并更新全局的CurrentPage
				
				//printf("%d",CurrentPage);
        // B. 根据 CurrentPage 的值，执行RLC测试仪的相应功能模块
        // RLC_test() 函数内部应根据 CurrentPage 的值来选择执行哪个模块
        // 并移除其内部对 CurrentPage 的强制赋值。
        RLC_test();

        // C. 可以在这里添加一些延时或低优先级任务，但要确保不会过度阻塞HMI响应
        // Delay_ms(1); // 例如，一个非常短的延时
    }
}

// --- 确保printf重定向到UART1 ---
// (这部分通常在 tjc_usart_hmi.c 或 serial.c 中，您已提供)
// int fputc(int ch, FILE *f)
// {
//     USART_SendData(USART1, (uint8_t)ch);
//     while( RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE) ){}
//     return ch;
// }

// --- SysTick中断处理函数 (如果Delay_ms是基于SysTick中断而非查询) ---
// (通常在 stm32f10x_it.c 或 sys.c 中)
// void SysTick_Handler(void)
// {
//   // 如果您的Delay是基于SysTick中断递减计数器的话，这里需要相应处理
//   // 例如，如果您使用了类似 TimingDelay_Decrement() 的机制
// }
