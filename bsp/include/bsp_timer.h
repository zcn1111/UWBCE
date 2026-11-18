/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx.h"
#include "bsp_rtc.h"

#define Delay100uSec	10
#define Delay1mSec 		100				//1*100
#define Delay10mSec 	1000			//10*100
#define Delay20mSec 	2000			//20*100
#define Delay50mSec 	5000			//50*100
#define Delay75mSec 	7500			//75*100
#define Delay100mSec 	10000			//100*100
#define Delay200mSec 	20000			//200*100
#define Delay300mSec 	30000			//300*100
#define Delay500mSec 	50000			//500*100
#define Delay700mSec 	70000			//700*100
#define Delay1Sec 		100000			//1*1000*100
#define Delay2Sec		200000			//2*1000*100
#define Delay3Sec		300000			//3*1000*100
#define Delay4Sec		400000			//4*1000*100
#define Delay4P5Sec		450000
#define Delay5Sec 		500000
#define Delay5P5Sec 	550000
#define Delay6Sec 		600000
#define Delay7Sec 		700000
#define Delay10Sec 		1000000
#define Delay13Sec 		1300000
#define Delay15Sec 		1500000
#define Delay17Sec 		1700000
#define Delay20Sec		2000000
#define Delay30Sec 		3000000
#define Delay1Min		6000000
#define Delay1p5Min 	9000000
#define Delay2Min 		12000000
#define Delay3Min 		18000000
#define Delay5Min  		30000000
#define Delay10Min 		60000000
#define Delay20Min 		120000000
#define Delay30Min 		180000000
#define Delay60Min 		360000000

#define  PERIOD_VALUE       (uint32_t)(1000 - 1)  /* Period Value  */
#define  PULSE1_VALUE       (uint32_t)(PERIOD_VALUE/2)        /* Capture Compare 1 Value  */
#define  PULSE2_VALUE       (uint32_t)(PERIOD_VALUE*37.5/100) /* Capture Compare 2 Value  */
#define  PULSE3_VALUE       (uint32_t)(PERIOD_VALUE/4)        /* Capture Compare 3 Value  */
#define  PULSE4_VALUE       (uint32_t)(PERIOD_VALUE*12.5/100) /* Capture Compare 4 Value  */

/******* 数据结构体定义 *******/
/* 定时器运行流程状态 */
typedef enum
{
	TICKTIMER_STOPPED	= 0,	/* 0 = 定时器已停止 */
	TICKTIMER_RUNNNING	= 1,	/* 1 = 定时器正在运行 */
	TICKTIMER_TIMEOUT	= 2,	/* 2 = 定时时间到 */
	TICKTIMER_OVERMAX	= 3,	/* 3 = 超过重新开始次数 */
} TimerRunProg_e;

/* 滴答定时器标记参数 */
typedef struct
{
	uint8_t Actived:1;			//b0:定时器激活 0=不活动;1=活动
	uint8_t AutoRestart:1;		//b1:自动重开始 0=关闭;1=自动重开始
	uint8_t :4;					//b2-b5:预留
	uint8_t ReStartMaxFlag:1;	//b6:定时器超过最大重启次数
	uint8_t TimeOutFlag:1;		//b7:定时时间到标记
} TimerFlag_t;

/* 滴答定时器参数 */
typedef struct
{
	TimerFlag_t		TimerFlag;		//定时器标记状态
	TimerRunProg_e	TimerProg;		//定时器流程状态
	uint32_t		StartTickVal;	//起始滴答值
	uint32_t		DelayTick;		//定时滴答值
	
	uint8_t			RestartCnt;		//重开始次数
	uint8_t			RestartMax;		//最大重开始次数
	
	volatile	uint32_t	*BaseTick;			//定时器对应滴答时基

	void		(*cbTimeoutFunc)(void);			//定时时间到回调函数
	void		(*cbOverMaxFunc)(void);			//超过最大重开始次数回调函数
} TickTimerPara_t;

// 定义返回值的枚举类型  
typedef enum {  
    BEFORE,  // 第一个时间在第二个时间之前  
    AFTER,   // 第一个时间在第二个时间之后  
    SAME     // 两个时间相同  
} TimeComparison; 
typedef struct 
{
  	uint8_t year;			//时间：年
	uint8_t month;			//时间：月
	uint8_t date;			//时间：日
	uint8_t hour;			//时间：时
	uint8_t minute;			//时间：分
	uint8_t second;			//时间：秒
}DATETIME;
extern DATETIME g_SysTime;
extern DATETIME g_BCDTime;
extern TIM_HandleTypeDef TIM6_Handler;      //定时器句柄 
extern TIM_HandleTypeDef TIM7_Handler;      //定时器句柄 
extern TIM_HandleTypeDef TIM2_Handler;
extern uint8_t g_CalHourFlag;
extern uint8_t g_CalMinuteFlag;

extern uint32_t tmr50cnt;

void delay_init(uint8_t SYSCLK);
void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);

void InitTimer2(void);
void TIM6Init(uint16_t arr, uint16_t psc);
void TIM7Init(uint16_t arr, uint16_t psc);
bool JudgeSomeDelayInProOk(uint32_t Timerval,uint32_t DelayN);
void InitTimer1PWM(uint32_t _ulFreq, uint32_t _ulDutyCycle);
TimeComparison JudgTimeOrder(DATETIME time1,DATETIME time2,uint32_t LapseTime);
void convertToUTC8(DATETIME *time);
bool JudgeTick(uint32_t Timerval,uint32_t DelayN);

#endif /* __BSP_TIMER_H */
