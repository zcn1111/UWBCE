#ifndef __BSP_SELFCHECK_H
#define __BSP_SELFCHECK_H

#include "stm32l4xx.h"
#include "stdbool.h"

typedef struct
{
	//自检结果
	uint8_t   Result;
	//发出去命令等待延时
	uint32_t  Delay;
	//发出去命令后，多少次没回复
	uint8_t	  Num;
	//自检是否完成
	uint8_t   Finish;
}UnitSelfCheck;

typedef struct
{
	UnitSelfCheck	Dtu;	          //DTU
	UnitSelfCheck   BlueTooth;        //蓝牙
	UnitSelfCheck   Rtk;	          //定位
	UnitSelfCheck	Imu;	          //IMU
	UnitSelfCheck   Flash;            //FLASH
	uint32_t        StartSelfCheckTime;  //开始自检的时间
	uint8_t         EnterSelfCheck;   //进入自检标志
	uint8_t         StartCharge;      //进入充电
	uint8_t         FirstSelfCheck;   //第一次自检
	uint8_t         SelfCheckSuccess; //自检完成
	uint32_t        PowerOnTime;
	uint8_t         Mode485;          //进入485模式
} SelfCheck;

extern SelfCheck g_SelfCheck;
extern uint32_t g_SendChargeSelfTime;

void ChargeInit(void);
void SelfCheckProcess(void);
uint8_t IsStartCharge(void);
void WorkSelfCheckProcess(void);
#endif


