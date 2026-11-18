#include "bsp.h"
#include "main.h"

uint8_t g_ResetAlarm;	//重置报警
uint8_t g_SensorStatus;	//传感器状态
uint8_t g_AlarmFlag;	//报警标志
uint32_t g_RecSensorStatusDelay;
uint32_t g_RecAlarmDelay;

/******************************************************************
函数名称: void SendACK(uint8_t flag, uint16_t no)
功能描述: 接收板向发送板回复信息
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void SendResetCmd(void)
{
	memset(g_TxBuf1,0,sizeof(g_TxBuf1));


}

/******************************************************************
函数名称: void SensorProcess(void)
功能描述: 与传感器主板通信处理
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void SensorProcess(void)
{
	if(g_Uart1.usRxCount > 0 && (JudgeSomeDelayInProOk(g_Uart1RxTimer,Delay1mSec) == true))
	{
		memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
				
		if(g_Uart1.usRxCount > UART1_RX_BUF_SIZE)
		{
			g_Uart1.usRxCount = UART1_RX_BUF_SIZE;
		}
		ReadRecBuf(&g_Uart1,g_RxBuf1,g_Uart1.usRxCount);
		g_Usart1RxSta = 1;
		g_Uart1.usRxCount = 0;
		g_Uart1.usRxRead = 0;
		g_Uart1.usRxWrite = 0;
		memset(g_DMARxBuf1,0,UART1_RX_BUF_SIZE);
	}
	
	if(g_Usart1RxSta == 1)
	{
		g_Usart1RxSta = 0;
		switch(g_RxBuf1[1])
		{
			case 1:
				if(g_RxBuf1[0] == 0xE1 && g_RxBuf1[7] == CheckSum(7,g_RxBuf1))
				{
					g_SensorStatus = g_RxBuf1[2];
					g_RecSensorStatusDelay = TIM2->CNT;
				}
				break;
			case 2:
				if(g_RxBuf1[0] == 0xE1 && g_RxBuf1[31] == CheckSum(31,g_RxBuf1))
				{

				}
				break;
			default:
				break;
		}
		
	}
	
	if(JudgeSomeDelayInProOk(g_Uart1.usTxTimer,Delay100mSec) == true && g_SensorBoardPowerOn == 1)
	{
		if(g_ResetAlarm == 1)
		{
			g_ResetAlarm = 0;
			SendResetCmd();
		}
	}
}

