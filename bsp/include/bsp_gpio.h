/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "stm32l4xx.h"
#include "stdbool.h"

typedef enum
{ 
	LED_RTK,
	LED_RGB_R,
    LED_ALL
} led_t;

/*C??????????л?????????????????????*/
#define TOOL_GPIO_PIN					GPIO_PIN_0	//????????????????

/*BT??USB?л?????*/
#define SWITCH_USB_BT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE();
#define SWITCH_USB_BT_GPIO_PORT			GPIOA
#define SWITCH_USB_BT_PIN				GPIO_PIN_12
#define SWITCH_USB()                    HAL_GPIO_WritePin(SWITCH_USB_BT_GPIO_PORT,SWITCH_USB_BT_PIN,GPIO_PIN_SET)
#define SWITCH_BT()                     HAL_GPIO_WritePin(SWITCH_USB_BT_GPIO_PORT,SWITCH_USB_BT_PIN,GPIO_PIN_RESET)


/*?????????*/
#define PWR_SYS_GPIO_PORT			    GPIOB
#define PWR_SYS_PIN				        GPIO_PIN_2
#define PWR_SYS_ENABLE()		        HAL_GPIO_WritePin(PWR_SYS_GPIO_PORT,PWR_SYS_PIN,GPIO_PIN_SET);
#define PWR_SYS_DISABLE()	            HAL_GPIO_WritePin(PWR_SYS_GPIO_PORT,PWR_SYS_PIN,GPIO_PIN_RESET);

/*RTK???????*/
#define PWR_RTK_GPIO_PORT			    GPIOB
#define PWR_RTK_PIN				        GPIO_PIN_13
#define PWR_RTK_ENABLE()		        HAL_GPIO_WritePin(PWR_RTK_GPIO_PORT,PWR_RTK_PIN,GPIO_PIN_SET);
#define PWR_RTK_DISABLE()	            HAL_GPIO_WritePin(PWR_RTK_GPIO_PORT,PWR_RTK_PIN,GPIO_PIN_RESET);

/*4G?????????*/
#define PWR_4G_GPIO_PORT			    GPIOA
#define PWR_4G_PIN				        GPIO_PIN_15
#define PWR_4G_ENABLE()                 HAL_GPIO_WritePin(PWR_4G_GPIO_PORT,PWR_4G_PIN,GPIO_PIN_SET);   
#define PWR_4G_DISABLE()	            HAL_GPIO_WritePin(PWR_4G_GPIO_PORT,PWR_4G_PIN,GPIO_PIN_RESET);

/*???????????*/
#define BTPOW_GPIO_CLK_ENABLE()     	__HAL_RCC_GPIOD_CLK_ENABLE()
#define BTPOW_GPIO_PORT              	GPIOD
#define BTPOW_PIN                    	GPIO_PIN_2
#define BT_POW_EN()				        HAL_GPIO_WritePin(BTPOW_GPIO_PORT,BTPOW_PIN,GPIO_PIN_RESET);
#define BT_POW_DISEN()				    HAL_GPIO_WritePin(BTPOW_GPIO_PORT,BTPOW_PIN,GPIO_PIN_SET);

/*4G??λ???????*/
#define RESET_4G_GPIO_PORT			    GPIOB
#define RESET_4G_PIN				    GPIO_PIN_12
#define RESET_4G_ENABLE()               HAL_GPIO_WritePin(RESET_4G_GPIO_PORT,RESET_4G_PIN,GPIO_PIN_SET);   
#define RESET_4G_DISABLE()	            HAL_GPIO_WritePin(RESET_4G_GPIO_PORT,RESET_4G_PIN,GPIO_PIN_RESET);
/*4G???????????*/
#define WAKEUP_4G_GPIO_PORT			    GPIOB
#define WAKEUP_4G_PIN				    GPIO_PIN_0
#define WAKEUP_4G_ENABLE()              HAL_GPIO_WritePin(WAKEUP_4G_GPIO_PORT,WAKEUP_4G_PIN,GPIO_PIN_SET);   
#define WAKEUP_4G_DISABLE()	            HAL_GPIO_WritePin(WAKEUP_4G_GPIO_PORT,WAKEUP_4G_PIN,GPIO_PIN_RESET);
/*4GINT???????*/
#define INT_4G_GPIO_PORT			    GPIOC
#define INT_4G_PIN				        GPIO_PIN_7
#define INT_4G_ENABLE()                 HAL_GPIO_WritePin(INT_4G_GPIO_PORT,INT_4G_PIN,GPIO_PIN_SET);   
#define INT_4G_DISABLE()	            HAL_GPIO_WritePin(INT_4G_GPIO_PORT,INT_4G_PIN,GPIO_PIN_RESET);
/*4GPWRKEY???????*/
/*4GPWRKEY控制端口*/
#define PWRKEY_4G_GPIO_PORT			    GPIOB
#define PWRKEY_4G_PIN				    GPIO_PIN_4
#define PWRKEY_4G_ENABLE()              HAL_GPIO_WritePin(PWRKEY_4G_GPIO_PORT,PWRKEY_4G_PIN,GPIO_PIN_SET);   
#define PWRKEY_4G_DISABLE()	            HAL_GPIO_WritePin(PWRKEY_4G_GPIO_PORT,PWRKEY_4G_PIN,GPIO_PIN_RESET);

/*DW1000 UWB模组控制引脚*/
#define DW1000_CS_GPIO_PORT             GPIOB
#define DW1000_CS_PIN                   GPIO_PIN_2
#define DW1000_CS_LOW()                 HAL_GPIO_WritePin(DW1000_CS_GPIO_PORT, DW1000_CS_PIN, GPIO_PIN_RESET)
#define DW1000_CS_HIGH()                HAL_GPIO_WritePin(DW1000_CS_GPIO_PORT, DW1000_CS_PIN, GPIO_PIN_SET)

#define DW1000_RST_GPIO_PORT            GPIOB
#define DW1000_RST_PIN                  GPIO_PIN_11
#define DW1000_RST_LOW()                HAL_GPIO_WritePin(DW1000_RST_GPIO_PORT, DW1000_RST_PIN, GPIO_PIN_RESET)
#define DW1000_RST_HIGH()               HAL_GPIO_WritePin(DW1000_RST_GPIO_PORT, DW1000_RST_PIN, GPIO_PIN_SET)

#define DW1000_IRQ_GPIO_PORT            GPIOB
#define DW1000_IRQ_PIN                  GPIO_PIN_14
#define DW1000_IRQ_EXTI_IRQn            EXTI15_10_IRQn

#define DW1000_PA_GPIO_PORT				GPIOA
#define DW1000_PA_GPIO_PIN				GPIO_PIN_8
#define DW1000_PA_LOW()					HAL_GPIO_WritePin(DW1000_PA_GPIO_PORT, DW1000_PA_GPIO_PIN, GPIO_PIN_RESET)
#define DW1000_PA_HIGH()				HAL_GPIO_WritePin(DW1000_PA_GPIO_PORT, DW1000_PA_GPIO_PIN, GPIO_PIN_SET)


extern uint8_t g_CheckTool;
extern uint8_t g_BoardType;

extern uint8_t g_AlarmOnFlag;
extern uint8_t SWITCHTYPE;  
extern uint32_t g_LedRToggleDelay;
extern uint32_t g_AlarmOnDelay;

void CheckToolInit(void);
void CheckTool(void);
void InitBeep(void);
void CheckKey(void);
void InitAlarmGpio(void);
void BoardTypeInit(void);
void PowerContrlInit(void);
void DTU_IO_Init(void);
void InitChargeGpio(void);
#endif /* __BSP_GPIO_H */
