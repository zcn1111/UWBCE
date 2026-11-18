
#include "bsp.h"
#include "main.h"
uint32_t g_SleepDelay;


void stm32_enter_sleep(void)
{
//	//休眠前的disable操作
//    if(USE_CW2015 == 1)
//        cw2015_sleep();
//	HAL_UART_DeInit(&huart1);
//	HAL_UART_DeInit(&huart2);
//	HAL_I2C_DeInit(&hi2c2);
//    HAL_SPI_DeInit(&hspi1);
//	HAL_SPI_DeInit(&hspi2);
//	HAL_TIM_Base_Stop(&htim1);//关闭定时器1
//	HAL_TIM_Base_Stop(&htim6);//关闭定时器6	
//	led_off(LED_ALL);	
//    PWR_4G_DISABLE();
//    PWR_RTK_DISABLE();	
//	
//	HAL_DBGMCU_EnableDBGStopMode();		
//	SysTick->CTRL = 0x00;			//关闭滴答定时器
//	SysTick->VAL = 0x00;			//清空		
//	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);//休眠
////	EXTI->IMR1 &= ~(ACC_GPIO_PIN);

//	__HAL_RCC_PWR_CLK_ENABLE();
//	__HAL_RCC_GPIOA_CLK_ENABLE();
//	__HAL_RCC_GPIOB_CLK_ENABLE();
//	__HAL_RCC_GPIOC_CLK_ENABLE();
//	__HAL_RCC_GPIOD_CLK_ENABLE();   
//	SystemClock_Config();
//	delay_init(64);		
//	MX_GPIO_Init();
//  MX_GPIO_Init();
//  MX_DMA_Init();
//  MX_I2C2_Init();
//  MX_SPI1_Init();
//  MX_SPI2_Init();
//  MX_TIM1_Init();
//  UartVarInit();
//  MX_USART1_UART_Init();
//  MX_ADC1_Init();
//  MX_USART2_UART_Init();
//  MX_TIM6_Init();
//  /* USER CODE BEGIN 2 */  
//  MX_NVIC_Init();
//  W25qxx_Init();
//  LYInit();
//  Key_Init(); 
//  e2prom_Init();            /*³õÊ¼»¯EEPROM*/
//  Read_ID_EEPROM();         /*¶ÁÈ¡¿¨Æ¬Éí·Ý*/
////  lis2dh12_init();          /*³õÊ¼»¯¼ÓËÙ¶È´«¸ÐÆ÷*/
//  USE_CW2015 = cw2015_init();
////  Ebyte_RF.Init();
////  Ebyte_RF.EnterReceiveMode( 0 );  
//  Read_Battery();
//  Get_Adc_Average(20);         
//  Led_Init();
////e2prom_data_write[0] = 0x01;
////e2prom_data_write[1] = 0x01;
////e2prom_data_write[2] = 0x01;
////e2prom_data_write[3] = 0x01;
////e2prom_data_write[4] = 0x01;
////e2prom_data_write[5] = 0x00;
////e2prom_data_write[6] = 0x04;

////E2prom_Write(EEPROM_ID_ADDR, e2prom_data_write, EEP_UNIT_SIZE); 
//  if(USE_CW2015 == 1)
//  {
//	if(battery < 50)
//		SendYYWarn(KJZSBDLD);
//	else
//		SendYYWarn(KJZSBZC);  
//  }
//  else
//	  SendYYWarn(KJZSBYC);
//	  
////  Ebyte_RF.EnterReceiveMode(0);

}


void no_move_to_sleep(void)
{
    if(flag_acc_irq == 1)  //产生运动中断
    {
        flag_acc_irq = 0;
		g_SleepDelay = TIM2->CNT;
    }
    else
    {
        if(JudgeSomeDelayInProOk(g_SleepDelay ,Delay2Min) == true)
        {
            g_SleepDelay = TIM2->CNT;
			stm32_enter_sleep();
        }
    }
}






