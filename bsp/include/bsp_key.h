#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "stdint.h"
#include "stdbool.h"

/*����1���ڴ�㣬����2���ڹػ�*/
#define KEY_GPIO_CLK_ENABLE()      		__HAL_RCC_GPIOC_CLK_ENABLE();
#define KEY_GPIO_PORT					GPIOC
#define KEY1_PIN						GPIO_PIN_0
#define KEY2_PIN						GPIO_PIN_1
#define READ_KEY1_STA()					HAL_GPIO_ReadPin(KEY_GPIO_PORT,KEY1_PIN)
#define READ_KEY2_STA()					HAL_GPIO_ReadPin(KEY_GPIO_PORT,KEY2_PIN)

extern uint8_t  g_Key1Sta;
extern uint32_t g_KeyDelay;
extern uint8_t  g_ManualPunFlag;    /*�ֶ����*/
extern volatile uint8_t  flag_acc_irq;
extern uint8_t g_KeySelfCheck;

void InitKeyGpio(void);
void Key_Init(void);
void Read_Flag_Key(void);

#endif
