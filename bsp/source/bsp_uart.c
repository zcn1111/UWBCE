#include "stm32l4xx.h"
#include "bsp.h"
#include "4G_Comm.h"
#include "app_cuart.h"
#include "app_ly.h"
#include "app_gps.h"

#define NMEA_MAX_LINE        256

UART_HandleTypeDef UART1_Handler; 		//UART句柄
DMA_HandleTypeDef  UART1RxDMA_Handler; 	//DMA句柄
UART_HandleTypeDef UART2_Handler; 		//UART句柄
DMA_HandleTypeDef UART2RxDMA_Handler;
UART_HandleTypeDef UART3_Handler; 		//UART句柄
DMA_HandleTypeDef  UART3RxDMA_Handler; 	//DMA句柄
UART_HandleTypeDef LPUART1_Handler; 	//UART句柄
DMA_HandleTypeDef  LPUART1RxDMA_Handler;//DMA句柄

uint8_t g_Usart1RxSta=0;  				//Usart1接收到的数据状态 0,没有接收到数据;1,接收到了一批数据.
uint8_t g_Usart2RxSta=0;  				//Usart1接收到的数据状态 0,没有接收到数据;1,接收到了一批数据.
uint8_t g_Usart3RxSta=0;  				//Usart1接收到的数据状态 0,没有接收到数据;1,接收到了一批数据.
uint8_t g_LPUart1RxSta=0;  				//LPUART接收到的数据状态 0,没有接收到数据;1,接收到了一批数据.

UART_T g_Uart1;
uint16_t g_TxBuf1[UART1_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_SecTxBuf1[UART1_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];		/* 接收缓冲区 */
uint8_t g_DMARxBuf1[UART1_RX_BUF_SIZE];		/* 接收缓冲区 */
uint32_t g_Uart1RxTimer;
uint8_t g_Uart1DirFlag;						//485方向标记 1-接收 2-发送

UART_T g_Uart2;
//uint8_t g_TxBuf2[UART2_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];		/* 接收缓冲区 */
uint8_t g_DMARxBuf2[UART2_RX_BUF_SIZE];		/* 接收缓冲区 */
uint32_t g_Uart2RxTimer;

UART_T g_Uart3;
//uint8_t g_TxBuf3[UART3_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBuf3[UART3_RX_BUF_SIZE];		/* 接收缓冲区 */
uint8_t g_DMARxBuf3[UART3_RX_BUF_SIZE];		/* 接收缓冲区 */
uint32_t g_Uart3RxTimer;
RingBufPos_t	g_Uart3RxRing;		//UART3环形缓存参数

UART_T g_LPUart;
//uint8_t g_TxBufLP[LPUART_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBufLP[LPUART_RX_BUF_SIZE];		/* 接收缓冲区 */
uint8_t g_DMARxBufLP[LPUART_RX_BUF_SIZE];		/* 接收缓冲区 */
uint32_t g_LPUartRxTimer;
uint8_t g_LPUart1DirFlag;						//485方向标记 1-接收 2-发送

//串口1重定向
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&UART1_Handler, (uint8_t *)&ch, 1, 100);
//	HAL_UART_Transmit(&hUART5, (uint8_t *)&ch, 1, 100);
	return ch;
}

int fgetc(FILE *f)
{
	uint8_t ch = 0;
	HAL_UART_Receive(&UART1_Handler, &ch, 1, 100);
//	HAL_UART_Receive(&hUART5, &ch, 1, 100);
	return ch;
}


/*环形BUFF配置函数
*/
/******************************************************************
函数名称: void InitRingBuf
功能描述: 环形队列初始化
参数输入: uint8_t		*Buf	队列缓存
		  uint16_t		Size	队列大小
参数输出: RingBufPos_t	*Ring	环形队列位置
返 回 值: 
其    他: 无
******************************************************************/
void InitRingBuf(RingBufPos_t *Ring, uint16_t Size, uint8_t *Buf)
{
	Ring->Head = 0;
	Ring->Tail = 0;
	Ring->Size = Size;
	if (Buf == NULL)
	{
		Ring->Buf = malloc(Size);		//申请内存
	}
	else
	{
		Ring->Buf = Buf;				//配置内存
	}
	memset(Ring->Buf, 0x00, Size);
}
/******************************************************************
函数名称: uint16_t GetRingUsedLen
功能描述: 获取当前环形缓存已用大小
参数输入: RingBufPos_t	*Ring	环形队列位置
参数输出: 
返 回 值: uint16_t 已用大小
其    他: 无
******************************************************************/
uint16_t GetRingUsedLen(RingBufPos_t *Ring)
{
	uint16_t len;
	
	if (Ring->Head >= Ring->Tail)
	{
		len = Ring->Head - Ring->Tail;
	}
	else
	{
		len = Ring->Size + Ring->Head - Ring->Tail + 1;
	}
	
	if (len > MAX_GSM_LEN)
	{
		return 0;
	}
	
	return len;
}
/******************************************************************
函数名称: uint16_t GetRingRemainLen
功能描述: 获取当前环形缓存剩余大小
参数输入: RingBufPos_t	*Ring	环形队列位置
参数输出: 
返 回 值: uint16_t 剩余大小
其    他: 无
******************************************************************/
uint16_t GetRingRemainLen(RingBufPos_t *Ring)
{
	uint16_t len;
	
	len = Ring->Size - GetRingUsedLen(Ring);
	return len;
}
/******************************************************************
函数名称: uint8_t IsRingFull
功能描述: 环形缓存是否已满
参数输入: RingBufPos_t	*Ring	环形队列位置
参数输出: 
返 回 值: uint8_t 0=未满;1=已满
其    他: 无
******************************************************************/
uint8_t IsRingFull(RingBufPos_t *Ring)
{
	if (Ring->Head == (Ring->Size - 1))
	{
		if (Ring->Tail == 0)
			return 1;
		else
			return 0;
	}
	else if (Ring->Tail == (Ring->Head + 1))
		return 1;
	else
		return 0;
}
/******************************************************************
函数名称: uint8_t IsRingEmpty
功能描述: 环形缓存是否空
参数输入: RingBufPos_t	*Ring	环形队列位置
参数输出: 
返 回 值: uint8_t 0=非空;1=空
其    他: 无
******************************************************************/
uint8_t IsRingEmpty(RingBufPos_t *Ring)
{
	if (Ring->Head == Ring->Tail)
		return 1;
	else
		return 0;
}

/******************************************************************
函数名称: uint16_t RingBufPush
功能描述: 入队列
参数输入: uint8_t		*InBuf		输入缓存
		  uint16_t		Size		缓存大小
		  uint8_t		PushMode	队列模式 0=队列满后停止;1=队列满后放弃旧数据
参数输出: RingBufPos_t	*Ring		环形缓存变量
返 回 值: uint16_t 入队数据大小 0=无数据
其    他: 无
******************************************************************/
uint16_t RingBufPush(uint8_t *InBuf, uint16_t Size, RingBufPos_t *Ring, uint8_t PushMode)
{
	uint16_t remainlen=0;
	uint16_t i;
	
	uint8_t full=0;
	uint8_t willfull=0;
		
	/* 判断队列已满 */
	if (IsRingFull(Ring) == 1)
		full = 1;
	
	/* 判断入队长度 如果不足本次数据入队 置队列即将满 */
	remainlen = GetRingRemainLen(Ring);
	if (Size > remainlen)
		willfull = 1;
	
	/* 数据入队 */
	if (PushMode == RING_PUSHMODE_STOP)			//队列满后停止模式
	{
		if ((full) || (willfull)) return 0;		//队列满或将满后停止入队
	}
	for (i=0; i<Size; i++)
	{
		Ring->Buf[Ring->Head] = InBuf[i];
		Ring->Head++;
		if (IsRingFull(Ring) == 1)
		{
			if (Ring->Head > (Ring->Size - 1))
			{
				Ring->Head = 0;
				Ring->Tail = Ring->Size - 1;
			}
			else
			{
				Ring->Tail = Ring->Head + 1;
			}
		}
		else if (Ring->Head > (Ring->Size - 1)) Ring->Head = 0;
	}
	
	return Size;
}
uint16_t RingBufPop(RingBufPos_t *Ring, uint8_t *OutBuf, uint16_t Size)
{
	uint16_t len=0;
	uint16_t i;
	
	/* 判断队列空 */
	if (IsRingEmpty(Ring) == 1)
		return 0;
	
	for (i=0; i<Size; i++)
	{
		OutBuf[i] = Ring->Buf[Ring->Tail];
		Ring->Tail++;
		if (IsRingEmpty(Ring) == 1)
			break;
		if (Ring->Tail > (Ring->Size - 1)) Ring->Tail = 0;
	}
	
	len = (i <= Size) ? i : Size;
	return len;
}
/******************************************************************
函数名称: uint16_t RingBufSeek
功能描述: 获取数据但不出队列
参数输入: RingBufPos_t	*Ring	环形缓存变量
参数输出: uint8_t		*OutBuf	输出缓存
		  uint16_t		Size	获取长度
返 回 值: uint16_t 获取数据大小 0=无数据
其    他: 无
******************************************************************/
uint16_t RingBufSeek(RingBufPos_t *Ring, uint8_t *OutBuf, uint16_t Size)
{
	uint16_t len=0;
	uint16_t i;
	uint16_t head;
	uint16_t tail;
	
	/* 判断队列空 */
	if (IsRingEmpty(Ring) == 1)
		return 0;
	
	head = Ring->Head;
	tail = Ring->Tail;
	for (i=0; i<Size; i++)
	{
		OutBuf[i] = Ring->Buf[tail];
		tail++;
		if (tail == head)		//判断是否队列已经结束
			break;
		if (tail > (Ring->Size - 1)) tail = 0;
	}
	
	len = (i <= Size) ? i : Size;
	return len;
}
/******************************************************************
函数名称: void RingBufClear
功能描述: 清除队列数据
参数输入: RingBufPos_t	*Ring	环形缓存变量
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RingBufClear(RingBufPos_t *Ring)
{
	memset(Ring->Buf, 0x00, Ring->Size);	//清缓存
	Ring->Head = 0;
	Ring->Tail = 0;
}
int IsDigit(uint8_t c) 
{ 
	return (c >= '0' && c <= '9'); 
}
void Uart1VarInit(void)
{
	//TOOL\蓝牙\CUART
	g_Uart1.uart = USART1;						/* STM32 串口设备 */
	g_Uart1.pRxBuf = g_DMARxBuf1;
	g_Uart1.usTxBufSize = UART1_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_Uart1.usRxBufSize = UART1_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_Uart1.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_Uart1.usRxRead = 0;						/* 接收FIFO读索引 */
	g_Uart1.usRxCount = 0;						/* 接收到的新数据个数 */
	g_Uart1.usTxTimer = 0;						/* 发送结束计时 */	
	if(SWITCHTYPE == 1) 
	{//TOOL
		g_Uart1.SendBefor = Tool485_SendBefor;	/* 发送数据前的回调函数 */
		g_Uart1.SendOver = Tool485_SendOver;	/* 发送完毕后的回调函数 */	
	}
	else
	{
		g_Uart1.SendBefor = 0;		            /* 发送数据前的回调函数 */
		g_Uart1.SendOver = 0;		            /* 发送完毕后的回调函数 */		
	}
	g_Uart1.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_Uart1.Sending = 0;						/* 正在发送中标志 */
}
/*
*********************************************************************************************************
*	函 数 名: UartVarInit
*	功能说明: 初始化串口参数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void UartVarInit(void)
{
	//TOOL\蓝牙\CUART
	g_Uart1.uart = USART1;						/* STM32 串口设备 */
	g_Uart1.pRxBuf = g_DMARxBuf1;
	g_Uart1.usTxBufSize = UART1_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_Uart1.usRxBufSize = UART1_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_Uart1.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_Uart1.usRxRead = 0;						/* 接收FIFO读索引 */
	g_Uart1.usRxCount = 0;						/* 接收到的新数据个数 */
	g_Uart1.usTxTimer = 0;						/* 发送结束计时 */	
	if(SWITCHTYPE == 1) 
	{//TOOL
		g_Uart1.SendBefor = Tool485_SendBefor;	/* 发送数据前的回调函数 */
		g_Uart1.SendOver = Tool485_SendOver;	/* 发送完毕后的回调函数 */	
	}
	else
	{
		g_Uart1.SendBefor = 0;		            /* 发送数据前的回调函数 */
		g_Uart1.SendOver = 0;		            /* 发送完毕后的回调函数 */		
	}
	g_Uart1.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_Uart1.Sending = 0;						/* 正在发送中标志 */
	
	//EXTRA BOARD
	g_Uart2.uart = USART2;						/* STM32 串口设备 */
	g_Uart2.pRxBuf = g_DMARxBuf2;					/* 接收缓冲区指针 */
	g_Uart2.usTxBufSize = 0;	                /* 发送缓冲区大小 */
	g_Uart2.usRxBufSize = UART2_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_Uart2.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_Uart2.usRxRead = 0;						/* 接收FIFO读索引 */
	g_Uart2.usRxCount = 0;						/* 接收到的新数据个数 */
	g_Uart2.usTxTimer = 0;						/* 发送结束计时 */
	g_Uart2.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_Uart2.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_Uart2.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_Uart2.Sending = 0;						/* 正在发送中标志 */
	
	//BLUETOOTH
	g_Uart3.uart = USART3;						/* STM32 串口设备 */
	g_Uart3.pRxBuf = g_DMARxBuf3;					/* 接收缓冲区指针 */
	g_Uart3.usTxBufSize = 0;	                /* 发送缓冲区大小 */
	g_Uart3.usRxBufSize = UART3_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_Uart3.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_Uart3.usRxRead = 0;						/* 接收FIFO读索引 */
	g_Uart3.usRxCount = 0;						/* 接收到的新数据个数 */
	g_Uart3.usTxTimer = 0;						/* 发送结束计时 */
	g_Uart3.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_Uart3.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_Uart3.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_Uart3.Sending = 0;						/* 正在发送中标志 */
	
	//CELLUAR
	g_LPUart.uart = LPUART1;					/* STM32 串口设备 */
	g_LPUart.pRxBuf = g_DMARxBufLP;				/* 接收缓冲区指针 */
	g_LPUart.usTxBufSize = 0;	                /* 发送缓冲区大小 */
	g_LPUart.usRxBufSize = LPUART_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_LPUart.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_LPUart.usRxRead = 0;						/* 接收FIFO读索引 */
	g_LPUart.usRxCount = 0;						/* 接收到的新数据个数 */
	g_LPUart.usTxTimer = 0;						/* 发送结束计时 */
	g_LPUart.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_LPUart.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_LPUart.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_LPUart.Sending = 0;						/* 正在发送中标志 */
}

/*
*********************************************************************************************************
*	函 数 名: UartSend
*	功能说明: 填写数据到UART发送缓冲区,并启动发送中断。中断处理函数发送完毕后，自动关闭发送中断
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen)
{	
	UART_HandleTypeDef UartHandle;	
		
	if (_pUart->SendBefor != 0)
	{
		_pUart->SendBefor();		/* 如果是RS485通信，可以在这个函数中将RS485设置为发送模式 */
		delay_ms(200);
	}
	if(_pUart->uart == USART1)
	{
		UartHandle =  UART1_Handler;
		g_Uart1DirFlag = 2;
	}
	else if(_pUart->uart == USART2)
	{
		UartHandle =  UART2_Handler;		
	}
	else if(_pUart->uart == USART3)
	{
		UartHandle =  UART3_Handler;
		//HAL_UART_Transmit(&UART1_Handler,_ucaBuf,_usLen,1000);
	}
	else if(_pUart->uart == LPUART1)
	{
		UartHandle =  LPUART1_Handler;
		g_LPUart1DirFlag = 2;
	}
	HAL_UART_Transmit(&UartHandle,_ucaBuf,_usLen,1000);
	
	_pUart->usTxTimer = TIM2->CNT;
	if( g_Uart1DirFlag == 2 && SWITCHTYPE == 1)
	{
		delay_ms(2);
		g_Uart1.SendOver();
		g_Uart1DirFlag = 1;
	}	
}

/*
*********************************************************************************************************
*	函 数 名: ReadRecBuf
*	功能说明: 从串口接收缓冲区读取数据 （用于主程序调用）
*	形    参: _pUart : 串口设备
*			  _ucaBuf : 存放读取数据的指针
*			  _usLen:	长度
*	返 回 值: 无
*********************************************************************************************************
*/
void ReadRecBuf(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen)
{
	uint16_t i;
	
	for (i = 0; i < _usLen; i++)
	{
		if (_pUart->usRxCount == 0)	/* 已经没有数据 */
		{
			break;
		}
		else
		{
			_ucaBuf[i] = _pUart->pRxBuf[_pUart->usRxRead];		/* 从串口接收FIFO取1个数据 */

			if (++_pUart->usRxRead >= _pUart->usRxBufSize)
			{
				_pUart->usRxRead = 0;
			}
			_pUart->usRxCount--;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: UartIRQ
*	功能说明:
*	形    参: 
*	返 回 值: 无
*********************************************************************************************************
*/
void UartIRQ(UART_T *_pUart)
{
	uint32_t isrflags   = READ_REG(_pUart->uart->ISR);
	uint32_t cr1its     = READ_REG(_pUart->uart->CR1);
	uint32_t cr3its     = READ_REG(_pUart->uart->CR3);
	uint8_t ch;
	
	/* 处理接收中断  */
	if ((isrflags & USART_ISR_RXNE) != RESET)
	{
		/* 从串口接收数据寄存器读取数据存放到接收FIFO */
		ch = READ_REG(_pUart->uart->RDR);
		_pUart->pRxBuf[_pUart->usRxWrite++] = ch;
		if (_pUart->usRxWrite >= _pUart->usRxBufSize)
		{
			_pUart->usRxWrite = 0;
		}
		if (_pUart->usRxCount < _pUart->usRxBufSize)
		{
			_pUart->usRxCount++;
		}
	}
	
	/* 清除中断标志 */
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_PEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_FEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_NEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_OREF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_IDLEF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_TCF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_LBDF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_CTSF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_RTOF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_CMF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_WUF);
	
}

void USAR_UART_IDLECallback(UART_T *_pUart)
{
	uint16_t length=0;
	
	static volatile uint16_t GpsReadIdx = 0;
	static volatile uint16_t LastPos = 0;
    uint16_t CurrentPos = 0;
    uint16_t NewLen = 0;
    uint16_t FirstCopyLen = 0;
	
	if(_pUart->uart == USART1)
	{
		HAL_UART_Abort(&UART1_Handler); 
		length = _pUart->usRxBufSize - __HAL_DMA_GET_COUNTER(&UART1RxDMA_Handler);	//计算接收到的数据长度		
		if(length > 0)
		{
			if(SWITCHTYPE==2)	//电脑
			{
				WriteMobileNumFlag = 1;	
				memset(g_RxBuf1,0,sizeof(g_RxBuf1));			
				memcpy(g_RxBuf1,_pUart->pRxBuf,length);
				_pUart->usRxCount = length;				
			}
			else if(SWITCHTYPE==0) 	//	LY
			{
				if(g_LYInitPro == 0)
					RecLYData(g_DMARxBuf1, length);
				else
				{
					memset(g_RxBuf1,0,sizeof(g_RxBuf1));			
					memcpy(g_RxBuf1,_pUart->pRxBuf,length);
					_pUart->usRxCount = length;
				}
			}
			else	//tool
			{
//				WriteMobileNumFlag = 1;	
				memset(g_RxBuf1,0,sizeof(g_RxBuf1));			
				memcpy(g_RxBuf1,_pUart->pRxBuf,length);
				_pUart->usRxCount = length;				
			}
		}	

		// 重置 DMA 传输计数器（以 USART1 接收 DMA 为例）
		// 1. 先禁用 DMA 通道
		UART1RxDMA_Handler.Instance->CCR &= ~DMA_CCR_EN;

		// 2. 重新设置传输计数器（如设置为缓冲区长度）
		UART1RxDMA_Handler.Instance->CNDTR = UART1_RX_BUF_SIZE;

		// 3. 重新使能 DMA 通道
		UART1RxDMA_Handler.Instance->CCR |= DMA_CCR_EN;
		g_Uart1RxTimer = TIM2->CNT;														//接收到了一批数据
		memset(_pUart->pRxBuf,0,_pUart->usRxBufSize);   
		HAL_UART_Receive_DMA(&UART1_Handler, _pUart->pRxBuf, _pUart->usRxBufSize);	//重启开始DMA传输 
	}
	else if(_pUart->uart == USART2)
	{
		char     NemaLine[NMEA_MAX_LINE];
		uint16_t NemaPos = 0;
		
		uint16_t DmaHead = _pUart->usRxBufSize - __HAL_DMA_GET_COUNTER(&UART2RxDMA_Handler);	//计算接收到的数据长度

		if (g_SelfCheck.Rtk.Num++ >= 255)
		{
			g_SelfCheck.Rtk.Num = 1;
		}
		
		while (GpsReadIdx != DmaHead) 
		{
			uint8_t ch = g_DMARxBuf2[GpsReadIdx];
			GpsReadIdx = (GpsReadIdx + 1) % UART2_RX_BUF_SIZE;

			// 行组装：忽略 \r，遇到 \n 结算
			if (ch == '\n') 
			{
				if (NemaPos > 0)
				{
					NemaLine[NemaPos] = '\0';
				}

				//UartSend(&g_Uart1,(uint8_t *)NemaLine, NemaPos + 1);
			
				// --- 前缀匹配，只保留 GGA / RMC ---
				// NMEA格式：$XXGGA 或 $XXRMC，XX为 talker ID（GP/GN/GL等）
				if (strncmp(NemaLine, "$GNGGA", 6) == 0) 
				{
					InitGGAVari();
					gpsGNGGA((uint8_t *)NemaLine, NemaPos);
				}
				else if (strncmp(NemaLine, "$GNRMC", 6) == 0) 
				{
					InitRMCVari();
					gpsGNRMC((uint8_t *)NemaLine, NemaPos);
					nmea_time tmptime;
					tmptime.year = g_tGPS.Year + 2000;
					tmptime.month = g_tGPS.Month;
					tmptime.date = g_tGPS.Day;
					tmptime.hour = g_tGPS.Hour;
					tmptime.min = g_tGPS.Min;
					tmptime.sec = g_tGPS.Sec;
					UTCDate(&tmptime,8);

					g_SysTime.year = tmptime.year - 2000;
					g_SysTime.month = tmptime.month;
					g_SysTime.date = tmptime.date;
					g_SysTime.hour = tmptime.hour;
					g_SysTime.minute = tmptime.min;
					g_SysTime.second = tmptime.sec;
				}
				else if (strncmp(NemaLine, "$OK", 3) == 0)
				{
					if (CurrentIndex >= CLOSE_BEITIAN_NMEA_NUM)
					{
						continue;
					}

					CurrentCLoseNMEA = 1;
					CurrentIndex++;
					SendCLoseNMEANum = 0;
				}
				else if (strncmp(NemaLine, "$PAIR001,062,0", 14) == 0)
				{
					if (CurrentIndex >= CLOSE_YIYUAN_NMEA_NUM)
					{
						continue;
					}
				
					CurrentCLoseNMEA = 1;
					CurrentIndex++;
					SendCLoseNMEANum = 0;
				}
				
				NemaPos = 0;
				memset(NemaLine, 0x00, NMEA_MAX_LINE);
			}
			else if (ch != '\r' && NemaPos < NMEA_MAX_LINE - 1)
			{
				NemaLine[NemaPos++] = ch;
			}
		}
		
		g_Uart2RxTimer = TIM2->CNT;		
	}	
	else if(_pUart->uart == USART3)
	{
		//获取DMA当前写指针（数据写入位置）
		CurrentPos = _pUart->usRxBufSize - __HAL_DMA_GET_COUNTER(&UART3RxDMA_Handler);
    
		//边界保护
		if (CurrentPos >= _pUart->usRxBufSize) 
		{
			CurrentPos = 0;
		}
    
		//计算新数据长度（处理环形缓冲区）
		if (CurrentPos >= LastPos) 
		{
			NewLen = CurrentPos - LastPos;
		}
		else 
		{
			NewLen = _pUart->usRxBufSize - LastPos + CurrentPos;
		}
    
		//限制拷贝长度
		if (NewLen > _pUart->usRxBufSize) 
		{
			NewLen = _pUart->usRxBufSize;
		}
    
		//执行数据拷贝
		if (NewLen > 0) 
		{
			if (CurrentPos >= LastPos) 
			{
				//情况1：没有环绕，数据是连续的
				RingBufPush(&g_DMARxBuf3[LastPos], NewLen, &g_Uart3RxRing, RING_PUSHMODE_CYCLE);
			} 
			else 
			{
				//情况2：发生了环绕，数据分成两段
				FirstCopyLen = _pUart->usRxBufSize - LastPos;
            
				if (NewLen <= FirstCopyLen) 
				{
					//只需要拷贝第一段
					RingBufPush(&g_DMARxBuf3[LastPos], NewLen, &g_Uart3RxRing, RING_PUSHMODE_CYCLE);
				} 
				else 
				{
					//需要拷贝两段
					RingBufPush(&g_DMARxBuf3[LastPos], FirstCopyLen, &g_Uart3RxRing, RING_PUSHMODE_CYCLE);
					RingBufPush(g_DMARxBuf3, NewLen - FirstCopyLen, &g_Uart3RxRing, RING_PUSHMODE_CYCLE);
				}
			}
		}

		//更新读取位置（重要：必须更新，否则下次会重复读取）
		LastPos = (LastPos + NewLen) % _pUart->usRxBufSize;
	}
	else if(_pUart->uart == LPUART1)
	{
		HAL_UART_Abort(&LPUART1_Handler); 
		length = _pUart->usRxBufSize - __HAL_DMA_GET_COUNTER(&LPUART1RxDMA_Handler);	//计算接收到的数据长度
		if(length > 0)
		{
			memset(g_RxBufLP,0,sizeof(g_RxBufLP));
			memcpy(g_RxBufLP,_pUart->pRxBuf,length);
			_pUart->usRxCount = length;
			g_LPUart1RxSta = 1;																//接收到了一批数据.
		}
		memset(_pUart->pRxBuf,0,_pUart->usRxBufSize);   
		HAL_UART_Receive_DMA(&LPUART1_Handler, _pUart->pRxBuf, _pUart->usRxBufSize);	//重启开始DMA传输 每次255字节数据
	}
}
/*
*********************************************************************************************************
*	函 数 名: USART1_IRQHandler
*	功能说明:
*	形    参: 
*	返 回 值: 无
*********************************************************************************************************
*/
void USART1_IRQHandler(void)
{
	if(__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_ORE))
	{ 				
		__HAL_UART_CLEAR_OREFLAG(&UART1_Handler);	//清楚空闲中断标志（否则会一直不断进入中断）
	}		
	if(__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_IDLE))
	{ 
		__HAL_UART_CLEAR_IDLEFLAG(&UART1_Handler);	
		USAR_UART_IDLECallback(&g_Uart1);
		g_Uart1RxTimer = TIM2->CNT;	
	}	
	HAL_UART_IRQHandler(&UART1_Handler);
}

/*
*********************************************************************************************************
*	函 数 名: USART2_IRQHandler
*	功能说明:
*	形    参: 
*	返 回 值: 无
*********************************************************************************************************
*/
void USART2_IRQHandler(void)
{
	if(__HAL_UART_GET_FLAG(&UART2_Handler,UART_FLAG_ORE))
	{ 			
		__HAL_UART_CLEAR_OREFLAG(&UART2_Handler);	//清楚空闲中断标志（否则会一直不断进入中断）
	}
	if(__HAL_UART_GET_FLAG(&UART2_Handler,UART_FLAG_IDLE))
	{ 
		__HAL_UART_CLEAR_IDLEFLAG(&UART2_Handler);
//		HAL_UART_DMAStop(&UART2_Handler);
		USAR_UART_IDLECallback(&g_Uart2);
	}
    HAL_UART_IRQHandler(&UART2_Handler);
}

/*
*********************************************************************************************************
*	函 数 名: USART2_IRQHandler
*	功能说明:
*	形    参: 
*	返 回 值: 无
*********************************************************************************************************
*/
void USART3_IRQHandler(void)
{
	uint32_t temp;
	if(__HAL_UART_GET_FLAG(&UART3_Handler,UART_FLAG_ORE))
	{ 			
		__HAL_UART_CLEAR_OREFLAG(&UART3_Handler);	//清楚空闲中断标志（否则会一直不断进入中断）
	}
	if(__HAL_UART_GET_FLAG(&UART3_Handler,UART_FLAG_IDLE))
	{ 			
		__HAL_UART_CLEAR_IDLEFLAG(&UART3_Handler);	//清楚空闲中断标志（否则会一直不断进入中断）
        temp = UART3_Handler.Instance->ISR;   /* 读状态寄存器 */
        temp = UART3_Handler.Instance->RDR;   /* 读数据寄存器清除标志 */		
		USAR_UART_IDLECallback(&g_Uart3);				//调用中断处理函数
		return;
	}
	HAL_UART_IRQHandler(&UART3_Handler);
}

/*
*********************************************************************************************************
*	函 数 名: LPUART1_IRQHandler
*	功能说明:
*	形    参: 
*	返 回 值: 无
*********************************************************************************************************
*/
void LPUART1_IRQHandler(void)
{
#if LPUARTDMA
	g_LPUartRxTimer = TIM2->CNT;
	if(__HAL_UART_GET_FLAG(&LPUART1_Handler,UART_FLAG_IDLE))
	{ 
		//clear idle flag				
		__HAL_UART_CLEAR_IDLEFLAG(&LPUART1_Handler);	//清楚空闲中断标志（否则会一直不断进入中断）
		USAR_UART_IDLECallback(&g_LPUart);				//调用中断处理函数
		return;
	}
	HAL_UART_IRQHandler(&LPUART1_Handler);
#else
	UartIRQ(&g_LPUart);
	g_LPUartRxTimer = TIM2->CNT;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RS485_InitTXE
*	功能说明: 配置RS485发送使能口线 TXE
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void Tool485_InitTXE(void)
{	
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;
	/* 打开GPIO时钟 */
	 __HAL_RCC_GPIOC_CLK_ENABLE();
	
	/* 配置引脚为推挽输出 */
	GPIO_Initure.Pin = GPIO_PIN_13;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			/* 推挽输出 */
	GPIO_Initure.Pull = GPIO_NOPULL;					/* 上下拉电阻不使能 */
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;	/* GPIO速度等级 */
	HAL_GPIO_Init(GPIOC, &GPIO_Initure);	
	
	TOOL485_RX_EN();
	g_Uart1DirFlag = 1;
}


/*
*********************************************************************************************************
*	函 数 名: RS485_SendBefor
*	功能说明: 发送数据前的准备工作。对于RS485通信，请设置RS485芯片为发送状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendBefor = RS485_SendBefor
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void Tool485_SendBefor(void)
{
	TOOL485_TX_EN();	/* 切换RS485收发芯片为发送模式 */
	delay_us(1000);
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendOver
*	功能说明: 发送一串数据结束后的善后处理。对于RS485通信，请设置RS485芯片为接收状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendOver = RS485_SendOver
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void Tool485_SendOver(void)
{
	TOOL485_RX_EN();	/* 切换RS485收发芯片为接收模式 */
	delay_us(1000);
}
void Select_485(void)
{
	memset((uint8_t*)g_TxBuf1,0x00,sizeof(g_TxBuf1));
    memset((uint8_t*)g_RxBuf1,0x00,sizeof(g_RxBuf1));
	memset((uint8_t*)g_SecTxBuf1,0x00,sizeof(g_SecTxBuf1));
	g_Uart1.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_Uart1.usRxRead = 0;						/* 接收FIFO读索引 */
	g_Uart1.usRxCount = 0;						/* 接收到的新数据个数 */
	g_Uart1.usTxTimer = 0;						/* 发送结束计时 */
	g_Uart1.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_Uart1.Sending = 0;						/* 正在发送中标志 */
	g_Uart1.SendBefor = Tool485_SendBefor;		/* 发送数据前的回调函数 */
	g_Uart1.SendOver = Tool485_SendOver;		/* 发送完毕后的回调函数 */	
	SWITCH_USB();
    Tool485_InitTXE();
	Tool485_SendOver();
}
void Select_uart(void)
{
	memset((uint8_t*)g_TxBuf1,0x00,sizeof(g_TxBuf1));
    memset((uint8_t*)g_RxBuf1,0x00,sizeof(g_RxBuf1));
	memset((uint8_t*)g_SecTxBuf1,0x00,sizeof(g_SecTxBuf1));
	g_Uart1.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_Uart1.usRxRead = 0;						/* 接收FIFO读索引 */
	g_Uart1.usRxCount = 0;						/* 接收到的新数据个数 */
	g_Uart1.usTxTimer = 0;						/* 发送结束计时 */
	g_Uart1.SendBefor = 0;		                /* 发送数据前的回调函数 */
	g_Uart1.SendOver = 0;		                /* 发送完毕后的回调函数 */
	g_Uart1.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_Uart1.Sending = 0;						/* 正在发送中标志 */
	SWITCH_BT();
}

/*
*********************************************************************************************************
*	函 数 名: RS485_ReciveNew
*	功能说明: 接收到新的数据
*	形    参: _byte 接收到的新数据
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_ReciveNew(void)
{
//	MODH_ReciveNew(_byte);
}


/**
  * Enable DMA controller clock
  */
void MX_DMA_Init(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();
}


void InitChargeUSART1(void)
{
	//UART 初始化设置
	UART1_Handler.Instance 			= USART1;					//USART2
	UART1_Handler.Init.BaudRate 	= 115200;			        //波特率
	UART1_Handler.Init.WordLength = UART_WORDLENGTH_8B;
	UART1_Handler.Init.StopBits = UART_STOPBITS_1;
	UART1_Handler.Init.Parity = UART_PARITY_NONE;
	UART1_Handler.Init.Mode = UART_MODE_TX_RX;
	//UART1_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	//UART1_Handler.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	//UART1_Handler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&UART1_Handler) != HAL_OK)
	{
		Error_Handler();
	}
	__HAL_UART_ENABLE_IT(&UART1_Handler, UART_IT_IDLE);	/*空闲中断开启*/	
	HAL_UART_Receive_DMA(&UART1_Handler,g_DMARxBuf1, UART1_RX_BUF_SIZE);     //设置DMA传输，将串口1的数据搬运到recvive_buff中，每次255个字节 		
//	__HAL_UART_ENABLE_IT(&UART1_Handler, UART_IT_RXNE);	/*空闲中断开启*/	
	Tool485_InitTXE();	
	SWITCH_USB(); 
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);					//抢占优先级1，子优先级1
    HAL_NVIC_EnableIRQ(USART1_IRQn);							//使能USART1中断通道
}

/*
*********************************************************************************************************
*	函 数 名: InitUSART1
*	功能说明:
*	形    参: 
*	返 回 值: 无
*********************************************************************************************************
*/
void InitUSART1(void)
{	
    //UART 初始化设置
	if (SWITCHTYPE==1)
	{
		//UART 初始化设置
		UART1_Handler.Instance 			= USART1;					//USART2
		UART1_Handler.Init.BaudRate 	= UART1_TOOL_BAUD;			//波特率
		UART1_Handler.Init.WordLength 	= UART_WORDLENGTH_9B; 		//字长为8位数据格式
		UART1_Handler.Init.StopBits 	= UART_STOPBITS_1;	  		//一个停止位
		UART1_Handler.Init.Parity 		= UART_PARITY_NONE;			//无奇偶校验位
		UART1_Handler.Init.HwFlowCtl 	= UART_HWCONTROL_NONE; 		//无硬件流控
		UART1_Handler.Init.Mode 		= UART_MODE_TX_RX;		  	//收发模式
		if (HAL_UART_Init(&UART1_Handler) != HAL_OK)
		{
			Error_Handler();
		}
		__HAL_UART_ENABLE_IT(&UART1_Handler, UART_IT_IDLE);	/*空闲中断开启*/	
		HAL_UART_Receive_DMA(&UART1_Handler,g_DMARxBuf1, UART1_RX_BUF_SIZE);     //设置DMA传输，将串口1的数据搬运到recvive_buff中，每次255个字节 		
//		__HAL_UART_ENABLE_IT(&UART1_Handler, UART_IT_RXNE);	/*空闲中断开启*/	
		Tool485_InitTXE();
	}
	else
	{
		UART1_Handler.Instance = USART1;
		UART1_Handler.Init.BaudRate = UART1_BT_BAUD;
		UART1_Handler.Init.WordLength = UART_WORDLENGTH_8B;
		UART1_Handler.Init.StopBits = UART_STOPBITS_1;
		UART1_Handler.Init.Parity = UART_PARITY_NONE;
		UART1_Handler.Init.Mode = UART_MODE_TX_RX;
		UART1_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		UART1_Handler.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
		UART1_Handler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
		if (HAL_UART_Init(&UART1_Handler) != HAL_OK)
		{
			Error_Handler();
		}
		__HAL_UART_ENABLE_IT(&UART1_Handler, UART_IT_IDLE);	/*空闲中断开启*/	
		HAL_UART_Receive_DMA(&UART1_Handler,g_DMARxBuf1, UART1_RX_BUF_SIZE);     //设置DMA传输，将串口1的数据搬运到recvive_buff中，每次255个字节 
	}
	if(SWITCHTYPE > 0)
		SWITCH_USB(); 
	else
		SWITCH_BT(); 
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);					//抢占优先级1，子优先级1
    HAL_NVIC_EnableIRQ(USART1_IRQn);							//使能USART1中断通道	  
}

/*
*********************************************************************************************************
*	函 数 名: InitUSART2
*	功能说明:
*	形    参: 
*	返 回 值: 无
*********************************************************************************************************
*/
void InitUSART2(uint32_t bound)
{
    //UART 初始化设置
    UART2_Handler.Instance 			= USART2;					//USART2
    UART2_Handler.Init.BaudRate 	= bound;					//波特率
    UART2_Handler.Init.WordLength 	= UART_WORDLENGTH_8B; 		//字长为8位数据格式
    UART2_Handler.Init.StopBits 	= UART_STOPBITS_1;	  		//一个停止位
    UART2_Handler.Init.Parity 		= UART_PARITY_NONE;			//无奇偶校验位
    UART2_Handler.Init.HwFlowCtl 	= UART_HWCONTROL_NONE; 		//无硬件流控
    UART2_Handler.Init.Mode 		= UART_MODE_TX_RX;		  	//收发模式
	UART2_Handler.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	UART2_Handler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;    
	HAL_UART_Init(&UART2_Handler);					   			//HAL_UART_Init()会使能USART2

	__HAL_UART_ENABLE_IT(&UART2_Handler, UART_IT_IDLE); 		//开启接收中断
    HAL_UART_Receive_DMA(&UART2_Handler, (uint8_t*)g_DMARxBuf2, UART2_RX_BUF_SIZE);     //设置DMA传输，将串口1的数据搬运到recvive_buff中，每次255个字节 
    HAL_NVIC_SetPriority(USART2_IRQn,2, 0);					//抢占优先级1，子优先级3	
	HAL_NVIC_EnableIRQ(USART2_IRQn);							//使能USART2中断通道
}

/*
*********************************************************************************************************
*	函 数 名: InitUSART2
*	功能说明:
*	形    参: 
*	返 回 值: 无
*********************************************************************************************************
*/
void InitUSART3(uint32_t bound)
{
    //UART 初始化设置
    UART3_Handler.Instance 			= USART3;					//USART2
    UART3_Handler.Init.BaudRate 	= bound;					//波特率
    UART3_Handler.Init.WordLength 	= UART_WORDLENGTH_8B; 		//字长为8位数据格式
    UART3_Handler.Init.StopBits 	= UART_STOPBITS_1;	  		//一个停止位
    UART3_Handler.Init.Parity 		= UART_PARITY_NONE;			//无奇偶校验位
    UART3_Handler.Init.HwFlowCtl 	= UART_HWCONTROL_NONE; 		//无硬件流控
    UART3_Handler.Init.Mode 		= UART_MODE_TX_RX;		  	//收发模式
	UART3_Handler.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	UART3_Handler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    HAL_UART_Init(&UART3_Handler);					   			//HAL_UART_Init()会使能USART2
	
	__HAL_UART_ENABLE_IT(&UART3_Handler, UART_IT_IDLE); 		//开启接收中断
	HAL_UART_Receive_DMA(&UART3_Handler,g_DMARxBuf3,UART3_RX_BUF_SIZE); //使能的DMA收
    
    HAL_NVIC_SetPriority(USART3_IRQn, 1, 0);					//抢占优先级1，子优先级3	
	HAL_NVIC_EnableIRQ(USART3_IRQn);							//使能USART2中断通道
}

/*
*********************************************************************************************************
*	函 数 名: InitLPUART1
*	功能说明:
*	形    参: 
*	返 回 值: 无
*********************************************************************************************************
*/

void InitLPUART1(uint32_t bound)
{
    LPUART1_Handler.Instance 		= LPUART1;					//LPUART1
    LPUART1_Handler.Init.BaudRate 	= bound;				  	//波特率
    LPUART1_Handler.Init.WordLength = UART_WORDLENGTH_8B; 		//字长为8位数据格式
    LPUART1_Handler.Init.StopBits 	= UART_STOPBITS_1;	  		//一个停止位
    LPUART1_Handler.Init.Parity 	= UART_PARITY_NONE;		  	//无奇偶校验位
    LPUART1_Handler.Init.HwFlowCtl 	= UART_HWCONTROL_NONE; 		//无硬件流控
    LPUART1_Handler.Init.Mode 		= UART_MODE_TX_RX;		 	//收发模式
	LPUART1_Handler.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	LPUART1_Handler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	HAL_UART_Init(&LPUART1_Handler);					    	//HAL_UART_Init()会使能LPUART1
#if LPUARTDMA
	__HAL_UART_ENABLE_IT(&LPUART1_Handler, UART_IT_IDLE); 		//开启接收中断
	HAL_UART_Receive_DMA(&LPUART1_Handler,g_DMARxBufLP,LPUART_RX_BUF_SIZE); //使能的DMA收
#else
	__HAL_UART_ENABLE_IT(&LPUART1_Handler, UART_IT_RXNE); 		//开启接收中断
#endif
    HAL_NVIC_SetPriority(LPUART1_IRQn, 3, 2);					//抢占优先级1，子优先级2
	HAL_NVIC_EnableIRQ(LPUART1_IRQn);							//使能LPUART1中断通道
}


/**
 * @brief	HAL库串口底层初始化，时钟使能，引脚配置，中断配置
 *
 * @param	huart	串口句柄
 *
 * @return  void
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStruct;

    if(huart->Instance == USART1) 
    {
		/* 使能 GPIO TX/RX 时钟 */
		USART1_TX_GPIO_CLK_ENABLE();
		USART1_RX_GPIO_CLK_ENABLE();
		__HAL_RCC_DMA1_CLK_ENABLE();//DMA1时钟使能		
		/* 使能 USARTx 时钟 */
		USART1_CLK_ENABLE();	

		/* 配置TX引脚 */
		GPIO_InitStruct.Pin       = USART1_TX_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = USART1_TX_AF;
		HAL_GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStruct);	
		
		/* 配置RX引脚 */
		GPIO_InitStruct.Pin = USART1_RX_PIN;
		GPIO_InitStruct.Alternate = USART1_RX_AF;
		HAL_GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStruct);
					
		UART1RxDMA_Handler.Instance = DMA1_Channel5;                        //通道选择
		UART1RxDMA_Handler.Init.Request = DMA_REQUEST_2;                    //请求选择
		UART1RxDMA_Handler.Init.Direction = DMA_PERIPH_TO_MEMORY;           //存储器到外设
		UART1RxDMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;               //外设非增量模式
		UART1RxDMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                   //存储器增量模式
		UART1RxDMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  //外设数据长度:8位
		UART1RxDMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     //存储器数据长度:8位
		UART1RxDMA_Handler.Init.Mode = DMA_NORMAL;                          //外设普通模式
		UART1RxDMA_Handler.Init.Priority = DMA_PRIORITY_MEDIUM;             //中等优先级
		HAL_DMA_DeInit(&UART1RxDMA_Handler);
		HAL_DMA_Init(&UART1RxDMA_Handler);		
		__HAL_LINKDMA(&UART1_Handler, hdmarx, UART1RxDMA_Handler);  //将DMA与USART1联系起来(接收DMA)
    }
	else if(huart->Instance == USART2)  
	{
		/* 使能 GPIO TX/RX 时钟 */
		USART2_TX_GPIO_CLK_ENABLE();
		USART2_RX_GPIO_CLK_ENABLE();
		__HAL_RCC_DMA1_CLK_ENABLE();//DMA1时钟使能
		/* 使能 USARTx 时钟 */
		USART2_CLK_ENABLE();	

		/* 配置TX引脚 */
		GPIO_InitStruct.Pin       = USART2_TX_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = USART2_TX_AF;
		HAL_GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStruct);	
		
		/* 配置RX引脚 */
		GPIO_InitStruct.Pin = USART2_RX_PIN;
		GPIO_InitStruct.Alternate = USART2_RX_AF;
		HAL_GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStruct);
		
		UART2RxDMA_Handler.Instance = DMA1_Channel6;                          //通道选择
		UART2RxDMA_Handler.Init.Request = DMA_REQUEST_2;                              //请求选择
		UART2RxDMA_Handler.Init.Direction = DMA_PERIPH_TO_MEMORY;           //存储器到外设
		UART2RxDMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;               //外设非增量模式
		UART2RxDMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                   //存储器增量模式
		UART2RxDMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  //外设数据长度:8位
		UART2RxDMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     //存储器数据长度:8位
		UART2RxDMA_Handler.Init.Mode = DMA_CIRCULAR;                        //外设普通模式
		UART2RxDMA_Handler.Init.Priority = DMA_PRIORITY_HIGH;             	//中等优先级
		HAL_DMA_DeInit(&UART2RxDMA_Handler);
		HAL_DMA_Init(&UART2RxDMA_Handler);		
		__HAL_LINKDMA(&UART2_Handler, hdmarx, UART2RxDMA_Handler);  //将DMA与USART1联系起来(接收DMA)		
	}
	else if(huart->Instance == USART3)  
	{
		/* 使能 GPIO TX/RX 时钟 */
		USART3_TX_GPIO_CLK_ENABLE();
		USART3_RX_GPIO_CLK_ENABLE();
		
		/* 使能 USARTx 时钟 */
		USART3_CLK_ENABLE();	

		/* 配置TX引脚 */
		GPIO_InitStruct.Pin       = USART3_TX_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = USART3_TX_AF;
		HAL_GPIO_Init(USART3_TX_GPIO_PORT, &GPIO_InitStruct);	
		
		/* 配置RX引脚 */
		GPIO_InitStruct.Pin = USART3_RX_PIN;
		GPIO_InitStruct.Alternate = USART3_RX_AF;
		HAL_GPIO_Init(USART3_RX_GPIO_PORT, &GPIO_InitStruct);
		
		__HAL_RCC_DMA1_CLK_ENABLE();//DMA2时钟使能
			
		UART3RxDMA_Handler.Instance = DMA1_Channel3;                        //通道选择
		UART3RxDMA_Handler.Init.Request = DMA_REQUEST_2;                    //请求选择
		UART3RxDMA_Handler.Init.Direction = DMA_PERIPH_TO_MEMORY;           //存储器到外设
		UART3RxDMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;               //外设非增量模式
		UART3RxDMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                   //存储器增量模式
		UART3RxDMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  //外设数据长度:8位
		UART3RxDMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     //存储器数据长度:8位
		UART3RxDMA_Handler.Init.Mode = DMA_CIRCULAR;//DMA_NORMAL;                          //外设普通模式
		UART3RxDMA_Handler.Init.Priority = DMA_PRIORITY_MEDIUM;             	//中等优先级
		//UART3RxDMA_Handler.Init.Priority = DMA_PRIORITY_VERY_HIGH;             	//中等优先级
		HAL_DMA_DeInit(&UART3RxDMA_Handler);
		HAL_DMA_Init(&UART3RxDMA_Handler);		
		__HAL_LINKDMA(&UART3_Handler, hdmarx, UART3RxDMA_Handler);  //将DMA与USART1联系起来(接收DMA)
		InitRingBuf(&g_Uart3RxRing, UART3_RX_BUF_SIZE, g_RxBuf3);		
	}
	else if (huart->Instance == LPUART1)
	{
		RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
		
		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
		PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
		{
			
		}
		LPUART1_CLK_ENABLE();			       		//使能LPUART1时钟
		LPUART1_TX_GPIO_CLK_ENABLE();				//使能GPIO时钟
		LPUART1_RX_GPIO_CLK_ENABLE();
		
		GPIO_InitStruct.Pin       = LPUART1_TX_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = LPUART1_TX_AF;
		HAL_GPIO_Init(LPUART1_TX_GPIO_PORT, &GPIO_InitStruct);	
		
		/* 配置RX引脚 */
		GPIO_InitStruct.Pin 		= LPUART1_RX_PIN;
		GPIO_InitStruct.Alternate 	= LPUART1_RX_AF;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		HAL_GPIO_Init(LPUART1_RX_GPIO_PORT, &GPIO_InitStruct);

#if LPUARTDMA		
		__HAL_RCC_DMA2_CLK_ENABLE();//DMA2时钟使能
			
		LPUART1RxDMA_Handler.Instance = DMA2_Channel7;                        //通道选择
		LPUART1RxDMA_Handler.Init.Request = DMA_REQUEST_4;                    //请求选择
		LPUART1RxDMA_Handler.Init.Direction = DMA_PERIPH_TO_MEMORY;           //存储器到外设
		LPUART1RxDMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;               //外设非增量模式
		LPUART1RxDMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                   //存储器增量模式
		LPUART1RxDMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  //外设数据长度:8位
		LPUART1RxDMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     //存储器数据长度:8位
		LPUART1RxDMA_Handler.Init.Mode = DMA_NORMAL;                          //外设普通模式
		LPUART1RxDMA_Handler.Init.Priority = DMA_PRIORITY_MEDIUM;             	//中等优先级

		HAL_DMA_DeInit(&LPUART1RxDMA_Handler);
		HAL_DMA_Init(&LPUART1RxDMA_Handler);
		
		__HAL_LINKDMA(&LPUART1_Handler, hdmarx, LPUART1RxDMA_Handler);  //将DMA与USART1联系起来(接收DMA)
#endif
	}
		
}

/**
  * @brief UART MSP DeInit
  * @param huart: uart handle
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef GPIO_InitStruct;
  
	GPIO_InitStruct.Mode      = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
    if(huart->Instance == USART1) 
    {
		/* 使能 GPIO TX/RX 时钟 */
		USART1_TX_GPIO_CLK_ENABLE();
		USART1_RX_GPIO_CLK_ENABLE();
		
		/* 配置TX引脚 */
		GPIO_InitStruct.Pin       = USART1_TX_PIN;		
		HAL_GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStruct);	
		
		/* 配置RX引脚 */
		GPIO_InitStruct.Pin = USART1_RX_PIN;
		HAL_GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStruct);
    }
	else if(huart->Instance == USART2)  
	{
		/* 使能 GPIO TX/RX 时钟 */
		USART2_TX_GPIO_CLK_ENABLE();
		USART2_RX_GPIO_CLK_ENABLE();
		
		/* 配置TX引脚 */
		GPIO_InitStruct.Pin       = USART2_TX_PIN;
		HAL_GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStruct);	
		
		/* 配置RX引脚 */
		GPIO_InitStruct.Pin = USART2_RX_PIN;
		HAL_GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStruct);
		
	}
	else if(huart->Instance == USART3)  
	{
		/* 使能 GPIO TX/RX 时钟 */
		USART3_TX_GPIO_CLK_ENABLE();
		USART3_RX_GPIO_CLK_ENABLE();
		
		/* 配置TX引脚 */
//		GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pin       = USART3_TX_PIN;
		HAL_GPIO_Init(USART3_TX_GPIO_PORT, &GPIO_InitStruct);
		
		
		/* 配置RX引脚 */
//		GPIO_InitStruct.Mode      = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pin = USART3_RX_PIN;
		HAL_GPIO_Init(USART3_RX_GPIO_PORT, &GPIO_InitStruct);
		
	}
	else if (huart->Instance == LPUART1)
	{
		LPUART1_TX_GPIO_CLK_ENABLE();				//使能GPIO时钟
		LPUART1_RX_GPIO_CLK_ENABLE();
		
		GPIO_InitStruct.Pin       = LPUART1_TX_PIN;
		HAL_GPIO_Init(LPUART1_TX_GPIO_PORT, &GPIO_InitStruct);	
		
		/* 配置RX引脚 */
		GPIO_InitStruct.Pin 		= LPUART1_RX_PIN;
		HAL_GPIO_Init(LPUART1_RX_GPIO_PORT, &GPIO_InitStruct);
	}
}


/**
 * @brief	DMAx的各通道配置
 *			从外设模式->存储器/8位数据宽度/存储器增量模式
 *
 * @param   DMA_Streamx		DMA通道选择,DMA1_Channel0~7/DMA2_Channel0~7
 * @param   chx				DMA请求选择,@ref DMA_REQUEST_0~DMA_REQUEST_7
 *
 * @return  void
 */
void RxDMA_Config(DMA_TypeDef *DMA, DMA_Channel_TypeDef *DMA_Channel, uint32_t seq)
{
    //Rx DMA配置
	if(DMA == DMA1)
	{
		__HAL_RCC_DMA1_CLK_ENABLE();//DMA1时钟使能
		
		UART1RxDMA_Handler.Instance = DMA_Channel;                          //通道选择
		UART1RxDMA_Handler.Init.Request = seq;                              //请求选择
		UART1RxDMA_Handler.Init.Direction = DMA_PERIPH_TO_MEMORY;           //存储器到外设
		UART1RxDMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;               //外设非增量模式
		UART1RxDMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                   //存储器增量模式
		UART1RxDMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  //外设数据长度:8位
		UART1RxDMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     //存储器数据长度:8位
		UART1RxDMA_Handler.Init.Mode = DMA_NORMAL;                          //外设普通模式
		UART1RxDMA_Handler.Init.Priority = DMA_PRIORITY_HIGH;             	//中等优先级

		HAL_DMA_DeInit(&UART1RxDMA_Handler);
		HAL_DMA_Init(&UART1RxDMA_Handler);
		
		__HAL_LINKDMA(&UART1_Handler, hdmarx, UART1RxDMA_Handler);  //将DMA与USART1联系起来(接收DMA)
	}
	else if(DMA == DMA2)
	{
		__HAL_RCC_DMA2_CLK_ENABLE();//DMA2时钟使能
			
		LPUART1RxDMA_Handler.Instance = DMA2_Channel7;                        //通道选择
		LPUART1RxDMA_Handler.Init.Request = DMA_REQUEST_4;                    //请求选择
		LPUART1RxDMA_Handler.Init.Direction = DMA_PERIPH_TO_MEMORY;           //存储器到外设
		LPUART1RxDMA_Handler.Init.PeriphInc = DMA_PINC_DISABLE;               //外设非增量模式
		LPUART1RxDMA_Handler.Init.MemInc = DMA_MINC_ENABLE;                   //存储器增量模式
		LPUART1RxDMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  //外设数据长度:8位
		LPUART1RxDMA_Handler.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     //存储器数据长度:8位
		LPUART1RxDMA_Handler.Init.Mode = DMA_NORMAL;                          //外设普通模式
		LPUART1RxDMA_Handler.Init.Priority = DMA_PRIORITY_MEDIUM;             	//中等优先级

		HAL_DMA_DeInit(&LPUART1RxDMA_Handler);
		HAL_DMA_Init(&LPUART1RxDMA_Handler);
		
		__HAL_LINKDMA(&LPUART1_Handler, hdmarx, LPUART1RxDMA_Handler);  //将DMA与USART1联系起来(接收DMA)
		
	}
}


/*
*********************************************************************************************************
*	函 数 名: InitUart
*	功能说明:
*	形    参: 
*	返 回 值: 无
*********************************************************************************************************
*/
void InitUart(void)
{
	UartVarInit();		/* 必须先初始化全局变量,再配置硬件 */
	InitUSART1();
	Tool485_InitTXE();
	InitUSART2(UART2_BAUD);
	InitUSART3(UART3_BAUD);
	InitLPUART1(LPUART1_BAUD);
}

/*
*********************************************************************************************************
*	函 数 名: DeInitUart
*	功能说明:
*	形    参: 
*	返 回 值: 无
*********************************************************************************************************
*/
void DeInitUart(void)
{
	HAL_UART_DeInit(&UART1_Handler);
	HAL_UART_DeInit(&UART2_Handler);
	HAL_UART_DeInit(&UART3_Handler);
	HAL_UART_DeInit(&LPUART1_Handler);
}

/******************************************************************
函数名称: void debug_xinfo
功能描述: 按指定宽度打印调试信息
参数输入: uint8_t	*Info	数据
		  uint16_t	Len		数据长度
		  uint8_t	Width	数据宽度
参数输出: 无
返 回 值: 无
其    他:
******************************************************************/
void debug_xinfo(uint8_t *Info, uint16_t Len, uint8_t Width)
{
	uint16_t i, row, col, reline;
	uint16_t pos;
	
	/* 接收数据显示 */
	if ((Len <= 128) && (Len != 0) && (Width <= 32))
	{
		/* 打印表头 */
		for (i=0; i<Width; i++)
		{
			debug_printf("%02X ", i);
		}
		debug_printf("\r\n");
		debug_printf("---------------------------------------\r\n");
		reline = 0;
		/* 打印数据 */
		for (i=0; i<Len; i++)
		{
			col = i % Width;
			if (col < Width)
			{
				debug_printf("%02X ", Info[i]);
				reline = 0;
				if (col == (Width - 1))
				{
					debug_printf("\r\n");
					reline = 1;
				}
			}
		}
		if (!reline) debug_printf("\r\n");
	}
	else
	{
		if (Width > 32)
		{
			debug_printf("Debug Inof: Parameter Error!\r\n");
		}
		if (Len > 128)
		{
			debug_printf("Debug Info: Len = %d Out of range!\r\n", Len);
		}
		else if (Len == 0)
		{
			debug_printf("Debug Info: No data!\r\n");
		}
	}
}
