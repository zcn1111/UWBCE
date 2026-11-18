
#include "stm32l4xx.h"
#include "bsp_adc.h"
#include "bsp_timer.h"
#include "app_glovar.h"

uint8_t g_BatVal = 0;
uint8_t g_BatVal1 = 0;
ADC_HandleTypeDef hadc1;//ADC句柄
ADC_HandleTypeDef hadc2;//ADC句柄
uint32_t   g_SampADCDelay;
/**
 * @brief	初始化ADC函数
 *
 * @param   void
 *
 * @return  void
 */
void ADCInit(void)
{
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4; //1分频，ADCCLK=SYSCLK=24MHZ	
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;           //12位模式
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;           //右对齐
    hadc1.Init.ScanConvMode = DISABLE;                    //非扫描模式
    hadc1.Init.EOCSelection = DISABLE;                    //关闭EOC中断
    hadc1.Init.ContinuousConvMode = DISABLE;              //关闭连续转换
    hadc1.Init.NbrOfConversion = 1;                       //1个转换在规则序列中 也就是只转换规则序列1
    hadc1.Init.DiscontinuousConvMode = DISABLE;           //禁止不连续采样模式
    hadc1.Init.NbrOfDiscConversion = 0;                   //不连续采样通道数为0
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;     //软件触发
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; //使用软件触发

    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode = DISABLE;

    hadc1.Init.DMAContinuousRequests = DISABLE;           //关闭DMA请求
    HAL_ADC_Init(&hadc1);                                 //初始化
}


/**
 * @brief	ADC底层驱动，引脚配置，时钟使能，此函数会被HAL_ADC_Init()调用
 *
 * @param   hadc	ADC句柄
 *
 * @return  void
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInit;
    GPIO_InitTypeDef GPIO_Initure;

    //选择ADC时钟源为SYSCLK(24Mhz)
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	__HAL_RCC_ADC_CLK_ENABLE();            //使能ADC1时钟
	__HAL_RCC_GPIOA_CLK_ENABLE();			//开启GPIOC时钟
	
	GPIO_Initure.Pin = GPIO_PIN_1;          //PA1
	GPIO_Initure.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;   //模拟
	GPIO_Initure.Pull = GPIO_NOPULL;        //不带上下拉
	HAL_GPIO_Init(GPIOA, &GPIO_Initure);
}

/**
 * @brief	获得ADC值
 *
 * @param   ch		通道值 0~16，取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_16
 *
 * @return  uint16_t		转换结果
 */
uint16_t GetAdc(uint32_t ch)
{
    ADC_ChannelConfTypeDef ADC1_ChanConf;

    ADC1_ChanConf.Channel = ch;                                 //通道
    ADC1_ChanConf.Rank = ADC_REGULAR_RANK_1;					//第1个序列，序列1
    ADC1_ChanConf.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;      //采样时间
    ADC1_ChanConf.SingleDiff = ADC_SINGLE_ENDED;				//ADC单端输入
    ADC1_ChanConf.OffsetNumber = ADC_OFFSET_NONE;				//偏移号选择
    ADC1_ChanConf.Offset = 0;
    HAL_ADC_ConfigChannel(&hadc1, &ADC1_ChanConf);       //通道配置
	HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
    HAL_ADC_Start(&hadc1);                               //开启ADC

    HAL_ADC_PollForConversion(&hadc1, 10);               //轮询转换

    return (uint16_t)HAL_ADC_GetValue(&hadc1);	        	//返回最近一次ADC1规则组的转换结果
}

/**
 * @brief	获取指定通道的转换值，取times次,然后平均
 *
 * @param   ch		通道值 0~16，取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_16
 * @param   times	获取次数
 *
 * @return  uint16_t		通道ch的times次转换结果平均值
 */
uint16_t Get_Adc_Average(uint32_t ch, uint8_t times)
{
    uint32_t temp_val = 0;
	
	float temp,temp1;
    uint8_t t;
	
	if(ch == ADC_CHANNEL_4)
	{
		delay_ms(5);
		for(t = 0; t < times; t++)
		{
			temp_val += GetAdc(ch);
			delay_ms(5);
		}
		temp_val = temp_val/times;
		temp = (float)temp_val*(3.3/4096)*2*10;
		g_BatVal1 = temp;
		
		return g_BatVal1;
	}
	else if(ch == ADC_CHANNEL_6)
	{
		for(t = 0; t < times; t++)
		{
			temp_val += GetAdc(ch);
			delay_ms(5);
		}
		temp1 = (float)temp_val/times;
		temp = (float)temp1 / 4096 * 3.3 * 28.0 / 18 * 10;
		g_BatVal = temp;
		//if (g_BatVal >= 40)
		//{
		//	g_BatVal = 40;
		//}
		
		return g_BatVal;
	}
	else if (ch == ADC_CHANNEL_16)
	{
		for(t = 0; t < times; t++)
		{
			temp_val += GetAdc(ch);
			delay_ms(5);
		}
		
		return (temp_val/times);
	}
	else
		return 0;
}