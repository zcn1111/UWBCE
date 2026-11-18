
#include "stm32l4xx.h"
#include "bsp.h"
#include "app_glovar.h"

UART_HandleTypeDef UART1_Handler; //UART句柄
UART_HandleTypeDef UART2_Handler; //UART句柄
UART_HandleTypeDef LPUART1_Handler; //UART句柄

//Usart1接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
uint16_t g_Usart1RxSta=0;  

extern uint8_t g_LoraMode;// 0:配置模式 1:接收模式 2:发送模式

UART_T g_Uart1;
uint8_t g_TxBuf1[UART1_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];		/* 接收缓冲区 */
uint32_t g_Uart1RxTimer;

UART_T g_Uart2;
uint8_t g_TxBuf2[UART2_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];		/* 接收缓冲区 */
uint32_t g_Uart2RxTimer;

UART_T g_LPUart;
uint8_t g_TxBufLP[LPUART_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBufLP[LPUART_RX_BUF_SIZE];		/* 接收缓冲区 */
uint32_t g_LPUartRxTimer;


void UartVarInit(void)
{
	//LORA
	g_Uart1.uart = USART1;						/* STM32 串口设备 */
	g_Uart1.pTxBuf = g_TxBuf1;					/* 发送缓冲区指针 */
	g_Uart1.pRxBuf = g_RxBuf1;					/* 接收缓冲区指针 */
	g_Uart1.usTxBufSize = UART1_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_Uart1.usRxBufSize = UART1_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_Uart1.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_Uart1.usTxRead = 0;						/* 发送FIFO读索引 */
	g_Uart1.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_Uart1.usRxRead = 0;						/* 接收FIFO读索引 */
	g_Uart1.usRxCount = 0;						/* 接收到的新数据个数 */
	g_Uart1.usTxCount = 0;						/* 待发送的数据个数 */
	g_Uart1.SendBefor = 0;						/* 发送数据前的回调函数 */
	g_Uart1.SendOver = 0;						/* 发送完毕后的回调函数 */
	g_Uart1.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_Uart1.Sending = 0;						/* 正在发送中标志 */
	//TOOL
	g_Uart2.uart = USART2;						/* STM32 串口设备 */
	g_Uart2.pTxBuf = g_TxBuf2;					/* 发送缓冲区指针 */
	g_Uart2.pRxBuf = g_RxBuf2;					/* 接收缓冲区指针 */
	g_Uart2.usTxBufSize = UART2_TX_BUF_SIZE;	/* 发送缓冲区大小 */
	g_Uart2.usRxBufSize = UART2_RX_BUF_SIZE;	/* 接收缓冲区大小 */
	g_Uart2.usTxWrite = 0;						/* 发送FIFO写索引 */
	g_Uart2.usTxRead = 0;						/* 发送FIFO读索引 */
	g_Uart2.usRxWrite = 0;						/* 接收FIFO写索引 */
	g_Uart2.usRxRead = 0;						/* 接收FIFO读索引 */
	g_Uart2.usRxCount = 0;						/* 接收到的新数据个数 */
	g_Uart2.usTxCount = 0;						/* 待发送的数据个数 */
	g_Uart2.SendBefor = RS485_SendBefor;		/* 发送数据前的回调函数 */
	g_Uart2.SendOver = RS485_SendOver;			/* 发送完毕后的回调函数 */
	g_Uart2.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
	g_Uart2.Sending = 0;						/* 正在发送中标志 */
	//DTU
	if(g_SysPar.type == RECIEVE && g_SysPar.no == 0x01)
	{
		g_LPUart.uart = LPUART1;					/* STM32 串口设备 */
		g_LPUart.pTxBuf = g_TxBufLP;				/* 发送缓冲区指针 */
		g_LPUart.pRxBuf = g_RxBufLP;				/* 接收缓冲区指针 */
		g_LPUart.usTxBufSize = LPUART_TX_BUF_SIZE;	/* 发送缓冲区大小 */
		g_LPUart.usRxBufSize = LPUART_RX_BUF_SIZE;	/* 接收缓冲区大小 */
		g_LPUart.usTxWrite = 0;						/* 发送FIFO写索引 */
		g_LPUart.usTxRead = 0;						/* 发送FIFO读索引 */
		g_LPUart.usRxWrite = 0;						/* 接收FIFO写索引 */
		g_LPUart.usRxRead = 0;						/* 接收FIFO读索引 */
		g_LPUart.usRxCount = 0;						/* 接收到的新数据个数 */
		g_LPUart.usTxCount = 0;						/* 待发送的数据个数 */
		g_LPUart.SendBefor = 0;						/* 发送数据前的回调函数 */
		g_LPUart.SendOver = 0;						/* 发送完毕后的回调函数 */
		g_LPUart.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
		g_LPUart.Sending = 0;						/* 正在发送中标志 */
	}
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
	uint16_t i;

	if (_pUart->SendBefor != 0)
	{
		_pUart->SendBefor();		/* 如果是RS485通信，可以在这个函数中将RS485设置为发送模式 */
		delay_us(2000);
	}
	
	for (i = 0; i < _usLen; i++)
	{
		/* 如果发送缓冲区已经满了，则等待缓冲区空 */
		while (1)
		{
			__IO uint16_t usCount;

			DISABLE_INT();
			usCount = _pUart->usTxCount;
			ENABLE_INT();

			if (usCount < _pUart->usTxBufSize)
			{
				break;
			}
			else if(usCount == _pUart->usTxBufSize)/* 数据已填满缓冲区 */
			{
				if((_pUart->uart->CR1 & USART_CR1_TXEIE) == 0)
				{
					SET_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);
				}  
			}
		}

		/* 将新数据填入发送缓冲区 */
		_pUart->pTxBuf[_pUart->usTxWrite] = _ucaBuf[i];

		DISABLE_INT();
		if (++_pUart->usTxWrite >= _pUart->usTxBufSize)
		{
			_pUart->usTxWrite = 0;
		}
		_pUart->usTxCount++;
		ENABLE_INT();
	}

	SET_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);	/* 使能发送中断（缓冲区空） */
}


/*
*********************************************************************************************************
*	函 数 名: UartGetChar
*	功能说明: 从串口接收缓冲区读取1字节数据 （用于主程序调用）
*	形    参: _pUart : 串口设备
*			  _pByte : 存放读取数据的指针
*	返 回 值: 0 表示无数据  1表示读取到数据
*********************************************************************************************************
*/
uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte)
{
	uint16_t usCount;

	/* usRxWrite 变量在中断函数中被改写，主程序读取该变量时，必须进行临界区保护 */
	DISABLE_INT();
	usCount = _pUart->usRxCount;
	ENABLE_INT();

	/* 如果读和写索引相同，则返回0 */
	if (usCount == 0)	/* 已经没有数据 */
	{
		return 0;
	}
	else
	{
		*_pByte = _pUart->pRxBuf[_pUart->usRxRead];		/* 从串口接收FIFO取1个数据 */

		/* 改写FIFO读索引 */
		DISABLE_INT();
		if (++_pUart->usRxRead >= _pUart->usRxBufSize)
		{
			_pUart->usRxRead = 0;
		}
		_pUart->usRxCount--;
		ENABLE_INT();
		return 1;
	}
}

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
*	功能说明: 供中断服务程序调用，通用串口中断处理函数
*	形    参: _pUart : 串口设备
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartIRQ(UART_T *_pUart)
{
	uint32_t isrflags   = READ_REG(_pUart->uart->ISR);
	uint32_t cr1its     = READ_REG(_pUart->uart->CR1);
	uint32_t cr3its     = READ_REG(_pUart->uart->CR3);
	
	/* 处理接收中断  */
	if ((isrflags & USART_ISR_RXNE) != RESET)
	{
		/* 从串口接收数据寄存器读取数据存放到接收FIFO */
		uint8_t ch;

		ch = READ_REG(_pUart->uart->RDR);
		_pUart->pRxBuf[_pUart->usRxWrite] = ch;
		if (++_pUart->usRxWrite >= _pUart->usRxBufSize)
		{
			_pUart->usRxWrite = 0;
		}
		if (_pUart->usRxCount < _pUart->usRxBufSize)
		{
			_pUart->usRxCount++;
		}
		
		if(_pUart->uart == USART1)
		{
			if(g_Usart1RxSta == 0 && g_LoraMode == 0)
			{
				HAL_TIM_Base_Start(&TIM7_Handler);//开启定时器7 
				
			}
			g_Usart1RxSta = _pUart->usRxCount;
			g_Uart1RxTimer = TIM2->CNT;
		}
		
		
		
		if(_pUart->uart == USART2)
		{
			g_Uart2RxTimer = TIM2->CNT;
		}
		
		if(_pUart->uart == LPUART1)
		{
			g_LPUartRxTimer = TIM2->CNT;
		}
		/* 回调函数,通知应用程序收到新数据,一般是发送1个消息或者设置一个标记 */
		//if (_pUart->usRxWrite == _pUart->usRxRead)
		//if (_pUart->usRxCount == 1)
		{
			if (_pUart->ReciveNew)
			{
				_pUart->ReciveNew(); /* 比如，交给MODBUS解码程序处理字节流 */
				
			}
		}
	}

	/* 处理发送缓冲区空中断 */
	if ( ((isrflags & USART_ISR_TXE) != RESET) && (cr1its & USART_CR1_TXEIE) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* 发送缓冲区的数据已取完时， 禁止发送缓冲区空中断 （注意：此时最后1个数据还未真正发送完毕）*/
			CLEAR_BIT(_pUart->uart->CR1, USART_CR1_TXEIE);

			/* 使能数据发送完毕中断 */
			//USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
			SET_BIT(_pUart->uart->CR1, USART_CR1_TCIE);
		}
		else
		{
			_pUart->Sending = 1;
			
			/* 从发送FIFO取1个字节写入串口发送数据寄存器 */
			_pUart->uart->TDR = _pUart->pTxBuf[_pUart->usTxRead];
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}
	}
	
	/* 数据bit位全部发送完毕的中断 */
	if (((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* 如果发送FIFO的数据全部发送完毕，禁止数据发送完毕中断 */
			CLEAR_BIT(_pUart->uart->CR1, USART_CR1_TCIE);

			/* 回调函数, 一般用来处理RS485通信，将RS485芯片设置为接收模式，避免抢占总线 */
			if (_pUart->SendOver)
			{
				_pUart->SendOver();
			}
			
			_pUart->Sending = 0;
		}
		else
		{
			/* 正常情况下，不会进入此分支 */

			/* 如果发送FIFO的数据还未完毕，则从发送FIFO取1个数据写入发送数据寄存器 */
			_pUart->uart->TDR = _pUart->pTxBuf[_pUart->usTxRead];
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
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
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_CMF);
	SET_BIT(_pUart->uart->ICR, UART_CLEAR_WUF);
	
//	*            @arg UART_CLEAR_PEF: Parity Error Clear Flag
//  *            @arg UART_CLEAR_FEF: Framing Error Clear Flag
//  *            @arg UART_CLEAR_NEF: Noise detected Clear Flag
//  *            @arg UART_CLEAR_OREF: OverRun Error Clear Flag
//  *            @arg UART_CLEAR_IDLEF: IDLE line detected Clear Flag
//  *            @arg UART_CLEAR_TCF: Transmission Complete Clear Flag
//  *            @arg UART_CLEAR_LBDF: LIN Break Detection Clear Flag
//  *            @arg UART_CLEAR_CTSF: CTS Interrupt Clear Flag
//  *            @arg UART_CLEAR_RTOF: Receiver Time Out Clear Flag
//  *            @arg UART_CLEAR_CMF: Character Match Clear Flag
//  *            @arg.UART_CLEAR_WUF:  Wake Up from stop mode Clear Flag
//  *            @arg UART_CLEAR_TXFECF: TXFIFO empty Clear Flag	
}

void USART1_IRQHandler(void)
{
	UartIRQ(&g_Uart1);
	g_ShouldPowerOffLoraDelay = TIM2->CNT;
}


void USART2_IRQHandler(void)
{
	UartIRQ(&g_Uart2);
	g_ShouldPowerOffToolDelay = TIM2->CNT;
}

void LPUART1_IRQHandler(void)
{
	UartIRQ(&g_LPUart);
	g_ShouldPowerOffDTUDelay = TIM2->CNT;
}

/*
*********************************************************************************************************
*	函 数 名: RS485_InitTXE
*	功能说明: 配置RS485发送使能口线 TXE
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_InitTXE(void)
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
	
	RS485_RX_EN();
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
void RS485_SendBefor(void)
{
	RS485_TX_EN();	/* 切换RS485收发芯片为发送模式 */
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
void RS485_SendOver(void)
{
	RS485_RX_EN();	/* 切换RS485收发芯片为接收模式 */
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

/*
*********************************************************************************************************
*	函 数 名: SetUartParam
*	功能说明: 配置串口的硬件参数（波特率，数据位，停止位，起始位，校验位，中断使能）
*	形    参: Instance   USART_TypeDef类型结构体
*             BaudRate   波特率
*             Parity     校验类型，奇校验或者偶校验
*	返 回 值: 无
*********************************************************************************************************
*/
void SetUartParam(USART_TypeDef *Instance,  uint32_t BaudRate, uint32_t Parity)
{
	UART_HandleTypeDef UartHandle;	
	
	/*##-1- 配置串口硬件参数 ######################################*/
	/* 异步串口模式 (UART Mode) */
	/* 配置如下:
	  - 字长    = 8 位
	  - 停止位  = 1 个停止位
	  - 校验    = 参数Parity
	  - 波特率  = 参数BaudRate
	  - 硬件流控制关闭 (RTS and CTS signals) */

	UartHandle.Instance        = Instance;

	UartHandle.Init.BaudRate   = BaudRate;
	if(Instance == USART2)
		UartHandle.Init.WordLength = UART_WORDLENGTH_9B;
	else
		UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits   = UART_STOPBITS_1;
	UartHandle.Init.Parity     = Parity;
	UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode       = UART_MODE_TX_RX;
	HAL_UART_Init(&UartHandle);	
}

/*
*********************************************************************************************************
*	函 数 名: InitHardUart
*	功能说明: 配置串口的硬件参数（波特率，数据位，停止位，起始位，校验位，中断使能）适合于STM32-H7开发板
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitHardUart(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	
/* UART1 */
	/* 使能 GPIO TX/RX 时钟 */
	USART1_TX_GPIO_CLK_ENABLE();
	USART1_RX_GPIO_CLK_ENABLE();
	
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

	/* 配置NVIC the NVIC for UART */   
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
  
	/* 配置波特率、奇偶校验 */
	SetUartParam(USART1,  UART1_BAUD, UART_PARITY_NONE);

	SET_BIT(USART1->ICR, USART_ICR_TCCF);	/* 清除TC发送完成标志 */
	SET_BIT(USART1->RQR, USART_RQR_RXFRQ);  /* 清除RXNE接收标志 */
	SET_BIT(USART1->CR1, USART_CR1_RXNEIE);	/* 使能PE. RX接受中断 */
	TIM7Init(100-1, 8000-1);              	//重新设置为10ms中断	
	HAL_TIM_Base_Stop(&TIM7_Handler);       //关闭定时器7
	g_Usart1RxSta=0;				        //清零
	
/* UART2 */
	/* 使能 GPIO TX/RX 时钟 */
	USART2_TX_GPIO_CLK_ENABLE();
	USART2_RX_GPIO_CLK_ENABLE();
	
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

	/* 配置NVIC the NVIC for UART */   
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 2);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
  
	/* 配置波特率、奇偶校验 */
	SetUartParam(USART2,  UART2_BAUD, UART_PARITY_NONE);	

	SET_BIT(USART2->ICR, USART_ICR_TCCF);	/* 清除TC发送完成标志 */
	SET_BIT(USART2->RQR, USART_RQR_RXFRQ);/* 清除RXNE接收标志 */
	SET_BIT(USART2->CR1, USART_CR1_RXNEIE);	/* 使能PE. RX接受中断 */
	
/* LPUART1 */
	if(g_SysPar.type == RECIEVE && g_SysPar.no == 0x01)
	{
		/* 使能 GPIO TX/RX 时钟 */
		LPUART1_TX_GPIO_CLK_ENABLE();
		LPUART1_RX_GPIO_CLK_ENABLE();
		
		/* 使能 USARTx 时钟 */
		LPUART1_CLK_ENABLE();	

		/* 配置TX引脚 */
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

		/* 配置NVIC the NVIC for UART */   
		HAL_NVIC_SetPriority(LPUART1_IRQn, 0, 3);
		HAL_NVIC_EnableIRQ(LPUART1_IRQn);
	  
		/* 配置波特率、奇偶校验 */
		SetUartParam(LPUART1,  LPUART1_BAUD, UART_PARITY_NONE);

		SET_BIT(LPUART1->ICR, USART_ICR_TCCF);	/* 清除TC发送完成标志 */
		SET_BIT(LPUART1->RQR, USART_RQR_RXFRQ);  /* 清除RXNE接收标志 */
		SET_BIT(LPUART1->CR1, USART_CR1_RXNEIE);	/* 使能PE. RX接受中断 */
	}
}

void InitUart(void)
{
	UartVarInit();		/* 必须先初始化全局变量,再配置硬件 */

	InitHardUart();		/* 配置串口的硬件参数(波特率等) */

	RS485_InitTXE();	/* 配置RS485芯片的发送使能硬件，配置为推挽输出 */
}

