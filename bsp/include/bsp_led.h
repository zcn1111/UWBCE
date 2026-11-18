#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "stdint.h"
#include "stdbool.h"


/*¬Ìº◊÷∏ æµ∆≈‰÷√--PB5*/
#define LED_R_GPIO_CLK_ENABLE()      	__HAL_RCC_GPIOB_CLK_ENABLE();
#define LED_R_GPIO_PORT					GPIOB
#define LED_R_PIN						GPIO_PIN_5
#define LED_R_ON()						HAL_GPIO_WritePin(LED_R_GPIO_PORT,LED_R_PIN,GPIO_PIN_SET)
#define LED_R_OFF()						HAL_GPIO_WritePin(LED_R_GPIO_PORT,LED_R_PIN,GPIO_PIN_RESET)
#define LED_R_TOGGLE()					HAL_GPIO_TogglePin(LED_R_GPIO_PORT,LED_R_PIN)

//±¨…¡µ∆
#define BSD_BLUE_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE();
#define BSD_BLUE_GPIO_PORT				GPIOC
#define BSD_BLUE_PIN					GPIO_PIN_8
#define BSD_BLUE_ON()					HAL_GPIO_WritePin(BSD_BLUE_GPIO_PORT,BSD_BLUE_PIN,GPIO_PIN_RESET)
#define BSD_BLUE_OFF()					HAL_GPIO_WritePin(BSD_BLUE_GPIO_PORT,BSD_BLUE_PIN,GPIO_PIN_SET)
#define BSD_BLUE_TOGGLE()				HAL_GPIO_TogglePin(BSD_BLUE_GPIO_PORT,BSD_BLUE_PIN)

#define BSD_RED_GPIO_CLK_ENABLE()      	__HAL_RCC_GPIOC_CLK_ENABLE();
#define BSD_RED_GPIO_PORT				GPIOC
#define BSD_RED_PIN						GPIO_PIN_9
#define BSD_RED_ON()					HAL_GPIO_WritePin(BSD_RED_GPIO_PORT,BSD_RED_PIN,GPIO_PIN_RESET)
#define BSD_RED_OFF()					HAL_GPIO_WritePin(BSD_RED_GPIO_PORT,BSD_RED_PIN,GPIO_PIN_SET)
#define BSD_RED_TOGGLE()				HAL_GPIO_TogglePin(BSD_RED_GPIO_PORT,BSD_RED_PIN)

/*RTK÷∏ æµ∆≈‰÷√--PB3*/
#define LED_RTK_GPIO_CLK_ENABLE()      	__HAL_RCC_GPIOB_CLK_ENABLE();
#define LED_RTK_GPIO_PORT				GPIOB
#define LED_RTK_PIN						GPIO_PIN_3
#define LED_RTK_ON()					HAL_GPIO_WritePin(LED_RTK_GPIO_PORT,LED_RTK_PIN,GPIO_PIN_RESET)
#define LED_RTK_OFF()					HAL_GPIO_WritePin(LED_RTK_GPIO_PORT,LED_RTK_PIN,GPIO_PIN_SET)
#define LED_RTK_TOGGLE()				HAL_GPIO_TogglePin(LED_RTK_GPIO_PORT,LED_RTK_PIN)

/*BEEP∑‰√˘∆˜*/
#define BEEP_GPIO_CLK_ENABLE()      	__HAL_RCC_GPIOC_CLK_ENABLE();
#define BEEP_GPIO_PORT				    GPIOC
#define BEEP_PIN						GPIO_PIN_6
#define BEEP_ON()					    HAL_GPIO_WritePin(BEEP_GPIO_PORT,BEEP_PIN,GPIO_PIN_RESET)
#define BEEP_OFF()					    HAL_GPIO_WritePin(BEEP_GPIO_PORT,BEEP_PIN,GPIO_PIN_SET)
#define BEEP_TOGGLE()				    HAL_GPIO_TogglePin(BEEP_GPIO_PORT,BEEP_PIN)

extern uint32_t g_PowLedDelay;
extern uint32_t g_RTKLedDelay;           /*rtk÷∏ æµ∆◊¥Ã¨«–ªª—” ±*/
extern uint32_t  g_WarnYYDelay;

extern uint8_t errornum ;
extern uint32_t errornum_timer ;
extern uint8_t flaglagao;
extern uint8_t g_RTKLedFlag;             /*rtk÷∏ æµ∆◊¥Ã¨«–ªª±Íº«*/



void Led_Init(void);
void InitLED(void);
void WarnLED_OFF(void);
void LEDPowONSta(void);
#endif

