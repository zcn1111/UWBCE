#include "bsp.h"

uint32_t    g_DTUConnDelay;
uint32_t    g_Diff_DTUConnDelay;
uint8_t 	g_ResetPowerFlag;         //重启电源标识  1-重启485 2-重启系统 3-重启DTU		-99RTC时间连续错误重启
uint8_t 	g_AskPowerControlFlag;    //返回电源控制信息
bool 		g_ChangeMyMobileNumFlag;
uint8_t 	g_ChangeMobileNumStep;
uint8_t 	g_DataIPBack[4];      //数据IP备份
uint8_t 	g_DiffDataIPBack[4];  //差分数据IP备份
uint16_t 	g_DataPortBack;       //数据端口号备份
uint16_t 	g_DiffDataPortBack;   //差分数据端口号备份
uint8_t 	MajorVersion;                     //主版本号   
uint8_t 	MinorVersion;                     //次版本号  

void DTU_4G_Init(void)
{
	g_DTUConnDelay = TIM2->CNT;   //
}
