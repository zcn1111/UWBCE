#include "bsp.h"
#include "app_ly.h"

uint8_t g_RTKLedFlag;             /*rtk指示灯状态切换标记*/
uint32_t g_PowLedDelay;           /*正常工作指示灯切换延时*/

uint32_t g_RTKLedDelay;           /*rtk指示灯状态切换延时*/
uint32_t  g_WarnYYDelay;

uint32_t LEDPowOnStaDelay = 0;
uint8_t  LEDPowOnStaInit = 1;

uint8_t errornum = 0;
uint32_t errornum_timer = 0;
uint8_t flaglagao = 0;

void Led_Init(void)
{	
	g_RTKLedDelay = TIM2->CNT;      /*rtk指示灯状态切换延时*/
	g_RTKLedFlag = 1;               /*rtk指示灯状态切换标记*/
	g_PowLedDelay = TIM2->CNT;
	LEDPowOnStaDelay = TIM2->CNT;
}

/*
*********************************************************************************************************
*	函 数 名: InitLEDGpio
*	功能说明: 配置LED的GPIO引脚
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void InitLED(void)
{	
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure = {0};;
	/* 打开GPIO时钟 */
	LED_R_GPIO_CLK_ENABLE();
	LED_RTK_GPIO_CLK_ENABLE();
	BSD_BLUE_GPIO_CLK_ENABLE();
	BSD_RED_GPIO_CLK_ENABLE();

	/* 配置引脚为输入 */
	GPIO_Initure.Pin = LED_R_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			/* out */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;	/* GPIO速度等级 */;
	HAL_GPIO_Init(LED_R_GPIO_PORT, &GPIO_Initure);	
	LED_R_OFF();
	
	GPIO_Initure.Pin = LED_RTK_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			/* out */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;	/* GPIO速度等级 */;
	HAL_GPIO_Init(LED_RTK_GPIO_PORT, &GPIO_Initure);	
	LED_RTK_OFF();

	GPIO_Initure.Pin = BEEP_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			/* out */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;	/* GPIO速度等级 */;
	HAL_GPIO_Init(BEEP_GPIO_PORT, &GPIO_Initure);	
	BEEP_OFF();
	Led_Init();
}

void WarnLED_OFF(void)
{
	LED_R_OFF();
	BEEP_OFF();
}

/*
*********************************************************************************************************
*	函 数 名: LEDPowONSta
*	功能说明: 马甲指示灯上电亮一次后灭
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LEDPowONSta(void)
{
	while (LEDPowOnStaInit == 1)
	{
		LED_R_ON();
		
		if(JudgeSomeDelayInProOk(LEDPowOnStaDelay,Delay3Sec) == true)
		{
			if (g_SelfCheck.StartCharge == 0)
			{
				LED_R_OFF();
			}
			
			LEDPowOnStaInit = 0;
		}
	}
}
