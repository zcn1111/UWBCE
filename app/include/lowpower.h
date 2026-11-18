/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOWPOWER_H
#define __LOWPOWER_H

#include "stdint.h"
#include "stdbool.h"


#define PWR_5V_ENABLE()				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET);
#define PWR_5V_DISABLE()			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);
#define PWR_CELLUAR_ENABLE()		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
#define PWR_CELLUAR_DISABLE()		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
#define PWR_3V3_ENABLE()			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
#define PWR_3V3_DISABLE()			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);

#define PWR_12V_ENABLE()			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET);
#define PWR_12V_DISABLE()			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_RESET);
#define	PWR_READER_ENABLE()			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
#define PWR_READER_DISABLE()		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);

extern uint8_t g_ReaderPowerOn;
extern uint8_t g_CelluarPowerOn;
extern uint32_t g_CelluarPowerOffDelay;
extern uint32_t g_CelluarPowerOnDelay;
extern uint8_t g_ShouldResetRTCAlarmFlag;
extern uint32_t g_LockPowerOnDelay;

void ExitStopMode(void);
void EnterStopMode(void);



#endif /* __LOWPOWER_H */
