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
uint32_t FFT_OutData[sample_num/2];	  //fft输出序列
uint32_t FFT_Mag[sample_num/2] ;	
uint16_t Adc_data[sample_num]={0};
uint16_t Adc_data1[CodeSize*Channel]={0};

unsigned short  secondMaxIndex = 0;
unsigned short  thirdMaxIndex=0;

static u16 ADC2_ConvertedValueTab[CodeSize];
static u16 ADC3_ConvertedValueTab[100];

float vpp_vpp;

uint8_t adc_finish_fg = 0;//adc获取数据标志位
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
		DMA_ClearITPendingBit(DMA1_IT_TC1);//清空DMA标志位
		DMA_Cmd(DMA1_Channel1, DISABLE);					 	// 关闭DMA1通道1和ADC1
		ADC_Cmd(ADC1, DISABLE);	 
		adc_finish_fg =1;//写adc标志位，表示adc接收完成
		
	}
	
}

void ADC_Initialization(ADC_TypeDef* ADCx)
{
	ADC_InitTypeDef ADC_InitStructure;
	// 设置每个ADC工作在规则独立模式 
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	// 规定了模数转换工作在扫描模式(多通道)模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;			 	
	// 规定了模数转换工作在连续模式	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	
	// 定义了使用外部触发来启动规则通道的模数转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	// 规定了ADC数据向左边对齐还是向右边对齐
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	
	// 规定了顺序进行规则转换的ADC通道的数目:1
	ADC_InitStructure.ADC_NbrOfChannel = Channel;
	// 设置ADC1的寄存器	
	ADC_Init(ADCx, &ADC_InitStructure);
}

void ADC_Calibration(ADC_TypeDef * ADCx)
{
	// 复位ADC自校准
	ADC_ResetCalibration(ADCx);		
	// 检查ADC复位校准结束否						
	while(ADC_GetResetCalibrationStatus(ADCx));
	// 启动ADC自校准程序
	ADC_StartCalibration(ADCx);							
	// 检查ADC自校准结束否
	while(ADC_GetCalibrationStatus(ADCx));	
}

//ADC2/3 Configuration
void ADC_RegularChannel_Configuration(void)
{
	// 打开ADC2和ADC3时钟
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_ADC2|RCC_APB2Periph_ADC3 , ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 				// 设置ADC时钟, 为PCLK2的6分频,即12MHz

																																								
	// 选择ADC2通道12，排在第一位采集，采样时间为1.5周期
	ADC_Initialization(ADC2);	
	ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 1,ADC_SampleTime_1Cycles5);		//PC2，采集输出交流
	
	// 选择ADC3通道10，排在第二位采集，采样时间为1.5周期
	ADC_Initialization(ADC3);	
	ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 1,ADC_SampleTime_1Cycles5);		//PC3，采集输出直流
	ADC_Cmd(ADC3, ENABLE);										// 使能ADC3
	ADC_Calibration(ADC3);										// ADC3自动校准
	ADC_Cmd(ADC2, ENABLE);										// 使能ADC2
	ADC_Calibration(ADC2);										// ADC2自动校准	
}

void Get_FFT_Source_Data(void)//将ADC数据赋给FFT_SourceData的前16位
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
	unsigned long thirdMaxMag_val = 0; // 使用一个新变量存第三大峰值
	unsigned short maxIndex = 0;

  for(i = 1; i < sample_num / 2; i++)//由于FFT的频域结果是关于奈奎斯特频率对称的，所以只计算一半
  {
		lX = (FFT_OutData[i] << 16) >> 16;		//取低十六位，虚部
    lY = (FFT_OutData[i] >> 16);					//取高十六位，实部

    X = sample_num * ((float)lX) / 32768;
    Y = sample_num * ((float)lY) / 32768;

    Mag = sqrt(X * X + Y * Y) / sample_num;
		//这些就是计算振幅sqrt(lx*lx+ly*ly)*2/NPT

    FFT_Mag[i] = (unsigned long)(Mag * 65536);
		//之所以先除以32768后乘以65536是为了符合浮点数的计算规律
		
		//以下代码为获取最强，第二强，第三强的信号索引和幅度
   for (i = 1; i < sample_num / 2; i++) {
    // ... (计算 Mag 和 FFT_Mag[i]) ...

    if (FFT_Mag[i] > maxMag) {
        thirdMaxMag_val = secondMaxMag; // 原来的第二大变成第三大
        thirdMaxIndex = secondMaxIndex; // 原来的第二大索引变成第三大索引

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
  Get_FFT_Source_Data();//更新FFT_SourceData数据
  cr4_fft_1024_stm32(FFT_OutData, FFT_SourceData, sample_num);
	//cr4_fft_1024_stm32(输出序列，输入序列，样本点)，执行fft命令（汇编）
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
// 功能程序区
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//	u16 ADC_MATH(void)
//-----------------------------------------------------------------------------
//     
// 函数功能:	获取批量ADC采集数据的峰峰值
// 入口参数:	无
// 出口参数:	峰峰值
// 注意事项:	
//-----------------------------------------------------------------------------
// 建议修改
float ADC_MATH(void) // 返回值类型改为 float
{
    u16 min_val = 65535, max_val = 0; // 变量名更清晰
    u16 i;

    for (i = 0; i < CodeSize * Channel; i++) // 确保 CodeSize*Channel 是 Adc_data1 的正确长度
    {
        if (Adc_data1[i] > max_val) 
					
					max_val = Adc_data1[i];
				
        //if (Adc_data1[i] < min_val)
					if (Adc_data1[i]>30 && Adc_data1[i] < min_val)//添加了一个下限制，取消ADC在开始时读取到0作为最小
					min_val = Adc_data1[i];
				
    }
		//添加三行新代码
		u16 umax_val =(float)max_val* VREF / 4095.0f; 
		u16 umin_val =(float)min_val* VREF / 4095.0f; 
		//printf("fengfengzhi：max;%u  min;%u \r\n",umax_val,umin_val);
    u16 MMM = max_val - min_val;
    // vpp_vpp 是全局变量，这里可以直接赋值
    vpp_vpp = (float)MMM * VREF / 4095.0f; // 使用 4095.0f 进行浮点除法，VREF 也应为浮点数或先转
    return vpp_vpp; // 返回计算得到的浮点值
}
//-----------------------------------------------------------------------------
//	void ADC_Samp_Sweep (void)
//-----------------------------------------------------------------------------
//     
// 函数功能:	扫频显示幅频曲线时的交流电压采集, 获取ADC2批量采集数据的峰峰值
// 入口参数:	无
// 出口参数:	无
// 注意事项:	
//-----------------------------------------------------------------------------
void ADC_Samp_Sweep (void) 	//
{
	int count;
	u16	adcx[2],CodeSize_1=1024;
	u16 max2=0,min2=4095,temp=0;
	
	for (count = 0; count < CodeSize_1-1; count ++)											// 采样N个数据
	{
		ADC2_ConvertedValueTab[count] = 0;
		ADC_SoftwareStartConvCmd(ADC2,ENABLE); 														// 软件启动ADC2采集
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
	Adc_data[1]	=	(float)adcx[1] * VREF / 4095;  // 交流电压存放数组
}

//-----------------------------------------------------------------------------
//	void ADC_Sampling (void)
//-----------------------------------------------------------------------------
//     
// 函数功能:	ADC2批量数据采集并获取峰峰值
// 入口参数:	无
// 出口参数:	无
// 注意事项:	ADC2需要准确测量输出交流电压峰峰值使用
//-----------------------------------------------------------------------------
void ADC_Sampling (void) 		
{
	int count;
	u16	adcx[2];
  u16 max2=0,min2=4095,temp=0;
	
	for (count = 0; count < CodeSize-1; count ++)									// 采集N个数据
	{
		ADC2_ConvertedValueTab[count] = 0;													// 初始赋值0
		ADC_SoftwareStartConvCmd(ADC2,ENABLE);											// 软件启动ADC2转换
		while(!ADC_GetFlagStatus(ADC2,ADC_FLAG_EOC));								// 等待转换结束
		ADC_ClearFlag(ADC2,ADC_FLAG_EOC);														// 转换结束标志清零, 为下一个数据采集做准备
		ADC2_ConvertedValueTab[count]=ADC_GetConversionValue(ADC2);	// 采集值保存
	}	
	for(count=0;count<CodeSize-1;count++)													// 获取采集值的峰峰值
	{
		 temp=ADC2_ConvertedValueTab[count];
			//printf("%u,",temp);
			if(temp>max2) max2=temp;
			//if(temp<min2) min2=temp;
			if (temp>1 && temp < min2)//添加了一个下限制，取消ADC在开始时读取到0作为最小
					min2 = temp;		
	}
			//printf("\r\n");
	adcx[1]		=  max2 - min2	;
	Adc_data[1]	=	(float)adcx[1] * VREF / 4095;     				// 交流电压峰峰值数字量转成模拟量后存放
}

//-----------------------------------------------------------------------------
//	void ADC3_Sampling (void)
//-----------------------------------------------------------------------------
//     
// 函数功能:	ADC3批量数据采集并获取平均值, 采集直流电压
// 入口参数:	无
// 出口参数:	无
// 注意事项:	
//-----------------------------------------------------------------------------
void ADC3_Sampling(void) 	  
{
	u16 i, j;
	u8 count;
	u32 sum=0,temp=0;
	
	for (count = 0; count < 100; count ++)											// 采集100个数据
	{
		ADC3_ConvertedValueTab[count] = 0;												// 初始赋值0
		ADC_SoftwareStartConvCmd(ADC3,ENABLE);										// 软件启动ADC3转换
		while(!ADC_GetFlagStatus(ADC3,ADC_FLAG_EOC));							// 等待AD转换完成
		ADC_ClearFlag(ADC3,ADC_FLAG_EOC);													// AD转换完成,清除标志位
  	ADC3_ConvertedValueTab[count]=ADC_GetConversionValue(ADC3);// 采集值保存
	}	
	for(i = 0; i < 100; i++)																		// 100个数据排序
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
	for(i = 10; i < 90; i++)																		// 去掉头尾10个数据求和
	{
		sum += ADC3_ConvertedValueTab[i];   
	}
	sum=sum/80;																									// 去掉头尾10个数据求和后取平均
	
	Adc_data[2]	=	(float)sum * VREF / 4095;								// 保存输出直流电压
}
