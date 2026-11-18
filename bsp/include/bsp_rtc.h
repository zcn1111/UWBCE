/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_RTC_H
#define __BSP_RTC_H

#include <stdint.h>
#include "stm32l4xx.h"



//extern DATETIME g_BCDTime;
extern RTC_HandleTypeDef RTC_Handler;  //RTC¾ä±ú

uint8_t RTC_Init(void);
HAL_StatusTypeDef RTC_Set_Time(uint8_t hour, uint8_t min, uint8_t sec, uint8_t ampm);
HAL_StatusTypeDef RTC_Set_Date(uint8_t year, uint8_t month, uint8_t date, uint8_t week);
void RTC_Set_WakeUp(uint32_t wksel, uint16_t cnt);
void RTC_Set_AlarmA(uint8_t week, uint8_t hour, uint8_t min, uint8_t sec);
void ReadTime(void);


#endif   /**__BSP_RTC_H**/
