#include "stm32l4xx.h"
#include "bsp.h"
#include "lowpower.h"
#include "4G_Comm.h"
#include "app_glovar.h"
#include "app_warn.h"

#include "IMU_Acc.h"
#include "madgwick_uwb_fusion.h"

TIM_HandleTypeDef TIM6_Handler;      //定时器句柄 
TIM_HandleTypeDef TIM7_Handler;      //定时器句柄 
TIM_HandleTypeDef TIM2_Handler;      //定时器句柄 
TIM_HandleTypeDef TIM1_Handler;      //定时器句柄 

uint32_t tmr50cnt=0;

TIM_OC_InitTypeDef sConfig;
/* 这2个全局变量转用于 bsp_DelayMS() 函数 */


/*
	全局运行时间，单位1ms
	最长可以表示 24.85天，如果你的产品连续运行时间超过这个数，则必须考虑溢出问题
*/
uint32_t fac_us = 0;
DATETIME g_SysTime;
DATETIME g_BCDTime;
uint32_t g_Second = 0;
uint8_t g_CalHourFlag = 0;
uint8_t g_CalMinuteFlag = 0;
/**
 * @brief	初始化延迟函数,SYSTICK的时钟固定为AHB时钟
 *
 * @param   SYSCLK	系统时钟频率
 *
 * @return  void
 */
void delay_init(uint8_t SYSCLK)
{
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);//SysTick频率为HCLK
    fac_us = SYSCLK;						
}


/**
 * @brief	延时微秒(us)函数
 *
 * @remark	nus:0~190887435(最大值即2^32/fac_us@fac_us=22.5)
 *
 * @param   nus		需要延时多少微秒
 *
 * @return  void
 */
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;			//LOAD的值
    ticks = nus * fac_us; 						//需要的节拍数
    told = SysTick->VAL;        				//刚进入时的计数器值

    while(1)
    {
        tnow = SysTick->VAL;

        if(tnow != told)
        {
            if(tnow < told)
				tcnt += told - tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
            else 
				tcnt += reload - tnow + told;
			
            told = tnow;
            if(tcnt >= ticks)
				break;			//时间超过/等于要延迟的时间,则退出.
        }
    }
}

/**
 * @brief	延时毫秒(ms)函数
 *
 * @param   nms		需要延时多少毫秒
 *
 * @return  void
 */
void delay_ms(uint16_t nms)
{
    uint32_t i;

    for(i = 0; i < nms; i++) 
		delay_us(1000);
}


/**
 * @brief	通用定时器6中断初始化函数
 *			定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
 *			Ft=定时器工作频率,单位:Mhz
 *			这里使用的是定时器6!(定时器3挂在APB1上，时钟为HCLK:72Mhz)
 *			预分频(每经过psc个时钟周期计数器累加) = HCLK / (psc + 1) = 72000000Hz / 7200 = 10000Hz
 *			计数(累加到arr后产生一次中断) = pCLK / (arr + 1) = 10000Hz / 800 = 12.5Hz
 *			
 * @param   arr		自动重装值。
 * @param   psc		时钟预分频数
 *
 * @return  void
 */
void TIM6Init(uint16_t arr, uint16_t psc)
{
	__HAL_RCC_TIM6_CLK_ENABLE();            //使能TIM6时钟
	
    TIM6_Handler.Instance = TIM6;                          	//通用定时器3
    TIM6_Handler.Init.Prescaler = psc;                     	//分频系数
    TIM6_Handler.Init.CounterMode = TIM_COUNTERMODE_UP;    	//向上计数器
    TIM6_Handler.Init.Period = arr;                        	//自动装载值
    TIM6_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;//时钟分频因子
	TIM6_Handler.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&TIM6_Handler);
	
	HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 5, 3);  //设置中断优先级，抢占优先级1，子优先级3
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);          //开启ITM6中断
	
	HAL_TIM_Base_Start_IT(&TIM6_Handler);					//使能定时器6和定时器6更新中断：TIM_IT_UPDATE
}

/**
 * @brief	通用定时器7中断初始化函数
 *			定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
 *			Ft=定时器工作频率,单位:Mhz
 *			这里使用的是定时器3!(定时器3挂在APB1上，时钟为HCLK:80Mhz)
 *
 * @param   arr		自动重装值。
 * @param   psc		时钟预分频数
 *
 * @return  void
 */
void TIM7Init(uint16_t arr, uint16_t psc)
{
	__HAL_RCC_TIM7_CLK_ENABLE();            //使能TIM6时钟
       
    TIM7_Handler.Instance = TIM7;                          	//通用定时器3
    TIM7_Handler.Init.Prescaler = psc;                     	//分频系数
    TIM7_Handler.Init.CounterMode = TIM_COUNTERMODE_UP;    	//向上计数器
    TIM7_Handler.Init.Period = arr;                        	//自动装载值
    TIM7_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM7_Handler);
	HAL_NVIC_SetPriority(TIM7_IRQn, 5, 4);  //设置中断优先级，抢占优先级1，子优先级3
    HAL_NVIC_EnableIRQ(TIM7_IRQn);          //开启ITM6中断
    HAL_TIM_Base_Start_IT(&TIM7_Handler);					//使能定时器7和定时器7更新中断：TIM_IT_UPDATE
}
/**
 * @brief	定时器底册驱动，开启时钟，设置中断优先级
 *			此函数会被HAL_TIM_Base_Init()函数调用
 *
 * @param   htim	定时器句柄
 *
 * @return  void
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM6)
    {
        __HAL_RCC_TIM6_CLK_ENABLE();            //使能TIM6时钟
        
    }
	else if(htim->Instance == TIM7)
	{
		__HAL_RCC_TIM7_CLK_ENABLE();            //使能TIM6时钟
	}
}

/**
 * @brief	定时器6中断服务函数
 *
 * @param   void
 *
 * @return  void
 */
void TIM6_DAC_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&TIM6_Handler);

}

/**
 * @brief	定时器6中断服务函数
 *
 * @param   void
 *
 * @return  void
 */
void TIM7_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM7_Handler);
}

/**
 * @brief	回调函数，定时器中断服务函数调用
 *
 * @param   htim	定时器句柄
 *
 * @return  void
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == (&TIM7_Handler))
    {	
		tmr50cnt++;
		if (g_SelfCheck.StartCharge == 1)
		{
			return;
		}
		
		//远程升级程序时，不做告警
		if (g_RemoteUpdateGCFlag != 0)
		{
			LED_R_OFF();
			return;
		}
		
		if (g_AreaTypeAlarm == 0)
		{
			LED_R_OFF();
			return;
		}
		
		//作业时间
		if ((JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == AFTER) && 
				(JudgTimeOrder(g_SysTime,g_WorkTime.stop,0) == BEFORE))
		{
			if (g_TrainInfo.AppTrainApproach == 1)
			{
				LED_R_TOGGLE();
			}
			else if (g_TrainInfo.TrainApproach == 1)
			{
				//马甲灯
				//常亮
				if ((g_AreaTypeAlarm > 0) && (AreaAlarmInfo[g_AreaTypeAlarm].armourled.trainnear == 1))
				{
					LED_R_ON();
				}
				//闪烁
				else if ((g_AreaTypeAlarm > 0) && AreaAlarmInfo[g_AreaTypeAlarm].armourled.trainnear == 2)
				{
					LED_R_TOGGLE();
				}
				//熄灭
				else
				{
					LED_R_OFF();
				}
			}
			else if ((CarFlag == 1) || (ProtectionPositionFlag == 1))
			{
				LED_R_TOGGLE();
			}
			//无列车
			else 
			{
				//马甲灯
				//常亮
				if ((g_AreaTypeAlarm > 0) && AreaAlarmInfo[g_AreaTypeAlarm].armourled.notrain == 1)
				{
					LED_R_ON();
				}
				//闪烁
				else if ((g_AreaTypeAlarm > 0) && AreaAlarmInfo[g_AreaTypeAlarm].armourled.notrain == 2)
				{
					LED_R_TOGGLE();
				}
				//熄灭
				else
				{
					LED_R_OFF();
				}
			}
		}
		//非作业时间
		else
		{
			//马甲灯
			if ((g_TrainInfo.TrainApproach == 1) || (g_TrainInfo.AppTrainApproach == 1))
			{
				LED_R_TOGGLE();
			}
			else if ((CarFlag == 1) || (ProtectionPositionFlag == 1))
			{
				LED_R_TOGGLE();
			}
			//常亮
			else if ((g_AreaTypeAlarm > 0) && AreaAlarmInfo[g_AreaTypeAlarm].armourled.nojobtime == 1)
			{
				LED_R_ON();
			}
			//闪烁
			else if ((g_AreaTypeAlarm > 0) && AreaAlarmInfo[g_AreaTypeAlarm].armourled.nojobtime == 2)
			{
				LED_R_TOGGLE();
			}
			//熄灭
			else
			{
				LED_R_OFF();
			}
		}		
	}
	
	/* 定时器6中断 IMU融合计算 */
	if (htim->Instance == TIM6)
	{
		float roll_rads=0, pitch_rads=0, yaw_rads=0;
		
		/* 更新测量值 */
		IMU_ConvAccMg2g(&g_IMUVal, &g_IMUValStand);
		IMU_ConvGyoMdps2rds(&g_IMUVal, &g_IMUValStand);
		madgwick_update_imu(&fusion.madgwick,
							g_IMUValStand.GyoX, g_IMUValStand.GyoY, g_IMUValStand.GyoZ,
							-g_IMUValStand.AccX, -g_IMUValStand.AccY, -g_IMUValStand.AccZ);
		fusion.yaw = madgwick_get_yaw(&fusion.madgwick);
		g_IMUValResult.Acc.X = g_IMUValStand.AccX;
		g_IMUValResult.Acc.Y = g_IMUValStand.AccY;
		g_IMUValResult.Acc.Z = g_IMUValStand.AccZ;
		
		/* 获取欧拉角 */
		madgwick_get_euler(&fusion.madgwick, &roll_rads, &pitch_rads, &yaw_rads);
		g_IMUValResult.Euler.Roll = roll_rads * 57.29578f;
		g_IMUValResult.Euler.Pitch = pitch_rads * 57.29578f;
		g_IMUValResult.Euler.Yaw = yaw_rads * 57.29578f;
	}
}

/**
 * @brief	Tim2初始化 定时10us
 *
 * @param	void
 *
 * @return  void
 */
void InitTimer2(void)
{
	__HAL_RCC_TIM2_CLK_ENABLE();
	
	TIM2_Handler.Instance 				= TIM2;
	TIM2_Handler.Init.Prescaler         = 720-1;
	TIM2_Handler.Init.Period            = 0xFFFFFFFF;	
	TIM2_Handler.Init.ClockDivision     = TIM_ETRPRESCALER_DIV1;
	TIM2_Handler.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TIM2_Handler.Init.RepetitionCounter = 0;
	TIM2_Handler.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(&TIM2_Handler);
	HAL_TIM_Base_Start(&TIM2_Handler);
}

/**
* @brief	Tim2初始化 定时10us
*
* @param	void
*
* @return  void
* -----------------------------------------------------------------------
  TIM1 Configuration: generate 4 PWM signals with 4 different duty cycles.

    In this example TIM1 input clock (TIM1CLK) is set to APB2 clock (PCLK2),
    since APB2 prescaler is equal to 1.
      TIM1CLK = PCLK2
      PCLK2 = HCLK
      => TIM1CLK = HCLK = SystemCoreClock

    To get TIM1 counter clock at 24 MHz, the prescaler is computed as follows:
       Prescaler = (TIM1CLK / TIM1 counter clock) - 1
       Prescaler = ((SystemCoreClock) /24 MHz) - 1

    To get TIM1 output clock at 24 KHz, the period (ARR)) is computed as follows:
       ARR = (TIM1 counter clock / TIM1 output clock) - 1
           = 999

    TIM1 Channel1 duty cycle = (TIM1_CCR1/ TIM1_ARR + 1)* 100 = 50%
    TIM1 Channel2 duty cycle = (TIM1_CCR2/ TIM1_ARR + 1)* 100 = 37.5%
    TIM1 Channel3 duty cycle = (TIM1_CCR3/ TIM1_ARR + 1)* 100 = 25%
    TIM1 Channel4 duty cycle = (TIM1_CCR4/ TIM1_ARR + 1)* 100 = 12.5%

    Note:
     SystemCoreClock variable holds HCLK frequency and is defined in system_stm32l4xx.c file.
     Each time the core clock (HCLK) changes, user had to update SystemCoreClock
     variable value. Otherwise, any configuration based on this variable will be incorrect.
     This variable is updated in three ways:
      1) by calling CMSIS function SystemCoreClockUpdate()
      2) by calling HAL API function HAL_RCC_GetSysClockFreq()
      3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
  ----------------------------------------------------------------------- 
*/
void InitTimer1PWM(uint32_t _ulFreq, uint32_t _ulDutyCycle)
{
	
	uint16_t usPeriod;
//	uint16_t usPrescaler;
	uint32_t pulse;
//	uint32_t uiTIMxCLK;
	
	usPeriod = _ulFreq  - 1;	/* 自动重装的值 */
	
	pulse = usPeriod*_ulDutyCycle/100;
	
	__HAL_RCC_TIM1_CLK_ENABLE();
	
	TIM1_Handler.Instance 				= TIM1;
	TIM1_Handler.Init.Prescaler         = SystemCoreClock/1000-1;
	TIM1_Handler.Init.Period            = usPeriod;	
	TIM1_Handler.Init.ClockDivision     = 0;
	TIM1_Handler.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TIM1_Handler.Init.RepetitionCounter = 0;
	HAL_TIM_PWM_Init(&TIM1_Handler);
	
	/*##-2- Configure the PWM channels #########################################*/
	/* Common configuration for all channels */
	sConfig.OCMode       = TIM_OCMODE_PWM1;
	sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
	sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;

	/* Set the pulse value for channel 2 */
	sConfig.Pulse = pulse;
	if (HAL_TIM_PWM_ConfigChannel(&TIM1_Handler, &sConfig, TIM_CHANNEL_3) != HAL_OK)
	{
		/* Configuration Error */
		Error_Handler();
	}
	
	/* Start channel 2 */
	if (HAL_TIM_PWM_Start(&TIM1_Handler, TIM_CHANNEL_3) != HAL_OK)
	{
		/* PWM Generation Error */
		Error_Handler();
	}

}


/**
  * @brief TIM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
//	GPIO_InitTypeDef   GPIO_InitStruct;
	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* TIMx Peripheral clock enable */
	__HAL_RCC_TIM1_CLK_ENABLE();

	/* Enable all GPIO Channels Clock requested */
	LED_R_GPIO_CLK_ENABLE();

	/* Configure PA.08 (connected to D9 (pin 12 in CN3 connector)) (TIM1_Channel1), PA.09 (connected to D1 (pin 1 in CN3 connector)) (TIM1_Channel2), PA.10 (connected to D0 (pin 2 in CN3 connector)) (TIM1_Channel3),
	PA.11 (connected to D10 (pin 13 in CN3 connector)) (TIM1_Channel4) in output, push-pull, alternate function mode
	*/
	/* Common configuration for all channels */
	/*
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
	GPIO_InitStruct.Pin = LED_R_PIN;
	HAL_GPIO_Init(LED_R_GPIO_PORT, &GPIO_InitStruct);
	*/
}

/**
 * @brief	判断定时时间
 *
 * @param	Timerval 定时器开始
 *
 * @return  void
 */
bool JudgeSomeDelayInProOk(uint32_t Timerval,uint32_t DelayN)
{
	bool ret;
	uint32_t xx,tt;

	ret = false;
	xx = TIM2->CNT;
	
	if(Timerval <= xx)	
		tt = xx - Timerval;	
	else
		tt = 0xffffffff - Timerval + xx;
	if( tt  > DelayN )
	{
		ret = true;		
	}		
	return ret;
}

bool JudgeTick(uint32_t Timerval,uint32_t DelayN)
{
	bool ret;
	uint32_t xx,tt;

	ret = false;
	xx = uwTick;
	
	if(Timerval <= xx)	
		tt = xx - Timerval;	
	else
		tt = 0xffffffff - Timerval + xx;
	if( tt  > DelayN )
	{
		ret = true;		
	}		
	return ret;
}


TimeComparison JudgTimeOrder(DATETIME time1,DATETIME time2,uint32_t LapseTime)
{
	uint32_t time_t1,time_t2 ;
	time_t1 = CalHowManyMinuteFromNow(time1);
	time_t2 = CalHowManyMinuteFromNow(time2);
	time_t2 += LapseTime;
	if(time_t1 <time_t2)
	{
		return BEFORE; 
	}
	else if(time_t1 > time_t2)
	{
		return AFTER;
	}
	else
	{
		return SAME;  
	}	
}



// 获取月份天数函数
uint8_t getMonthDays(uint16_t year, uint8_t month)
{
    uint8_t days;
    uint8_t leap = 0;
    
    // 判断闰年
    if(((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
        leap = 1;
    
    switch(month)
    {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            days = 31;
            break;
        case 4: case 6: case 9: case 11:
            days = 30;
            break;
        case 2:
            days = leap ? 29 : 28;
            break;
        default:
            days = 31;
    }
    return days;
}

// 时间转换函数
void convertToUTC8(DATETIME *time)
{
    time->hour += 8;
    
    if(time->hour >= 24)
    {
        time->hour -= 24;
        time->date += 1;
        
        if(time->date > getMonthDays((time->year+2000), time->month))
        {
            time->date = 1;
            time->month += 1;
            
            if(time->month > 12)
            {
                time->month = 1;
                time->year += 1;
                // 如果年份超过99，回绕到0（根据您的两位数年份格式）
                if(time->year > 99)
                    time->year = 0;				
            }
        }
    }
}
