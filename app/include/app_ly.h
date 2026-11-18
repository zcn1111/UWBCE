#ifndef __APP_LY_H
#define __APP_LY_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "app_4G.h"
#include "4G_Comm.h"

#define LYSendLocalTime    Delay1Sec
extern uint32_t	g_LYSendLocalDelay;        /*给蓝牙发送定位数据的间隔时间*/
extern uint8_t ReceivelLYData;
extern uint32_t g_ReceivelLYDataTime;
extern uint8_t g_LYLink;
void LYParam_Init(void);
void LYProcess(void);
void RecLYData(uint8_t *bPtr,int32_t len);
uint8_t ChargeSelfBlueTooth(void);


/*****定位卡信息**********/
void RYDW_InitCardTable(CardInfoTable_t *CardTable);
void RYDW_UpdateCardInfoTable(CardData_t NowCardData, uint8_t RecvCnt, CardInfoTable_t *CardTable);
void RYDW_CheckCardOffline(uint8_t SendCnt, CardInfoTable_t *CardTable);

#endif



