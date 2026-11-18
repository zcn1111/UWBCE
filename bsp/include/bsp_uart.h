/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_UART_H
#define __BSP_UART_H

#include "stm32l4xx.h"

#define debug_printf(format, args...)	printf(format, ##args);
//#define debug_printf(format, args...)	;

/* 串口1的GPIO  PB6, PB7   客服工具 */
#define USART1_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE()

#define USART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define USART1_TX_GPIO_PORT              GPIOB
#define USART1_TX_PIN                    GPIO_PIN_6
#define USART1_TX_AF                     GPIO_AF7_USART1

#define USART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define USART1_RX_GPIO_PORT              GPIOB
#define USART1_RX_PIN                    GPIO_PIN_7
#define USART1_RX_AF                     GPIO_AF7_USART1

/* 串口2的GPIO --- PA2 PA3  激光雷达  */
#define USART2_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE()

#define USART2_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART2_TX_GPIO_PORT              GPIOA
#define USART2_TX_PIN                    GPIO_PIN_2
#define USART2_TX_AF                     GPIO_AF7_USART2

#define USART2_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART2_RX_GPIO_PORT              GPIOA
#define USART2_RX_PIN                    GPIO_PIN_3
#define USART2_RX_AF                     GPIO_AF7_USART2

/* 串口3的GPIO --- PC4 PC5  激光雷达  */
#define USART3_CLK_ENABLE()              __HAL_RCC_USART3_CLK_ENABLE()

#define USART3_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define USART3_TX_GPIO_PORT              GPIOC
#define USART3_TX_PIN                    GPIO_PIN_4
#define USART3_TX_AF                     GPIO_AF7_USART3

#define USART3_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define USART3_RX_GPIO_PORT              GPIOC
#define USART3_RX_PIN                    GPIO_PIN_5
#define USART3_RX_AF                     GPIO_AF7_USART3

/* 低功耗串口1的GPIO --- PB10 PB11  雨量计 */
#define LPUARTDMA							0

#define LPUART1_CLK_ENABLE()              __HAL_RCC_LPUART1_CLK_ENABLE()

#define LPUART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define LPUART1_TX_GPIO_PORT              GPIOB
#define LPUART1_TX_PIN                    GPIO_PIN_11
#define LPUART1_TX_AF                     GPIO_AF8_LPUART1

#define LPUART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define LPUART1_RX_GPIO_PORT              GPIOB
#define LPUART1_RX_PIN                    GPIO_PIN_10
#define LPUART1_RX_AF                     GPIO_AF8_LPUART1


#define UART1_TOOL_BAUD	  	345600
#define UART1_BT_BAUD	  	115200
#define UART1_TX_BUF_SIZE 	800
#define UART1_RX_BUF_SIZE 	530

#define UART2_BAUD		  	115200
//#define UART2_TX_BUF_SIZE 	256
#define UART2_RX_BUF_SIZE 	3072

#define UART3_BAUD		  	115200
//#define UART3_TX_BUF_SIZE 	800
#define UART3_RX_BUF_SIZE 	3072

#define LPUART1_BAUD		115200
//#define LPUART_TX_BUF_SIZE 	2048
#define LPUART_RX_BUF_SIZE 	1024

#define TOOL485_RX_EN()	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);
#define TOOL485_TX_EN()	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);

/* 串口环形缓存位置指针 */
typedef struct
{
	uint8_t		*Buf;	//缓存变量指针
	uint16_t	Head;	//头位置
	uint16_t	Tail;	//尾位置
	uint16_t	Size;	//队列长度
} RingBufPos_t;
/* 入队列模式 */
#define RING_PUSHMODE_STOP		0	//队列满后停止入队列
#define RING_PUSHMODE_CYCLE		1	//队列满后丢弃旧数据


/* 串口设备结构体 */
typedef struct
{
	USART_TypeDef *uart;		/* STM32内部串口设备指针 */
	uint8_t *pRxBuf;			/* 接收缓冲区 */
	uint16_t usTxBufSize;		/* 发送缓冲区大小 */
	uint16_t usRxBufSize;		/* 接收缓冲区大小 */
	uint32_t usTxTimer;			/* 发送完成计时   */
	
	uint16_t usRxWrite;	/* 接收缓冲区写指针 */
	uint16_t usRxRead;		/* 接收缓冲区读指针 */
	uint16_t usRxCount;	/* 还未读取的新数据个数 */
	
	void (*SendBefor)(void); 	/* 开始发送之前的回调函数指针（用于RS485切换到发送模式） */
	void (*SendOver)(void); 	/* 发送完毕的回调函数指针（用于RS485将发送模式切换为接收模式） */
	void (*ReciveNew)(void);	/* 串口收到数据的回调函数指针 */
	uint8_t Sending;			/* 正在发送中 */
}UART_T;

extern UART_T g_Uart1;
extern uint16_t g_TxBuf1[UART1_TX_BUF_SIZE];		/* 发送缓冲区 */
extern uint8_t g_SecTxBuf1[UART1_TX_BUF_SIZE];		/* 发送缓冲区 */
extern uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];		    /* 接收缓冲区 */
extern uint8_t g_DMARxBuf1[UART1_RX_BUF_SIZE];		/* 接收缓冲区 */
extern uint32_t g_Uart1RxTimer;
extern uint8_t g_Uart1DirFlag;						//485方向标记 1-接收 2-发送

extern UART_T g_Uart2;
//extern uint8_t g_TxBuf2[UART2_TX_BUF_SIZE];		    /* 发送缓冲区 */
extern uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];		    /* 接收缓冲区 */
extern uint8_t g_DMARxBuf2[UART2_RX_BUF_SIZE];	    /* 接收缓冲区 */
extern uint32_t g_Uart2RxTimer;

extern UART_T g_Uart3;
//extern uint8_t g_TxBuf3[UART3_TX_BUF_SIZE];		    /* 发送缓冲区 */
extern uint8_t g_RxBuf3[UART3_RX_BUF_SIZE];		    /* 接收缓冲区 */
extern uint32_t g_Uart3RxTimer;
extern	RingBufPos_t	g_Uart3RxRing;		        //UART3环形缓存参数

extern UART_T g_LPUart;
//extern uint8_t g_TxBufLP[LPUART_TX_BUF_SIZE];		/* 发送缓冲区 */
extern uint8_t g_RxBufLP[LPUART_RX_BUF_SIZE];		/* 接收缓冲区 */
extern uint8_t g_DMARxBufLP[LPUART_RX_BUF_SIZE];	/* 接收缓冲区 */
extern uint32_t g_LPUartRxTimer;
extern uint8_t g_LPUart1DirFlag;

extern UART_HandleTypeDef UART1_Handler;       //UART句柄
extern UART_HandleTypeDef UART2_Handler;       //UART句柄
extern UART_HandleTypeDef UART3_Handler;       //UART句柄
extern UART_HandleTypeDef LPUART1_Handler;     //UART句柄

extern uint8_t g_Usart1RxSta;
extern uint8_t g_Usart2RxSta;
extern uint8_t g_Usart3RxSta;
extern uint8_t g_LPUart1RxSta;

void MX_DMA_Init(void);
void InitUart(void);

void Tool485_SendBefor(void);
void Tool485_SendOver(void);
void Select_485(void);
void Select_uart(void);

void Uart1VarInit(void);
void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen);
void ReadRecBuf(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen);
void InitUSART1(void);
void InitChargeUSART1(void);
void InitLPUART1(uint32_t bound);
void InitUSART2(uint32_t bound);
void InitUSART3(uint32_t bound);
void RxDMA_Config(DMA_TypeDef *DMA, DMA_Channel_TypeDef *DMA_Channel, uint32_t seq);

uint16_t GetRingUsedLen(RingBufPos_t *Ring);
void RingBufClear(RingBufPos_t *Ring);
uint16_t RingBufPop(RingBufPos_t *Ring, uint8_t *OutBuf, uint16_t Size);

void debug_xinfo(uint8_t *Info, uint16_t Len, uint8_t Width);

#endif /* __BSP_UART_H */
