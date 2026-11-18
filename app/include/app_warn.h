#ifndef __APP_WARN_H
#define __APP_WARN_H

#include "stdint.h"
#include "stdbool.h"
#include "app_ly.h"
#include "bsp.h"
#include "app_glovar.h"



typedef struct {
    EFenceDataType     variable;     
	GPSPoint           fence[15];     /*电子围栏数据*/
    uint8_t            num;           /*每个区域有效点个数*/
	uint8_t            order[4];      //电子围栏序号
}EFValid;

typedef struct {
	//已经记录标志
	uint8_t RecordFlag;
}RecordUnit;

typedef struct {
	//1开机记录
	RecordUnit StartUpRecord;
	//2晚开机记录
	RecordUnit LateStartUpRecord;
}RecordData;

typedef struct {

	uint8_t ly;
	uint8_t cart;
	uint8_t sgp;
	uint8_t postionprotection;
	uint8_t status;
}YYMprg;

extern YYMprg     g_YYPlay;
extern RecordData g_RecordData;
extern EFValid g_EFenceValid[24];
extern uint8_t g_DtuOutage;
extern uint8_t g_YYPro;    //=0报区域和来车；=1报遥控器；=2报小车；=3报施工牌；=4报设备状态
extern uint32_t g_YYProDelay;
extern uint32_t g_YYProDelayOut;

extern uint8_t  g_NewGPSData;         /*收到新测试点数据标记*/	
extern uint8_t EFencesta;
extern uint8_t g_WarnYYFlag;
extern uint8_t ProtectionPositionFlag;


int is_point_in_polygon(GPSPoint point, GPSPoint polygon[], int num_vertices);
void JudgEFence(void);
void LedStatus(void) ;
void InitCalculateEndorsement(void);
void CalculateEndorsement(void);
void CalculateProtectionPosition(void);

#endif   


