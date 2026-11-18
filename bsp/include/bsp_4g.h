#ifndef __BSP_4g_H
#define __BSP_4g_H

#include "stm32l4xx.h"
#include "stdbool.h"

extern uint32_t g_DTUConnDelay;
extern uint32_t g_Diff_DTUConnDelay;
extern uint32_t g_ChargeSelfDTUDelay;
extern uint8_t  g_ResetPowerFlag;         //重启电源标识  1-重启485 2-重启系统 3-重启DTU		-99RTC时间连续错误重启
extern uint8_t  g_AskPowerControlFlag;    //返回电源控制信息
extern bool 	g_ChangeMyMobileNumFlag;
extern uint8_t 	g_ChangeMobileNumStep;
extern uint8_t 	g_DataIPBack[4];      //数据IP备份
extern uint16_t g_DataPortBack;      //数据端口号备份
extern uint8_t 	g_DiffDataIPBack[4];  //差分数据IP备份
extern uint16_t 	g_DiffDataPortBack;   //差分数据端口号备份
extern uint8_t  MajorVersion;                     //主版本号   
extern uint8_t  MinorVersion;                     //次版本号  

void DTU_4G_Init(void);

#endif



