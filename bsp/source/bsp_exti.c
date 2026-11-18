
#include "bsp.h"
#include "lowpower.h"
#include "app_tool.h"
#include "deca_device_api.h"


uint32_t g_KeyCnt;
volatile uint8_t g_recvirq=0;

/**
  * @brief  EXTI0 ?��??????
  */
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(TOOL_GPIO_PIN);
}

/**
  * @brief  EXTI10-15 ?��??????
  * @note   KEY1=PC0(??EXTI0), DW1000_IRQ=PB14(EXTI14)
  */
void EXTI15_10_IRQHandler(void)
{	
    HAL_GPIO_EXTI_IRQHandler(KEY1_PIN);
    HAL_GPIO_EXTI_IRQHandler(DW1000_IRQ_PIN);
}

/**
  * @brief  �ⲿ�жϻص�����
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == DW1000_IRQ_PIN)
    {
        g_dw1000_irq_flag = 1;
        dwt_isr();  /* 调用官方驱动中断处理，触发TX/RX回调 */
		g_recvirq = 0;
    }
}

