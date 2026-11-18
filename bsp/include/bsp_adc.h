/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "stdint.h"

extern uint8_t g_BatVal;
extern uint8_t g_BatVal1;
extern uint32_t   g_SampADCDelay;
void ADCInit(void);
uint16_t Get_Adc_Average(uint32_t ch, uint8_t times);

#endif   /**__BSP_ADC_H**/




