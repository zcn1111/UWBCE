#include "bsp.h"
#include "app_glovar.h"

uint32_t g_KeyDelay;           //???????????
uint8_t  g_Key1Sta;            //????1??
volatile uint8_t  flag_acc_irq = 0;     //??????
uint8_t  g_ManualPunFlag = 0;
uint8_t PowerKey_flag;
uint32_t g_PowerKeyDelay;
uint8_t g_FnKeyNum = 0;
uint8_t g_KeySelfCheck = 0;

void Key_Init(void)
{
	g_Key1Sta = 0;
	PowerKey_flag = 0;
	g_PowerKeyDelay = TIM2->CNT;
}

/**
 * @brief	CheckToolInit ?????????????
 *
 * @param	void
 *
 * @return  void
 */
void InitKeyGpio(void)
{
	//GPIO???????
	GPIO_InitTypeDef GPIO_Initure;	
	/* ??GPIO??? */
	KEY_GPIO_CLK_ENABLE();
	
	/* ????????????? */
	GPIO_Initure.Pin = KEY1_PIN;
	GPIO_Initure.Mode = GPIO_MODE_INPUT;			
	GPIO_Initure.Pull = GPIO_NOPULL;					/* ?????????�M??? */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;			/* GPIO????? */
	HAL_GPIO_Init(KEY_GPIO_PORT, &GPIO_Initure);	
	
	GPIO_Initure.Pin = KEY2_PIN;
	GPIO_Initure.Mode = GPIO_MODE_INPUT;			
	GPIO_Initure.Pull = GPIO_PULLUP;					/* ?????????�M??? */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;			/* GPIO????? */
	HAL_GPIO_Init(KEY_GPIO_PORT, &GPIO_Initure);	
	
	Key_Init();
}

void Read_Flag_Key(void)
{ 
	if ((READ_KEY1_STA() == 1) && JudgeSomeDelayInProOk(g_KeyDelay,Delay1Sec) == true)
	{
		if (g_Key1Sta == 1)
		{
			//if (g_FnKeyNum == 0)
			//{
			//	g_KeySelfCheck = 1;
			//}
		}
		
		g_Key1Sta = 0;
		
		if (g_FnKeyNum < 5)
		{
			g_FnKeyNum = 0;
		}
	}
	else if(g_Key1Sta == 1) 
    {
		if(READ_KEY1_STA() == 1) 
        {
            if(JudgeSomeDelayInProOk(g_KeyDelay,Delay1Sec) == false) 
            {
				g_KeySelfCheck = 0;
                g_Key1Sta = 2;
                g_KeyDelay = TIM2->CNT;
            }
        }	
    }
    else if(g_Key1Sta == 2) 
    {
		g_KeySelfCheck = 1;
        if((READ_KEY1_STA() == 0) && (JudgeSomeDelayInProOk(g_KeyDelay,Delay1Sec) == false))
        {
			g_KeySelfCheck = 0;
			g_FnKeyNum++;
			g_KeyDelay = TIM2->CNT;
			g_Key1Sta = 0;                                          				
        }
    } 
    else if((READ_KEY1_STA() == 0) && (g_Key1Sta == 0)) 
    {
        g_KeyDelay = TIM2->CNT;
        g_Key1Sta = 1;
    }
	
	if (g_FnKeyNum >= 5)
	{
		g_KeySelfCheck = 0;
		
		if(SWITCHTYPE == 0 || (SWITCHTYPE == 1 && g_SelfCheck.StartCharge == 1 ))
			SWITCHTYPE = 2;
		else if(SWITCHTYPE == 2)
			SWITCHTYPE = 0;		
		CheckToolInit();
		Uart1VarInit();
		InitUSART1();
		HAL_UART_MspInit(&UART1_Handler);
		g_FnKeyNum = 0;
		g_SelfCheck.Mode485 = 0;
		LED_R_OFF();		
	}
	
	#if 0
	//???????
	if(g_Key1Sta == 1) 
    {//??????????��???
        flag_acc_irq = 1;  //????????????????????????????????????        		
		if(READ_KEY1_STA() == 1) 
        { //?????????????
            if(JudgeSomeDelayInProOk(g_KeyDelay,Delay300mSec) == false) 
            {//300ms??????????????????��????????
				if (g_VoiceSwitch.num == 1)
				{
					g_VoiceSwitch.num = 0;
				}
				else
				{
					g_VoiceSwitch.num = 1;
				}
				WriteVoiceSwitchToFLASH();
				
                g_Key1Sta = 2;
                g_KeyDelay = TIM2->CNT;
            }
            if(JudgeSomeDelayInProOk(g_KeyDelay,Delay3Sec) == true )
			{	//????3s???
				if (g_VoiceSwitch.num == 1)
				{
					g_VoiceSwitch.num = 0;
				}
				else
				{
					g_VoiceSwitch.num = 1;
				}
				WriteVoiceSwitchToFLASH();
				g_Key1Sta = 0;                         
			}
        }
		if(JudgeSomeDelayInProOk(g_KeyDelay,Delay30Sec) == true )
		{
			g_Key1Sta = 0;  
		}		
    }
    else if(g_Key1Sta == 2) 
    {//?��????????
        if((READ_KEY1_STA() == 0) && (JudgeSomeDelayInProOk(g_KeyDelay,Delay300mSec) == false) && (SWITCHTYPE != 0x01)) //????300ms???????????��?????
        {
			g_KeyDelay = TIM2->CNT;
			if(SWITCHTYPE == 0)
				SWITCHTYPE = 2;
			else if(SWITCHTYPE == 2)
				SWITCHTYPE = 0;		
			Uart1VarInit();
			InitUSART1();
			HAL_UART_MspInit(&UART1_Handler);
			g_Key1Sta = 0;                                              /*?��????????????????*/					
        }
        if((READ_KEY1_STA() == 1)&&JudgeSomeDelayInProOk(g_KeyDelay,Delay300mSec) == true) 
        {//?????��???????	
            g_Key1Sta = 0;
        }
    } 
    else if((READ_KEY1_STA() == 0) && (g_Key1Sta == 0))  //????????��?????
    {
        g_KeyDelay = TIM2->CNT;
        g_Key1Sta = 1;
    }
	#endif

	#if 0
	//???????
	if(PowerKey_flag == 1) 
    {//??????????��???        		
		if(READ_KEY2_STA() == 1) 
        { //?????????????
            if(JudgeSomeDelayInProOk(g_PowerKeyDelay,Delay300mSec) == false) 
            {//300ms??????????????????��????????
                PowerKey_flag = 2;
                g_PowerKeyDelay = TIM2->CNT;
            }
            if(JudgeSomeDelayInProOk(g_PowerKeyDelay,Delay3Sec) == true)
			{//????3s???
				PowerKey_flag = 0;                         
			}
        }
		if(JudgeSomeDelayInProOk(g_PowerKeyDelay,Delay5Sec) == true)
		{
			PowerKey_flag = 0;  
		}		
    }
    else if(PowerKey_flag == 2) 
    {//?��????????
        if(READ_KEY2_STA() == 0) //????300ms???????????��?????
        {
			if (g_VoiceSwitch.num == 1)
			{
				g_VoiceSwitch.num = 0;
			}
			else
			{
				g_VoiceSwitch.num = 1;
			}
			WriteVoiceSwitchToFLASH();
			
			PowerKey_flag = 0;                                     				
        }
		
		if (JudgeSomeDelayInProOk(g_PowerKeyDelay,Delay500mSec) == true)
		{
			PowerKey_flag = 0;
		}
    } 
    else if((READ_KEY2_STA() == 0) && (PowerKey_flag == 0))  //????????��?????
    {
        g_PowerKeyDelay = TIM2->CNT;
        PowerKey_flag = 1;
    }
	#endif
	
	#if 1
	//???????
	if(PowerKey_flag == 1)
	{
		if(READ_KEY2_STA() == 1)
		{
			g_PowerKeyDelay = TIM2->CNT;
			PowerKey_flag = 0;			
		}
		else if(READ_KEY2_STA() == 0 && JudgeSomeDelayInProOk(g_PowerKeyDelay,Delay1Sec) == true)
		{
			//PWR_SYS_DISABLE();
		}	
	}
	else if((PowerKey_flag == 0) && (READ_KEY2_STA() == 0))   /*?��?????��???????*/
	{
		PowerKey_flag = 1;
		g_PowerKeyDelay = TIM2->CNT;
	}
	else if((PowerKey_flag == 2) && JudgeSomeDelayInProOk(g_PowerKeyDelay,Delay5Sec) == true)   /*?��?????��???????*/
	{
		PowerKey_flag = 0;
	}
	#endif
}
