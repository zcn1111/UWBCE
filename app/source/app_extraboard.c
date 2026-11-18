
#include "bsp.h"
#include "main.h"







/****************************************************************************
函数名称: void BTSendAT(char* s)
功能描述: 
参数输入: 
参数输出: 
返  回  值: 无
其      他: 无
*****************************************************************************/
void TestSend(void)
{
	uint8_t i;
	
	for(i=0;i<16;i++)
		g_TxBuf2[i] = i;
	UartSend(&g_Uart2,g_TxBuf2,16);		
}

/******************************************************************
函数名称: void BluetoothPro(void)
功能描述: 
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
uint8_t testuart2;

void TestUart2(void)
{
	uint16_t len;
	
	if(g_Uart2.usRxCount > 0 && JudgeSomeDelayInProOk(g_Uart2RxTimer,Delay50mSec))
	{
		memset(g_tmpBuf,0,sizeof(g_tmpBuf));
		
		if(g_Uart2.usRxCount <= UART2_RX_BUF_SIZE)
		{
			len = g_Uart2.usRxCount;
			ReadRecBuf(&g_Uart2,g_tmpBuf,g_Uart2.usRxCount);	
			g_Usart2RxSta = 1;			
		}
		else
		{
			ReadRecBuf(&g_Uart2,g_tmpBuf,UART2_RX_BUF_SIZE);
			g_Uart2.usRxCount = 0;
			g_Uart2.usRxRead = 0;
			g_Uart2.usRxWrite = 0;
			memset(g_RxBuf2,0,UART2_RX_BUF_SIZE);
			len = UART2_RX_BUF_SIZE;
			g_Usart2RxSta = 1;
		}
		
	}
	if(g_Usart2RxSta == 1)
	{
		g_Usart2RxSta = 0;
	}
	
	if(testuart2 == 1)
	{
		testuart2 = 0;
		TestSend();
		Get_Adc_Average(ADC_CHANNEL_6,10);
	}
}



