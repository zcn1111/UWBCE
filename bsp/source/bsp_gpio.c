
#include "bsp.h"
#include "app_glovar.h"
#include "lowpower.h"
#include "app_tool.h"

uint8_t g_CheckTool;	//1-检测到客服工具
uint8_t SWITCHTYPE = 0; //0开启蓝牙，=1tool,	=2上位机

/**
 * @brief	CheckToolInit 检测客服工具插入引脚初始化
 *
 * @param	void
 *
 * @return  void
 */
void CheckToolInit(void)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;
	
	/* 打开GPIO时钟 */
	 __HAL_RCC_GPIOA_CLK_ENABLE();
	
	/* 配置引脚为推挽输出 */
	GPIO_Initure.Pin = TOOL_GPIO_PIN;
	GPIO_Initure.Mode = GPIO_MODE_IT_FALLING;			/* 下降沿触发 */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;			/* GPIO速度等级 */
	HAL_GPIO_Init(GPIOA, &GPIO_Initure);	
	__HAL_GPIO_EXTI_CLEAR_IT(TOOL_GPIO_PIN);				//清除中断标志位
	HAL_NVIC_SetPriority(EXTI0_IRQn,1,3); 				//抢占优先级为2，子优先级为3
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);						//使能
	
	GPIO_Initure.Pin = SWITCH_USB_BT_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			/* 下降沿触发 */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;			/* GPIO速度等级 */
	HAL_GPIO_Init(SWITCH_USB_BT_GPIO_PORT, &GPIO_Initure);	
	if(HAL_GPIO_ReadPin(GPIOA,TOOL_GPIO_PIN) == GPIO_PIN_SET)
		g_CheckTool = 0;
	else
	{
		g_CheckTool = 1;
		SWITCHTYPE=1;
		Select_485();
		InitUSART1();			
		/*切换串口*/
		InitToolVar();				
	}
	if (SWITCHTYPE>0)
		SWITCH_USB(); 
	else
		SWITCH_BT(); 	
}

void InitChargeGpio(void)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;	
	/* 打开GPIO时钟 */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* 配置引脚为输入 */
	GPIO_Initure.Pin = TOOL_GPIO_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			/* out */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;	/* GPIO速度等级 */;
	HAL_GPIO_Init(GPIOA, &GPIO_Initure);
}

/**
 * @brief	CheckTool 检测客服工具
 *
 * @param	void
 *
 * @return  void
 */
void CheckTool(void)
{
	//充电时，不进行客服工具检测
	if (g_SelfCheck.StartCharge == 1)
	{
		return;
	}
	
	if(HAL_GPIO_ReadPin(GPIOA,TOOL_GPIO_PIN) == GPIO_PIN_SET && g_CheckTool == 1)
	{
		g_CheckTool = 0;
		SWITCHTYPE=0;
		Select_uart();
		InitUSART1();		
	}
	else if(HAL_GPIO_ReadPin(GPIOA,TOOL_GPIO_PIN) == GPIO_PIN_RESET && g_CheckTool == 0)
	{
		g_CheckTool = 1;
		SWITCHTYPE=1;
		Select_485();
		InitUSART1();			
		/*切换串口*/
		InitToolVar();
	}
	
    if(SWITCHTYPE == 1)
	{
		ToolProcess();
	}		
}

/**
 * @brief	PowerContrlInit
 *
 * @param	初始化电源控制
 *
 * @return  void
 */
void PowerContrlInit(void)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;
	
	/* 打开GPIO时钟 */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	BTPOW_GPIO_CLK_ENABLE();	
	/*RTK*/
	GPIO_Initure.Pin = PWR_RTK_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			/* 推挽输出 */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;	/* GPIO速度等级 */
	HAL_GPIO_Init(PWR_RTK_GPIO_PORT, &GPIO_Initure);	
	PWR_RTK_ENABLE();
	delay_ms(100);
	
	/*4G模块*/
	GPIO_Initure.Pin = PWR_4G_PIN;	
	HAL_GPIO_Init(PWR_4G_GPIO_PORT, &GPIO_Initure);
	PWR_4G_ENABLE();
	delay_ms(100);
	
	GPIO_Initure.Pin = BTPOW_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			/* 推挽输出 */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;			/* GPIO速度等级 */
	HAL_GPIO_Init(BTPOW_GPIO_PORT, &GPIO_Initure);	
	BT_POW_EN();	

	delay_ms(100);	
}

/**
 * @brief	PowerContrlInit
 *
 * @param	初始化4G模块引脚
 *
 * @return  void
 */

void DTU_IO_Init(void)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;
	
	/* 打开GPIO时钟 */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	/*4G复位引脚控制*/
	GPIO_Initure.Pin = RESET_4G_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			/* 推挽输出 */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;	/* GPIO速度等级 */
	HAL_GPIO_Init(RESET_4G_GPIO_PORT, &GPIO_Initure);	
	/*4G唤醒引脚控制*/
	GPIO_Initure.Pin = WAKEUP_4G_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			/* 推挽输出 */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;	/* GPIO速度等级 */
	HAL_GPIO_Init(WAKEUP_4G_GPIO_PORT, &GPIO_Initure);		
	/*4GINT引脚控制*/
	GPIO_Initure.Pin = INT_4G_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			/* 推挽输出 */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;	/* GPIO速度等级 */
	HAL_GPIO_Init(INT_4G_GPIO_PORT, &GPIO_Initure);		
	/*4GPWRKEY引脚控制*/
	GPIO_Initure.Pin = PWRKEY_4G_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			/* 推挽输出 */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;	/* GPIO速度等级 */
	HAL_GPIO_Init(PWRKEY_4G_GPIO_PORT, &GPIO_Initure);	
	PWRKEY_4G_ENABLE();
}

