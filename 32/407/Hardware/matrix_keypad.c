#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include "lcd.h"
#include "serial.h"
#include "Timing Sampling.h"

#define ROW0_PIN    GPIO_Pin_1
#define ROW1_PIN    GPIO_Pin_2
#define ROW2_PIN    GPIO_Pin_3
#define ROW3_PIN    GPIO_Pin_4
#define ROW_PORT    GPIOF


#define COL0_PIN    GPIO_Pin_5
#define COL1_PIN    GPIO_Pin_6
#define COL2_PIN    GPIO_Pin_7
#define COL3_PIN    GPIO_Pin_0
#define COL_PORT    GPIOF

volatile uint8_t key_pressed = 0;
volatile char key_value = 0;

void KeyConrolIO_GPIO_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD , ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0 |  GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_6;	//PD0  red sig_ctr,	PD1 orange out_ctr, PD3 yellow C1_ctr
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD , &GPIO_InitStructure);
	GPIO_ResetBits(GPIOD ,GPIO_Pin_0 |  GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_6);
	GPIO_ResetBits(GPIOD ,GPIO_Pin_0 |  GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_6);
}

void GPIO_Turn(uint8_t Key)
{
	switch (Key)
		{
				case 1:
					{		
						if( GPIO_ReadOutputDataBit(GPIOD , GPIO_Pin_0) == 0)
						{
							GPIO_SetBits(GPIOD , GPIO_Pin_0);
						}
						else
						{
							GPIO_ResetBits(GPIOD , GPIO_Pin_0);
						}
					}
					break;
				case 2:
					{		
						if( GPIO_ReadOutputDataBit(GPIOD , GPIO_Pin_1) == 0)
						{
							GPIO_SetBits(GPIOD , GPIO_Pin_1);
						}
						else
						{
							GPIO_ResetBits(GPIOD , GPIO_Pin_1);
						}
					}
					break;
				case 3:
					{		
						if( GPIO_ReadOutputDataBit(GPIOD , GPIO_Pin_5) == 0)
						{
							GPIO_SetBits(GPIOD , GPIO_Pin_5);
						}
						else
						{
							GPIO_ResetBits(GPIOD , GPIO_Pin_5);
						}
					}
					break;
				case 4:
					{		
						if( GPIO_ReadOutputDataBit(GPIOD , GPIO_Pin_6) == 0)
						{
							GPIO_SetBits(GPIOD , GPIO_Pin_6);
						}
						else
						{
							GPIO_ResetBits(GPIOD , GPIO_Pin_6);
						}
					}
					break;
				default:
					break;
		}
}

void Keypad_GPIO_EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG | RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO, ENABLE);

   
    GPIO_InitStructure.GPIO_Pin = ROW0_PIN | ROW1_PIN | ROW2_PIN | ROW3_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(ROW_PORT, &GPIO_InitStructure);

   
    GPIO_InitStructure.GPIO_Pin = COL0_PIN | COL1_PIN | COL2_PIN | COL3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(COL_PORT, &GPIO_InitStructure);

    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOF, GPIO_PinSource5);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOF, GPIO_PinSource6);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOF, GPIO_PinSource7);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOF, GPIO_PinSource0);

		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
		EXTI_InitStructure.EXTI_LineCmd = DISABLE;

		EXTI_InitStructure.EXTI_Line = EXTI_Line0;
		EXTI_Init(&EXTI_InitStructure);

		EXTI_InitStructure.EXTI_Line = EXTI_Line5;
		EXTI_Init(&EXTI_InitStructure);

		EXTI_InitStructure.EXTI_Line = EXTI_Line6;
		EXTI_Init(&EXTI_InitStructure);

		EXTI_InitStructure.EXTI_Line = EXTI_Line7;
		EXTI_Init(&EXTI_InitStructure);


    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

//    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
//		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn; 
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    GPIO_ResetBits(ROW_PORT, ROW0_PIN | ROW1_PIN | ROW2_PIN | ROW3_PIN);
		
		KeyConrolIO_GPIO_init();
}


const char key_map[4][4] = {
    {'1', '2', '3', '4'},
    {'5', '6', '7', '8'},
    {'9', '0', 'A', 'B'},
    {'C', 'D', 'E', 'F'}
};



char Keypad_Scan(void)
{
    uint8_t row, col;
    uint8_t row_pins[4] = {ROW0_PIN, ROW1_PIN, ROW2_PIN, ROW3_PIN};
    uint8_t col_pins[4] = {COL0_PIN, COL1_PIN, COL2_PIN, COL3_PIN};

    for (row = 0; row < 4; row++)
    {
			
        GPIO_SetBits(ROW_PORT, ROW0_PIN | ROW1_PIN | ROW2_PIN | ROW3_PIN);
       
        GPIO_ResetBits(ROW_PORT, row_pins[row]);

       

        for (col = 0; col < 4; col++)
        {
            if (GPIO_ReadInputDataBit(COL_PORT, col_pins[col]) == 0)
            {
               
                Delay_ms(10);

                if (GPIO_ReadInputDataBit(COL_PORT, col_pins[col]) == 0)
                {
										while (GPIO_ReadInputDataBit(COL_PORT, col_pins[col]) == 0);
                    GPIO_ResetBits(ROW_PORT, ROW0_PIN | ROW1_PIN | ROW2_PIN | ROW3_PIN);
                    return key_map[row][col];
										key_pressed = 1; // Key_Pressed Flag NoIT_Version
                }
            }
        }
    }

    GPIO_ResetBits(ROW_PORT, ROW0_PIN | ROW1_PIN | ROW2_PIN | ROW3_PIN);
    return 0;
}

void Key_Control(void) //Non_IT Version
	{
		key_value = Keypad_Scan(); 
		if (key_pressed) {
        key_pressed = 0;
        GPIO_Turn(key_value - '0');
    }
		
	}

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    { 
        EXTI_ClearITPendingBit(EXTI_Line0);
       // key_value = Keypad_Scan();  
				Key_Control();
        key_pressed = 1;
				ResetADC();
    }
}


//void EXTI4_IRQHandler(void)
//{
//    EXTI_ClearITPendingBit(EXTI_Line6);
				//Serial_SendByte('o'); 
				//char Key = Keypad_Scan();
				//Serial_SendByte(Key);
				//GPIO_Turn(Key - '0');
//}


void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET)
    { 
        EXTI_ClearITPendingBit(EXTI_Line5);
        //key_value = Keypad_Scan();  
				Key_Control();
        key_pressed = 1;
				ResetADC();
    }

    if (EXTI_GetITStatus(EXTI_Line6) != RESET)
    { 
        EXTI_ClearITPendingBit(EXTI_Line6);
        //key_value = Keypad_Scan();  
				Key_Control();
        key_pressed = 1;
				ResetADC();
    }
		
    if (EXTI_GetITStatus(EXTI_Line7) != RESET)
    { 
        EXTI_ClearITPendingBit(EXTI_Line7);
        //key_value = Keypad_Scan(); 
				Key_Control();
        key_pressed = 1;
				ResetADC();
    }
}


