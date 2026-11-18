#ifndef __APP_WORKPLAN_H
#define __APP_WORKPLAN_H

#include "stdint.h"
#include "bsp.h"
#include "app_glovar.h"

typedef struct
{
	uint8_t WorkPlanID[4];
	uint8_t FlashIndex;
	uint8_t OrderNumber;
    uint16_t Crc16;
}WorkPlanRecord_t;

extern WorkPlanRecord_t g_WorkPlanRecord;

typedef struct
{
	//施工开始时间
	DATETIME start;
	//施工结束时间
	DATETIME stop;
	//经度
	double lat;
	//纬度
    double lng;
	//施工计划ID
	uint32_t WorkPlanID;
	//是否对电子围栏数据解析成功
	uint8_t isanalysis;
}WorkPlanSummary_t;

extern WorkPlanSummary_t g_WorkPlanSummary[MAX_WORK_PLAN_NUM];

uint8_t IsWorkPlanPerfectly(uint8_t FlashIndex);
void UpdateWorkPlanSummary(uint8_t FlashIndex);
int8_t QueryFreeFlash(void);
void CleanWorkPlanFlash(uint8_t FlashIndex);
void WriteWorkPlanRecordToFlash(void);
void ReadWorkPlanRecordFromFlash(void);
void CleanWorkPlanRecordFromFlash(void);
void DeleteWorkPlanSummary(uint8_t FlashIndex);
void InitWorkPlan(void);
void WriteWorkPlanToFlash(uint8_t FlashIndex, uint8_t *Data, uint16_t DataLen, uint8_t PackageOrderNumber);
//void DeleteWorkPlan(uint8_t FlashIndex);
void DeleteWorkPlan(uint32_t WorkPlanID);
void DeleteALLWorkPlan(void);
void ModWorkPlan(uint32_t WorkPlanID);
void WorkPlanProcess(void);
void CleanWorkPlanSummary(void);
void CleanEFenceValid(void);
void CleanWorkTime(void);
void PrintWorkPlanSummary(void);
void WriteModWorkPlanToFlash(uint8_t *Data, uint16_t DataLen, uint8_t PackageOrderNumber);
void CleanModWorkPlanFlash(void);
uint8_t IsModWorkPlanPerfectly(void);
uint8_t AskFlashWorkplan(uint8_t FlashIndex, uint8_t *DataBuff);
void AskSummaryWorkplan(uint8_t FlashIndex, uint8_t *DataBuff);
#endif