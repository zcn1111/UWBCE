#ifndef __BSP_IIC_H
#define __BSP_IIC_H

#include "stm32l4xx.h"
#include "stdbool.h"

extern I2C_HandleTypeDef hi2c1;



void I2C1_Init(void);

#endif

