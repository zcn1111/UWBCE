#include "bsp.h"
#include "main.h"

// 定义电子围栏的边界点（多边形）
//GPSPoint fence[] = {
//						{37.80445587,112.55434637},  // 左下角
//						{37.80450052,112.55435107},  // 右下角
//						{37.80449979,112.55394583},  // 右上角
//						{37.80445456,112.55396019}   // 左上角
//                   };
EFValid g_EFenceValid[24];	   
uint8_t  EFencesta = 0;		
uint32_t  g_GetAreaTypeTime = 0;
uint8_t g_DtuOutage = 0;
uint8_t g_EFenceFlag = 0;
RecordData g_RecordData;
uint32_t g_BatteryLowerTime = 0;
uint8_t g_PreAreaTypeAlarm = 0;
uint8_t g_WarnYYFlag = 0;

uint8_t g_YYPro;    //=0报区域和来车；=1报遥控器；=2报小车；=3报施工牌；=4报设备状态
uint32_t g_YYProDelay;
YYMprg   g_YYPlay;
uint32_t g_YYProDelayOut;
uint8_t  g_CartBroNum =0;
uint8_t  g_lyBroNum =0;
uint8_t  g_SGPBroNum =0;
uint8_t  g_StatusBroNum =0;
uint8_t  g_PostionprotectionBroNum = 0;
uint8_t ProtectionPositionFlag = 0;
/**************************************************
* @brief	检查点是否在多边形内部
*
* @param 	point：待检测的点。
*           polygon：多边形的顶点数组。
*           num_vertices：多边形顶点的数量
*
* @return  c 布尔标志，初始值为0。用于记录射线与多边形边界的交点数
*            如果 c 为1，表示交点数为奇数，点在多边形内部。
*            如果 c 为0，表示交点数为偶数，点在多边形外部
**************************************************/
int is_point_in_polygon(GPSPoint point, GPSPoint polygon[], int num_vertices)
{
    int i, j, c = 0;
    for (i = 0, j = num_vertices - 1; i < num_vertices; j = i++) {
        if (((polygon[i].latitude > point.latitude) != (polygon[j].latitude > point.latitude)) &&
            (point.longitude < (polygon[j].longitude - polygon[i].longitude) * 
             (point.latitude - polygon[i].latitude) / 
             (polygon[j].latitude - polygon[i].latitude) + polygon[i].longitude)) {
            c = !c;
        }
    }
    return c;
}

void SetAlarm(void)
{
	//1作业安全区
	if(g_AreaType == 1)
	{
		//清除进入危险区
		g_CarWarnFlag.bit.Risk = 0x00;
		g_CarWarnFlag.bit.KeepAwayWorkArea = 0x00;
		g_CarWarnFlag.bit.KeepAwaySafeArea = 0x00;
		g_CarWarnFlag.bit.CrossAdjacentLine = 0x00;
						
		g_CarWarnFlag.bit.TrainApproach = 0X00;
						
		//提前上道
		if(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE)
		{
			//置位提前上道
			g_CarWarnFlag.bit.Ahead = 0x01;
		}
		//施工期间
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_LAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			if(g_CarWarnFlag.bit.TimeOut == 1)
			{
				g_CarWarnFlag.bit.TimeOut = 0x00;                            
			}
		}
		//超时施工
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_STOPLAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			//置位超时施工
			g_CarWarnFlag.bit.TimeOut = 0x01;
		}
	}
	//2作业区
	else if(g_AreaType == 2)
	{
		//清除进入危险区
		g_CarWarnFlag.bit.Risk = 0x00;
		g_CarWarnFlag.bit.KeepAwayWorkArea = 0x00;
		g_CarWarnFlag.bit.KeepAwaySafeArea = 0x00;
		g_CarWarnFlag.bit.CrossAdjacentLine = 0x00;
						
		//列车接近
		if ((g_TrainInfo.TrainApproach == 1) || (g_TrainInfo.AppTrainApproach == 1))
		{
			g_CarWarnFlag.bit.TrainApproach = 0X01;
		}
		else
		{
			g_CarWarnFlag.bit.TrainApproach = 0X00;
		}
						
		//提前上道
		if(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE)
		{
			//置位提前上道
			g_CarWarnFlag.bit.Ahead = 0x01;
		}
		//施工期间
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_LAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			if(g_CarWarnFlag.bit.TimeOut == 1)
			{
				g_CarWarnFlag.bit.TimeOut = 0x00;                            
			}
		}
		//超时施工
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_STOPLAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			//置位超时施工
			g_CarWarnFlag.bit.TimeOut = 0x01;
		}
	}
	//3非作业区
	else if(g_AreaType == 3)
	{
		//置位进入危险区
		g_CarWarnFlag.bit.Risk = 0x01;
		g_CarWarnFlag.bit.KeepAwayWorkArea = 0x00;
		g_CarWarnFlag.bit.KeepAwaySafeArea = 0x00;
						
		//列车接近
		if ((g_TrainInfo.TrainApproach == 1) || (g_TrainInfo.AppTrainApproach == 1))
		{
			g_CarWarnFlag.bit.TrainApproach = 0X01;
		}
		else
		{
			g_CarWarnFlag.bit.TrainApproach = 0X00;
		}
						
		//提前上道
		if(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE)
		{
			//置位提前上道
			g_CarWarnFlag.bit.Ahead = 0x01;
		}
		//施工期间
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_LAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			if(g_CarWarnFlag.bit.TimeOut == 1)
			{
				g_CarWarnFlag.bit.TimeOut = 0x00;                            
			}
		}
		//超时施工
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_STOPLAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			//置位超时施工
			g_CarWarnFlag.bit.TimeOut = 0x01;
		}
	}
	//4远离作业区
	else if(g_AreaType == 4)
	{
		//清除进入危险区
		g_CarWarnFlag.bit.Risk = 0x00;
		g_CarWarnFlag.bit.KeepAwayWorkArea = 0x01;
		g_CarWarnFlag.bit.KeepAwaySafeArea = 0x00;
		g_CarWarnFlag.bit.CrossAdjacentLine = 0x00;
						
		//列车接近
		if ((g_TrainInfo.TrainApproach == 1) || (g_TrainInfo.AppTrainApproach == 1))
		{
			g_CarWarnFlag.bit.TrainApproach = 0X01;
		}
		else
		{
			g_CarWarnFlag.bit.TrainApproach = 0X00;
		}
						
		//提前上道
		if(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE)
		{
			//置位提前上道
			g_CarWarnFlag.bit.Ahead = 0x01;
		}
		//施工期间
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_LAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			if(g_CarWarnFlag.bit.TimeOut == 1)
			{
				g_CarWarnFlag.bit.TimeOut = 0x00;                            
			}
		}
		//超时施工
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_STOPLAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			//置位超时施工
			g_CarWarnFlag.bit.TimeOut = 0x01;
		}
	}
	//5邻线安全区
	else if(g_AreaType == 5)
	{
		if (g_CarWarnFlag.bit.Risk == 0x01)
		{
			g_CarWarnFlag.bit.CrossAdjacentLine = 0x01;
		}
						
		//清除进入危险区
		g_CarWarnFlag.bit.Risk = 0x00;
		g_CarWarnFlag.bit.KeepAwayWorkArea = 0x00;
		g_CarWarnFlag.bit.KeepAwaySafeArea = 0x00;
						
		g_CarWarnFlag.bit.TrainApproach = 0X00;
						
		//提前上道
		if(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE)
		{
			//置位提前上道
			g_CarWarnFlag.bit.Ahead = 0x01;
		}
		//施工期间
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_LAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			if(g_CarWarnFlag.bit.TimeOut == 1)
			{
				g_CarWarnFlag.bit.TimeOut = 0x00;                            
			}
		}
		//超时施工
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_STOPLAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			//置位超时施工
			g_CarWarnFlag.bit.TimeOut = 0x01;
		}
	}
	//6相邻安全区
	else if(g_AreaType == 6)
	{
		//清除进入危险区
		g_CarWarnFlag.bit.Risk = 0x00;
		g_CarWarnFlag.bit.KeepAwayWorkArea = 0x00;
		g_CarWarnFlag.bit.KeepAwaySafeArea = 0x00;
		g_CarWarnFlag.bit.CrossAdjacentLine = 0x00;
						
		g_CarWarnFlag.bit.TrainApproach = 0X00;
						
		//提前上道
		if(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE)
		{
			//置位提前上道
			g_CarWarnFlag.bit.Ahead = 0x01;
		}
		//施工期间
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_LAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			if(g_CarWarnFlag.bit.TimeOut == 1)
			{
				g_CarWarnFlag.bit.TimeOut = 0x00;                            
			}
		}
		//超时施工
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_STOPLAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			//置位超时施工
			g_CarWarnFlag.bit.TimeOut = 0x01;
		}
	}
	//7远离安全区
	else if(g_AreaType == 7)
	{
		//清除进入危险区
		g_CarWarnFlag.bit.Risk = 0x00;
		g_CarWarnFlag.bit.KeepAwayWorkArea = 0x00;
		g_CarWarnFlag.bit.KeepAwaySafeArea = 0x01;
		g_CarWarnFlag.bit.CrossAdjacentLine = 0x00;
						
		//列车接近
		if ((g_TrainInfo.TrainApproach == 1) || (g_TrainInfo.AppTrainApproach == 1))
		{
			g_CarWarnFlag.bit.TrainApproach = 0X01;
		}
		else
		{
			g_CarWarnFlag.bit.TrainApproach = 0X00;
		}
						
		//提前上道
		if(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE)
		{
			//置位提前上道
			g_CarWarnFlag.bit.Ahead = 0x01;
		}
		//施工期间
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_LAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			if(g_CarWarnFlag.bit.TimeOut == 1)
			{
				g_CarWarnFlag.bit.TimeOut = 0x00;                            
			}
		}
		//超时施工
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_STOPLAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			//置位超时施工
			g_CarWarnFlag.bit.TimeOut = 0x01;
		}
	}
	//8走行区
	else if(g_AreaType == 8)
	{
		//清除进入危险区
		g_CarWarnFlag.bit.Risk = 0x00;
		g_CarWarnFlag.bit.KeepAwayWorkArea = 0x00;
		g_CarWarnFlag.bit.KeepAwaySafeArea = 0x00;
		g_CarWarnFlag.bit.CrossAdjacentLine = 0x00;
						
		//列车接近
		if ((g_TrainInfo.TrainApproach == 1) || (g_TrainInfo.AppTrainApproach == 1))
		{
			g_CarWarnFlag.bit.TrainApproach = 0X01;
		}
		else
		{
			g_CarWarnFlag.bit.TrainApproach = 0X00;
		}
						
		//提前上道
		if(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE)
		{
			//置位提前上道
			g_CarWarnFlag.bit.Ahead = 0x01;
		}
		//施工期间
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_LAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			if(g_CarWarnFlag.bit.TimeOut == 1)
			{
				g_CarWarnFlag.bit.TimeOut = 0x00;                            
			}
		}
			//超时施工
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_STOPLAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			//置位超时施工
			g_CarWarnFlag.bit.TimeOut = 0x01;
		}
	}
	//9相邻作业区
	else if(g_AreaType == 9)
	{
		//清除进入危险区
		g_CarWarnFlag.bit.Risk = 0x00;
		g_CarWarnFlag.bit.KeepAwayWorkArea = 0x00;
		g_CarWarnFlag.bit.KeepAwaySafeArea = 0x00;
		g_CarWarnFlag.bit.CrossAdjacentLine = 0x00;
						
		//列车接近
		if ((g_TrainInfo.TrainApproach == 1) || (g_TrainInfo.AppTrainApproach == 1))
		{
			g_CarWarnFlag.bit.TrainApproach = 0X01;
		}
		else
		{
			g_CarWarnFlag.bit.TrainApproach = 0X00;
		}
						
		//提前上道
		if(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE)
		{
			//置位提前上道
			g_CarWarnFlag.bit.Ahead = 0x01;
		}
		//施工期间
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_LAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			if(g_CarWarnFlag.bit.TimeOut == 1)
			{
				g_CarWarnFlag.bit.TimeOut = 0x00;                            
			}
		}
		//超时施工
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_STOPLAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			//置位超时施工
			g_CarWarnFlag.bit.TimeOut = 0x01;
		}
	}
	//10缓冲区
	else if(g_AreaType == 10)
	{
		//清除进入危险区
		g_CarWarnFlag.bit.Risk = 0x00;
		g_CarWarnFlag.bit.KeepAwayWorkArea = 0x00;
		g_CarWarnFlag.bit.KeepAwaySafeArea = 0x00;
		g_CarWarnFlag.bit.CrossAdjacentLine = 0x00;
						
		//列车接近
		if ((g_TrainInfo.TrainApproach == 1) || (g_TrainInfo.AppTrainApproach == 1))
		{
			g_CarWarnFlag.bit.TrainApproach = 0X01;
		}
		else
		{
			g_CarWarnFlag.bit.TrainApproach = 0X00;
		}
						
		//提前上道
		if(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE)
		{
			//置位提前上道
			g_CarWarnFlag.bit.Ahead = 0x01;
		}
		//施工期间
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_LAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			if(g_CarWarnFlag.bit.TimeOut == 1)
			{
				g_CarWarnFlag.bit.TimeOut = 0x00;                            
			}
		}
		//超时施工
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_STOPLAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			//置位超时施工
			g_CarWarnFlag.bit.TimeOut = 0x01;
		}
	}
	//11相邻缓冲区
	else if(g_AreaType == 11)
	{
		//清除进入危险区
		g_CarWarnFlag.bit.Risk = 0x00;
		g_CarWarnFlag.bit.KeepAwayWorkArea = 0x00;
		g_CarWarnFlag.bit.KeepAwaySafeArea = 0x00;
		g_CarWarnFlag.bit.CrossAdjacentLine = 0x00;
						
		//列车接近
		if ((g_TrainInfo.TrainApproach == 1) || (g_TrainInfo.AppTrainApproach == 1))
		{
			g_CarWarnFlag.bit.TrainApproach = 0X01;
		}
		else
		{
			g_CarWarnFlag.bit.TrainApproach = 0X00;
		}
						
		//提前上道
		if(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE)
		{
			//置位提前上道
			g_CarWarnFlag.bit.Ahead = 0x01;
		}
		//施工期间
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_LAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			if(g_CarWarnFlag.bit.TimeOut == 1)
			{
				g_CarWarnFlag.bit.TimeOut = 0x00;                            
			}
		}
		//超时施工
		else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,_STOPLAPSETIME) == BEFORE)
		{
			if(g_CarWarnFlag.bit.Ahead == 1)
			{
				g_CarWarnFlag.bit.Ahead = 0x00;
			}
			//置位超时施工
			g_CarWarnFlag.bit.TimeOut = 0x01;
		}
	}
}

/**************************************************
* @brief	检查电子围栏是否需要报警
*
* @param 	
*
* @return  
**************************************************/
void JudgEFence(void)
{
	uint8_t  i=0;
	static uint32_t PreCalaTime;
	static uint8_t PreAreaType = 0;
	static uint8_t Count = 0;
	static uint8_t AreaTypeValue[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	
	//大区域由服务器计算
	if (SelectEFenceInfo.IsBigArea == 1)
	{
		if (TIM2->CNT - PreCalaTime > 50000)
		{
			if ((BigAreaTypeValue[0] == BigAreaTypeValue[1]) &&
				(BigAreaTypeValue[0] == BigAreaTypeValue[2]))
			{
				g_AreaType = BigAreaTypeValue[0];
				
				if (g_AreaType > 0)
				{
					if ((g_AreaType == 5) && (PreAreaType == 3))
					{
						g_CarWarnFlag.bit.CrossAdjacentLine = 0x01;
					}
			
					SetAlarm();	
				}
				else if (g_AreaType == 0)
				{
					g_AreaTypeAlarm = 0;
					g_CarWarnFlag.all = 0x00;
				}
			
				PreAreaType = g_AreaType;
			}
			
			PreCalaTime = TIM2->CNT;
		}
			
		//10S没有收到服务器下发下来的数据
		if (JudgeSomeDelayInProOk(g_ReceiveAreaTime, Delay20Sec) == true)
		{
			g_AreaType = 0;
			g_AreaTypeAlarm = 0;
			PreAreaType = 0;
			g_IssueWorkPlan = 0;
			PreWorkPlanID = 0;
			g_CarWarnFlag.all = 0X00;
			g_TrainInfo.TrainApproach = 0;
			g_TrainInfo.AppTrainApproach = 0;
			memset(&g_WorkTime,0,sizeof(g_WorkTime));
			memset(BigAreaTypeValue, 0, sizeof(BigAreaTypeValue));
			g_ReceiveAreaTime = TIM2->CNT;
		}
	}
		
	if(g_NewGPSData == 1)
	{
		//出来浮点解或者固定解后再进行区域判断
		if((g_PosQuality == 4) || (g_PosQuality == 5))
		{	
			for(i = 0; i < SelectEFenceInfo.total; i++)    /*几个区域*/
			{
				if(is_point_in_polygon(polygonbuff,g_EFenceValid[i].fence,g_EFenceValid[i].num) == 1)          /*判断是否在电子围栏区域里面*/ 
				{
					//小区域时定位卡自己计算
					if (SelectEFenceInfo.IsBigArea == 0)
					{
						AreaTypeValue[Count] = g_EFenceValid[i].variable.AreaType;
						g_AreaTypeAlarm = g_EFenceValid[i].variable.AreaType;
					
						if (Count >= 8)
						{
							Count = 0;
						}
						else 
						{
							Count++;
						}
					
						if ((AreaTypeValue[0] == AreaTypeValue[1]) && 
							(AreaTypeValue[0] == AreaTypeValue[2]) &&
							(AreaTypeValue[0] == AreaTypeValue[3]) &&
							(AreaTypeValue[0] == AreaTypeValue[4]) &&
							(AreaTypeValue[0] == AreaTypeValue[5]) &&
							(AreaTypeValue[0] == AreaTypeValue[6]) &&
							(AreaTypeValue[0] == AreaTypeValue[7]) &&
							(AreaTypeValue[0] == AreaTypeValue[8]))
						{
							if ((g_AreaType == 5) && (AreaTypeValue[0] == 3))
							{
								g_CarWarnFlag.bit.CrossAdjacentLine = 0x01;
							}
						
							//区域类型
							g_AreaType = AreaTypeValue[0];
						}
						else 
						{
							return;
						}
						
						SetAlarm();
						
						g_BigAreaType = 0;
					}
					//大区域由服务器计算
					if (SelectEFenceInfo.IsBigArea == 1)
					{
						g_BigAreaType = g_EFenceValid[i].variable.AreaType;
					}

					g_EFenceFlag = 1;	
				}
			}
			
			if (g_EFenceFlag == 1)
			{
				g_EFenceFlag = 0;
			}
			else if (g_EFenceFlag == 0)
			{
				g_AreaType = 0;
				g_AreaTypeAlarm = 0;
				g_BigAreaType = 0;
				g_CarWarnFlag.all = 0x00;
			}
		}
		else
		{
			//区域类型置0
			g_AreaType = 0;
			g_AreaTypeAlarm = 0;
			g_BigAreaType = 0;
			g_CarWarnFlag.all = 0x00;
		}
		
		g_NewGPSData = 0;
	}
}

//计算违章记录初始化
void InitCalculateEndorsement(void)
{
	//1开机记录
	g_RecordData.StartUpRecord.RecordFlag = 0;
	//2晚开机记录
	g_RecordData.LateStartUpRecord.RecordFlag = 0;
}

//计算违章记录
void CalculateEndorsement(void)
{	
	if((g_PosQuality == 4) || (g_PosQuality == 5))
	{
		//1记录第一次开机记录
		if (g_RecordData.StartUpRecord.RecordFlag <= 10)
		{
			g_CarWarnFlag.bit.StartUp = 0X01;
			//g_RecordData.StartUpRecord.RecordFlag++;
		}
		else 
		{
			g_CarWarnFlag.bit.StartUp = 0X00;
		}
		
		//2记录晚开机的时间记录
		if ((JudgTimeOrder(g_WorkTime.start,g_SysTime,0) == BEFORE) && 
					(g_WorkTime.start.year != 0) && (g_RecordData.LateStartUpRecord.RecordFlag <= 10))
		{
			g_CarWarnFlag.bit.LateStartUp = 0X01;
			//g_RecordData.LateStartUpRecord.RecordFlag++;
		}
		else
		{
			g_CarWarnFlag.bit.LateStartUp = 0X00;
		}
		
		//3记录提前上线的时间记录(施工未开始且在所有安全区)
		if ((JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE) && (g_WorkTime.start.year != 0) &&
				((g_AreaType == 1) || (g_AreaType == 5) || (g_AreaType == 6)))
		{
			g_CarWarnFlag.bit.PreOnLine = 0X01;
		}
		else if (g_AreaType == 0)
		{
			g_CarWarnFlag.bit.PreOnLine = 0X00;
		}
		
		//4记录提前上道开始的时间记录(施工未开始且进入非安全区)
		if ((JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE) && (g_WorkTime.start.year != 0) &&
				((g_AreaType == 2) || (g_AreaType == 3) || (g_AreaType == 4) || (g_AreaType == 7) ||
				(g_AreaType == 8) || (g_AreaType == 9) || (g_AreaType == 10) || (g_AreaType == 11)))
		{
			g_CarWarnFlag.bit.PreUpperTrackStart = 0X01;
		}
		//5记录提前上道结束的时间记录(施工未开始且由非安全区到安全区或者出去)
		else if ((JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE) && (g_WorkTime.start.year != 0) &&
				((g_AreaType == 1) || (g_AreaType == 5) || (g_AreaType == 6) || (g_AreaType == 0)))
		{
			g_CarWarnFlag.bit.PreUpperTrackStart = 0X00;
		}
		
		//6记录晚下线开始的时间记录(施工结束时间到后，还在安全区)
		if ((JudgTimeOrder(g_SysTime,g_WorkTime.stop,0) == AFTER) && (g_WorkTime.start.year != 0) &&
				((g_AreaType == 1) || (g_AreaType == 5) || (g_AreaType == 6)))
		{
			g_CarWarnFlag.bit.LateOffLineStart = 0X01;
		}
		//7记录晚下线结束的时间记录(施工结束时间到后，由安全区进入别的区或者出去的时间)
		else if ((JudgTimeOrder(g_SysTime,g_WorkTime.stop,0) == AFTER) && (g_WorkTime.start.year != 0) &&
				((g_AreaType == 2) || (g_AreaType == 3) || (g_AreaType == 4) ||(g_AreaType == 7) ||
				(g_AreaType == 8) || (g_AreaType == 9) || (g_AreaType == 10) || (g_AreaType == 11) || (g_AreaType == 0)))
		{
			g_CarWarnFlag.bit.LateOffLineStart = 0X00;
		}
		
		//8记录晚下道开始记录的时间记录(施工结束时间到后，在非安全区)
		if ((JudgTimeOrder(g_SysTime,g_WorkTime.stop,0) == AFTER) && (g_WorkTime.start.year != 0) &&
				((g_AreaType == 2) || (g_AreaType == 3) || (g_AreaType == 4) || (g_AreaType == 7) || 
				(g_AreaType == 8) || (g_AreaType == 9) || (g_AreaType == 10) || (g_AreaType == 11)))
		{
			g_CarWarnFlag.bit.LateLowerTrackStart = 0X01;
		}
		//9记录晚下道结束记录的时间记录(施工结束时间到后，从非安全区到安全区或者外面)
		else if ((JudgTimeOrder(g_SysTime,g_WorkTime.stop,0) == AFTER) && (g_WorkTime.start.year != 0) &&
				((g_AreaType == 1) || (g_AreaType == 5) || (g_AreaType == 6) || (g_AreaType == 0)))
		{
			g_CarWarnFlag.bit.LateLowerTrackStart = 0X00;
		}
	}
	
	//下一次施工开始前
	if (JudgTimeOrder(g_WorkTime.start,g_SysTime,0) == AFTER)
	{
//		g_RecordData.LateStartUpRecord.RecordFlag = 0;
		g_CarWarnFlag.bit.LateOffLineStart = 0X00;
		g_CarWarnFlag.bit.LateLowerTrackStart = 0X00;
	}
	
	//施工开始后
	if ((JudgTimeOrder(g_SysTime, g_WorkTime.start, 0) == SAME) || 
			(JudgTimeOrder(g_SysTime, g_WorkTime.start, 0) == AFTER))
	{
		g_CarWarnFlag.bit.PreUpperTrackStart = 0X00;
		g_CarWarnFlag.bit.PreOnLine = 0X00;
	}
}

/*
*********************************************************************************************************
*	函 数 名: LedStatus
*	功能说明: LED状态
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LedStatus(void) 
{
	uint8_t g_completed = 0;
	if (g_AreaTypeAlarm != g_PreAreaTypeAlarm)
	{
		g_YYPro = 0x00;
		g_WarnYYFlag = 1;
		
		g_PreAreaTypeAlarm = g_AreaTypeAlarm;
	}
	switch(g_YYPro)
	{
		case 0x00: //判断来车报警和区域报警
			
			if ((g_WorkTime.start.year !=0 ) && (g_WorkTime.start.month != 0) && (g_PosQuality > 3) && (g_AreaTypeAlarm > 0))
			{
				if ((JudgeSomeDelayInProOk(g_WarnYYDelay,Delay7Sec) == true) || (g_WarnYYFlag == 1))
				{	
					if ((JudgeSomeDelayInProOk(g_YYProDelay,g_YYProDelayOut) == true) || (g_WarnYYFlag == 1))
					{					
						g_WarnYYFlag = 0;
						
						//作业时间
						if (((JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == SAME) || 
								(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == AFTER)) && 
								((JudgTimeOrder(g_SysTime,g_WorkTime.stop,0) == SAME) || 
								(JudgTimeOrder(g_SysTime,g_WorkTime.stop,0) == BEFORE)))
						{
							//上行或者下行列车接近
							if(g_TrainInfo.AppTrainApproach == 1)
							{//报语音"来车下道"
								WT588E_PLAY(LCXD);
							}				
							else if (g_TrainInfo.TrainApproach == 1)
							{
								//语音
								//报语音"准备下道"
								if (AreaAlarmInfo[g_AreaTypeAlarm].voice.trainnear == 1)
								{
									WT588E_PLAY(ZBXD);
								}
								//报语音"尽快下道"
								else if (AreaAlarmInfo[g_AreaTypeAlarm].voice.trainnear == 2)
								{
									WT588E_PLAY(JKXD);
								}
								//报来车语音
								else if (((AreaAlarmInfo[g_AreaTypeAlarm].voice.trainnear == 3) || (AreaAlarmInfo[g_AreaType].voice.trainnear == 4)))
								{
									WT588E_PLAY(LCXD);
								}
								//报语音"远离作业区"
								else if (AreaAlarmInfo[g_AreaTypeAlarm].voice.trainnear == 5)
								{
									WT588E_PLAY(YLZYQ);
								}
								//报语音"靠近邻线"
								else if (AreaAlarmInfo[g_AreaTypeAlarm].voice.trainnear == 6)
								{
									WT588E_PLAY(KJLX);
								}
								//报语音"进入邻线"
								else if (AreaAlarmInfo[g_AreaTypeAlarm].voice.trainnear == 7)
								{
									WT588E_PLAY(JRLX);
								}
							}
							//无列车
							else 
							{	
								//语音
								//报语音"准备下道"
								if (AreaAlarmInfo[g_AreaTypeAlarm].voice.notrain == 1)
								{
									WT588E_PLAY(ZBXD);
								}
								//报语音"尽快下道"
								else if (AreaAlarmInfo[g_AreaTypeAlarm].voice.notrain == 2)
								{
									WT588E_PLAY(JKXD);
								}
								//报语音"远离作业区"
								else if (AreaAlarmInfo[g_AreaTypeAlarm].voice.notrain == 5)
								{
									WT588E_PLAY(YLZYQ);
								}
								//报语音"靠近邻线"
								else if (AreaAlarmInfo[g_AreaTypeAlarm].voice.notrain == 6)
								{
									WT588E_PLAY(KJLX);
								}
								//报语音"进入邻线"
								else if (AreaAlarmInfo[g_AreaTypeAlarm].voice.notrain == 7)
								{
									WT588E_PLAY(JRLX);
								}									
							}
						}	
						//非作业时间
						else
						{	
							//上行或者下行列车接近
							if(g_TrainInfo.AppTrainApproach == 1)
							{//报语音"来车下道"
								WT588E_PLAY(LCXD);
							}					
							else if (g_TrainInfo.TrainApproach == 1)
							{
								//语音
								//报语音"准备下道"
								if (AreaAlarmInfo[g_AreaTypeAlarm].voice.trainnear == 1)
								{
									WT588E_PLAY(ZBXD);
								}
								//报语音"尽快下道"
								else if (AreaAlarmInfo[g_AreaTypeAlarm].voice.trainnear == 2)
								{
									WT588E_PLAY(JKXD);
								}
								//报来车语音
								else if (((AreaAlarmInfo[g_AreaTypeAlarm].voice.trainnear == 3) || (AreaAlarmInfo[g_AreaType].voice.trainnear == 4)))
								{
									//报语音"来车下道"
									WT588E_PLAY(LCXD);
								}
								//报语音"远离作业区"
								else if (AreaAlarmInfo[g_AreaTypeAlarm].voice.trainnear == 5)
								{
									WT588E_PLAY(YLZYQ);
								}
								//报语音"靠近邻线"
								else if (AreaAlarmInfo[g_AreaTypeAlarm].voice.trainnear == 6)
								{
									WT588E_PLAY(KJLX);
								}
								//报语音"进入邻线"
								else if (AreaAlarmInfo[g_AreaTypeAlarm].voice.trainnear == 7)
								{
									WT588E_PLAY(JRLX);
								}
							}
							//判断出在区域里时，再进行报警
							else if (g_AreaTypeAlarm > 0)
							{
								//提前上道
								if(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == BEFORE)
								{
									WT588E_PLAY(TQSD);
								}
								//超时施工
								else if (JudgTimeOrder(g_SysTime,g_WorkTime.stop,0) == AFTER)
								{
									WT588E_PLAY(CSSG);
								}									
							}
						}
						
						g_WarnYYDelay = TIM2->CNT;
						g_YYProDelay = TIM2->CNT;
						g_YYProDelayOut = Delay7Sec;
					}
				}
				else
				{
					g_YYPro = 0x01;
				}				
			}
			else
			{
				g_YYPro = 0x01;
			}
			
			break;
		case 0x01: //判断是否播报遥控器语音
			if(g_YYPlay.ly != 0x00)
			{
				if (JudgeSomeDelayInProOk(g_YYProDelay,g_YYProDelayOut) == true)
				{					
					if(g_lyBroNum == 0 && g_completed == 0)
					{					
						if (g_YYPlay.ly & 0x01)
						{
							WT588E_PLAY(XXJC);
							g_YYPlay.ly &=0xFE;
							g_completed = 1;
						}
						g_lyBroNum++;
					}						
					if(g_lyBroNum == 1 && g_completed == 0)
					{					
						if (g_YYPlay.ly & 0x02)
						{
							WT588E_PLAY(SXJC);
							g_YYPlay.ly &=0xFD;
							g_completed = 1;
						}
						g_lyBroNum++;
					}	
					if(g_lyBroNum == 2 && g_completed == 0)
					{					
						if (g_YYPlay.ly & 0x04)
						{
							WT588E_PLAY(XXYG);
							g_YYPlay.ly &=0xFB;
							g_completed = 1;
						}
						g_lyBroNum++;
					}	
					if(g_lyBroNum == 3 && g_completed == 0)
					{					
						if (g_YYPlay.ly & 0x08)
						{
							WT588E_PLAY(SXYG);
							g_YYPlay.ly &=0xF7;
						}
						g_lyBroNum =0;
						g_YYPro = 0x02;	
					}	
					g_YYProDelay = TIM2->CNT;
					g_YYProDelayOut = Delay3Sec;
				}
			}
			else
			{
				g_YYPro = 0x02;	
			}
			break;			
		case 0x02: //判断是否播报小车信息
			if (g_YYPlay.cart > 0)
			{
				if (JudgeSomeDelayInProOk(g_YYProDelay,g_YYProDelayOut) == true)
				{				
					if(g_CartBroNum == 0 && g_completed == 0)
					{
						if (g_YYPlay.cart & 0x01)
						{
							WT588E_PLAY(XCCFW);
							g_YYPlay.cart &=0xFE;
							g_completed = 1;
						}
						g_CartBroNum++;
					}
					if(g_CartBroNum ==1 && g_completed == 0)
					{
						if (g_YYPlay.cart & 0x02)
						{
							WT588E_PLAY(XCHBBF);
							g_YYPlay.cart &=0xFD;
							g_completed = 1;
						}
						g_CartBroNum++;				
					}
					if(g_CartBroNum ==2 && g_completed == 0)
					{
						if (g_YYPlay.cart & 0x04)
						{
							WT588E_PLAY(XCTQSD);
							g_YYPlay.cart &=0xFB;
							g_completed = 1;
						} 	
						g_CartBroNum++;				
					}
					if(g_CartBroNum ==3 && g_completed == 0)
					{
						if (g_YYPlay.cart & 0x08)
						{
							WT588E_PLAY(XCCSSG);
							g_YYPlay.cart &=0xF7;
							g_completed = 1;
						} 
						g_CartBroNum++;				
					}			
					if(g_CartBroNum ==4 && g_completed == 0)
					{
						if (g_YYPlay.cart & 0x10)
						{
							WT588E_PLAY(XCCFWCSJ);
							g_YYPlay.cart &=0xEF;
							g_completed = 1;
						} 
						g_CartBroNum++;				
					}				
					if(g_CartBroNum ==5 && g_completed == 0)
					{
						if (g_YYPlay.cart & 0x20)
						{
							WT588E_PLAY(XCZC);
							g_YYPlay.cart &=0xDE;
						}
						g_CartBroNum=0;
						g_YYPro = 0x03;						
					}		
					g_YYProDelay = TIM2->CNT;
					g_YYProDelayOut = Delay4Sec;
				}					
			}
			else
			{
				g_YYPro = 0x03;	
			}		
			break;
		case 0x03: //判断是否播报远离防护信息
			if (g_YYPlay.postionprotection > 0)
			{
				if (JudgeSomeDelayInProOk(g_YYProDelay,g_YYProDelayOut) == true)
				{				
					if(g_PostionprotectionBroNum ==0 && g_completed == 0)
					{
						if (g_YYPlay.postionprotection & 0x01)
						{
							WT588E_PLAY(YLFH);
							g_YYPlay.postionprotection &=0xFE;
						}
						g_PostionprotectionBroNum=0;
						g_YYPro = 0x04;						
					}		
					g_YYProDelay = TIM2->CNT;
					g_YYProDelayOut = Delay4Sec;
				}					
			}
			else
			{
				g_YYPro = 0x04;	
			}		
			break;
		case 0x04: //判断是否播报施工牌
			if (g_YYPlay.sgp > 0)
			{
				if (JudgeSomeDelayInProOk(g_YYProDelay,g_YYProDelayOut) == true)
				{
					if(g_SGPBroNum == 0 && g_completed == 0)
					{
						if (g_YYPlay.sgp & 0x01)
						{
							WT588E_PLAY(SGPQX);
							g_YYPlay.sgp &=0xFE;
							g_completed = 1;
						} 						
						g_SGPBroNum++;
					}						
					if(g_SGPBroNum == 1 && g_completed == 0)
					{
						if (g_YYPlay.sgp & 0x02)
						{
							WT588E_PLAY(SGPQD);
							g_YYPlay.sgp &=0xFD;
							g_completed = 1;
						} 						
						g_SGPBroNum++;
					}						
					if(g_SGPBroNum == 2 && g_completed == 0)
					{
						if (g_YYPlay.sgp & 0x04)
						{
							WT588E_PLAY(SGPWZCW);
							g_YYPlay.sgp &=0xFB;
							g_completed = 1;
						} 	 						
						g_SGPBroNum++;
					}						
					if(g_SGPBroNum == 3 && g_completed == 0)
					{
						if (g_YYPlay.sgp & 0x08)
						{
							WT588E_PLAY(SGPWB);
							g_YYPlay.sgp &=0xF7;
						}						
						g_SGPBroNum=0;
						g_YYPro = 0x05;	
					}						
					g_YYProDelay = TIM2->CNT;
					g_YYProDelayOut = Delay3Sec;
				}					
			}
			else
			{
				g_YYPro = 0x05;	
			}		
			break;			
		case 0x05: //判断是否播报设备状态
			if (g_YYPlay.status > 0)
			{
				if (JudgeSomeDelayInProOk(g_YYProDelay,g_YYProDelayOut) == true)
				{
					if(g_StatusBroNum == 0 && g_completed == 0)
					{
						if (g_YYPlay.status & 0x01)
						{
							WT588E_PLAY(WXTXYC);
							g_YYPlay.status &=0xFE;
							g_completed =1;
						} 						
						g_StatusBroNum++;
					}
					if(g_StatusBroNum == 1 && g_completed == 0)
					{
						if (g_YYPlay.status & 0x02)
						{
							WT588E_PLAY(BDDWZDQDWJDD);
							g_YYPlay.status &=0xFD;
							g_completed =1;
						} 						
						g_StatusBroNum++;
					}
					if(g_StatusBroNum == 2 && g_completed == 0)
					{
						if (g_YYPlay.status & 0x04)
						{
							WT588E_PLAY(ZJZSBDLD);
							g_YYPlay.status &=0xFB;
							g_completed =1;
						}						
						g_StatusBroNum++;
					}
					if(g_StatusBroNum == 3 && g_completed == 0)
					{
						if (g_YYPlay.status & 0x08)
						{
							WT588E_PLAY(SBZC);
							g_YYPlay.status &=0xF7;
						} 						
						g_StatusBroNum =0;
						g_YYPro = 0x00;	
					}
					g_YYProDelay = TIM2->CNT;
					g_YYProDelayOut = Delay3Sec;
				}
			}
			else
			{
				g_YYPro = 0x00;	
			}		
			break;
		default:		
			break;	
	}
	/*RTK初始化常灭，初始化完成搜星中常亮，定位后闪烁*/ 
	if(g_tGPS.PositionOk == 1)  /*根据定位情况闪烁*/
	{
		if(g_PosQuality	> 0 && g_PosQuality	< 3)
		{
			if(JudgeSomeDelayInProOk(g_RTKLedDelay,Delay500mSec) == true)
			{	
				LED_RTK_TOGGLE();           
				g_RTKLedDelay = TIM2->CNT;
			}		
		}
		else if(g_PosQuality == 5)    /*浮点解*/
		{
			if(g_RTKLedFlag == 0 && JudgeSomeDelayInProOk(g_RTKLedDelay,Delay2Sec) == true)
			{	
				LED_RTK_OFF();				
				g_RTKLedDelay = TIM2->CNT;
				g_RTKLedFlag = 1;
			}	
			if(g_RTKLedFlag == 1 && JudgeSomeDelayInProOk(g_RTKLedDelay,Delay2Sec) == true)
			{	
				LED_RTK_ON();
				g_RTKLedDelay = TIM2->CNT;
				g_RTKLedFlag = 0;				
			}			
		}
		else if(g_PosQuality == 4)   /*固定解*/
		{
			LED_RTK_ON();	
		}
		else
		{
			LED_RTK_OFF();
		}
	}
	else
	{
		LED_RTK_OFF();
	}
	
	//多次给4G模块发送AT指令没回应时，重启4G模块
	if(errornum >= 5)
	{
		if(flaglagao==0)
		{
			PWR_4G_DISABLE();
			errornum_timer = TIM2->CNT;
			flaglagao = 1;
			g_DtuOutage = 1;
			//HAL_UART_MspDeInit(&UART3_Handler);
		}
		else if(flaglagao==1)
		{
			if(JudgeSomeDelayInProOk(errornum_timer,Delay3Sec) == true)
			{
				PWR_4G_ENABLE();
				flaglagao = 0;			
				current_step = 0x10;
				//InitUSART3(UART3_BAUD);
				//HAL_UART_MspInit(&UART3_Handler);
				g_DtuOutage = 0;
				errornum = 0;
				gprs_link_status = 0;
				diff_gprs_link_status = 0;
			}
		}
	}
	
	#if 0
	//报电量低，及时充电 (在电量小于40%，且未充电，10分钟报一次)
	if ((battery < 40) && (Get_Adc_Average(ADC_CHANNEL_16, 10) < 500) &&
			(TIM2->CNT - g_BatteryLowerTime > Delay1Min))
	{
		g_BatteryLowerTime = TIM2->CNT;
		WT588E_PLAY(DLD);
	}
	#endif
	
	if(WriteMobileNumFlag == 1)
	{
		//解析C口数据，手机号，卡号，报警类型
		c_uart_pro(g_RxBuf1, g_Uart1.usRxCount);
		WriteMobileNumFlag = 0;
	}
	
	if (g_VoiceSwitch.num == 1)
	{
		/*定位不是固定解或者浮点解，提示*/
		if((g_LastPosQuality != g_PosQuality)&&g_LastPosQuality!=1&&g_LastPosQuality!=2)
		{
			if((g_PosQuality == 1) || (g_PosQuality == 2))
			{
				WT588E_PLAY(DDJDW);
				g_testDelay = TIM2->CNT;
			}
		}
		g_LastPosQuality = g_PosQuality;
		
		if(((g_PosQuality == 1||g_PosQuality == 2))&& (JudgeSomeDelayInProOk(g_testDelay,Delay30Sec) == true)) //单点解
		{
			WT588E_PLAY(DDJDW);
			g_testDelay = TIM2->CNT;
		}
	}
	
	#if 0
	if((g_PosQuality == 5)&& (JudgeSomeDelayInProOk(g_testDelay,Delay5Sec) == true))   /*浮点解*/
	{
		WT588E_PLAY(FDJDW);
		g_testDelay = TIM2->CNT;
	}
	if((g_PosQuality == 4)&& (JudgeSomeDelayInProOk(g_testDelay,Delay5Sec) == true))     /*固定解*/
	{
		WT588E_PLAY(GDJDW);
		g_testDelay = TIM2->CNT;
	}
	#endif
}

//计算防护位置
void CalculateProtectionPosition(void)
{
	uint8_t EligibleNum = 0;

	if ((JudgeSomeDelayInProOk(ProtectionPositionTime, 180 * Delay1Sec) == true))
	{
		g_YYPlay.postionprotection = 0x00;
		g_CarWarnFlag.bit.ProtectionPosition = 0x00;
		
		memset((uint8_t *)&ProtectionPositionInfo, 0x00, sizeof(ProtectionPositionInfo));
	}

	if (ProtectionPositionInfo.NewData == 1)
	{	
		if (g_AreaType != 2)
		{
			return;
		}
		
		ProtectionPositionInfo.NewData = 0;
		
		for (uint8_t i = 0; i < ProtectionPositionInfo.Num; i++)
		{
			//如果不是高精度定位就不进行计算
			if (ProtectionPositionInfo.ProtectionPositionEle[i].PositionQuality < 4)
			{
				continue;
			}
			
			if (ProtectionPositionInfo.ProtectionPositionEle[i].Area != g_AreaType)
			{
				EligibleNum++;
			}
		}
		
		if (EligibleNum == ProtectionPositionInfo.Num)
		{
			for (uint8_t i = 0; i < ProtectionPositionInfo.Num; i++)
			{
				if (ProtectionPositionInfo.ProtectionPositionEle[i].VoiceCode == 0x41)
				{
					g_YYPlay.postionprotection = 0x01;
				}
			}
			
			g_CarWarnFlag.bit.ProtectionPosition = 0x01;
			
			ProtectionPositionFlag = 1;
			
			return;
		}
		else 
		{
			EligibleNum = 0;
			ProtectionPositionFlag = 0;
		}
		
		for (uint8_t i = 0; i < ProtectionPositionInfo.Num; i++)
		{
			//如果不是高精度定位就不进行计算
			if (ProtectionPositionInfo.ProtectionPositionEle[i].PositionQuality < 4)
			{
				continue;
			}
			
			//纬度
			double Latitude = (double)(ProtectionPositionInfo.ProtectionPositionEle[i].Latitude[0] |
					(ProtectionPositionInfo.ProtectionPositionEle[i].Latitude[1] << 8) |
					(ProtectionPositionInfo.ProtectionPositionEle[i].Latitude[2] << 16) |
					(ProtectionPositionInfo.ProtectionPositionEle[i].Latitude[3] << 24)) / 10000000;
			
			//经度
			double Longitude = (double)(ProtectionPositionInfo.ProtectionPositionEle[i].Longitude[0] |
					(ProtectionPositionInfo.ProtectionPositionEle[i].Longitude[1] << 8) |
					(ProtectionPositionInfo.ProtectionPositionEle[i].Longitude[2] << 16) |
					(ProtectionPositionInfo.ProtectionPositionEle[i].Longitude[3] << 24)) / 10000000;
			
			double CurrentLongitude = (double)g_CardLocaData.Longitude /10000000;
			double CurrentLatitude = (double)g_CardLocaData.Latitude /10000000;
			
			double TmpDistance = haversine_distance(Latitude,Longitude,CurrentLatitude,CurrentLongitude);
			
			//小于
			if (ProtectionPositionInfo.ProtectionPositionEle[i].JudgementCondition == 1)
			{
				if (TmpDistance > (double)ProtectionPositionInfo.ProtectionPositionEle[i].Distance)
				{
					EligibleNum++;
				}
			}
			//大于
			else if (ProtectionPositionInfo.ProtectionPositionEle[i].JudgementCondition == 2)
			{
				if (TmpDistance < (double)ProtectionPositionInfo.ProtectionPositionEle[i].Distance)
				{
					EligibleNum++;
				}
			}
		}
		
		if (EligibleNum == ProtectionPositionInfo.Num)
		{
			for (uint8_t i = 0; i < ProtectionPositionInfo.Num; i++)
			{
				if (ProtectionPositionInfo.ProtectionPositionEle[i].VoiceCode == 0x41)
				{
					g_YYPlay.postionprotection = 0x01;
				}
			}
			g_CarWarnFlag.bit.ProtectionPosition = 0x01;
			
			ProtectionPositionFlag = 1;
		}
		else
		{
			g_CarWarnFlag.bit.ProtectionPosition = 0x00;
			
			ProtectionPositionFlag = 0;
		}
		
		memset((uint8_t *)&ProtectionPositionInfo, 0x00, sizeof(ProtectionPositionInfo));
	}
}
