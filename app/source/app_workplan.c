#include "app_workplan.h"
#include "app_warn.h"

uint32_t Time = 0;
uint32_t WorkPlanProcessTime = 0;
WorkPlanRecord_t g_WorkPlanRecord;
WorkPlanSummary_t g_WorkPlanSummary[MAX_WORK_PLAN_NUM];

//清除falsh中施工计划
void CleanWorkPlanFlash(uint8_t FlashIndex)
{
	W25qxx_EraseSector((FLASH_EFENCE_FLASH_START / 4096) + FlashIndex);
}

//清除暂存修改falsh中施工计划
void CleanModWorkPlanFlash(void)
{
	W25qxx_EraseSector((FLASH_MOD_EFENCE_FLASH_START / 4096));
}

//从flash中删除所有的施工计划
void DeleteALLWorkPlan(void)
{
	uint8_t i = 0;
	for(i = 0; i < MAX_WORK_PLAN_NUM; i++)
	{
		W25qxx_EraseSector(FLASH_EFENCE_FLASH_START / 4096 + i);
	}
}

//写施工计划记录到flash中
void WriteWorkPlanRecordToFlash(void)
{
	uint16_t crc16;	
	
	W25qxx_EraseSector(FLASH_WORK_PLAN_RECORD_START / 4096);
	
	crc16 = CalcCRC16((uint8_t*)&g_WorkPlanRecord, sizeof(g_WorkPlanRecord) - 2);
	g_WorkPlanRecord.Crc16 = crc16;
	FlashWriteData((uint8_t*)&g_WorkPlanRecord, FLASH_WORK_PLAN_RECORD_START, sizeof(g_WorkPlanRecord));	
}

//从flash中读取施工计划记录
void ReadWorkPlanRecordFromFlash(void)
{	
	W25qxx_ReadBytes((uint8_t*)&g_WorkPlanRecord, FLASH_WORK_PLAN_RECORD_START, sizeof(g_WorkPlanRecord));
	
	if (g_WorkPlanRecord.Crc16 != CalcCRC16((uint8_t*)&g_WorkPlanRecord, sizeof(g_WorkPlanRecord) - 2))
	{
		memset((uint8_t*)&g_WorkPlanRecord, 0xFF, sizeof(g_WorkPlanRecord));
        g_WorkPlanRecord.Crc16 = CalcCRC16((uint8_t*)&g_WorkPlanRecord, sizeof(g_WorkPlanRecord) - 2);
		WriteWorkPlanRecordToFlash();
	}
}

//从flash中擦除施工计划记录
void CleanWorkPlanRecordFromFlash(void)
{
	W25qxx_EraseSector(FLASH_WORK_PLAN_RECORD_START / 4096);
}

//删除施工计划概要
void DeleteWorkPlanSummary(uint8_t FlashIndex)
{
	memset((uint8_t *)&g_WorkPlanSummary[FlashIndex], 0x00, sizeof(g_WorkPlanSummary[FlashIndex]));
}

//从flash中取出数据然后更新施工计划概要
void UpdateWorkPlanSummary(uint8_t FlashIndex)
{
	uint32_t StartAddr = 0;
	uint8_t FlashData[FLASH_EFENCE_FLASH_LEN];
	uint8_t Offset = 32;
	
	StartAddr = FLASH_EFENCE_FLASH_START + FLASH_EFENCE_FLASH_LEN * FlashIndex;
	DeleteWorkPlanSummary(FlashIndex);
	g_WorkPlanSummary[FlashIndex].isanalysis = 0;
	memset((uint8_t *)FlashData, 0x00, sizeof(FlashData));
	FlashReadData(FlashData, StartAddr, FLASH_EFENCE_FLASH_LEN, 0);
	
	if ((FlashData[Offset + 0] != 0X5A) && (FlashData[Offset + 1] != 0XA5))
	{
		g_WorkPlanSummary[FlashIndex].isanalysis = 0;
		
		return;
	}
	
	//取到长度
	uint16_t DataLen = (FlashData[Offset + 5] << 8) + FlashData[Offset + 4];
	//长度判断(这里没有判断加上区域的长度)
	if (DataLen < 34)
	{
		g_WorkPlanSummary[FlashIndex].isanalysis = 0;
		
		return;
	}
	//校验和验证CheckSum
	uint8_t calc_sum = CheckSum(DataLen + 6, FlashData + Offset);
	if(calc_sum != FlashData[Offset + DataLen + 6]) 
	{
		return ;
	}
	
	//获取电子围栏中心点和开始结束时间
	//解析施工中心点
	g_WorkPlanSummary[FlashIndex].lat = (double)(FlashData[Offset + 6] | (FlashData[Offset + 7]<<8) | 
			(FlashData[Offset + 8]<<16) | (FlashData[Offset + 9]<<24)) / 10000000;
	g_WorkPlanSummary[FlashIndex].lng = (double)(FlashData[Offset + 10] | (FlashData[Offset + 11]<<8) | 
			(FlashData[Offset + 12]<<16) | (FlashData[Offset + 13]<<24)) / 10000000;
	//解析施工时间
	memcpy(&g_WorkPlanSummary[FlashIndex].start, FlashData + Offset + 14, 6);
	memcpy(&g_WorkPlanSummary[FlashIndex].stop, FlashData + Offset + 20, 6);
	
	g_WorkPlanSummary[FlashIndex].WorkPlanID = ConvArrayToUint32(FlashData, 3, M_BIG_ENDIAN);
	
	g_WorkPlanSummary[FlashIndex].isanalysis = 1;
	
	return;
}

//找到空闲的flash
int8_t QueryFreeFlash(void)
{
	for (int8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
	{
		if (g_WorkPlanSummary[i].isanalysis == 0)
		{
			return i;
		}
	}
	
	return -1;
}

//删除施工计划
void DeleteWorkPlan(uint32_t WorkPlanID)
{
	uint32_t CurrentWorkPlanID = 0;
	
	for (uint8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
	{
		if (g_WorkPlanSummary[i].WorkPlanID == WorkPlanID)
		{
			memset((uint8_t *)&g_WorkPlanSummary[i], 0x00, sizeof(WorkPlanSummary_t));
			CleanWorkPlanFlash(i);
		}
	}
	
	CurrentWorkPlanID = ConvArrayToUint32(SelectEFenceInfo.order, 0, M_BIG_ENDIAN);
	if (WorkPlanID == CurrentWorkPlanID)
	{
		memset((uint8_t *)&g_EFenceValid,0,sizeof(g_EFenceValid));
	}
}

//修改施工计划
void ModWorkPlan(uint32_t WorkPlanID)
{
	uint8_t ModFlag = 0;
	uint32_t CurrentWorkPlanID = 0;
	
	//删除掉旧的施工计划
	for (uint8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
	{
		if (g_WorkPlanSummary[i].WorkPlanID == WorkPlanID)
		{
			memset((uint8_t *)&g_WorkPlanSummary[i], 0x00, sizeof(WorkPlanSummary_t));
			CleanWorkPlanFlash(i);
	
			uint8_t FlashData[FLASH_EFENCE_FLASH_LEN];
	
			memset((uint8_t *)FlashData, 0x00, sizeof(FlashData));
			FlashReadData(FlashData, FLASH_MOD_EFENCE_FLASH_START, FLASH_EFENCE_FLASH_LEN, 0);
			
			FlashWriteData((uint8_t*)&FlashData, FLASH_EFENCE_FLASH_START + FLASH_EFENCE_FLASH_LEN * i, sizeof(FlashData));
			
			UpdateWorkPlanSummary(i);
			
			ModFlag = 1;
		}
	}
	
	if (ModFlag == 0)
	{
		int8_t FlashIndex = QueryFreeFlash();
		if (FlashIndex != -1)
		{
			uint8_t FlashData[FLASH_EFENCE_FLASH_LEN];
	
			memset((uint8_t *)FlashData, 0x00, sizeof(FlashData));
			FlashReadData(FlashData, FLASH_MOD_EFENCE_FLASH_START, FLASH_EFENCE_FLASH_LEN, 0);
			
			CleanWorkPlanFlash(FlashIndex);
			FlashWriteData((uint8_t*)&FlashData, FLASH_EFENCE_FLASH_START + FLASH_EFENCE_FLASH_LEN * FlashIndex, sizeof(FlashData));
			UpdateWorkPlanSummary(FlashIndex);
			
			return; 
		}
	}
	
	CurrentWorkPlanID = ConvArrayToUint32(SelectEFenceInfo.order, 0, M_BIG_ENDIAN);
	if (WorkPlanID == CurrentWorkPlanID)
	{
		memset((uint8_t *)&g_EFenceValid, 0x00 ,sizeof(g_EFenceValid));
	}
}

//判断这条施工计划是否完整
uint8_t IsWorkPlanPerfectly(uint8_t FlashIndex)
{
	uint32_t SatartAddr = 0;
	uint8_t FlashData[FLASH_EFENCE_FLASH_LEN];
	uint8_t PrePackageOrder = 0;
	
	memset((uint8_t *)FlashData, 0x00, sizeof(FlashData));
	
	SatartAddr = FLASH_EFENCE_FLASH_START + FLASH_EFENCE_FLASH_LEN * FlashIndex;
	FlashReadData(FlashData, SatartAddr, FLASH_EFENCE_FLASH_LEN, 0);
	
	for (uint16_t i = 0; i < FLASH_EFENCE_FLASH_LEN; i++)
	{
		if (((i + 38) > FLASH_EFENCE_FLASH_LEN))
		{
			continue;
		}
		
		if ((FlashData[i] != 0XFA) || (FlashData[i + 1] != 0X00) || (FlashData[i + 2] != 0X04))
		{
			continue;
		}
		
		//数据总长度
		uint16_t Totaldatalen = (FlashData[i + 31]<<8) + FlashData[i + 30];
		
		if ((FlashData[i + 32] != 0X5A) || (FlashData[i + 33] != 0XA5))
		{
			continue;
		}
		
		//取到长度
		uint16_t DataLen = (FlashData[i + 37] << 8) + FlashData[i + 36];
		//长度判断(这里没有判断加上区域的长度)
		if ((DataLen < 26) || (DataLen > (Totaldatalen - 4)))
		{	
			continue;
		}
		
		//校验和验证
		uint8_t calc_sum = CheckSum(DataLen + 6, FlashData + 32 + i );
		if(calc_sum != FlashData[i + DataLen + 38]) 
		{	
			continue ;
		}
		
		//取出这是第几包
		uint8_t PackageOrder = FlashData[i + 35];
		if ((PrePackageOrder + 1) != PackageOrder)
		{
			//不是一个完整的施工计划
			return 1;
		}
		//上一个的施工计划包数自增
		PrePackageOrder++;
		
		//取出总包数
		uint8_t PackageTotal = FlashData[i + 34];
		if (PackageTotal == PackageOrder)
		{
			return 0;
		}
	}
	
	return 1;
}

//判断这条施工计划是否完整
uint8_t IsModWorkPlanPerfectly(void)
{
	uint32_t SatartAddr = 0;
	uint8_t FlashData[FLASH_EFENCE_FLASH_LEN];
	uint8_t PrePackageOrder = 0;
	
	memset((uint8_t *)FlashData, 0x00, sizeof(FlashData));
	
	SatartAddr = FLASH_MOD_EFENCE_FLASH_START;
	FlashReadData(FlashData, SatartAddr, FLASH_EFENCE_FLASH_LEN, 0);
	
	for (uint16_t i = 0; i < FLASH_EFENCE_FLASH_LEN; i++)
	{
		if (((i + 38) > FLASH_EFENCE_FLASH_LEN))
		{
			continue;
		}
		
		if ((FlashData[i] != 0XFA) || (FlashData[i + 1] != 0X00) || (FlashData[i + 2] != 0X04))
		{
			continue;
		}
		
		//数据总长度
		uint16_t Totaldatalen = (FlashData[i + 31]<<8) + FlashData[i + 30];
		
		if ((FlashData[i + 32] != 0X5A) || (FlashData[i + 33] != 0XA5))
		{
			continue;
		}
		
		//取到长度
		uint16_t DataLen = (FlashData[i + 37] << 8) + FlashData[i + 36];
		//长度判断(这里没有判断加上区域的长度)
		if ((DataLen < 26) || (DataLen > (Totaldatalen - 4)))
		{	
			continue;
		}
		
		//校验和验证
		uint8_t calc_sum = CheckSum(DataLen + 6, FlashData + 32 + i );
		if(calc_sum != FlashData[i + DataLen + 38]) 
		{	
			continue ;
		}
		
		//取出这是第几包
		uint8_t PackageOrder = FlashData[i + 35];
		if ((PrePackageOrder + 1) != PackageOrder)
		{
			//不是一个完整的施工计划
			return 1;
		}
		//上一个的施工计划包数自增
		PrePackageOrder++;
		
		//取出总包数
		uint8_t PackageTotal = FlashData[i + 34];
		if (PackageTotal == PackageOrder)
		{
			return 0;
		}
	}
	
	return 1;
}

void InitWorkPlan(void)
{
	uint32_t SatartAddr = 0;
	
	for (uint8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
	{
		DeleteWorkPlanSummary(i);
	}
	
	for (uint8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
	{
		//判断这条施工计划是否完整
		if (IsWorkPlanPerfectly(i) == 0)
		{
			UpdateWorkPlanSummary(i);
		}
	}
	
	memset(&SelectEFenceInfo.order,0xFF,sizeof(SelectEFenceInfo.order));
}

//将接收到施工计划写入到flash中
void WriteWorkPlanToFlash(uint8_t FlashIndex, uint8_t *Data, uint16_t DataLen, uint8_t PackageOrderNumber)
{
	uint32_t StartAddr = 0;
	
	StartAddr = FLASH_EFENCE_FLASH_START + FLASH_EFENCE_FLASH_LEN * FlashIndex + 1089 * (PackageOrderNumber - 1);
		
	//将施工计划写入flash中
	W25qxx_WritePages(Data, StartAddr, DataLen);
}

//将接收到修改的施工计划写入到flash中
void WriteModWorkPlanToFlash(uint8_t *Data, uint16_t DataLen, uint8_t PackageOrderNumber)
{
	uint32_t StartAddr = 0;
	
	StartAddr = FLASH_MOD_EFENCE_FLASH_START + 1089 * (PackageOrderNumber - 1);
		
	//将施工计划写入flash中
	W25qxx_WritePages(Data, StartAddr, DataLen);
}

//删除计划结束1小时后的施工计划
void WorkPlanProcess(void)
{
	//1分钟检查一次
	if(JudgeSomeDelayInProOk(WorkPlanProcessTime,Delay1Min) == true)
	{		
		for (uint8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
		{
			if ((JudgTimeOrder(g_SysTime,g_WorkPlanSummary[i].stop,MIN_60) == AFTER) && 
					(g_WorkPlanSummary[i].isanalysis == 1)) 
			{
				memset((uint8_t *)&g_WorkPlanSummary[i], 0x00, sizeof(WorkPlanSummary_t));
				CleanWorkPlanFlash(i);
			}
		}
		
		WorkPlanProcessTime = TIM2->CNT;			
	}
}

void CleanWorkPlanSummary(void)
{	
	for (uint8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
	{
		memset((uint8_t *)&g_WorkPlanSummary[i], 0x00, sizeof(WorkPlanSummary_t));
	}
}	

void CleanEFenceValid(void)
{
	for (uint8_t i = 0; i < 24; i++)
	{
		memset((uint8_t *)&g_EFenceValid[i], 0x00, sizeof(EFValid));
	}
}

void CleanWorkTime(void)
{
	memset(&g_WorkTime, 0x00, sizeof(g_WorkTime));
}

uint8_t AskFlashWorkplan(uint8_t FlashIndex, uint8_t *DataBuff)
{
	uint32_t StartAddr = 0;
	uint8_t FlashData[FLASH_EFENCE_FLASH_LEN];
	uint8_t Offset = 32;
	
	StartAddr = FLASH_EFENCE_FLASH_START + FLASH_EFENCE_FLASH_LEN * FlashIndex;
	memset((uint8_t *)FlashData, 0x00, sizeof(FlashData));
	FlashReadData(FlashData, StartAddr, FLASH_EFENCE_FLASH_LEN, 0);
	
	if ((FlashData[Offset + 0] != 0X5A) && (FlashData[Offset + 1] != 0XA5))
	{	
		return 0;
	}
	
	//取到长度
	uint16_t DataLen = (FlashData[Offset + 5] << 8) + FlashData[Offset + 4];
	//长度判断(这里没有判断加上区域的长度)
	if (DataLen < 34)
	{	
		return 0;
	}
	//校验和验证CheckSum
	uint8_t calc_sum = CheckSum(DataLen + 6, FlashData + Offset);
	if(calc_sum != FlashData[Offset + DataLen + 6]) 
	{
		return 0;
	}
	
	//施工计划ID
	memcpy(DataBuff, FlashData + 3, 4);
	//开始时间
	memcpy(DataBuff + 4, FlashData + Offset + 14, 6);
	//结束时间
	memcpy(DataBuff + 10, FlashData + Offset + 20, 6);
	//纬度
	memcpy(DataBuff + 16, FlashData + Offset + 6, 4);
	//经度
	memcpy(DataBuff + 20, FlashData + Offset + 10, 4);
	//预留
	memset(DataBuff + 24, 0x00, 8);
	
	return 1;
}

void AskSummaryWorkplan(uint8_t FlashIndex, uint8_t *DataBuff)
{
	uint32_t lat = 0;
	uint32_t lng = 0;
	
	//施工计划ID
	ConvUint32ToArray(g_WorkPlanSummary[FlashIndex].WorkPlanID, DataBuff, M_BIG_ENDIAN);
	//开始时间
	memcpy(DataBuff + 4, &g_WorkPlanSummary[FlashIndex].start, 6);
	//结束时间
	memcpy(DataBuff + 10, &g_WorkPlanSummary[FlashIndex].stop, 6);
	//纬度
	lat = g_WorkPlanSummary[FlashIndex].lat * 10000000;
	DataBuff[16] = (uint8_t)(lat & 0xFF);
	DataBuff[17] = (uint8_t)((lat >> 8) & 0xFF);
	DataBuff[18] = (uint8_t)((lat >> 16) & 0xFF);
	DataBuff[19] = (uint8_t)((lat >> 24) & 0xFF);
	//经度
	lng = g_WorkPlanSummary[FlashIndex].lng * 10000000;
	DataBuff[20] = (uint8_t)(lng & 0xFF);
	DataBuff[21] = (uint8_t)((lng >> 8) & 0xFF);
	DataBuff[22] = (uint8_t)((lng >> 16) & 0xFF);
	DataBuff[23] = (uint8_t)((lng >> 24) & 0xFF);
	//预留
	memset(DataBuff + 24, 0x00, 8);
}

void PrintWorkPlanSummary(void)
{
	if (TIM2->CNT - Time >= 1 * 100000)
	{
		for (uint8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
		{
			if (g_WorkPlanSummary[i].isanalysis != 0)
			{
				uint8_t WorkPlanID[4];
			
				ConvUint32ToArray(g_WorkPlanSummary[i].WorkPlanID, WorkPlanID, M_BIG_ENDIAN);
			
				UartSend(&g_Uart1,WorkPlanID,4);
				UartSend(&g_Uart1,(uint8_t[]){0x0D, 0x0A},2);
			}
		}
		
		Time = TIM2->CNT;
	}
}