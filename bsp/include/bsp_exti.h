/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_EXTI_H
#define __BSP_EXTI_H

#include <stdint.h>
#include "stm32l4xx.h"

extern volatile uint8_t g_recvirq;
void DW1000_IrqTimingEnable(uint8_t enabled);
uint32_t DW1000_IrqTimeHi(void);

#endif /* __BSP_EXTI_H */
