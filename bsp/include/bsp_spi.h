/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "stm32l4xx.h"

extern SPI_HandleTypeDef	hspi1;
extern SPI_HandleTypeDef 	hspi2;  //SPI2¾ä±ú

void SPI1_Init(void);
void SPI2_Init(void);
void DeInitSPI(SPI_HandleTypeDef *hspi);


#endif /* __BSP_SPI_H */

