#include "stm32f10x.h"                  // Device header
#include "Timer.h"
#include "stdio.h"
#include "Timing Sampling.h"
#include "serial.h"
#include "math.h"
#include "arm_math.h"
#include "stm32_dsp.h"

//volatile  uint16_t ADC_Value;
#define ADC1_DR_Address ((u32)0x4001244C)
uint32_t FFT_SourceData[sample_num];
uint32_t FFT_OutData[sample_num/2];	  //fft�������
uint32_t FFT_Mag[sample_num/2] ;	
uint16_t Adc_data[sample_num]={0};
uint16_t Adc_data1[CodeSize*Channel]={0};

unsigned short  secondMaxIndex = 0;
unsigned short  thirdMaxIndex=0;

static u16 ADC2_ConvertedValueTab[CodeSize];
static u16 ADC3_ConvertedValueTab[100];

float vpp_vpp;

uint8_t adc_finish_fg = 0;//adc��ȡ���ݱ�־λ
void ADC_GPIO_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE); 
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;								//ADC1 PA1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;  //ADC2 PC2  ADC3 PC3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void ADC_TIM3_Init(u16 arr, u16 psc)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_InternalClockConfig(TIM3);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = arr-1;											//(PSC+1)*(ARR+1)/TCLK
	TIM_TimeBaseStructure.TIM_Prescaler = psc-1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
//	
//	TIM_OCInitTypeDef TIM_OCInitStructure;
//	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//	TIM_OCInitStructure.TIM_Pulse = 1000;
//	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
//	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
//	
	TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update);
	
	TIM_Cmd(TIM3, ENABLE);
//	TIM_CtrlPWMOutputs(TIM2, ENABLE);
}

void ADC_DMA1_Init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);
	
	DMA_InitTypeDef DMA_InitStructure;
	
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&Adc_data1; 									//DIY memory Location
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(ADC1->DR)); 									  //Peripheral memory Location
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  		//D-Data Size
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;   								 		//D-Memory Location Incremented
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //P_Data Size
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 								 //P-Memory Location Incremented
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;          												 // Priority
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;																			//M-To-M mode program-tri
	DMA_InitStructure.DMA_BufferSize = sample_num;  																										//Number of  Data (channel num)
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; 																//Transportation Direction
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   	//Tsp Mode 	
	
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	DMA_Cmd(DMA1_Channel1, ENABLE);
	DMA_ITConfig(DMA1_Channel1, DMA1_IT_TC1, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	DMA_ClearITPendingBit(DMA1_IT_TC1);
	
}


void ADC_Init_Init(void)//ADCpeizhihanhsu
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	//ADC_DeInit(ADC1);
	ADC_InitTypeDef  ADC_InitStructure;
	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;    //continuous or single scan
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;   //Trigger Method
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;  		//multi-channel scan
	ADC_InitStructure.ADC_NbrOfChannel = 1;            //Channel_Number Value 1 when scan mode disabled
	ADC_Init(ADC1, &ADC_InitStructure);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_1Cycles5);  //Trans_Time (1.5+12.5)/12M Max:857KHz
	
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
	
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1) == SET);
	
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);
}
	

void myADC1_Init(void)
{
	ADC_TIM3_Init(360, 2);
	ADC_DMA1_Init();
	ADC_GPIO_Init();
	ADC_Init_Init();
	
}

void DMA1_Channel1_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC1 ) != RESET)
	{
		DMA_ClearITPendingBit(DMA1_IT_TC1);//���DMA��־λ
		DMA_Cmd(DMA1_Channel1, DISABLE);					 	// �ر�DMA1ͨ��1��ADC1
		ADC_Cmd(ADC1, DISABLE);	 
		adc_finish_fg =1;//дadc��־λ����ʾadc�������
		
	}
	
}

void ADC_Initialization(ADC_TypeDef* ADCx)
{
	ADC_InitTypeDef ADC_InitStructure;
	// ����ÿ��ADC�����ڹ������ģʽ 
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	// �涨��ģ��ת��������ɨ��ģʽ(��ͨ��)ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;			 	
	// �涨��ģ��ת������������ģʽ	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	
	// ������ʹ���ⲿ��������������ͨ����ģ��ת��
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	// �涨��ADC��������߶��뻹�����ұ߶���
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	
	// �涨��˳����й���ת����ADCͨ������Ŀ:1
	ADC_InitStructure.ADC_NbrOfChannel = Channel;
	// ����ADC1�ļĴ���	
	ADC_Init(ADCx, &ADC_InitStructure);
}

void ADC_Calibration(ADC_TypeDef * ADCx)
{
	// ��λADC��У׼
	ADC_ResetCalibration(ADCx);		
	// ���ADC��λУ׼������						
	while(ADC_GetResetCalibrationStatus(ADCx));
	// ����ADC��У׼����
	ADC_StartCalibration(ADCx);							
	// ���ADC��У׼������
	while(ADC_GetCalibrationStatus(ADCx));	
}

//ADC2/3 Configuration
void ADC_RegularChannel_Configuration(void)
{
	// ��ADC2��ADC3ʱ��
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_ADC2|RCC_APB2Periph_ADC3 , ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 				// ����ADCʱ��, ΪPCLK2��6��Ƶ,��12MHz

																																								
	// ѡ��ADC2ͨ��12�����ڵ�һλ�ɼ�������ʱ��Ϊ1.5����
	ADC_Initialization(ADC2);	
	ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 1,ADC_SampleTime_1Cycles5);		//PC2���ɼ��������
	
	// ѡ��ADC3ͨ��10�����ڵڶ�λ�ɼ�������ʱ��Ϊ1.5����
	ADC_Initialization(ADC3);	
	ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 1,ADC_SampleTime_1Cycles5);		//PC3���ɼ����ֱ��
	ADC_Cmd(ADC3, ENABLE);										// ʹ��ADC3
	ADC_Calibration(ADC3);										// ADC3�Զ�У׼
	ADC_Cmd(ADC2, ENABLE);										// ʹ��ADC2
	ADC_Calibration(ADC2);										// ADC2�Զ�У׼	
}

void Get_FFT_Source_Data(void)//��ADC���ݸ���FFT_SourceData��ǰ16λ
{
	uint16_t i;
	for(i=0; i<sample_num; i++)
	{
		FFT_SourceData[i] = ((signed short)Adc_data[i]) << 16;
	}
}



void GetPowerMag(void)
{
  signed short lX, lY;
  float X, Y, Mag;
  unsigned short i;
  unsigned long maxMag = 0;
	unsigned long secondMaxMag = 0;
	unsigned long thirdMaxMag_val = 0; // ʹ��һ���±�����������ֵ
	unsigned short maxIndex = 0;

  for(i = 1; i < sample_num / 2; i++)//����FFT��Ƶ�����ǹ����ο�˹��Ƶ�ʶԳƵģ�����ֻ����һ��
  {
		lX = (FFT_OutData[i] << 16) >> 16;		//ȡ��ʮ��λ���鲿
    lY = (FFT_OutData[i] >> 16);					//ȡ��ʮ��λ��ʵ��

    X = sample_num * ((float)lX) / 32768;
    Y = sample_num * ((float)lY) / 32768;

    Mag = sqrt(X * X + Y * Y) / sample_num;
		//��Щ���Ǽ������sqrt(lx*lx+ly*ly)*2/NPT

    FFT_Mag[i] = (unsigned long)(Mag * 65536);
		//֮�����ȳ���32768�����65536��Ϊ�˷��ϸ������ļ������
		
		//���´���Ϊ��ȡ��ǿ���ڶ�ǿ������ǿ���ź������ͷ���
   for (i = 1; i < sample_num / 2; i++) {
    // ... (���� Mag �� FFT_Mag[i]) ...

    if (FFT_Mag[i] > maxMag) {
        thirdMaxMag_val = secondMaxMag; // ԭ���ĵڶ����ɵ�����
        thirdMaxIndex = secondMaxIndex; // ԭ���ĵڶ���������ɵ���������

        secondMaxMag = maxMag;
        secondMaxIndex = maxIndex;

        maxMag = FFT_Mag[i];
        maxIndex = i;
    } else if (FFT_Mag[i] > secondMaxMag) {
        thirdMaxMag_val = secondMaxMag;
        thirdMaxIndex = secondMaxIndex;

        secondMaxMag = FFT_Mag[i];
        secondMaxIndex = i;
    } else if (FFT_Mag[i] > thirdMaxMag_val) {
        thirdMaxMag_val = FFT_Mag[i];
        thirdMaxIndex = i;
    }
	}          
	}
}
void FFT_test(void)
{
  Get_FFT_Source_Data();//����FFT_SourceData����
  cr4_fft_1024_stm32(FFT_OutData, FFT_SourceData, sample_num);
	//cr4_fft_1024_stm32(������У��������У�������)��ִ��fft�����ࣩ
  GetPowerMag();
}

void ResetADC(void)
{
//	TIM_Cmd(TIM3, DISABLE);
//	ADC_Cmd(ADC1, DISABLE);
//	DMA_Cmd(DMA1_Channel1, DISABLE);
//	DMA_ClearFlag(DMA1_FLAG_TC1 | DMA1_FLAG_HT1 | DMA1_FLAG_TE1);
//	DMA_ClearITPendingBit(DMA1_IT_TC1 | DMA1_IT_HT1 | DMA1_IT_TE1);
//	TIM_Cmd(TIM3, ENABLE);
//	DMA_Cmd(DMA1_Channel1, ENABLE);
//	ADC_Cmd(ADC1, ENABLE);
////	ADC_ResetCalibration(ADC1);
////	while(ADC_GetResetCalibrationStatus(ADC1) == SET);
////	ADC_StartCalibration(ADC1);
////	while(ADC_GetCalibrationStatus(ADC1) == SET);
//	ADC_ExternalTrigConvCmd(ADC1, ENABLE);
	myADC1_Init();
}

//-----------------------------------------------------------------------------
// ���ܳ�����
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//	u16 ADC_MATH(void)
//-----------------------------------------------------------------------------
//     
// ��������:	��ȡ����ADC�ɼ����ݵķ��ֵ
// ��ڲ���:	��
// ���ڲ���:	���ֵ
// ע������:	
//-----------------------------------------------------------------------------
// �����޸�
float ADC_MATH(void) // ����ֵ���͸�Ϊ float
{
    u16 min_val = 65535, max_val = 0; // ������������
    u16 i;

    for (i = 0; i < CodeSize * Channel; i++) // ȷ�� CodeSize*Channel �� Adc_data1 ����ȷ����
    {
        if (Adc_data1[i] > max_val) 
					
					max_val = Adc_data1[i];
				
        //if (Adc_data1[i] < min_val)
					if (Adc_data1[i]>30 && Adc_data1[i] < min_val)//�����һ�������ƣ�ȡ��ADC�ڿ�ʼʱ��ȡ��0��Ϊ��С
					min_val = Adc_data1[i];
				
    }
		//��������´���
		u16 umax_val =(float)max_val* VREF / 4095.0f; 
		u16 umin_val =(float)min_val* VREF / 4095.0f; 
		//printf("fengfengzhi��max;%u  min;%u \r\n",umax_val,umin_val);
    u16 MMM = max_val - min_val;
    // vpp_vpp ��ȫ�ֱ������������ֱ�Ӹ�ֵ
    vpp_vpp = (float)MMM * VREF / 4095.0f; // ʹ�� 4095.0f ���и��������VREF ҲӦΪ����������ת
    return vpp_vpp; // ���ؼ���õ��ĸ���ֵ
}
//-----------------------------------------------------------------------------
//	void ADC_Samp_Sweep (void)
//-----------------------------------------------------------------------------
//     
// ��������:	ɨƵ��ʾ��Ƶ����ʱ�Ľ�����ѹ�ɼ�, ��ȡADC2�����ɼ����ݵķ��ֵ
// ��ڲ���:	��
// ���ڲ���:	��
// ע������:	
//-----------------------------------------------------------------------------
void ADC_Samp_Sweep (void) 	//
{
	int count;
	u16	adcx[2],CodeSize_1=1024;
	u16 max2=0,min2=4095,temp=0;
	
	for (count = 0; count < CodeSize_1-1; count ++)											// ����N������
	{
		ADC2_ConvertedValueTab[count] = 0;
		ADC_SoftwareStartConvCmd(ADC2,ENABLE); 														// �������ADC2�ɼ�
		while(!ADC_GetFlagStatus(ADC2,ADC_FLAG_EOC));	
		ADC_ClearFlag(ADC2,ADC_FLAG_EOC);
		ADC2_ConvertedValueTab[count]=ADC_GetConversionValue(ADC2);
	}	
	for(count=0;count<CodeSize_1-1;count++)
	{
		temp=ADC2_ConvertedValueTab[count];
		if(temp>max2) max2=temp;
		if(temp<min2) min2=temp;	
	}	
	adcx[1]		=  max2 - min2	;
	Adc_data[1]	=	(float)adcx[1] * VREF / 4095;  // ������ѹ�������
}

//-----------------------------------------------------------------------------
//	void ADC_Sampling (void)
//-----------------------------------------------------------------------------
//     
// ��������:	ADC2�������ݲɼ�����ȡ���ֵ
// ��ڲ���:	��
// ���ڲ���:	��
// ע������:	ADC2��Ҫ׼ȷ�������������ѹ���ֵʹ��
//-----------------------------------------------------------------------------
void ADC_Sampling (void) 		
{
	int count;
	u16	adcx[2];
  u16 max2=0,min2=4095,temp=0;
	
	for (count = 0; count < CodeSize-1; count ++)									// �ɼ�N������
	{
		ADC2_ConvertedValueTab[count] = 0;													// ��ʼ��ֵ0
		ADC_SoftwareStartConvCmd(ADC2,ENABLE);											// �������ADC2ת��
		while(!ADC_GetFlagStatus(ADC2,ADC_FLAG_EOC));								// �ȴ�ת������
		ADC_ClearFlag(ADC2,ADC_FLAG_EOC);														// ת��������־����, Ϊ��һ�����ݲɼ���׼��
		ADC2_ConvertedValueTab[count]=ADC_GetConversionValue(ADC2);	// �ɼ�ֵ����
	}	
	for(count=0;count<CodeSize-1;count++)													// ��ȡ�ɼ�ֵ�ķ��ֵ
	{
		 temp=ADC2_ConvertedValueTab[count];
			//printf("%u,",temp);
			if(temp>max2) max2=temp;
			//if(temp<min2) min2=temp;
			if (temp>1 && temp < min2)//�����һ�������ƣ�ȡ��ADC�ڿ�ʼʱ��ȡ��0��Ϊ��С
					min2 = temp;		
	}
			//printf("\r\n");
	adcx[1]		=  max2 - min2	;
	Adc_data[1]	=	(float)adcx[1] * VREF / 4095;     				// ������ѹ���ֵ������ת��ģ��������
}

//-----------------------------------------------------------------------------
//	void ADC3_Sampling (void)
//-----------------------------------------------------------------------------
//     
// ��������:	ADC3�������ݲɼ�����ȡƽ��ֵ, �ɼ�ֱ����ѹ
// ��ڲ���:	��
// ���ڲ���:	��
// ע������:	
//-----------------------------------------------------------------------------
void ADC3_Sampling(void) 	  
{
	u16 i, j;
	u8 count;
	u32 sum=0,temp=0;
	
	for (count = 0; count < 100; count ++)											// �ɼ�100������
	{
		ADC3_ConvertedValueTab[count] = 0;												// ��ʼ��ֵ0
		ADC_SoftwareStartConvCmd(ADC3,ENABLE);										// �������ADC3ת��
		while(!ADC_GetFlagStatus(ADC3,ADC_FLAG_EOC));							// �ȴ�ADת�����
		ADC_ClearFlag(ADC3,ADC_FLAG_EOC);													// ADת�����,�����־λ
  	ADC3_ConvertedValueTab[count]=ADC_GetConversionValue(ADC3);// �ɼ�ֵ����
	}	
	for(i = 0; i < 100; i++)																		// 100����������
	{
		for(j = 0; j < 100-i; j++)
		{
			if(ADC3_ConvertedValueTab[j]>ADC3_ConvertedValueTab[j+1])
			{
				temp = ADC3_ConvertedValueTab[j];
				ADC3_ConvertedValueTab[j]=ADC3_ConvertedValueTab[j+1];
				ADC3_ConvertedValueTab[j+1] = temp;
			}
		}
	}
	for(i = 10; i < 90; i++)																		// ȥ��ͷβ10���������
	{
		sum += ADC3_ConvertedValueTab[i];   
	}
	sum=sum/80;																									// ȥ��ͷβ10��������ͺ�ȡƽ��
	
	Adc_data[2]	=	(float)sum * VREF / 4095;								// �������ֱ����ѹ
}
