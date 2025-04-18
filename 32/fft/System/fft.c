#include "stm32f10x.h"                  // Device header
#include "Timing Sampling.h"
#include "math.h"
#include "arm_math.h"
#include "stm32_dsp.h"
#define  NPT  256             
uint32_t lBufInArray[NPT];    
uint32_t lBufOutArray[NPT/2]; 
uint32_t lBufMagArray[NPT/2]; 



void GetPowerMag(void)
{
    signed short lX,lY;
    float X,Y,Mag;
    unsigned short i;
    for(i=0; i<NPT/2; i++)
    {
        lX  = (lBufOutArray[i] << 16) >> 16;
        lY  = (lBufOutArray[i] >> 16);
			
		//??32768??65536????????????
        X = NPT * ((float)lX) / 32768;
        Y = NPT * ((float)lY) / 32768;
        Mag = sqrt(X * X + Y * Y)*1.0/ NPT;
        if(i == 0)	
            lBufMagArray[i] = (unsigned long)(Mag * 32768);
        else
            lBufMagArray[i] = (unsigned long)(Mag * 65536);
    }
}

void adc_sample(void)
{
	int i ;
	for (i = 0; i < NPT; i++)
	{
		//lBufInArray[i] = ADC_Value;
	}
	cr4_fft_256_stm32(lBufOutArray,lBufInArray, NPT);
	GetPowerMag();
}
