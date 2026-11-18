#include "app_WDG.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_iwdg.h"

IWDG_HandleTypeDef hiwdg;  //IWDG句柄

void MX_IWDG_Init(void)
{
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_64;    //分频系数64
	hiwdg.Init.Reload = 2500;                    //重装载值
	hiwdg.Init.Window = 2500;                    //IWDG窗口值=重载值即可
	if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
	{
		Error_Handler();  //初始化失败报错
	}
}

void IWDG_Feed(void)
{
	HAL_IWDG_Refresh(&hiwdg);  //喂狗：重载计数器，核心API
}