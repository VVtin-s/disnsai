#include "main.h"                  // Device header

int main(void)
{
	Serial_Init();
	//lcd_init();
	Keypad_GPIO_EXTI_Init();
//	uint16_t i=0;
	myADC1_Init();
	//lcd_clear(YELLOW);
	//lcd_show_string(10, 40, 240, 32, 32, "start", RED);

	//fft test
	//FFT_test();
//	for (uint8_t j=0; j<3 ; j++)
//	{
//		Serial_Printf("%d\r\n", FFT_Mag[j]);
//	}
	
	//9959 Test
//	GPIO_AD9959_Configuration();
//	AD9959_Init();
//	AD9959_enablechannel0();
//	AD9959_Setwavefrequency (1000);
//	//AD9959_Setwavephase(1000, 0);
//	uint16_t 	a=0;
//	Serial_SendString("Start");

	//MY_NVIC_PriorityGroup_Config(NVIC_PriorityGroup_2);	//设置中断分组
	
	//Delay_ms(500);//延时一会儿，等待上电稳定,确保AD9833比控制板先上电。
	
	
	
	//AD9833_Init(1);//IO口及AD9833寄存器初始化
	
	//频率入口参数为float，可使信号的频率更精确
	//AD9833_SetFrequencyQuick(1000.0,AD9833_OUT_TRIANGLE, 1);//写输出频率1000.0Hz,输出正弦波
	
    while (1)
    { 

		//ADC Test
			if (adc_finish_fg == 1)  
        {
            adc_finish_fg = 0;

            for (int i = 0; i < sample_num; i++)
            {
                Serial_Printf("add 8,0,%d", (Adc_data[i])*195/4095);  
                
            } 
			
			
			Delay_ms(1000);
			//Serial_Printf("One done");
					}

		}
}
