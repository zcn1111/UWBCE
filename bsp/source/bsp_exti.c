
#include "bsp.h"
#include "lowpower.h"
#include "app_tool.h"
#include "deca_device_api.h"


uint32_t g_KeyCnt;
volatile uint8_t g_recvirq=0;
static volatile uint8_t dw_irq_timing_enabled;
static volatile uint32_t dw_irq_time_hi;

void DW1000_IrqTimingEnable(uint8_t enabled)
{
    dw_irq_timing_enabled = enabled != 0;
}

uint32_t DW1000_IrqTimeHi(void)
{
    return dw_irq_time_hi;
}

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
        /* 窗口诊断时在驱动清状态、TRXOFF、RX reset 之前采样。
         * 仍包含外部中断响应和本次 SPI 读的延迟，不是硬件错误时间戳。 */
        if (dw_irq_timing_enabled)
            dw_irq_time_hi = dwt_readsystimestamphi32();
        dwt_isr();  /* 调用官方驱动中断处理，触发TX/RX回调 */
		g_recvirq = 0;
    }
}

