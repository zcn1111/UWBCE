#include "bsp.h"
#include "4G_Comm.h"
#include "app_glovar.h"
#include "app_4G.h"
#include "iap.h"
#include "main.h"
#include "Update.h"
//#include "arm_math.h"
#include "app_workplan.h"

/********* 变量定义 **********/
CardInfoTable_t	 g_CardTable;	  //定位卡信息表
TaskState   currentTask = TASK_NONE;
TrainInfo 	g_TrainInfo;
uint32_t 	ServiceAnsewerTime = 0;  //服务器应答的时间
uint32_t 	DiffServiceAnsewerTime = 0;  //服务器应答的时间
uint8_t 	readConstructionSigninfo=0;           //读施工牌信息标记
uint8_t     readareaalarm = 0;    //区域报警
uint8_t		WorkPlanSNum[4];
uint8_t		TrainInfoSNum[4];
uint8_t     ConstructionSignSNum[4];
uint8_t		AreaAlarmSNum[4];
uint8_t		AskWorkPlanSNum[4];
uint8_t		CarSNum[4];
uint8_t 	ProtectionPositionSum[4];
uint8_t     ProtectionPositionState = 0;
uint16_t 	RXlen = 0;
uint32_t	DiffTime;
uint8_t 	Cardinfo[69];
uint8_t		sendATflag = 1;
uint8_t     planState = 0;
uint8_t     trainState = 0;
uint8_t     CarState = 0;
uint8_t     CarFlag = 0;
uint8_t     sendonline;
uint32_t    lastCardTriggerTime = 0;
uint32_t    lastSendLocationToDiffServiceTime = 0;
volatile uint32_t lastGGATriggerTime = 0;
volatile uint32_t lastONLINETriggerTime = 0;
uint32_t cardSendCompleteTime = 0;       // 新增：卡片数据发送完成时间
bool        expectingPrompt = false;
bool        ggaPending = false;          // 新增：GGA任务等待发送标志
uint8_t     gprs_link_status = 0;
uint8_t     diff_gprs_link_status = 0;
uint8_t     cors_gprs_link_status = 0;   //连服状态标记
uint32_t g_RecD6BagNo = 0;	
uint32_t LastTotalBagNum = 0;
uint8_t g_RemoteUpdateGCFlag = 0;
uint8_t g_UpdateType = 0;	            //0-全量升级 1-增量升级
uint16_t g_RemoteUpdatePackNo = 0;
uint8_t g_RemoteUpdateNum = 0;
uint8_t g_AskParFlag = 0;
uint8_t g_RecdateToGprsFlag = 0;
uint8_t 	g_D6Phone[6];
uint32_t 	g_SendD6BagNo;	//发送D6数据包序号
uint32_t 	g_RecD6BagNo;	//接收D6数据包序号
const uint8_t TotalControlMobleNo[2][6]={{0x13,0x75,0x31,0x24,0x37,0x2F},{0x13,0x75,0x31,0x20,0x58,0x3F}};
uint8_t g_AskAuthorityFlag = 0;
uint8_t g_AskIDFlag = 0;
uint8_t g_NetAskIPAndPortFlag = 0;
WorkMode_t  g_ModeInFlash;
LocationAndSelfcheckDiff_t g_LocationAndSelfcheckDiffFlash;
uint32_t g_CardSNum = 0;           //卡序号
uint32_t g_LYHeartBeatSNum = 0;    //记录蓝牙开机与心跳序号
uint32_t g_LYAlarmSNum = 0;        //卡序号
uint8_t g_ServiceResponseLY = 0;
uint8_t     g_NeedReInitDTU=0;
uint32_t    g_ControlMsgDelay;
uint8_t		g_CtrlMsgFlag =0;
uint32_t   g_TrainForeshowTime = 0;
uint16_t g_RemoteUpgradeSendNum = 0;
uint32_t    lastSendLyDataTime = 0;
uint8_t g_SendLocationFirst = 0;
uint8_t g_WorkPlanReceiveFlag = 0;
uint32_t g_WorkPlanTime = 0;
uint32_t g_ChargingBoxSNum = 0;
uint32_t g_ServiceAnsewerUpgradeTime = 0;
uint8_t g_FirstSendChargeSelf = 1;
uint8_t g_AskWorkPlan = 0;
uint8_t g_RemoteStartFlag = 0;
uint32_t g_ReceiveAreaTime = 0;
uint8_t BigAreaTypeValueCount = 0;
uint8_t BigAreaTypeValue[3] = {0, 0, 0};
ProtectionPosition_t ProtectionPositionInfo;
uint32_t ProtectionPositionTime = 0;
uint8_t g_SendLocationDataToDiffService = 0;
uint32_t g_SendDataTime = 0;
uint32_t g_SendDataNum = 0;
uint8_t g_SetIpAndPort = 0;
uint32_t g_IssueWorkPlan = 0;
uint8_t g_IsUpdateWorkTime = 0;
uint8_t g_FirstSendID=0;
/******************************************************************
函数名称: void DTUProcess(void)
功能描述: DTU数据处理
参数输入: 无
参数输出: 
返  回  值 : 无
其           他: 无
******************************************************************/
void DTUProcess(void)
{
	RXlen  = GetRingUsedLen(&g_Uart3RxRing);
	if(RXlen > 0)
	{
		memset(GsmReceBuff,0,sizeof(GsmReceBuff));
		if(RXlen > sizeof(GsmReceBuff))
		{
			RingBufClear(&g_Uart3RxRing);
		}
		else
		{
			RingBufPop(&g_Uart3RxRing, GsmReceBuff, RXlen);
		}
		
		//判断">"
		if (expectingPrompt) 
		{
			if (mem_mem(GsmReceBuff,RXlen,_mipsend,strlen(_mipsend)) != NULL)
			{
				g_SendDataNum = 0;
				expectingPrompt = false;  // 重置期待标志	
			}
		}
		
		//判断业务服务器是否断开
		if (mem_mem(GsmReceBuff,RXlen,IsDisconnect0,strlen(IsDisconnect0)) != NULL)
		{
			ServiceAnsewerTime = TIM2->CNT;
		}
		
		//判断差分服务器是否断开
		if (mem_mem(GsmReceBuff,RXlen,IsDisconnect1,strlen(IsDisconnect1)) != NULL)
		{
			DiffServiceAnsewerTime = TIM2->CNT;
		}
		
		uint8_t ResRecRes;
		
		if(cors_gprs_link_status == 1)
		{
			ResRecRes = RTK_GetRTCMInfo(GsmReceBuff, RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					DiffTime = TIM2->CNT;
					break;
				default:
					if((g_CardLocaData.Longitude != 0)&&(g_CardLocaData.Latitude!=0))
					{
						if((gprs_link_status==1 ) &&(JudgeSomeDelayInProOk(DiffTime,Delay20Sec) == true))
						{
							DiffTime = TIM2->CNT;
						}
					}
					break;
			}
		}
		else
		{	
			/* 接收差分数据并转发至RTK */
			ResRecRes = AnalysisDiffData(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					//memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					DiffTime = TIM2->CNT;
					break;
				default:
					if((g_CardLocaData.Longitude != 0)&&(g_CardLocaData.Latitude!=0))
					{
						if((gprs_link_status==1 ) &&(JudgeSomeDelayInProOk(DiffTime,Delay20Sec) == true))
						{
							DiffTime = TIM2->CNT;
						}
					}
					break;
			}
		}
		/* 接收服务器应答 */
		if((gprs_link_status == 1) || (diff_gprs_link_status == 1))
		{
			ResRecRes =AnalysisAnswerData(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					//memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					break;
				default:			
					break;
			}
			//接收服务器发下来对蓝牙数据的应答
			ResRecRes =AnalysisLYResponseData(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					//memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					break;
				default:
					break;
			}
			/*接收来车和施工牌信息和探伤小车*/
			ResRecRes =AnalysisTrainAndConstructionSignInfo(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					//memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					break;
				default:
					break;
			}
			/*接收远离防护信息*/
			ResRecRes =AnalysisProtectionPositionInfo(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					//memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					break;
				default:
					break;
			}
			//接收服务器下发的区域
			ResRecRes =AnalysisAreaTypeData(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					//memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					break;
				default:
					break;
			}
			//接收区域报警
			ResRecRes =AnalysisAreaAlarmInfo(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					//memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					readareaalarm = 1;
					g_ServiceAnsewerUpgradeTime = TIM2->CNT;
					break;
				default:
					break;
			}
			/*接收施工计划*/
			ResRecRes =AnalysisWorkPlan(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					planState = 1;
					g_ServiceAnsewerUpgradeTime = TIM2->CNT;
					break;
				default:
					break;
			}
			//解析远程升级信息
			ResRecRes = AnalysisRemoteUpgradeData(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					g_RemoteStartFlag = 1;
					//g_RecdateToGprsFlag = 1;
					g_ServiceAnsewerUpgradeTime = TIM2->CNT;
					break;
				default:
					break;
			}
			//获取远程升级程序的类型
			ResRecRes = AnalysisRemoteUpgradeTypeData(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					g_RemoteUpdateGCFlag = 3;
					g_ServiceAnsewerUpgradeTime = TIM2->CNT;
					break;
				default:
					break;
			}
			//解析远程升级服务器发过来的bin文件
			ResRecRes = AnalysisRemoteUpgradeBinData(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{	
				case _4GRET_OK:
					memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					g_ServiceAnsewerUpgradeTime = TIM2->CNT;
					break;
				default:
					break;
			}
			//索要施工计划
			ResRecRes = AnalysisAskWorkPlanData(GsmReceBuff, RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					g_ServiceAnsewerUpgradeTime = TIM2->CNT;
					break;
				default:
					break;
			}
			//解析服务器下发下来的参数设置
			ResRecRes = AnalysisParameterData(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					g_ServiceAnsewerUpgradeTime = TIM2->CNT;
					break;
				default:
					break;
			}
			//解析服务器发下来的参数应答包
			ResRecRes = AnalysisParameterResponseData(GsmReceBuff,RXlen);
			switch(ResRecRes)
			{
				case _4GRET_OK:
					memset(GsmReceBuff,0,sizeof(GsmReceBuff));
					g_ServiceAnsewerUpgradeTime = TIM2->CNT;
					break;
				default:
					break;
			}
		}
	}	
	
	/*判断读取施工计划*/
	SelectWorkPlan();
}

/******************************************************************
函数名称: _4GResult AnalysisProtectionPositionInfo
功能描述: 解析防护位置
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
_4GResult AnalysisProtectionPositionInfo(uint8_t *receBuf, uint16_t buffer_size) 
{
	uint16_t datalen = 0;
	uint16_t rlen = 0;
	uint8_t calc_sum = 0;
	uint8_t *p = NULL;
	
	/* 参数校验 */
    if(!receBuf) return _4GRET_INCOMPLETE_ERR;
    
    /* 基础长度检查 */
    if(buffer_size < 32) return _4GRET_LEN_ERR;
    for(uint16_t k = 0; k < buffer_size; k++)
	{
		if ((k + 32) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		
		/* 包头校验 */
		if((receBuf[k] != 0XFA) || (receBuf[k+1] != 0x00)) 
		{
			continue;
		}
		
		if (receBuf[k + 2] != 0x10)
		{
			continue;
		}
		
		/* 获取数据长度 */
		datalen = (receBuf[k + 31] << 8)+receBuf[k + 30];
		rlen = datalen + 33;
		
		/* 二次长度检查 */
		if ((k + rlen + 4) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		
		calc_sum = CheckSum(rlen - 1, receBuf + k);
		if(calc_sum != receBuf[k + rlen - 1]) 
		{
			continue;
		}
		
		/*获取序列号*/
		memcpy(ProtectionPositionSum, receBuf + k + rlen, 4);
		p = k + receBuf + 32; //起始位置
		
		ProtectionPositionInfo.Num = datalen / 32;
		for (uint8_t i = 0; i < ProtectionPositionInfo.Num; i++)
		{
			//纬度
			memcpy(ProtectionPositionInfo.ProtectionPositionEle[i].Latitude, &p[i * 32 + 1], 4);
			//经度
			memcpy(ProtectionPositionInfo.ProtectionPositionEle[i].Longitude, &p[i * 32 + 5], 4);
			//定位质量
			ProtectionPositionInfo.ProtectionPositionEle[i].PositionQuality = p[i * 32 + 9];
			//距离
			ProtectionPositionInfo.ProtectionPositionEle[i].Distance = (p[i * 32 + 19] << 8) + p[i * 32 + 18];
			//判断条件
			ProtectionPositionInfo.ProtectionPositionEle[i].JudgementCondition = p[i * 32 + 17];
			//语音序号
			ProtectionPositionInfo.ProtectionPositionEle[i].VoiceCode = (p[i * 32 + 21] << 8) + p[i * 32 + 20];
			//区域类型
			ProtectionPositionInfo.ProtectionPositionEle[i].Area = p[i * 32 + 22];
		}
		
		ProtectionPositionInfo.NewData = 1;
		ProtectionPositionState = 1;
		
		ProtectionPositionTime = TIM2->CNT;
		
		return _4GRET_OK;
	}

	return _4GRET_INCOMPLETE_ERR;
}

_4GResult AnalysisAreaTypeData(uint8_t *receBuf, uint16_t buffer_size)
{
	uint8_t calc_sum = 0;
	uint8_t *p = NULL;
	
	if (SelectEFenceInfo.IsBigArea == 0)
	{
		return _4GRET_INCOMPLETE_ERR;
	}
	
	/* 参数校验 */
    if(!receBuf) return _4GRET_INCOMPLETE_ERR;
    
    /* 基础长度检查 */
    if(buffer_size < 32) return _4GRET_LEN_ERR;
    for(uint16_t k = 0; k < buffer_size; k++)
	{
		if ((k + 53) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		/* 包头校验 */
		if((receBuf[k] != 0XFA) || (receBuf[k+1] != 0x00)) 
		{
			continue;
		}
		
		if (receBuf[k + 2] != 0x07)
		{
			continue;
		}
		
		p = k + receBuf + 32;
		/* 帧头校验 (0x5A 0xA5) */
		if (p[0] != 0x5A || p[1] != 0xA5) 
		{
			return _4GRET_KEYWORD_ERR;
		}
		
		calc_sum = CheckSum(48, receBuf + k);
		if(calc_sum != receBuf[k + 48]) 
		{
			continue;
		}
		
		g_IssueWorkPlan = ConvArrayToUint32((uint8_t *)&receBuf[k + 23], 0, M_BIG_ENDIAN);
		
		//判断施工计划ID
		if (memcmp((uint8_t *)&SelectEFenceInfo.order, (uint8_t *)&receBuf[k + 23], 4) == 0)
		{
			DATETIME Time;
			
			TimeBcdToHex(&Time, (uint8_t *)&receBuf[k + 17]);	
			if (compare_time_second(g_SysTime, Time, 10) <= 0)
			{
			
			}
		}
		
		g_AreaTypeAlarm = p[2];
		BigAreaTypeValue[BigAreaTypeValueCount] = g_AreaTypeAlarm;
		if (BigAreaTypeValueCount >= 2)
		{
			BigAreaTypeValueCount = 0;
		}
		else 
		{
			BigAreaTypeValueCount++;
		}
		
		g_ReceiveAreaTime = TIM2->CNT;
		
		return _4GRET_OK; // 需定义成功返回值
	}
	
	return _4GRET_INCOMPLETE_ERR;
}

_4GResult AnalysisAskWorkPlanData(uint8_t *receBuf, uint16_t buffer_size)
{	
	/* 参数校验 */
    if(!receBuf) return _4GRET_INCOMPLETE_ERR;
    
    /* 基础长度检查 */
    if(buffer_size < 32) return _4GRET_LEN_ERR;
    for(uint16_t i = 0; i < buffer_size; i++)
	{
		if ((i + 45) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		
		/* 包头校验 */
		if((receBuf[i] != 0XFA) || (receBuf[i+1] != 0x00)) 
		{
			continue;
		}
		/* 数据类型验证 */
		if(receBuf[i + 2] != 0x80)
		{
			continue;
		}
		/* 获取数据长度 */
		//datalen = (receBuf[i + 31] << 8) + receBuf[i + 30];
		//uint16_t rlen = datalen + 33;
		//FA008000000000000000000000000000000000000000000000000000000020005AA50100000000002445678937
		/* 二次长度检查 */
		//if(buffer_size - i < rlen + 4) 
		//{
		//	return _4GRET_LEN_ERR;
		//}
		
		/* 校验和验证 */
		//uint8_t calc_sum = CheckSum(rlen - 1, receBuf + i);
		//if(calc_sum != receBuf[i + rlen - 1]) 
		//{
		//	continue;
		//}
		
		uint8_t *p = i + receBuf + 32; // 来车信息起始位置
		/* 帧头校验 (0x5A 0xA5) */
		if (p[0] != 0x5A || p[1] != 0xA5) 
		{
			return _4GRET_KEYWORD_ERR;
		}
		
		//索要FLASH中电子围栏
		if (p[2] == 1)
		{
			g_AskWorkPlan = 1;
		}
		//索要概要概要内的施工计划
		else if (p[2] == 2)
		{
			g_AskWorkPlan = 2;
		}
		
		/*获取序列号*/
		memcpy(AskWorkPlanSNum, receBuf + i + 41, 4);
		
		return _4GRET_OK; // 需定义成功返回值
	}
	
	return _4GRET_INCOMPLETE_ERR;
}

//接收服务器发下来对蓝牙数据的应答
_4GResult AnalysisLYResponseData(uint8_t *receBuf, uint16_t buffer_size) 
{
	uint32_t OrderSNum = 0;
	
	/* 参数校验 */
    if(!receBuf) return _4GRET_INCOMPLETE_ERR;
    
    /* 基础长度检查 */
    if(buffer_size < 11) return _4GRET_LEN_ERR;
    for(uint16_t k = 0; k < buffer_size; k++)
	{
		if ((k + 11) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		
		/* 包头校验 */
		if((receBuf[k] != 0X5A) || (receBuf[k+1] != 0xA5)) 
		{
			continue;
		}
		/* 数据类型验证 */
		if(receBuf[k + 4]!= 0xD8)
		{
			continue;
		}

		/* 校验和验证 */
		uint8_t calc_sum = CheckSum(10, receBuf + k);
		if(calc_sum != receBuf[k + 10]) 
		{
			continue;
		}
		
		OrderSNum = ConvArrayToUint32(receBuf + k, 5, M_BIG_ENDIAN);
		
		//开机/心跳应答
		if (OrderSNum == g_LYHeartBeatSNum)
		{
			//g_ServiceResponseLY = 0x01;
			
			ReceivelLYData = 0;
		}
		//报警与解除报警应答
		else if (OrderSNum == g_LYAlarmSNum)
		{
			g_ServiceResponseLY = 0x02;
			
			ReceivelLYData = 0;
		}
		
		return _4GRET_OK; // 需定义成功返回值
	}
	
	return _4GRET_INCOMPLETE_ERR;
}

//解析服务器发下来的参数应答包
_4GResult AnalysisParameterResponseData(uint8_t *ResponseBuf, uint16_t buffer_size)
{
	//参数校验
    if (!ResponseBuf || buffer_size < 9) 
	{
        return _4GRET_INCOMPLETE_ERR;
    }
	
	for (uint16_t i = 0; i < buffer_size; i++)
	{
		if ((i + 9) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		
		//包头校验
		if (ResponseBuf[i] != 0xD7)
		{
			continue;
		}
		
		if(ResponseBuf[i+ 8] == CheckSum(8,ResponseBuf + i))
		{
			uint32_t recd6bagno = ResponseBuf[i + 7];
			if(recd6bagno == g_SendD6BagNo)//判断包序号、是否发送成功
			{
				g_SendD6BagNo++;
			}
		}
		return _4GRET_OK;
	}
	
	return _4GRET_INCOMPLETE_ERR;
}

//解析服务器下发下来的参数设置
_4GResult AnalysisParameterData(uint8_t *ParameterDataBuf, uint16_t buffer_size)
{
	uint8_t tmpmobile[6],typebuf[32],cmpbuf[6],tmp,type,equalflag;
	uint16_t m = 0;
	uint16_t n = 0;
	uint16_t k = 0;
	uint8_t gsmcode7[140];
	uint8_t recbuf[300];
	uint8_t cmdbuf[140];
	uint8_t mobile_rxflag = 0;
	uint8_t tempmobile[5][6];
	
	//参数校验
    if (!ParameterDataBuf || buffer_size < 322) 
	{
        return _4GRET_INCOMPLETE_ERR;
    }
	
	for (uint16_t i = 0; i < buffer_size; i++)
	{
		if ((i + 322) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		
		if ((ParameterDataBuf[i] != 0x5A) || (ParameterDataBuf[i + 1] != 0xA5) || (ParameterDataBuf[i + 2] != 0x5A) ||
			(ParameterDataBuf[i + 3] != 0xA5))
		{
			continue;
		}
		
		if (ParameterDataBuf[i + 4] != 0XD6)
		{
			continue;
		}
		
		if (ParameterDataBuf[i + 317] != CheckSum(313,ParameterDataBuf + i + 4))
		{
			continue;
		}
		
		for(m = 5; m < 37; m += 2)
		{
			tmp = Asc2Hex(ParameterDataBuf[i + m]);
			tmp = (tmp<<4);
			tmp += Asc2Hex(ParameterDataBuf[i + m + 1]);									
			typebuf[n++] = tmp;
		}
		type = typebuf[7];
		
		g_RecD6BagNo = ParameterDataBuf[i + 318]+(ParameterDataBuf[i + 319]<<8)+(ParameterDataBuf[i + 320]<<16)+(ParameterDataBuf[i + 321]<<24);
		g_RecdateToGprsFlag = 1;
		memset(cmpbuf,0,6);
		for(m=0;m<6;m++)
		{
			tmpmobile[m]=CharSwap(typebuf[m]);
		}
		n=0;
		for(m=37;m<317;m+=2)
		{
			tmp = Asc2Hex(ParameterDataBuf[i + m]);
			tmp = (tmp<<4);
			tmp += Asc2Hex(ParameterDataBuf[i + m +1]);									
			gsmcode7[n++] = tmp;
		}
		if(type == 0x00)
		{
			gsmDecode7bit(gsmcode7,(char*)recbuf,140);	
		}			
//		if(memcmp(tmpmobile,g_MobileInFlash.mobileno[0],6)&& 
//			memcmp(tmpmobile,g_MobileInFlash.mobileno[1],6)&&
//			memcmp(tmpmobile,TotalControlMobleNo[0],6)&&
//			memcmp(tmpmobile,TotalControlMobleNo[1],6))
//		{
//			return _4GRET_OK;
//		}
//		else
		{
			memcpy(g_D6Phone,tmpmobile,6);
			if(type == 0x00)//7bit
			{
				g_RecD6BagNo = ParameterDataBuf[i + 318]+(ParameterDataBuf[i + 319]<<8)+(ParameterDataBuf[i + 320]<<16)+(ParameterDataBuf[i + 321]<<24);
				n=0;
				for(m=37;m<317;m+=2)
				{
					tmp = Asc2Hex(ParameterDataBuf[i + m]);
					tmp = (tmp<<4);
					tmp += Asc2Hex(ParameterDataBuf[i + m + 1]);									
					gsmcode7[n++] = tmp;
				}
				gsmDecode7bit(gsmcode7,(char*)recbuf,140); 
										
				for(m=0;m<80;m++)
				{
					cmdbuf[m] = (Asc2Hex(recbuf[m*2])<<4) + Asc2Hex(recbuf[m*2+1]);
				}
										
				if(cmdbuf[0] == 0xA8)
				{
					memset(cmpbuf,0xA8,6);
					if(memcmp(cmdbuf, cmpbuf, 6) == 0)
						mobile_rxflag = 1;
					else
						mobile_rxflag = 0;											
				}
				else if(cmdbuf[0] == 0xA9)
				{
					memset(cmpbuf,0xA9,6);
					if(memcmp(cmdbuf, cmpbuf, 6) == 0)
					{
						if(cmdbuf[9] == 0x08)
							mobile_rxflag = 3;	
					}					
				}
				if(mobile_rxflag == 1)
				{
					mobile_rxflag = 0;
					for(k=0;k<5;k++)
					{
						for(m=0;m<6;m++)
						{
							tempmobile[k][m] = cmdbuf[10+m+8*k];
						}
					}
					if(ReadMobileNoFromFLASH() == true)
					{
						;
					}
					else
					{
						return _4GRET_OK;
					}
					for(m=0;m<5;m++)
					{
						if(CheckNumIsBCD((uint8_t*)tempmobile[m],5) == true && memcmp((uint8_t*)tempmobile[m],cmpbuf,5)!=0)
						{
							if(m == 4)
							{
								if(0 != memcmp(g_MobileInFlash.mobileno[4],tempmobile[m],5) || ((g_MobileInFlash.mobileno[4][5]&0xF0) != (tempmobile[4][5]&0xF0)))
								{
									equalflag = 1;
									tempmobile[m][5] |= 0x0F;
									memcpy(g_MobileInFlash.mobileno[4],tempmobile[m],sizeof(tempmobile[m]));
								}
							}
							else
								memcpy(g_MobileInFlash.mobileno[m],tempmobile[m],sizeof(tempmobile[m]));												
						}
					}   
					WriteMobileNoToFLASH();										
					if(ReadMobileNoFromFLASH() == true)
					{
						if(equalflag == 1)
						{
							equalflag = 0;
							g_NeedReInitDTU = 1;
//							current_step = 0x10;
						}
						g_GetMyMobileOKFlag = true;
						g_AskAuthorityFlag = 1;
						g_CtrlMsgFlag = 1;					
						return _4GRET_OK;
					}
				}					
				else if(mobile_rxflag == 3 && g_CtrlMsgFlag == 0)
				{                                            
					mobile_rxflag = 0;
					if(ReadMobileNoFromFLASH() == true)
					{
						g_AskAuthorityFlag = 1; 
						g_CtrlMsgFlag = 1;							
					}
					return _4GRET_OK;
				}								
				if(cmdbuf[0] == 0xCF && cmdbuf[1] == 0x80 && cmdbuf[2] == 0x09)
				{
					if(0 == g_CtrlMsgFlag)
					{
						n = 0;
						ReadServerInfoPar();  
						uint8_t cmpipbuf[2][6];												
						memset(cmpipbuf[0],0,sizeof(cmpipbuf[0]));
						memset(cmpipbuf[1],0xff,sizeof(cmpipbuf[1]));								
						for( k = 0 ; k < 4 ; k++ )
						{
							for( m = 0 ; m < 6 ;m++ )             
							{
								tempmobile[k][m] = cmdbuf[3+6*k+m];
							}
							if( memcmp(tempmobile[k],cmpipbuf[0],4)== 0 || memcmp(tempmobile[k]+4,cmpipbuf[0],2)== 0 
													|| memcmp(tempmobile[k],cmpipbuf[1],4) == 0 || memcmp(tempmobile[k]+4,cmpipbuf[1],2)== 0 )
							{
							}
							else
							{
								n = 1;
								if( k == 0)                                                 //数据
								{
									memcpy(g_DataIPBack,g_ServerInfo.serveripaddr[2],4);  //备份数据IP和端口号
									g_DataPortBack = g_ServerInfo.serverportaddr[2];
									memcpy(g_ServerInfo.serveripaddr[2],tempmobile[k],4);
									g_ServerInfo.serverportaddr[2] = tempmobile[k][4]*100+tempmobile[k][5];
								}
								else if(k == 1 )                                            //升级
								{
									memcpy(g_ServerInfo.serveripaddr[0],tempmobile[k],4);
									g_ServerInfo.serverportaddr[0] = tempmobile[k][4]*100+tempmobile[k][5];
								}
								else if( k == 2)                                            //转储
								{
									memcpy(g_ServerInfo.serveripaddr[1],tempmobile[k],4);
									g_ServerInfo.serverportaddr[1] = tempmobile[k][4]*100+tempmobile[k][5];
								}
								else if( k == 3)
								{
									memcpy(g_DiffDataIPBack,g_ServerInfo.serveripaddr[2],4);  //备份数据IP和端口号
									g_DiffDataPortBack = g_ServerInfo.serverportaddr[2];
									memcpy(g_ServerInfo.serveripaddr[3],tempmobile[k],4);
									g_ServerInfo.serverportaddr[3] = tempmobile[k][4]*100+tempmobile[k][5];
								}									
							}
						}
						if(n == 1)
						{
							WriteServerInfoPar(); 
							if(ReadServerInfoPar() == true)         //
							{                                       //
								CmpIPAndPort();                                         //若有全零或全F的替换为默认值
								if((memcmp(g_DataIPBack,g_ServerInfo.serveripaddr[2],4) != 0) || (g_DataPortBack != g_ServerInfo.serverportaddr[2]) ||
									(memcmp(g_DiffDataIPBack,g_ServerInfo.serveripaddr[3],4) != 0) || (g_DiffDataPortBack != g_ServerInfo.serverportaddr[3]))  //若数据IP修改了
								{
									g_NeedReInitDTU = 1;
//									current_step = 0x10;
								}
							}
							g_CtrlMsgFlag = 1; 
						}																		
					}
					return _4GRET_OK;
				}
				else if(cmdbuf[0] == 0xCF && cmdbuf[1] == 0xFF && cmdbuf[2] == 0x80 && cmdbuf[3] == 0x09)
				{
					if(0 == g_CtrlMsgFlag)
					{
						g_NetAskIPAndPortFlag = 1;
						g_CtrlMsgFlag = 1;
					}
					return _4GRET_OK;					
				}
				else if(cmdbuf[0] == 0xCF && cmdbuf[1] == 0xFF && cmdbuf[2] == 0x80 && cmdbuf[3] == 0x0B)
				{
					if(0 == g_CtrlMsgFlag)
					{
						g_AskIDFlag = 1;
						g_CtrlMsgFlag = 1;
					}
					return _4GRET_OK;
				}
				else if(cmdbuf[0] == 0xCF && cmdbuf[1] == 0xFF && cmdbuf[2] == 0x91 && cmdbuf[3] == 0x02)
				{//索要定位卡参数
					if(0 == g_CtrlMsgFlag)
					{
						g_AskParFlag = 1;
						g_CtrlMsgFlag = 1;											
					}
					return _4GRET_OK;						
				}					
			}				
			else if(type == 0x08)
			{
				g_RecD6BagNo = ParameterDataBuf[i + 318]+(ParameterDataBuf[i + 319]<<8)+(ParameterDataBuf[i + 320]<<16)+(ParameterDataBuf[i + 321]<<24);
				g_RecdateToGprsFlag = 1;
				n=0;									
				for(m=0;m<140;m++)
				{
					cmdbuf[m] = (Asc2Hex(ParameterDataBuf[i + 37+m*2])<<4) + Asc2Hex(ParameterDataBuf[i+m*2+38]);
				}
				if(cmdbuf[0] == 0xCF && cmdbuf[1] == 0x91 && cmdbuf[2] == 0x02)
				{
					if(0 == g_CtrlMsgFlag)
					{												
						if(cmdbuf[3] !=0xFF && cmdbuf[3] != g_ModeInFlash.type)
						{//设置定位卡工作模式												
							//g_ModeInFlash.type = cmdbuf[3];
							//WriteModeToFlash();																												
						}
						if((cmdbuf[4] !=0xFF && cmdbuf[5] !=0xFF) && (((cmdbuf[5] << 8) | cmdbuf[4]) != g_LocationAndSelfcheckDiffFlash.Diff))
						{//发送间隔
							g_LocationAndSelfcheckDiffFlash.Diff = (cmdbuf[5] << 8) | cmdbuf[4];
							
							WriteLocationDiffToFlash();
						}
						if (cmdbuf[57] != 0xFF)
						{//是否播报单点解定位
							g_VoiceSwitch.num = cmdbuf[57];
							
							WriteVoiceSwitchToFLASH();
						}
						//定时播报设备状态
						if (cmdbuf[58] != 0xFF)
						{
							g_LocationAndSelfcheckDiffFlash.SelfcheckDiff = cmdbuf[58];
							
							WriteLocationDiffToFlash();
						}
						
						uint8_t BuffFF[7];
						uint8_t Buff00[7];
						memset(BuffFF, 0xFF, 7);
						memset(Buff00, 0x00, 7);
						//远程设置设备ID
						if(memcmp(cmdbuf+59,BuffFF,7) == 0 || memcmp(cmdbuf+59,Buff00,7) == 0)
						;//设置ID
						else
						{				
							memcpy(g_IDInFlash.IDnum,cmdbuf+59,7);						
							WriteIDToFlash();
							
							g_NeedReInitDTU = 1;
						}
						
						g_AskParFlag = 1;
						g_CtrlMsgFlag = 1;													
					}
					g_AskParFlag = 1;							
				}
			}
		}
	}
	
	return _4GRET_OK;
}

//解析远程升级程序二进制数据
_4GResult AnalysisRemoteUpgradeBinData(uint8_t *UpgradeBuf, uint16_t buffer_size)
{
	uint8_t UartUPBuf[512];
	uint8_t checkbuf[512];
	//参数校验
    if (!UpgradeBuf || buffer_size < 24) 
	{
        return _4GRET_INCOMPLETE_ERR;
    }
	
	for (uint16_t i = 0; i < buffer_size; i++)
	{
		if ((i + 512) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		
		//包头校验
		if ((UpgradeBuf[i] != 0XDC) || (UpgradeBuf[i + 1] != 0XDC) || (UpgradeBuf[i + 2] != 0XDC) || (UpgradeBuf[i + 3] != 0XDC))
		{
			continue;
		}
		
		//校验和判断
		if (UpgradeBuf[i + 511] != CheckSum(511,UpgradeBuf + i))
		{
			continue;
		}
		
		if((g_UpdateCodePageTotal.programpage == Get16ValueFrom8Buf(UpgradeBuf+ i,10))&&(g_RemoteUpdatePackNo ==Get16ValueFrom8Buf(UpgradeBuf + i,12)))
		{
			memcpy(UartUPBuf,UpgradeBuf + i + 14,496);
			//升级类型			
			g_UpdateType = UpgradeBuf[i + 4];
			
			if(g_UpdateVoiceTypeFlag == 1)
			{
				W25q_Write_N(UartUPBuf,FLASH_VOICE_START+496*g_RemoteUpdatePackNo,496);								
				W25qxx_ReadBytes(checkbuf,FLASH_VOICE_START+496*g_RemoteUpdatePackNo,496);				
			}
			else
			{			
				if(g_UpdateType == 1)
				{
					W25q_Write_N(UartUPBuf,FLASH_IAP_DIFFUSER_START1+496*g_RemoteUpdatePackNo,496);
					W25qxx_ReadBytes(checkbuf,FLASH_IAP_DIFFUSER_START1+496*g_RemoteUpdatePackNo,496);
				}
				else
				{
					W25q_Write_N(UartUPBuf,FLASH_IAP_USER_START1+496*g_RemoteUpdatePackNo,496);								
					W25qxx_ReadBytes(checkbuf,FLASH_IAP_USER_START1+496*g_RemoteUpdatePackNo,496);	
				}
			}
									
			if(memcmp(UartUPBuf,checkbuf,496) == 0)
				g_RemoteUpdatePackNo++;

			if( g_RemoteUpdatePackNo == g_UpdateCodePageTotal.programpage )
			{
				if(g_UpdateVoiceTypeFlag == 1)
				{	
					VoiceFileSize.FileSize = g_UpdateCodePageTotal.programpage;
					WriteVoiceFileSizeToFlash();
					g_UpdateVoiceTypeFlag = 0;	
					g_RemoteUpdateGCFlag = 0;   					
				}
				else
				{									
					if(g_UpdateType == 1)
					{
						DiffUpdateFileSize.FileSize = g_UpdateCodePageTotal.programpage;
						WriteDiffFileSizeToFlash();										
					}									
					else
					{	
						NormalUpdateFileSize.FileSize = g_UpdateCodePageTotal.programpage;
						WriteNormalFileSizeToFlash();
					}
					g_RemoteUpdateGCFlag = 4;   
				}
				g_UpFlag = 0;
				g_RemoteUpdatePackNo = 0;
			}
			else
			{
				g_RemoteUpdateGCFlag = 3;								
			}
		}
			
		g_RemoteUpgradeSendNum = 0;
		
		return _4GRET_OK;
	}
	
	return _4GRET_INCOMPLETE_ERR;
}

//解析远程升级升级程序类型
_4GResult AnalysisRemoteUpgradeTypeData(uint8_t *UpgradeBuf, uint16_t buffer_size)
{
	//参数校验
    if (!UpgradeBuf || buffer_size < 24) 
	{
        return _4GRET_INCOMPLETE_ERR;
    }
	
	for (uint16_t i = 0; i < buffer_size; i++)
	{
		if ((i + 24) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		
		//包头校验
		if ((UpgradeBuf[i] != 0X5A) || (UpgradeBuf[i + 1] != 0XA5) || (UpgradeBuf[i + 4] != 0XEB))
		{
			continue;
		}
		//判断校验和
		if(UpgradeBuf[i + 24] != CheckSum(24,UpgradeBuf + i))
		{
			continue; 
		}
		
		memcpy((uint8_t*)&g_UpdateCodePageTotal,UpgradeBuf + i + 11,3);							
		//if(UpgradeBuf[i + 12] == g_VersionHex[0] && UpgradeBuf[i + 13] == g_VersionHex[1])
		//{
			g_UpdateType = UpgradeBuf[i + 16];
			g_UpdateCodePageTotal.programpage = Get16ValueFrom8Buf(UpgradeBuf + i,17);
		//}
		
		g_RemoteUpgradeSendNum = 0;
		
		return _4GRET_OK;
	}
	
	return _4GRET_INCOMPLETE_ERR;
}

//解析服务器发过来的远程升级
_4GResult AnalysisRemoteUpgradeData(uint8_t *UpgradeBuf, uint16_t buffer_size)
{
	//参数校验
    if (!UpgradeBuf || buffer_size < 24) 
	{
        return _4GRET_INCOMPLETE_ERR;
    }
	
	for (uint16_t i = 0; i < buffer_size; i++)
	{
		if ((i + 24) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		
		//包头校验
		if ((UpgradeBuf[i] != 0X5A) || (UpgradeBuf[i + 1] != 0XA5) || (UpgradeBuf[i + 2] != 0X5A) || (UpgradeBuf[i + 3] != 0XA5))
		{
			continue;
		}
		
		uint8_t TempBuf[8];
		memset(TempBuf, 0XEE, 8);
		//判断8个0XEE
		if(memcmp(UpgradeBuf + i+ 4, TempBuf, 8) != 0)
		{
			continue;
		}
		
		//包序号
		g_RecD6BagNo = UpgradeBuf[i + 20]+(UpgradeBuf[i + 21]<<8)+(UpgradeBuf[i + 22]<<16)+(UpgradeBuf[i + 23]<<24);
		//程序类型、主版本号、次版本号
		memcpy((uint8_t*)&g_UpdateCodePageTotal, UpgradeBuf + i + 12, 3);
		g_UpdateCodePageTotal.programpage = Get16ValueFrom8Buf(UpgradeBuf + i,15);
		if(g_UpdateCodePageTotal.programtype == CODETYPE)
		{
			g_RemoteUpdateGCFlag = 1;								
									
			MajorVersion = UpgradeBuf[i + 13];                        		//主版本号
			MinorVersion = UpgradeBuf[i + 14];                        		//次版本号
			LastTotalBagNum = Get16ValueFrom8Buf(UpgradeBuf + i,15);  		//包总数
			NormalUpdateFileSize.FileSize = 0;
			WriteNormalFileSizeToFlash();
			DiffUpdateFileSize.FileSize = 0;
			WriteDiffFileSizeToFlash();	
			g_UpFlag =1;
//			g_NeedReInitDTU = 0;
		}
		else if(g_UpdateCodePageTotal.programtype == VOICECODETYPE && g_ShouldUpdateVoiceFlag == 0)
		{
			ReplaceVoicePackage = 1;
			g_RemoteUpdateGCFlag = 1;								
									
			MajorVersion = UpgradeBuf[i + 13];                        		//主版本号
			MinorVersion = UpgradeBuf[i + 14];                        		//次版本号
			LastTotalBagNum = Get16ValueFrom8Buf(UpgradeBuf + i,15);  		//包总数
			VoiceFileSize.FileSize = 0;
			WriteVoiceFileSizeToFlash();
			g_UpdateVoiceTypeFlag = 1;
			g_UpFlag =1;
		}	
		
		g_RemoteUpgradeSendNum = 0;
		
		return _4GRET_OK;
	}
	
	return _4GRET_INCOMPLETE_ERR;
}

/******************************************************************
函数名称: _4GResult AnalysisDiffData
功能描述: 解析服务器发送差分数据转发至RTK
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
_4GResult AnalysisDiffData(uint8_t *diffBuf, uint16_t buffer_size) 
{
	uint16_t RTKDiffNum = 0;
	uint16_t Len = 0;
	/* 参数校验 */
	if (!diffBuf || buffer_size < 32) // 最小有效包长度检查
	{ 
        return _4GRET_INCOMPLETE_ERR;
	}
	for(uint16_t i = 0;i<buffer_size;i++)
	{
		if ((i + 32) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		
		/* 包头校验 */
		if((diffBuf[i] != _4G_HEADER) || (diffBuf[i+1] != 0x00)) 
		{
			continue;
		}
		
		/* 数据类型验证 */
		if(diffBuf[i+2]!=0x03)
		{
			continue;
		}
		
		/* 获取数据长度 */
		//g_RTKDiffNum是差分数据的长度
		RTKDiffNum = (diffBuf[i+31]<<8)+diffBuf[i+30];
		//rlen是算上校验和的总长度
		uint16_t rlen = RTKDiffNum + 32;
		/* 二次长度检查 */
		if(buffer_size-i < rlen) 
		{
				delay_ms(100);
				uint16_t Rxlen = GetRingUsedLen(&g_Uart3RxRing);
				if(Rxlen > 0)
				{
					Len = Rxlen;
					RingBufPop(&g_Uart3RxRing, GsmReceBuff + buffer_size, Rxlen);
					buffer_size = buffer_size + Rxlen;
				}
			//continue;
		}

		#if 1
		//0X0D 0X0A 0X0D 0X0A 0X2B 0X4D 0X49 0X50 0X55 0X52 0X43 0X3A 0X20 0X22 0X72 0X74 0X63 0X70 0X22 0X2C 0X30 0X2C 0X31 0X37 0X32 0X2C
		//0D 0A 0D 0A 2B 4D 49 50 55 52 43 3A 20 22 72 74 63 70 22 2C 30 2C 37 31 2C
		uint16_t FirstPositon = 0;
		uint16_t SecondPositon = 0;
		uint16_t Num = 0;
		
		for(uint16_t m = i; m < buffer_size; m++)
		{
			Num++;
			
			if (Num >= rlen + Len)
			{
				break;
			}
			#if 0
			if (((m + 6) < buffer_size) && (diffBuf[m] == 0X0D) && (diffBuf[m + 1] == 0X0A) && 
				(diffBuf[m + 2] == 0X0D) && (diffBuf[m + 3] == 0X0A))
			{
				FirstPositon = m;
				
				//二次判断可能还有一个0D0A
				if ((diffBuf[m + 4] == 0X0D) && (diffBuf[m + 5] == 0X0A))
				{
					for (uint16_t n = m + 24; n < buffer_size; n++)
					{
						if (diffBuf[n] == 0X2C)
						{
							SecondPositon = n;
						
							break;
						}
					}
				}
				else
				{
					for (uint16_t n = m + 22; n < buffer_size; n++)
					{
						if (diffBuf[n] == 0X2C)
						{
							SecondPositon = n;
							break;
						}
					}
				}
				for (uint16_t j = SecondPositon + 1; j < buffer_size; j++, FirstPositon++)
				{
					diffBuf[FirstPositon] = diffBuf[j];
				}
				
				break;
			}
			#endif
			#if 1
			if (((m + 6) < buffer_size) && (diffBuf[m] == 0X0D) && (diffBuf[m + 1] == 0X0A) && 
				(diffBuf[m + 2] == 0X0D) && (diffBuf[m + 3] == 0X0A) && (diffBuf[m + 4] == 0X2B) &&
				(diffBuf[m + 5] == 0X4D) && (diffBuf[m + 6] == 0X49))
			{
				FirstPositon = m;
				
				for (uint16_t n = m + 22; n < buffer_size; n++)
				{
					if (diffBuf[n] == 0X2C)
					{
						SecondPositon = n;
						
						for (uint16_t j = SecondPositon + 1; j < buffer_size; j++, FirstPositon++)
						{
							diffBuf[FirstPositon] = diffBuf[j];
						}
						
						break;
					}
				}

				break;
			}
			#endif
		}
		#endif
		/* 校验和验证 */
		if ((i+rlen) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		uint8_t calc_sum = CheckSum(rlen, diffBuf+i);
		if(calc_sum != diffBuf[i+rlen]) 
		{
			//UartSend(&g_Uart1,g_RxBuf3,UART3_RX_BUF_SIZE);
			continue;
		}
		/* 其他业务逻辑... */
		UartSend(&g_Uart2,diffBuf+i+32,RTKDiffNum);
		
		//delay_ms(1000);
		//UartSend(&g_Uart1,diffBuf+i+32,RTKDiffNum);
		return _4GRET_OK; // 需定义成功返回值
	}
	return _4GRET_INCOMPLETE_ERR;
}

/******************************************************************
函数名称: Bool SendD6Data(void)
功能描述: 发送D6数据
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：hly
******************************************************************/
void SendD6Data(uint8_t *data, uint8_t type)
{
	uint8_t tmpsendbuf[560];
    uint8_t i,cmpbuf[5],tmpmobile[6],gsm7bit[140],tmptime[6];

	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
    memset(cmpbuf,0,sizeof(cmpbuf));
	
	tmpsendbuf[0] = 0xD6;
	memcpy(tmpsendbuf + 1,g_D6Phone,6);	//接收手机号
	tmpsendbuf[7] = g_SendD6BagNo;						//D6数据包
	if(g_MobileInFlash.Crc16 == CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2))
    {
        if((CheckNumIsBCD((uint8_t*)g_MobileInFlash.mobileno[4],5) == true)&&
            (memcmp((uint8_t*)g_MobileInFlash.mobileno[4],cmpbuf,5)!=0))
		{
			for(i=0;i<6;i++)
			{
				tmpmobile[i]=CharSwap(g_MobileInFlash.mobileno[4][i]);	//手机号每个字节高低位交换
			}
			for( i = 0 ; i < 6  ; i++ )
			{
				Bcd2Asc(tmpmobile[i],tmpsendbuf+8+2*i);				//转换ASCII码
			}
			if(type == D67bit)
			{
				memset(tmpsendbuf+20,0x30,4);							//协议标识为0，编码方式为00-7bit					
				TimeHexToBcd(&g_BCDTime,(uint8_t*)&g_SysTime);
				tmptime[0] = CharSwap(g_BCDTime.year);
				tmptime[1] = CharSwap(g_BCDTime.month);
				tmptime[2] = CharSwap(g_BCDTime.date);
				tmptime[3] = CharSwap(g_BCDTime.hour);
				tmptime[4] = CharSwap(g_BCDTime.minute);
				tmptime[5] = CharSwap(g_BCDTime.second);
				for( i = 0 ; i < 6  ; i++ )
				{
					Bcd2Asc(tmptime[i],tmpsendbuf+24+2*i);		//年月日时分秒
				}
				i=0;														//时区
				Bcd2Asc(i,tmpsendbuf+36);
				i=160;														//短信长度
				Bcd2Asc(i,tmpsendbuf+38);
				
				for( i = 0 ; i < 80  ; i++ )								//80字节转换ASCII码-160
				{
					Bcd2Asc(*data,g_CommanBuf+2*i);
					data++;
				}
				gsmEncode7bit((char*)g_CommanBuf,gsm7bit,160);					//转换7位编码-140
				for( i = 0 ; i < 140  ; i++ )								//140字节转换ASCII码-280
				{
					Bcd2Asc(gsm7bit[i],tmpsendbuf+40+2*i);
				}
				tmpsendbuf[320] = CheckSum(320,tmpsendbuf);
				UartSend(&g_Uart3,tmpsendbuf,321);
			}
			else
			{
				memset(tmpsendbuf+20,0x30,2);							//协议标识为0
				tmpsendbuf[22] = 0x30;									//编码方式为04-8bit
				tmpsendbuf[23] = 0x34;
				TimeHexToBcd(&g_BCDTime,(uint8_t*)&g_SysTime);
				tmptime[0] = CharSwap(g_BCDTime.year);
				tmptime[1] = CharSwap(g_BCDTime.month);
				tmptime[2] = CharSwap(g_BCDTime.date);
				tmptime[3] = CharSwap(g_BCDTime.hour);
				tmptime[4] = CharSwap(g_BCDTime.minute);
				tmptime[5] = CharSwap(g_BCDTime.second);
				for( i = 0 ; i < 6  ; i++ )
				{
					Bcd2Asc(tmptime[i],tmpsendbuf+24+2*i);		//年月日时分秒
				}
				i=0;														//时区
				Bcd2Asc(i,tmpsendbuf+36);
				i=140;														//短信长度
				Bcd2Asc(i,tmpsendbuf+38);
				
				for( i = 0 ; i < 140  ; i++ )								//140字节转换ASCII码-280
				{
					Bcd2Asc(data[i],tmpsendbuf+40+2*i);
				}
				tmpsendbuf[320] = CheckSum(320,tmpsendbuf);
				UartSend(&g_Uart3,tmpsendbuf,321);
			}
		}
	}
}

/******************************************************************
函数名称: void ReturnMobileNoToNet(void)
功能描述: 向服务器返回控制权限号
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReturnMobileNoToNet(void)
{
    uint8_t i,tmpbuf[80];

    memset(tmpbuf,0,sizeof(tmpbuf));
	
    memset(tmpbuf,0xA8,6);
    tmpbuf[6] = 0x01;

    for(i=0;i<2;i++)
    {
        memcpy(tmpbuf+7+6*i,g_MobileInFlash.mobileno[i],6);    
    }
    memset(tmpbuf+19,0xff,12);                        //路局号和工务段号
    memset(tmpbuf+31,0,4);            
    memcpy(tmpbuf+35,g_MobileInFlash.mobileno[4],6);  //本机号
    memcpy(tmpbuf+41,g_MobileInFlash.mobileno[3],6);  //短信接收号
    memset(tmpbuf+47,0,33);
    SendD6Data(tmpbuf,D67bit);
}

/******************************************************************
函数名称: void ReturnIDtoNet(void)
功能描述: 向服务器返回ID版本号
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
void ReturnIDtoNet(void)
{
	uint8_t tmpbuf[80];
	
    memset(tmpbuf,0,sizeof(tmpbuf));

    tmpbuf[0] = 0xCF;
	tmpbuf[1] = 0x80;	
	tmpbuf[2] = 0x0B;
	tmpbuf[3] = 0x01;
	memcpy(tmpbuf+36,g_VersionAscii,4);
	memcpy(tmpbuf+40,g_VersioninFlash.bootversion, 4); 
    memset(tmpbuf + 44,0,35);	
	tmpbuf[79] = CheckSum(79,tmpbuf);	
	SendD6Data(tmpbuf,D67bit);
}

void ReturnIpAndPort(void)
{
	#if 0
	uint8_t tmpbuf[80];
	
    memset(tmpbuf,0,sizeof(tmpbuf));

    tmpbuf[0] = 0xCF;
	tmpbuf[1] = 0x80;	
	tmpbuf[2] = 0x0B;
	tmpbuf[3] = 0x01;
	memcpy(tmpbuf+36,g_VersionAscii,4);
	memcpy(tmpbuf+40,g_VersioninFlash.bootversion, 4); 
    memset(tmpbuf + 44,0,35);	
	tmpbuf[79] = CheckSum(79,tmpbuf);	
	SendD6Data(tmpbuf,D67bit);
	#endif
}

void ReturnPartoNet(void)
{
	uint8_t tmpbuf[140];
	
    memset(tmpbuf,0,sizeof(tmpbuf));

    tmpbuf[0] = 0xCF;
	tmpbuf[1] = 0x91;	
	tmpbuf[2] = 0x02;
	tmpbuf[3] = g_ModeInFlash.type;
	tmpbuf[4] = g_LocationAndSelfcheckDiffFlash.Diff & 0xFF; 
    tmpbuf[5] = (g_LocationAndSelfcheckDiffFlash.Diff >> 8) & 0xFF;
	tmpbuf[57] = g_VoiceSwitch.num;
	tmpbuf[58] = g_LocationAndSelfcheckDiffFlash.SelfcheckDiff;
	memcpy(tmpbuf + 59, g_IDInFlash.IDnum, 7);
	tmpbuf[139] = CheckSum(139,tmpbuf);	
	SendD6Data(tmpbuf,D68bit);
}

/******************************************************************
函数名称: void SendRecSuc(void)
功能描述: 向服务器返回接收成功指令
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：hly
******************************************************************/
void SendRecSuc(void)
{
	uint8_t tmpsendbuf[6];
	
	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
	tmpsendbuf[0] = 0xDF;
	tmpsendbuf[1] = g_RecD6BagNo&0xFF;
	tmpsendbuf[2] = (g_RecD6BagNo>>8)&0xFF;
	tmpsendbuf[3] = (g_RecD6BagNo>>16)&0xFF;
	tmpsendbuf[4] = (g_RecD6BagNo>>24)&0xFF;
	tmpsendbuf[5] = CheckSum(5,tmpsendbuf);		
	UartSend(&g_Uart3,tmpsendbuf,6);
}

void RYDW_GenerateCardInfo(CardInfoTable_t CardTable, uint8_t *RecordBuf)
{
	BcdDateTime_t bcdtime;
	uint8_t chksum_t;
	uint8_t SendWarnFlag = 0;
	static uint8_t SendPreWorkPlanIDWarnNum = 0;
	
	/* 生成记录数据头 */
	RecordBuf[0] = 0xFA;
	RecordBuf[1] = 0x00;
	RecordBuf[2] = 0x01;
	/* 本机号 */
	RecordBuf[3] = g_MobileInFlash.mobileno[4][0];
	RecordBuf[4] = g_MobileInFlash.mobileno[4][1];
	RecordBuf[5] = g_MobileInFlash.mobileno[4][2];
	RecordBuf[6] = g_MobileInFlash.mobileno[4][3];
	RecordBuf[7] = g_MobileInFlash.mobileno[4][4];
	RecordBuf[8] = g_MobileInFlash.mobileno[4][5];
	//卡ID
	memcpy(RecordBuf + 9, CardTable.Info[0].ID.array, 7);
	//设备类型
	RecordBuf[16] = 0X01;
	//时间(BCD码)
	bcdtime = HexTime2BcdTime(g_SysTime);
	memcpy(RecordBuf + 17, (uint8_t*)&bcdtime, 6);
	
	RecordBuf[23] = 0XFF;
	RecordBuf[24] = 0XFF;
	RecordBuf[25] = 0XFF;
	RecordBuf[26] = 0XFF;
	
	//定位质量大于3和在区域里
	if (g_PosQuality > 3)
	{
		//定位卡选出来时，g_BigAreaType = 12
		if (g_BigAreaType > 0)
		{
			//服务器没有下发下来计划时，上传定位卡自己选出来的计划
			if (g_IssueWorkPlan == 0)
			{
				memcpy(RecordBuf + 23, SelectEFenceInfo.order, 4);
				SendPreWorkPlanIDWarnNum = 0;
				
				SendWarnFlag = 1;
			}
			//则上传服务器下发下来的计划
			else 
			{
				if ((SelectEFenceInfo.order[0] == 0xFF) && (SelectEFenceInfo.order[1] == 0xFF) && 
					(SelectEFenceInfo.order[2] == 0xFF) && (SelectEFenceInfo.order[3] == 0xFF))
				{
					g_IssueWorkPlan = 0;
					PreWorkPlanID = 0;
				}
				else
				{
					if (g_IssueWorkPlan == PreWorkPlanID)
					{
						if (g_IsUpdateWorkTime == 1)
						{
							for (int8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
							{
								if ((g_WorkPlanSummary[i].isanalysis == 1) && (g_WorkPlanSummary[i].WorkPlanID == g_IssueWorkPlan))
								{
									g_WorkTime.start = g_WorkPlanSummary[i].start;
									g_WorkTime.stop = g_WorkPlanSummary[i].stop;
								}
							}
							
							g_IsUpdateWorkTime = 0;
						}
						
						ConvUint32ToArray(g_IssueWorkPlan, RecordBuf + 23, M_BIG_ENDIAN);
						SendPreWorkPlanIDWarnNum = 0;
					
						PreWorkPlanID = g_IssueWorkPlan;
					}
					else if (g_IssueWorkPlan != PreWorkPlanID)
					{
						if ((SendPreWorkPlanIDWarnNum > 5) || (PreWorkPlanID == 0))
						{
							PreWorkPlanID = g_IssueWorkPlan;
							SendPreWorkPlanIDWarnNum = 0;
						
							for (int8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
							{
								if ((g_WorkPlanSummary[i].isanalysis == 1) && (g_WorkPlanSummary[i].WorkPlanID == g_IssueWorkPlan))
								{
									g_WorkTime.start = g_WorkPlanSummary[i].start;
									g_WorkTime.stop = g_WorkPlanSummary[i].stop;
								}
							}
						}
						else
						{
							SendPreWorkPlanIDWarnNum++;
						}
		
						g_CarWarnFlag.all = 0X00;
						g_TrainInfo.TrainApproach = 0;
						g_TrainInfo.AppTrainApproach = 0;
		
						ConvUint32ToArray(PreWorkPlanID, RecordBuf + 23, M_BIG_ENDIAN);
					}
					
					SendWarnFlag = 1;
				}
			}
		}
		//服务器下发下来时的计划的开始时间和结束时间
		else if ((JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == SAME) || 
			(JudgTimeOrder(g_SysTime,g_WorkTime.start,0) == AFTER))
		{
			if ((SelectEFenceInfo.order[0] == 0xFF) && (SelectEFenceInfo.order[1] == 0xFF) && 
					(SelectEFenceInfo.order[2] == 0xFF) && (SelectEFenceInfo.order[3] == 0xFF))
			{
				g_IssueWorkPlan = 0;
				PreWorkPlanID = 0;
			}
			else
			{
				if (g_IssueWorkPlan == PreWorkPlanID)
				{
					if (g_IsUpdateWorkTime == 1)
					{
						for (int8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
						{
							if ((g_WorkPlanSummary[i].isanalysis == 1) && (g_WorkPlanSummary[i].WorkPlanID == g_IssueWorkPlan))
							{
								g_WorkTime.start = g_WorkPlanSummary[i].start;
								g_WorkTime.stop = g_WorkPlanSummary[i].stop;
							}
						}
							
						g_IsUpdateWorkTime = 0;
					}
					
					ConvUint32ToArray(g_IssueWorkPlan, RecordBuf + 23, M_BIG_ENDIAN);
					SendPreWorkPlanIDWarnNum = 0;
					
					PreWorkPlanID = g_IssueWorkPlan;
				}
				else if (g_IssueWorkPlan != PreWorkPlanID)
				{
					if ((SendPreWorkPlanIDWarnNum > 5) || (PreWorkPlanID == 0))
					{
						PreWorkPlanID = g_IssueWorkPlan;
						SendPreWorkPlanIDWarnNum = 0;
						
						for (int8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
						{
							if ((g_WorkPlanSummary[i].isanalysis == 1) && (g_WorkPlanSummary[i].WorkPlanID == g_IssueWorkPlan))
							{
								g_WorkTime.start = g_WorkPlanSummary[i].start;
								g_WorkTime.stop = g_WorkPlanSummary[i].stop;
							}
						}
					}
					else
					{
						SendPreWorkPlanIDWarnNum++;
					}
		
					g_CarWarnFlag.all = 0X00;
					g_TrainInfo.TrainApproach = 0;
					g_TrainInfo.AppTrainApproach = 0;
		
					ConvUint32ToArray(PreWorkPlanID, RecordBuf + 23, M_BIG_ENDIAN);
				}
					
				SendWarnFlag = 1;
			}
		}
	}
	
	//预留
	RecordBuf[27] = 0X00;
	RecordBuf[28] = 0X00;
	RecordBuf[29] = 0X00;
	//数据长度
	RecordBuf[30] = 32;
	RecordBuf[31] = 0;
	//定位数据部分
	//时间
	RecordBuf[32] = g_SysTime.year;
	RecordBuf[33] = g_SysTime.month;
	RecordBuf[34] = g_SysTime.date;
	RecordBuf[35] = g_SysTime.hour;
	RecordBuf[36] = g_SysTime.minute;
	RecordBuf[37] = g_SysTime.second;
	//卡纬度
	ConvUint32ToArray(CardTable.Info[0].Latitude, RecordBuf + 38, M_LITTLE_ENDIAN);
	//卡经度
	ConvUint32ToArray(CardTable.Info[0].Longitude, RecordBuf + 42, M_LITTLE_ENDIAN);
	//定位质量
	RecordBuf[46] = CardTable.Info[0].PosQuality;
	//海拔高度
	memcpy(RecordBuf + 47, CardTable.Info[0].Altitude, 3);
	//报警标记
	RecordBuf[50] = CardTable.Info[0].Alram.all;
	//电池电量
	RecordBuf[51] = CardTable.Info[0].BatVoltage;
	
	//报警类型
	if (SendWarnFlag == 1)
	{
		ConvUint16ToArray(g_CarWarnFlag.all, RecordBuf + 52, M_LITTLE_ENDIAN);
		RecordBuf[54] = (uint8_t)(g_CarWarnFlag.all >> 16);
	}
	else
	{
		RecordBuf[52] = 0X00;
		RecordBuf[53] = 0X00;
		RecordBuf[54] = 0X00;
	}
	
	//速度
	memcpy(RecordBuf + 55, CardTable.Info[0].SpeedKnots, 2);
	//星数
	RecordBuf[57] = CardTable.Info[0].ViewNumber;
	//区域类型
	RecordBuf[58] = CardTable.Info[0].AreaType;
	//预留
	RecordBuf[59] = g_tGPS.fDiffAge&0xFF;	//差分龄期
	RecordBuf[60] = (g_tGPS.fDiffAge>>8)&0xFF;
	RecordBuf[61] = 0X00;
	RecordBuf[62] = 0X00;
	RecordBuf[63] = 0X00;
	//校验和
	chksum_t = CheckSum(64, RecordBuf);
	RecordBuf[64] = chksum_t;
	
	if (g_CardSNum >= 0xFFFFFFFF)
	{
		g_CardSNum = 0;
	}
	else
	{
		g_CardSNum++;
	}
	
	ConvUint32ToArray(g_CardSNum, RecordBuf + 65, M_BIG_ENDIAN);
}

/******************************************************************************
函数名称: BcdDateTime_t HexTime2BcdTime
功能描述: 十六进制时间转换为BCD格式时间
参数输入: HexDateTime_t	*HexTime	十六进制时间
参数输出: 
返 回 值: BcdDateTime_t	*BcdTime	BCD格式时间
其    他: 无
********************************************************************************/
BcdDateTime_t HexTime2BcdTime(DATETIME HexTime)
{
	BcdDateTime_t tmpd;
	
	tmpd.Year = Hex2Bcd(HexTime.year);
	tmpd.Month = Hex2Bcd(HexTime.month);
	tmpd.Day = Hex2Bcd(HexTime.date);
	tmpd.Hour = Hex2Bcd(HexTime.hour);
	tmpd.Minute = Hex2Bcd(HexTime.minute);
	tmpd.Second = Hex2Bcd(HexTime.second);
	
	return tmpd;
}

/******************************************************************
函数名称: void RYDW_SendCardInfo
功能描述: 发送定位卡数据记录
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_SendCardInfo(void)
{
	memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&(g_IDInFlash.IDnum), sizeof(g_IDInFlash.IDnum));			
	g_CardLocaData.BatVoltage = g_BatVal; 
	if(g_CarWarnFlag.all > 0)
		g_CardLocaData.Alram.bit.Alram= 1;
	g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
	g_CardLocaData.Alram.bit.SOS=0 ;
	if(g_ManualPunFlag == 2)
	{
		g_CardLocaData.Alram.bit.Bribe = 0x01;
		g_ManualPunFlag = 0;
	}
	else
	g_CardLocaData.Alram.bit.Bribe = 0;

	if (g_SelfCheck.StartCharge == 1)
	{
		g_CardLocaData.Alram.bit.Charge = 1;
	}		
	else 
	{
		g_CardLocaData.Alram.bit.Charge = 0;
	}
	
	g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
	g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
	g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;		
			
	g_CardLocaData.PosQuality = g_PosQuality;
	g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>16)&0xFF;
	g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>8)&0xFF;
	g_CardLocaData.Altitude[2] = (g_tGPS.Altitude)&0xFF;
	
	g_CardLocaData.SpeedKnots[0] = (g_tGPS.SpeedKnots)&0xFF;
	g_CardLocaData.SpeedKnots[1] = (g_tGPS.SpeedKnots>>8)&0xFF;

	g_CardLocaData.FenceOrder= 0;                //电子围栏编号
	g_CardLocaData.CTimeOrder = g_WorkTime.num;             //施工时间编号
	g_CardLocaData.ViewNumber = g_tGPS.ViewNumber;            /*卫星个数*/            //星数
	g_CardLocaData.AreaType = g_AreaType;               //区域类型							
	for (uint8_t i=0; i<7; i++)
	{
		g_CardTable.Info->ID.array[i] = g_CardLocaData.ID.array[i];
	}
	g_CardTable.Info->Latitude = g_CardLocaData.Latitude;
	g_CardTable.Info->Longitude = g_CardLocaData.Longitude;
	g_CardTable.Info->PosQuality = g_CardLocaData.PosQuality;
	memcpy(g_CardTable.Info->Altitude, g_CardLocaData.Altitude, 3);
	memcpy(g_CardTable.Info->SpeedKnots, g_CardLocaData.SpeedKnots, 2);
	g_CardTable.Info->Alram.all = g_CardLocaData.Alram.all;
	g_CardTable.Info->BatVoltage = g_CardLocaData.BatVoltage;
	g_CardTable.Info->AlramType.all = g_CardLocaData.AlramType.all;
	g_CardTable.Info->FenceOrder = g_CardLocaData.FenceOrder;	
	g_CardTable.Info->CTimeOrder = g_CardLocaData.CTimeOrder;	
	g_CardTable.Info->ViewNumber = g_CardLocaData.ViewNumber;
	g_CardTable.Info->AreaType = g_CardLocaData.AreaType;
	memcpy(g_CardTable.Info->Reserver, g_CardLocaData.Reserver, 6);
	memset(Cardinfo,0,sizeof(Cardinfo));
	RYDW_GenerateCardInfo(g_CardTable,Cardinfo);
}

//索要施工计划
void RYDW_AskWorkPlan()
{
	uint8_t Result = 0;
	uint8_t DataBuff[32];
	BcdDateTime_t bcdtime;
	uint8_t chksum_t;
	uint16_t Index = 0;
	uint8_t answerbuff[3072];
	
	/* 生成记录数据头 */
	answerbuff[0] = 0xFA;
	answerbuff[1] = 0x00;
	answerbuff[2] = 0x80;
	/* 本机号 */
	answerbuff[3] = g_MobileInFlash.mobileno[4][0];
	answerbuff[4] = g_MobileInFlash.mobileno[4][1];
	answerbuff[5] = g_MobileInFlash.mobileno[4][2];
	answerbuff[6] = g_MobileInFlash.mobileno[4][3];
	answerbuff[7] = g_MobileInFlash.mobileno[4][4];
	answerbuff[8] = g_MobileInFlash.mobileno[4][5];
	//卡ID
	memcpy(answerbuff + 9, g_CardLocaData.ID.array, 7);

	//设备类型
	answerbuff[16] = 0X01;
	//时间(BCD码)
	bcdtime = HexTime2BcdTime(g_SysTime);
	memcpy(answerbuff + 17, (uint8_t*)&bcdtime, 6);
	
	answerbuff[23] = 0XFF;
	answerbuff[24] = 0XFF;
	answerbuff[25] = 0XFF;
	answerbuff[26] = 0XFF;
	
	//索要概要的施工计划
	if (g_AskWorkPlan == 2)
	{
		memcpy(answerbuff + 23, SelectEFenceInfo.order, 4);
	}
	
	//预留
	answerbuff[27] = 0X00;
	answerbuff[28] = 0X00;
	answerbuff[29] = 0X00;
	
	//标记
	answerbuff[32] = g_AskWorkPlan;
	//预留
	answerbuff[33] = 0x00;
	answerbuff[34] = 0x00;
	answerbuff[35] = 0x00;
	answerbuff[36] = 0x00;
	answerbuff[37] = 0x00;
	answerbuff[38] = 0x00;
	answerbuff[39] = 0x00;

	Index = 40;
	//索要flash中施工计划
	if (g_AskWorkPlan == 1)
	{
		for (uint8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
		{
			memset(DataBuff, 0x00, 32);
			Result = AskFlashWorkplan(i, DataBuff);
			if (Result == 1)
			{
				memcpy(answerbuff + Index, DataBuff, 32);
				Index = Index + 32;
			}
		}
	}
	//索要概要中施工计划
	else if (g_AskWorkPlan == 2)
	{
		for (uint8_t i = 0; i < MAX_WORK_PLAN_NUM; i++)
		{
			memset(DataBuff, 0x00, 32);
			if (g_WorkPlanSummary[i].isanalysis == 1)
			{
				AskSummaryWorkplan(i, DataBuff);
				memcpy(answerbuff + Index, DataBuff, 32);
				Index = Index + 32;
			}
		}
	}
	
	//数据长度
	answerbuff[30] = (Index- 32) & 0xFF;
	answerbuff[31] = ((Index - 32)>> 8) & 0xFF;
	
	//校验和
	chksum_t = CheckSum(Index, answerbuff);
	answerbuff[Index] = chksum_t;
	
	memcpy(answerbuff + Index + 1, AskWorkPlanSNum, 4);
	UartSend(&g_Uart3, answerbuff, Index + 5);
}

//给服务器发送蓝牙信息(报警和心跳)
void RYDW_SendLYInfo()
{
	BcdDateTime_t bcdtime;
	uint8_t answerbuff[41];
	memset(answerbuff,0,sizeof(answerbuff));
	
	//包头
	answerbuff[0] = 0X5A;
	answerbuff[1] = 0XA5;
	//长度
	answerbuff[2] = 0X20;
	answerbuff[3] = 0X00;
	answerbuff[4] = 0XB9;
	answerbuff[5] = 0X63;
	
	/* 本机号 */
	answerbuff[6] = g_MobileInFlash.mobileno[4][0];
	answerbuff[7] = g_MobileInFlash.mobileno[4][1];
	answerbuff[8] = g_MobileInFlash.mobileno[4][2];
	answerbuff[9] = g_MobileInFlash.mobileno[4][3];
	answerbuff[10] = g_MobileInFlash.mobileno[4][4];
	answerbuff[11] = g_MobileInFlash.mobileno[4][5];
	
	//时间(BCD码)
	bcdtime = HexTime2BcdTime(g_SysTime);
	memcpy(answerbuff + 12, (uint8_t*)&bcdtime, 6);
	
	//如果是开机/心跳包
	if ((RemoteControl_data[0] == 0X00) || (RemoteControl_data[0] == 0X02))
	{
		//如果是开机信息
		if (RemoteControl_data[0] == 0X00)
		{
			answerbuff[18] = 0X00;
		}
		//如果是心跳信息
		else if (RemoteControl_data[0] == 0X02)
		{
			answerbuff[18] = 0X01;
		}
		else
		{
			return;
		}
		//设备编号
		answerbuff[19] = 0XFF;
		//数据类型
		answerbuff[20] = 0X00;
		//报警主机版本号
		answerbuff[21] = 0X00;
		answerbuff[22] = 0X00;
		//检测从机版本号
		answerbuff[23] = 0X00;
		answerbuff[24] = 0X00;
		//手机模块信号强度
		answerbuff[25] = 0X00;
		//设备类型
		answerbuff[26] = 0X04;
		//施工计划ID
		if(((g_PosQuality == 4) || (g_PosQuality == 5)) && (g_WorkTime.start.year !=0 ) && 
				(g_WorkTime.start.month != 0))
		{
			ConvUint32ToArray(g_IssueWorkPlan, answerbuff + 27, M_BIG_ENDIAN);
		}
		else 
		{
			answerbuff[27] = 0XFF;
			answerbuff[28] = 0XFF;
			answerbuff[29] = 0XFF;
			answerbuff[30] = 0XFF;
		}
		//报警主机电量
		answerbuff[31] = 0X00;
		answerbuff[32] = 0X00;
		answerbuff[33] = 0X00;
		//检测从机电量
		answerbuff[34] = 0X00;
		//校验和
		answerbuff[35] = CheckSum(31, answerbuff + 4);
	}
	//如果是报警数据
	else if (RemoteControl_data[0] == 0X01)
	{
		//类型
		answerbuff[18] = 0X02;
		//设备编号
		//上行
		if ((RemoteControl_data[1] == 0X02) || (RemoteControl_data[1] == 0X04))
		{
			answerbuff[19] = 0X01;
		}
		//下行
		else if ((RemoteControl_data[1] == 0X01) || (RemoteControl_data[1] == 0X03))
		{
			answerbuff[19] = 0X02;
		}
		else 
		{
			return;
		}
		
		//数据类型
		//列车接近预告报警
		if ((RemoteControl_data[1] == 0X03) || (RemoteControl_data[1] == 0X04))
		{
			answerbuff[20] = 0X01;
		}
		//列车接近预告报警解除
		else if ((RemoteControl_data[1] == 0X01) || (RemoteControl_data[1] == 0X02))
		{
			answerbuff[20] = 0X02;
		}
		else 
		{
			return;
		}
		
		//设备类型
		answerbuff[21] = 0X04;
		//预留
		answerbuff[22] = 0XFF;
		//校验和
		answerbuff[35] = CheckSum(31, answerbuff + 4);
	}
	else
	{
		return;
	}
	
	g_CardSNum++;
	
	if ((RemoteControl_data[0] == 0X00) || (RemoteControl_data[0] == 0X02))
	{
		g_LYHeartBeatSNum = g_CardSNum;
	}
	else if (RemoteControl_data[0] == 0X01)
	{
		g_LYAlarmSNum = g_CardSNum;
	}
	else
	{
		return;
	}
	
	ConvUint32ToArray(g_CardSNum, answerbuff + 36, M_BIG_ENDIAN);
	answerbuff[40] = CheckSum(40, answerbuff);
	
	UartSend(&g_Uart3,answerbuff,sizeof(answerbuff));
}

/******************************************************************
函数名称: void RYDW_AnswerServer
功能描述: 应答服务器
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_AnswerPlanServer(void)
{
	uint8_t answerbuff[41];
	memset(answerbuff,0,sizeof(answerbuff));
	//头
	answerbuff[0]=0x5A;
	answerbuff[1]=0xA5;
	//长度
	answerbuff[2]=0x20;
	answerbuff[3]=0x00;
	//类型码
	answerbuff[4]=0x01;
	//数据类型
	answerbuff[5]=0x04;
	//本机号
	answerbuff[6]=g_MobileInFlash.mobileno[4][0];
	answerbuff[7]=g_MobileInFlash.mobileno[4][1];
	answerbuff[8]=g_MobileInFlash.mobileno[4][2];
	answerbuff[9]=g_MobileInFlash.mobileno[4][3];
	answerbuff[10]=g_MobileInFlash.mobileno[4][4];
	answerbuff[11]=g_MobileInFlash.mobileno[4][5];
	//卡ID
	memcpy(answerbuff + 12, &(g_IDInFlash.IDnum), 7);
	//设备类型
	answerbuff[19]=0x01;
	//预留
	memset(answerbuff + 20, 0, 15);
	//校验和
	answerbuff[35]=CheckSum(31, answerbuff+4);
	//序列号
	memcpy(answerbuff + 36, WorkPlanSNum, 4);
	//校验和
	answerbuff[40]=CheckSum(40, answerbuff);
	
	UartSend(&g_Uart3,answerbuff,sizeof(answerbuff));
}

/******************************************************************
函数名称: void RYDW_AnswerTrainServer
功能描述: 应答服务器
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_AnswerTrainServer(void)
{
	uint8_t answerbuff[41];
	memset(answerbuff,0,sizeof(answerbuff));
	//头
	answerbuff[0]=0x5A;
	answerbuff[1]=0xA5;
	//长度
	answerbuff[2]=0x20;
	answerbuff[3]=0x00;
	//类型码
	answerbuff[4]=0x01;
	//数据类型
	answerbuff[5]=0x06;
	//本机号
	answerbuff[6]=g_MobileInFlash.mobileno[4][0];
	answerbuff[7]=g_MobileInFlash.mobileno[4][1];
	answerbuff[8]=g_MobileInFlash.mobileno[4][2];
	answerbuff[9]=g_MobileInFlash.mobileno[4][3];
	answerbuff[10]=g_MobileInFlash.mobileno[4][4];
	answerbuff[11]=g_MobileInFlash.mobileno[4][5];
	//卡ID
	memcpy(answerbuff + 12, &(g_IDInFlash.IDnum), 7);
	//设备类型
	answerbuff[19]=0x01;
	//预留
	memset(answerbuff + 20, 0, 15);
	//校验和
	answerbuff[35]=CheckSum(31, answerbuff+4);
	//序列号
	memcpy(answerbuff + 36, TrainInfoSNum, 4);
	//校验和
	answerbuff[40]=CheckSum(40, answerbuff);
	UartSend(&g_Uart3,answerbuff,sizeof(answerbuff));
}

/******************************************************************
函数名称: void RYDW_AnswerCarServer
功能描述: 应答服务器
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_AnswerCarServer(void)
{
	uint8_t answerbuff[41];
	memset(answerbuff,0,sizeof(answerbuff));
	//头
	answerbuff[0]=0x5A;
	answerbuff[1]=0xA5;
	//长度
	answerbuff[2]=0x20;
	answerbuff[3]=0x00;
	//类型码
	answerbuff[4]=0x01;
	//数据类型
	answerbuff[5]=0x0F;
	//本机号
	answerbuff[6]=g_MobileInFlash.mobileno[4][0];
	answerbuff[7]=g_MobileInFlash.mobileno[4][1];
	answerbuff[8]=g_MobileInFlash.mobileno[4][2];
	answerbuff[9]=g_MobileInFlash.mobileno[4][3];
	answerbuff[10]=g_MobileInFlash.mobileno[4][4];
	answerbuff[11]=g_MobileInFlash.mobileno[4][5];
	//卡ID
	memcpy(answerbuff + 12, &(g_IDInFlash.IDnum), 7);
	//设备类型
	answerbuff[19]=0x01;
	//预留
	memset(answerbuff + 20, 0, 15);
	//校验和
	answerbuff[35]=CheckSum(31, answerbuff+4);
	//序列号
	memcpy(answerbuff + 36, CarSNum, 4);
	//校验和
	answerbuff[40]=CheckSum(40, answerbuff);
	UartSend(&g_Uart3,answerbuff,sizeof(answerbuff));
}

/******************************************************************
函数名称: void RYDW_AnswerCarServer
功能描述: 应答服务器
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_AnswerProtectionPositionServer(void)
{
	uint8_t answerbuff[41];
	memset(answerbuff,0,sizeof(answerbuff));
	//头
	answerbuff[0]=0x5A;
	answerbuff[1]=0xA5;
	//长度
	answerbuff[2]=0x20;
	answerbuff[3]=0x00;
	//类型码
	answerbuff[4]=0x01;
	//数据类型
	answerbuff[5]=0x10;
	//本机号
	answerbuff[6]=g_MobileInFlash.mobileno[4][0];
	answerbuff[7]=g_MobileInFlash.mobileno[4][1];
	answerbuff[8]=g_MobileInFlash.mobileno[4][2];
	answerbuff[9]=g_MobileInFlash.mobileno[4][3];
	answerbuff[10]=g_MobileInFlash.mobileno[4][4];
	answerbuff[11]=g_MobileInFlash.mobileno[4][5];
	//卡ID
	memcpy(answerbuff + 12, &(g_IDInFlash.IDnum), 7);
	//设备类型
	answerbuff[19]=0x01;
	//预留
	memset(answerbuff + 20, 0, 15);
	//校验和
	answerbuff[35]=CheckSum(31, answerbuff+4);
	//序列号
	memcpy(answerbuff + 36, ProtectionPositionSum, 4);
	//校验和
	answerbuff[40]=CheckSum(40, answerbuff);
	UartSend(&g_Uart3,answerbuff,sizeof(answerbuff));
}

/******************************************************************
函数名称: void RYDW_AnswerConstructionSignServer
功能描述: 应答服务器
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_AnswerConstructionSignServer(void)
{
	uint8_t answerbuff[41];
	memset(answerbuff,0,sizeof(answerbuff));
	//头
	answerbuff[0]=0x5A;
	answerbuff[1]=0xA5;
	//长度
	answerbuff[2]=0x20;
	answerbuff[3]=0x00;
	//类型码
	answerbuff[4]=0x01;
	//数据类型
	answerbuff[5]=0x0C;
	//本机号
	answerbuff[6]=g_MobileInFlash.mobileno[4][0];
	answerbuff[7]=g_MobileInFlash.mobileno[4][1];
	answerbuff[8]=g_MobileInFlash.mobileno[4][2];
	answerbuff[9]=g_MobileInFlash.mobileno[4][3];
	answerbuff[10]=g_MobileInFlash.mobileno[4][4];
	answerbuff[11]=g_MobileInFlash.mobileno[4][5];
	//卡ID
	memcpy(answerbuff + 12, &(g_IDInFlash.IDnum), 7);
	//设备类型
	answerbuff[19]=0x01;
	//预留
	memset(answerbuff + 20, 0, 15);
	//校验和
	answerbuff[35]=CheckSum(31, answerbuff+4);
	//序列号
	memcpy(answerbuff + 36, ConstructionSignSNum, 4);
	//校验和
	answerbuff[40]=CheckSum(40, answerbuff);
	UartSend(&g_Uart3,answerbuff,sizeof(answerbuff));
}

//区域告警回复信息
void RYDW_AnswerAreaAlarmServer(void)
{
	uint8_t answerbuff[41];
	memset(answerbuff,0,sizeof(answerbuff));
	//头
	answerbuff[0]=0x5A;
	answerbuff[1]=0xA5;
	//长度
	answerbuff[2]=0x20;
	answerbuff[3]=0x00;
	//类型码
	answerbuff[4]=0x01;
	//数据类型
	answerbuff[5]=0x08;
	//本机号
	answerbuff[6]=g_MobileInFlash.mobileno[4][0];
	answerbuff[7]=g_MobileInFlash.mobileno[4][1];
	answerbuff[8]=g_MobileInFlash.mobileno[4][2];
	answerbuff[9]=g_MobileInFlash.mobileno[4][3];
	answerbuff[10]=g_MobileInFlash.mobileno[4][4];
	answerbuff[11]=g_MobileInFlash.mobileno[4][5];
	//卡ID
	memcpy(answerbuff + 12, &(g_IDInFlash.IDnum), 7);
	//设备类型
	answerbuff[19]=0x01;
	//预留
	memset(answerbuff + 20, 0, 15);
	//校验和
	answerbuff[35]=CheckSum(31, answerbuff+4);
	//序列号
	memcpy(answerbuff + 36, AreaAlarmSNum, 4);
	//校验和
	answerbuff[40]=CheckSum(40, answerbuff);
	UartSend(&g_Uart3,answerbuff,sizeof(answerbuff));
}

/******************************************************************
函数名称: _4GResult AnalysisAnswerData
功能描述: 解析服务器应答数据
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
_4GResult AnalysisAnswerData(uint8_t *receBuf, uint16_t buffer_size) 
{
	/* 参数校验 */
    if(!receBuf) return _4GRET_INCOMPLETE_ERR;
    
	for(uint16_t i = 0;i < buffer_size; i++)
	{
		/* 包头校验 */
		if((receBuf[i + 0] != 0X5A) || (receBuf[i + 1] != 0xA5)) 
		{
			continue;
		}
		/* 校验和验证 */
		uint8_t calc_sum = CheckSum(16, receBuf + i);
		if(calc_sum != receBuf[i + 16]) 
		{
			continue;
		}
		
		//判断是否是服务器应答数据
		if (receBuf[i + 4] == 0X00)
		{
			//定位数据应答
			if (receBuf[i + 5] == 0x01)
			{
				return _4GRET_OK;
			}
		}
		else
		{
			continue;
		}
	}
	
	return _4GRET_INCOMPLETE_ERR;
}

//解析电子围栏信息
_4GResult AnalysisWorkPlan(uint8_t *receBuf, uint16_t buffer_size) 
{
	uint16_t datalen = 0;
	
	/* 参数校验 */
    if(!receBuf) return _4GRET_INCOMPLETE_ERR;
    
    /* 基础长度检查 */
    if(buffer_size < 32) return _4GRET_LEN_ERR;
	
	for(uint16_t i = 0; i < buffer_size; i++)
	{
		if ((i + 58) > buffer_size)
		{
			return _4GRET_LEN_ERR;
		}
		
		//包头校验
		if((receBuf[i] != 0XFA) || (receBuf[i + 1] != 0x00)) 
		{
			continue;
		}
		//类型校验
		if(receBuf[i + 2]!=0x04)
		{
			continue;
		}
		
		//区域数据的总长度
		datalen = (receBuf[i + 31]<<8) + receBuf[i + 30];
		//总长度（32字节头+电子围栏长度+校验位+序列号）
		uint16_t rlen = datalen + 33 + 4;
		if ((i + rlen) > buffer_size)
		{
			return _4GRET_LEN_ERR;
		}
		
		//计算校验和
		uint8_t calc_sum = CheckSum(rlen - 5, receBuf + i);
		if(calc_sum != receBuf[i + rlen - 5]) 
		{
			continue;
		}
		
		//电子围栏数据解析
		if ((receBuf[i + 32] != 0X5A) && (receBuf[i + 33] != 0XA5))
		{
			return _4GRET_KEYWORD_ERR;
		}
		
		uint8_t *WorkPlanData = &receBuf[i + 32];
		
		//取到长度
		uint16_t WorkPlanDataLen = (WorkPlanData[5] << 8) + WorkPlanData[4];
		//校验和验证
		uint8_t WorkPlanSum = CheckSum(WorkPlanDataLen + 6, WorkPlanData);
		if(WorkPlanSum != WorkPlanData[WorkPlanDataLen + 6]) 
		{
			return _4GRET_CHK_ERR ;
		}
		
		//取出施工计划ID
		uint32_t WorkPlanID = ConvArrayToUint32(receBuf + i, 3, M_BIG_ENDIAN);
		//拿到总包个数
		uint8_t PackageTotal = receBuf[i + 34];
		//拿到第几包
		uint8_t PackageOrder = receBuf[i + 35];
		
		//长度判断
		if ((i + 1086) > buffer_size)
		{
			return _4GRET_CHK_ERR ;
		}
		
		//新增操作
		if (receBuf[i + 1086] == 0)
		{
			//先判断是否有相同的施工计划
			for (uint8_t j = 0; j < MAX_WORK_PLAN_NUM; j++)
			{
				if (g_WorkPlanSummary[j].WorkPlanID == WorkPlanID)
				{
					g_WorkPlanTime = TIM2->CNT;
					g_WorkPlanReceiveFlag = 1;
					//获取序列号
					memcpy(WorkPlanSNum, receBuf + i + rlen - 4, 4);
		
					//需定义成功返回值
					return _4GRET_OK; 
				}
			}
			
			//执行新增操作
			uint8_t Data[2048];
			memset((uint8_t *)Data, 0x00, sizeof(Data));
			//加上校验和
			memcpy(Data, receBuf + i, datalen + 33);
			//查找flash中空闲区
			int8_t FlashIndex = QueryFreeFlash();
			if (FlashIndex == -1)
			{
				//获取序列号
				memcpy(WorkPlanSNum, receBuf + i + rlen - 4, 4);
		
				//需定义成功返回值
				return _4GRET_OK; 
			}
			//先删除flash
			if (PackageOrder == 1)
			{
				CleanWorkPlanFlash(FlashIndex);
			}
			//将施工计划的包放入Flash中
			WriteWorkPlanToFlash(FlashIndex, Data, datalen + 33, PackageOrder);
				
			if (PackageTotal == PackageOrder)
			{
				//判断这个施工计划的完整性
				if (IsWorkPlanPerfectly(FlashIndex) == 0)
				{
					last_selected_idx = -1;
					UpdateWorkPlanSummary(FlashIndex);
					g_IsUpdateWorkTime = 1;
				}
			}
		}
		//修改操作
		else if (receBuf[i + 1086] == 1)
		{
			if (PackageOrder == 1)
			{
				CleanModWorkPlanFlash();
			}
			
			uint8_t Data[2048];
			memset((uint8_t *)Data, 0x00, sizeof(Data));
			//加上校验和
			memcpy(Data, receBuf + i, datalen + 33);
			WriteModWorkPlanToFlash(Data, datalen + 33, PackageOrder);
				
			if (PackageTotal == PackageOrder)
			{
				//判断这个施工计划的完整性
				if (IsModWorkPlanPerfectly() == 0)
				{
					last_selected_idx = -1;
					ModWorkPlan(WorkPlanID);
					g_IsUpdateWorkTime = 1;
				}
			}
		}
		//删除操作
		else if (receBuf[i + 1086] == 2)
		{
			last_selected_idx = -1;
			DeleteWorkPlan(WorkPlanID);
			g_IsUpdateWorkTime = 1;
		}
		//清空操作
		else if (receBuf[i + 1086] == 3)
		{
			last_selected_idx = -1;
			DeleteALLWorkPlan();
			CleanWorkPlanSummary();
			CleanWorkTime();
			CleanEFenceValid();
			g_IsUpdateWorkTime = 1;			
		}
		
		g_WorkPlanTime = TIM2->CNT;
		g_WorkPlanReceiveFlag = 1;
		//获取序列号
		memcpy(WorkPlanSNum, receBuf + i + rlen - 4, 4);
		
		//需定义成功返回值
		return _4GRET_OK; 
	}
	
	return _4GRET_INCOMPLETE_ERR;
}

/******************************************************************
函数名称: _4GResult AnalysisTrainAndConstructionSignInfo
功能描述: 解析来车和施工牌数据和探伤小车
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
_4GResult AnalysisTrainAndConstructionSignInfo(uint8_t *receBuf, uint16_t buffer_size) 
{
	uint16_t datalen = 0;
	uint16_t rlen = 0;
	uint8_t calc_sum = 0;
	uint8_t *p = NULL;
	
	/* 参数校验 */
    if(!receBuf) return _4GRET_INCOMPLETE_ERR;
    
    /* 基础长度检查 */
    if(buffer_size < 32) return _4GRET_LEN_ERR;
    for(uint16_t k = 0; k < buffer_size; k++)
	{
		if ((k + 32) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		
		/* 包头校验 */
		if((receBuf[k] != 0XFA) || (receBuf[k+1] != 0x00)) 
		{
			continue;
		}
		
		switch (receBuf[k + 2])
		{
			case 0x06:
				/* 获取数据长度 */
				datalen = (receBuf[k + 31] << 8)+receBuf[k + 30];
				rlen = datalen + 33;
		
				/* 二次长度检查 */
				if ((k + rlen + 4) > buffer_size)
				{
					return _4GRET_INCOMPLETE_ERR;
				}
				/*获取序列号*/
				memcpy(TrainInfoSNum, receBuf + k + rlen, 4);
				/* 校验和验证 */
				calc_sum = CheckSum(rlen - 1, receBuf + k);
				if(calc_sum != receBuf[k + rlen - 1]) 
				{
					continue;
				}
		
				p = k + receBuf + 32; // 来车信息起始位置
				/* 帧头校验 (0x5A 0xA5) */
				if (p[0] != 0x5A || p[1] != 0xA5) 
				{
					return _4GRET_KEYWORD_ERR;
				}
				
				if (p[6] == 1)
				{
					g_ServiceResponseLY = 0x02;
				}
				else
				{
					//上行预告
					if ((p[2] == 1) && (p[3] == 1))
					{
						g_TrainInfo.TrainApproach = 1;
					}
					//下行预告
					else if ((p[2] == 2) && (p[3] == 1))
					{
						g_TrainInfo.TrainApproach = 1;
					}
					//手机一键下道
					else if((p[2] == 3) && (p[3] == 1))
					{
						g_TrainInfo.AppTrainApproach = 1;						
					}
					else if ((p[2] == 0XFF) && (p[3] == 1))
					{
						g_TrainInfo.TrainApproach = 1;
					}
					//上行消警
					else if ((p[2] == 1) && (p[3] == 0))
					{
						g_TrainInfo.TrainApproach = 0;
						g_TrainInfo.AppTrainApproach = 0;
					}
					//下行消警
					else if ((p[2] == 2) && (p[3] == 0))
					{
						g_TrainInfo.TrainApproach = 0;
						g_TrainInfo.AppTrainApproach = 0;
					}
					else if ((p[2] == 0XFF) && (p[3] == 0))
					{
						g_TrainInfo.TrainApproach = 0;
						g_TrainInfo.AppTrainApproach = 0;
					}
					
					g_YYPro = 0x00;
					g_WarnYYFlag = 1;
				}
				trainState = 1;
				//列车接近标记清0
				
				return _4GRET_OK; // 需定义成功返回值
			case 0x0C:
				/* 获取数据长度 */
				datalen = (receBuf[k + 31] << 8)+receBuf[k + 30];
				rlen = datalen + 33;
		
				/* 二次长度检查 */
				if(buffer_size-k < rlen) 
				{
					return _4GRET_LEN_ERR;
				}
				if ((k + rlen + 4) > buffer_size)
				{
					return _4GRET_INCOMPLETE_ERR;
				}
				/*获取序列号*/
				memcpy(ConstructionSignSNum, receBuf + k + rlen, 4);
				/* 校验和验证 */
				calc_sum = CheckSum(rlen - 1, receBuf + k);
				if(calc_sum != receBuf[k + rlen - 1]) 
				{
					continue;
				}
		
				p = k + receBuf + 32; // 施工牌信息起始位置
				/* 帧头校验 (0x5A 0xA5) */
				if (p[0] != 0x5A || p[1] != 0xA5) 
				{
					return _4GRET_KEYWORD_ERR;
				}
				//倾斜
				if (p[2] == 1)
				{
					g_YYPlay.sgp |=0x01;
//					WT588E_PLAY(SGPQX);
				}
				//倾倒
				else if (p[2] == 2)
				{
					g_YYPlay.sgp |=0x02;
//					WT588E_PLAY(SGPQD);
				}
				else if(p[2] == 0)
				{
					g_YYPlay.sgp &=0xFC;
				}
				//未插入正确位置
				if (p[3] == 1)
				{
					g_YYPlay.sgp |=0x04;
//					WT588E_PLAY(SGPWZCW);
				}
				//未拔
				else if (p[3] == 2)
				{
					g_YYPlay.sgp |=0x08;
//					WT588E_PLAY(SGPWB);
				}
				else if(p[3] == 0)
				{
					g_YYPlay.sgp &=0xF3;
				}
				
				readConstructionSigninfo = 1;
				
				return _4GRET_OK; // 需定义成功返回值
			case 0x0F:
				/* 获取数据长度 */
				datalen = (receBuf[k + 31] << 8)+receBuf[k + 30];
				rlen = datalen + 33;
		
				/* 二次长度检查 */
				if ((k + rlen + 4) > buffer_size)
				{
					return _4GRET_INCOMPLETE_ERR;
				}
				/*获取序列号*/
				memcpy(CarSNum, receBuf + k + rlen, 4);
				/* 校验和验证 */
				calc_sum = CheckSum(rlen - 1, receBuf + k);
				if(calc_sum != receBuf[k + rlen - 1]) 
				{
					continue;
				}
		
				p = k + receBuf + 32; // 来车信息起始位置
				/* 帧头校验 (0x5A 0xA5) */
				if (p[0] != 0x5A || p[1] != 0xA5) 
				{
					return _4GRET_KEYWORD_ERR;
				}
				
				if (p[2] == 0x01)
				{
					if(p[3] == 0x3B)         //小车超范围
						g_YYPlay.cart |= 0x01;     
					else if(p[3] == 0x3C)    //小车与计划行别不符
						g_YYPlay.cart |= 0x02;					
					else if(p[3] == 0x3D)    //小车提前上道
						g_YYPlay.cart |= 0x04;							
					else if(p[3] == 0x3E)    //小车超时施工
						g_YYPlay.cart |= 0x08;	
					else if(p[3] == 0x3F)    //小车超范围超时间
						g_YYPlay.cart |= 0x10;						
					CarFlag = 1;
				}
				else if ((p[2] == 0x00) && (CarFlag == 1))
				{
					g_YYPlay.cart = 0x20;
					CarFlag = 0;
				}
				
				CarState = 1;
				//g_WarnYYFlag = 1;
				//列车接近标记清0
				
				//return _4GRET_OK; // 需定义成功返回值
			default:
				break;
		}
	}
	
	return _4GRET_OK;
}

//解析区域报警信息
_4GResult AnalysisAreaAlarmInfo(uint8_t *receBuf, uint16_t buffer_size)
{
	uint16_t datalen = 0;
	
	/* 参数校验 */
    if(!receBuf) return _4GRET_INCOMPLETE_ERR;
    
    /* 基础长度检查 */
    if(buffer_size < 32) return _4GRET_LEN_ERR;
    for(uint16_t i = 0; i < buffer_size; i++)
	{
		if ((i + 32) > buffer_size)
		{
			return _4GRET_INCOMPLETE_ERR;
		}
		
		/* 包头校验 */
		if((receBuf[i] != 0XFA) || (receBuf[i+1] != 0x00)) 
		{
			continue;
		}
		/* 数据类型验证 */
		if(receBuf[i + 2] != 0x08)
		{
			continue;
		}
		/* 获取数据长度 */
		datalen = (receBuf[i + 31] << 8) + receBuf[i + 30];
		uint16_t rlen = datalen + 33;
		
		if ((i + rlen + 4) > buffer_size)
		{
			return _4GRET_LEN_ERR;
		}
		
		/*获取序列号*/
		memcpy(AreaAlarmSNum, receBuf + i + rlen, 4);
		/* 校验和验证 */
		uint8_t calc_sum = CheckSum(rlen - 1, receBuf + i);
		if(calc_sum != receBuf[i + rlen - 1]) 
		{
			continue;
		}
		
		uint8_t *p = i + receBuf + 32; // 来车信息起始位置
		/* 帧头校验 (0x5A 0xA5) */
		if (p[0] != 0x5A || p[1] != 0xA5) 
		{
			return _4GRET_KEYWORD_ERR;
		}
		//区域个数
		uint8_t AreaNum = p[4];
		//区域个数小于20
		if (AreaNum > 20)
		{
			return _4GRET_LEN_ERR;
		}
		
		//放入flash中
		WriteAreaAlarmToFLASH(p, datalen + 1);
		
		//再从flash中将区域报警信息读出来
		ReadAreaAlarmToFLASH();
		
		return _4GRET_OK; // 需定义成功返回值
	}
	
	return _4GRET_INCOMPLETE_ERR;
}

/******************************************************************
函数名称: void ControlMsg(void)
功能描述: 控制信息
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ControlMsg(void)
{
	if(g_AskAuthorityFlag == 1)
	{
		ReturnMobileNoToNet();
		g_AskAuthorityFlag = 0;
		g_CtrlMsgFlag = 0;
	}
	else if(g_AskIDFlag == 1)
	{
		ReturnIDtoNet();
		g_AskIDFlag = 0;
		g_CtrlMsgFlag = 0;
	}
	else if(g_AskParFlag == 1)   //返回基站基础信息
    {
		ReturnPartoNet();
		g_AskParFlag = 0;
		g_CtrlMsgFlag = 0;
	}
	else if (g_NetAskIPAndPortFlag == 1)
	{
		ReturnIpAndPort();
		g_NetAskIPAndPortFlag = 0;
		g_CtrlMsgFlag = 0;
	}
	else
		g_CtrlMsgFlag = 0;
	//g_ControlMsgDelay = uwTick;
}

void GetUpdateType(void)
{
	uint8_t TmpSendBuf[25];
	memset(TmpSendBuf,0,sizeof(TmpSendBuf));
	
	TmpSendBuf[0] = 0x5A;
	TmpSendBuf[1] = 0xA5;
	TmpSendBuf[2] = 0x10;
	TmpSendBuf[3] = 0x00;
	TmpSendBuf[4] = 0xEA;
	TmpSendBuf[5]=g_MobileInFlash.mobileno[4][0];
	TmpSendBuf[6]=g_MobileInFlash.mobileno[4][1];
	TmpSendBuf[7]=g_MobileInFlash.mobileno[4][2];
	TmpSendBuf[8]=g_MobileInFlash.mobileno[4][3];
	TmpSendBuf[9]=g_MobileInFlash.mobileno[4][4];
	TmpSendBuf[10]=g_MobileInFlash.mobileno[4][5];
	memcpy(TmpSendBuf+11,(uint8_t*)&g_UpdateCodePageTotal,3);	
	TmpSendBuf[14] = g_VersionHex[0];
	TmpSendBuf[15] = g_VersionHex[1];	
	TmpSendBuf[19] = CheckSum(15,TmpSendBuf+4);	
	TmpSendBuf[24] = CheckSum(24,TmpSendBuf);	
	
	UartSend(&g_Uart3,(uint8_t*)TmpSendBuf,25);
}

/******************************************************************
函数名称: void SendFullUpdateBag(void)
功能描述: 向服务器请求升级数据包(全量升级)
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：hly
******************************************************************/
void SendFullUpdateBag(void)
{
	uint8_t TmpSendBuf[32];
	memset(TmpSendBuf,0,sizeof(TmpSendBuf));	
	TmpSendBuf[0] = 0x5A;
	TmpSendBuf[1] = 0xA5;
	TmpSendBuf[2] = 0x10;
	TmpSendBuf[3] = 0x00;
	TmpSendBuf[4] = 0xEE;	
	
	memcpy(TmpSendBuf+5,g_MobileInFlash.mobileno[4],6);
	memcpy(TmpSendBuf+11,(uint8_t*)&g_UpdateCodePageTotal,3);
	TmpSendBuf[14] = ( g_UpdateCodePageTotal.programpage & 0xff );		
	TmpSendBuf[15] = (( g_UpdateCodePageTotal.programpage >>8 ) & 0xff );	
	TmpSendBuf[16] = ( g_RemoteUpdatePackNo & 0xff );		
	TmpSendBuf[17] = (( g_RemoteUpdatePackNo >>8 ) & 0xff );	
	TmpSendBuf[19] = CheckSum(15,TmpSendBuf+4);	
	TmpSendBuf[24] = CheckSum(24,TmpSendBuf);		
	UartSend(&g_Uart3,(uint8_t*)TmpSendBuf,25);	
}

void GetUpdateBinData(void)
{
	#if 0
	uint8_t TmpSendBuf[41];
	memset(TmpSendBuf,0,sizeof(TmpSendBuf));
	
	TmpSendBuf[0] = 0x5A;
	TmpSendBuf[1] = 0xA5;
	TmpSendBuf[2] = 0x20;
	TmpSendBuf[3] = 0x00;
	TmpSendBuf[4] = 0xEF;	
	
	TmpSendBuf[5]=g_MobileInFlash.mobileno[4][0];
	TmpSendBuf[6]=g_MobileInFlash.mobileno[4][1];
	TmpSendBuf[7]=g_MobileInFlash.mobileno[4][2];
	TmpSendBuf[8]=g_MobileInFlash.mobileno[4][3];
	TmpSendBuf[9]=g_MobileInFlash.mobileno[4][4];
	TmpSendBuf[10]=g_MobileInFlash.mobileno[4][5];
	memcpy(TmpSendBuf+11,(uint8_t*)&g_UpdateCodePageTotal,3);
	TmpSendBuf[14] = ( g_UpdateCodePageTotal.programpage & 0xff );		
	TmpSendBuf[15] = (( g_UpdateCodePageTotal.programpage >>8 ) & 0xff );	
	TmpSendBuf[16] = ( g_RemoteUpdatePackNo & 0xff );		
	TmpSendBuf[17] = (( g_RemoteUpdatePackNo >>8 ) & 0xff );	
	if(g_UpdateType == 0xDC)
		TmpSendBuf[18] = 0;	
	else
		TmpSendBuf[18] = g_UpdateType;
	if(g_UpdateType == 1)
	{
		TmpSendBuf[19] = g_VersionHex[0];
		TmpSendBuf[20] = g_VersionHex[1];
	}
	TmpSendBuf[35] = CheckSum(31,TmpSendBuf+4);	
	TmpSendBuf[40] = CheckSum(40,TmpSendBuf);	
	
	UartSend(&g_Uart3,(uint8_t*)TmpSendBuf,41);
	#endif

#if 1
	uint8_t TmpSendBuf[32];
	memset(TmpSendBuf,0,sizeof(TmpSendBuf));	
	TmpSendBuf[0] = 0x5A;
	TmpSendBuf[1] = 0xA5;
	TmpSendBuf[2] = 0x10;
	TmpSendBuf[3] = 0x00;
	TmpSendBuf[4] = 0xEE;	
	
	memcpy(TmpSendBuf+5,g_MobileInFlash.mobileno[4],6);
	memcpy(TmpSendBuf+11,(uint8_t*)&g_UpdateCodePageTotal,3);
	TmpSendBuf[14] = ( g_UpdateCodePageTotal.programpage & 0xff );		
	TmpSendBuf[15] = (( g_UpdateCodePageTotal.programpage >>8 ) & 0xff );	
	TmpSendBuf[16] = ( g_RemoteUpdatePackNo & 0xff );		
	TmpSendBuf[17] = (( g_RemoteUpdatePackNo >>8 ) & 0xff );	
	TmpSendBuf[19] = CheckSum(15,TmpSendBuf+4);	
	TmpSendBuf[24] = CheckSum(24,TmpSendBuf);		
	UartSend(&g_Uart3,(uint8_t*)TmpSendBuf,25);	
	#endif
}

void Gsmsenddata(void)
{	
	if (expectingPrompt == false) 
    {
		// 根据当前任务类型发送数据
        switch (currentTask) 
        {
			case TASK_SEND_D6:	
				//发送AT命令
				if (g_RemoteStartFlag == 1)
				{
					SendRecSuc();
					if(g_RemoteUpdateGCFlag == 1)
					{
						if(g_UpdateVoiceTypeFlag == 1)					
							g_RemoteUpdateGCFlag = 3;
						else							
							g_RemoteUpdateGCFlag = 2;
					}
							
					g_RemoteStartFlag = 0;
				}
				else if ((g_RecdateToGprsFlag == 1) && (g_CtrlMsgFlag == 1))
				{
					SendRecSuc();
					ControlMsg();
					g_ControlMsgDelay = TIM2->CNT;
					g_RecdateToGprsFlag = 0;
				}
				else if (g_CtrlMsgFlag == 1)
				{
					ControlMsg();
				}
				else if (g_RecdateToGprsFlag == 1)
				{
					SendRecSuc();
					
					g_RecdateToGprsFlag = 0;
				}

				currentTask = TASK_NONE;
				
				break;
			case TASK_RESPONSE:	
				if(planState == 1)
				{
					RYDW_AnswerPlanServer();
					planState = 0;
				}
				if(trainState == 1)
				{
					RYDW_AnswerTrainServer();
					trainState = 0;
				}
				if (readConstructionSigninfo == 1)
				{
					RYDW_AnswerConstructionSignServer();
					readConstructionSigninfo = 0;
				}
				if (readareaalarm == 1)
				{
					RYDW_AnswerAreaAlarmServer();
					readareaalarm = 0;
				}
				if (CarState == 1)
				{
					RYDW_AnswerCarServer();
					CarState = 0;
				}
				if (ProtectionPositionState == 1)
				{
					RYDW_AnswerProtectionPositionServer();
					
					ProtectionPositionState = 0;
				}
				
				currentTask = TASK_NONE;				
				break;	
			case TASK_CARD_WAIT_PROMPT:							
				/*发送定位卡信息*/
				RYDW_SendCardInfo();
				UartSend(&g_Uart3,(uint8_t*)Cardinfo,69);		        /*发送给服务器*/
				
				//第一次先发送定位信息，再发送第一包的D6
				if (g_SendLocationFirst == 0)
				{
					g_AskParFlag = 1;
					g_SendLocationFirst = 1;
					g_CtrlMsgFlag = 1;
					g_FirstSendID = 0;
					currentTask = TASK_NONE;  // 完成任务
					return;
				}	
				
				if (g_CarWarnFlag.bit.StartUp == 0X01)
				{
					g_RecordData.StartUpRecord.RecordFlag++;
				}
				
				if (g_CarWarnFlag.bit.LateStartUp == 0X01)
				{
					g_RecordData.LateStartUpRecord.RecordFlag++;
				}
				//ggaPending = true;  // 标记需要发送GGA
				//cardSendCompleteTime = TIM2->CNT;
				currentTask = TASK_NONE;  // 完成任务
				
				//g_SendLocationDataToDiffService = 1;
				//lastSendLocationToDiffServiceTime = TIM2->CNT;
                break;           
			case TASK_GGA_WAIT_PROMPT:
				UartSend(&g_Uart3,(uint8_t *)ggadata,ggadatalen);
				ggadatalen = 0;
				memset(ggadata,0,sizeof(ggadata));	
				currentTask = TASK_NONE;  // 完成任务
                break;
			case TASK_REMOTE_UPGRADE_TYPE_WAIT_PROMPT:
				//获取升级类型
				GetUpdateType();
				g_RemoteUpgradeSendNum++;
				currentTask = TASK_NONE;
				break;
			case TASK_REMOTE_UPGRADE_BIN_WAIT_PROMPT:
				//获取升级bin数据
				if(g_UpdateVoiceTypeFlag == 1)
				{
					SendFullUpdateBag();
				}
				else
				{
					GetUpdateBinData();
				}	
				g_RemoteUpgradeSendNum++;				
				currentTask = TASK_NONE;
				break;
			case TASK_LY_DATA_UPLOAD:
				//发送蓝牙数据
				RYDW_SendLYInfo();
				currentTask = TASK_NONE;
				break;
			case TASK_ASK_WORKPLAN:
				//索要施工计划
				RYDW_AskWorkPlan();
				delay_ms(10);
				UartSend(&g_Uart3, &(uint8_t){0x1A}, 1);
				delay_ms(5);
				UartSend(&g_Uart3, &(uint8_t){0x1A}, 1);
				currentTask = TASK_NONE;
				g_AskWorkPlan = 0;
				break;
			case TASK_SEND_DATA_TO_DIFFSERVICE:							
				/*发送定位卡信息*/
				RYDW_SendCardInfo();
				UartSend(&g_Uart3,(uint8_t*)Cardinfo,69);
				currentTask = TASK_NONE;
				g_SendLocationDataToDiffService = 0;
            default:
               // 不应该发生的情况
                break;
        }
	}
    // 处理其他响应，如SEND OK等
    if (inclson("+MIPSEND",GsmReceBuff) != 0 &&	expectingPrompt == false) 
    {
		if (g_SelfCheck.Dtu.Num++ >= 255)
		{
			g_SelfCheck.Dtu.Num = 1;
		}
		// 发送完成处理
		expectingPrompt = false;
		currentTask = TASK_NONE;  // 完成任务
    }
    // 错误处理
    else if (inclson("+ERROR",GsmReceBuff) != 0 ) 
    {
		if (g_SelfCheck.Dtu.Num++ >= 255)
		{
			g_SelfCheck.Dtu.Num = 1;
		}
        expectingPrompt = false;
        currentTask = TASK_NONE;  // 重置状态
    }
    else if (inclson("+CCLK:",GsmReceBuff) != 0 )
	{		
		if (g_SelfCheck.Dtu.Num++ >= 255)
		{
			g_SelfCheck.Dtu.Num = 1;
		}
		
		 g_SysTime.year = StrToTime(GsmReceBuff+10);
		 g_SysTime.month = StrToTime(GsmReceBuff+13);
		 g_SysTime.date = StrToTime(GsmReceBuff+16);
		 g_SysTime.hour = StrToTime(GsmReceBuff+19);
		 g_SysTime.minute = StrToTime(GsmReceBuff+22);
		 g_SysTime.second = StrToTime(GsmReceBuff+25);
		 convertToUTC8(&g_SysTime);
		 g_AskTimeFlag = 1;			
	}
	
	if(gprs_link_status == 1)
	{
		if((g_SendLocationFirst == 1) && (g_FirstSendID == 0) && (g_CtrlMsgFlag == 0))
		{
			g_FirstSendID = 1;
			g_CtrlMsgFlag = 1;
			g_AskIDFlag = 1;
		}	
		
		if ((JudgeSomeDelayInProOk(lastCardTriggerTime, Delay1Sec) == true) && (currentTask == TASK_NONE) && 
				((g_CtrlMsgFlag == 1) || (g_RemoteStartFlag == 1) || (g_RecdateToGprsFlag == 1)) && 
				(!g_SendLocationDataToDiffService) && (g_SendLocationFirst == 1))
		{
			expectingPrompt = true;  // 设置等待提示符标志
			currentTask = TASK_SEND_D6;
			
			//发送AT命令
			if (g_RemoteStartFlag == 1)
			{
				AT_Mipsend(6);
			}
			else if ((g_RecdateToGprsFlag == 1) && (g_CtrlMsgFlag == 1))
			{
				AT_Mipsend(327);
			}
			else if (g_CtrlMsgFlag == 1)
			{
				AT_Mipsend(321);
			}
			else if (g_RecdateToGprsFlag == 1)
			{
				AT_Mipsend(6);
			}
			else
			{
				currentTask = TASK_NONE;
				expectingPrompt = false;
			}

			lastCardTriggerTime = TIM2->CNT;
			g_SendDataTime = TIM2->CNT;
			g_SendDataNum++;
		}
		//处理各种应答包
		else if ((JudgeSomeDelayInProOk(lastCardTriggerTime, Delay1Sec) == true) && (currentTask == TASK_NONE) && 
				((planState == 1) || (trainState == 1) || (CarState == 1) || (ProtectionPositionState == 1) || 
				(readConstructionSigninfo == 1) || (readareaalarm == 1)) && (g_RemoteUpdateGCFlag == 0) &&
				(!g_SendLocationDataToDiffService) && (g_SendLocationFirst == 1))
		{	
			currentTask = TASK_RESPONSE;
			expectingPrompt = true;  // 设置等待提示符标志
			
			//施工计划应答
			if (planState == 1)
			{
				AT_Mipsend(41);
			}
			//来车下道应答
			else if (trainState == 1)
			{
				AT_Mipsend(41);
			}
			//施工牌应答
			else if (readConstructionSigninfo == 1)
			{
				AT_Mipsend(41);
			}
			//设置区域报警应答
			else if (readareaalarm == 1)
			{
				AT_Mipsend(41);
			}
			else if (CarState == 1)
			{
				AT_Mipsend(41);
			}
			else if (ProtectionPositionState == 1)
			{
				AT_Mipsend(41);
			}
			else
			{
				currentTask = TASK_NONE;
				expectingPrompt = false;
			}
			
			lastCardTriggerTime = TIM2->CNT;
			g_SendDataTime = TIM2->CNT;
			g_SendDataNum++;
		}
		//发送请求升级类型的包
		else if ((JudgeSomeDelayInProOk(lastCardTriggerTime, Delay500mSec) == true) && (g_RemoteUpdateGCFlag == 2) && 
			(currentTask == TASK_NONE) && (g_SendLocationFirst == 1) && (!g_SendLocationDataToDiffService))
		{
			#if 0
				//发送AT命令
				at_tx(mipsend0);
				expectingPrompt = true;  // 设置等待提示符标志
				currentTask = TASK_REMOTE_UPGRADE_TYPE_WAIT_PROMPT;
				
				lastCardTriggerTime = now;
				g_RemoteUpdateNum++;
			#endif
			g_RemoteUpdateGCFlag = 3;
		}
		//发送请求bin数据的包
		else if ((JudgeSomeDelayInProOk(lastCardTriggerTime, Delay500mSec) == true) && (g_RemoteUpdateGCFlag == 3) && 
			(currentTask == TASK_NONE) && (g_SendLocationFirst == 1) && (!g_SendLocationDataToDiffService))
		{
			//发送AT命令
			AT_Mipsend(25);
			expectingPrompt = true;  // 设置等待提示符标志
			currentTask = TASK_REMOTE_UPGRADE_BIN_WAIT_PROMPT;
				
			lastCardTriggerTime = TIM2->CNT;
			g_SendDataTime = TIM2->CNT;
			g_RemoteUpdateNum++;
			g_SendDataNum++;
		}
		else if (g_RemoteUpdateGCFlag == 4)
		{
			if(g_UpdateType == 1)
			{
				g_UpdateType = 0;
				DiffProgramUserData();
			}
				
			if(flaglagao == 0)
			{
				PWR_4G_DISABLE();
				errornum_timer = TIM2->CNT;
				flaglagao = 1;
				HAL_UART_MspDeInit(&UART3_Handler);
			}
			else if(flaglagao == 1)
		    {
			    if(JudgeSomeDelayInProOk(errornum_timer,Delay20Sec) == true)
			    {
				    PWR_4G_ENABLE();
				    flaglagao = 0;			

					InitUSART3(UART3_BAUD);
					HAL_UART_MspInit(&UART3_Handler);
					g_RemoteUpdateNum = 0;
					g_RemoteUpgradeSendNum = 0;
					g_RemoteUpdateGCFlag = 0;
					SoftReset();
				}
			}
		}
		//索要时间
		else if ((JudgeSomeDelayInProOk(lastCardTriggerTime, Delay1Sec) == true) && (g_SelfCheck.SelfCheckSuccess == 1) && 
				(g_SendLocationFirst == 1) && (currentTask == TASK_NONE) && (g_RemoteUpdateGCFlag == 0) &&
				(!g_SendLocationDataToDiffService) && (g_AskTimeFlag == 0))		
		{
			at_tx(chkcsca);
			HAL_Delay(1000);
			g_DTUConnDelay = TIM2->CNT;		
			lastCardTriggerTime = TIM2->CNT;
		}
		//从第二次开始每10秒重发一次
		else if ((JudgeSomeDelayInProOk(lastCardTriggerTime, Delay1Sec) == true) && (ReceivelLYData == 1) && 
			(currentTask == TASK_NONE) && (g_RemoteUpdateGCFlag == 0)&& 
			(g_SendLocationFirst == 1) &&(!g_SendLocationDataToDiffService))
		{
			//发送AT命令
			AT_Mipsend(41);
			expectingPrompt = true;  //设置等待提示符标志
			currentTask = TASK_LY_DATA_UPLOAD;
			ReceivelLYData = 2;
			lastSendLyDataTime = TIM2->CNT;
			lastCardTriggerTime = TIM2->CNT;
			g_SendDataTime = TIM2->CNT;
			g_SendDataNum++;
		}
		//索要施工计划
		else if ((JudgeSomeDelayInProOk(lastCardTriggerTime, Delay1Sec) == true) && (g_AskWorkPlan > 0) && 
			(currentTask == TASK_NONE) && (g_RemoteUpdateGCFlag == 0)&&(!g_SendLocationDataToDiffService))
		{
			//发送AT命令
			at_tx(mipsend0);
			expectingPrompt = true;  // 设置等待提示符标志
			currentTask = TASK_ASK_WORKPLAN;
			lastCardTriggerTime = TIM2->CNT;
			g_SendDataTime = TIM2->CNT;
			g_SendDataNum++;
		}
		else if ((g_SendLocationDataToDiffService == 1) && (currentTask == TASK_NONE))
		{
			AT_DiffMipsend(69);	
			expectingPrompt = true;  // 设置等待提示符标志
			currentTask = TASK_SEND_DATA_TO_DIFFSERVICE;
			g_SendDataTime = TIM2->CNT;
			lastCardTriggerTime = TIM2->CNT;
			g_SendDataNum++;
		}
		else if ((JudgeSomeDelayInProOk(lastCardTriggerTime, Delay1Sec) == true) && (g_SendLocationData == 1) &&
			(currentTask == TASK_NONE) && (!g_SendLocationDataToDiffService))
		{
			//发送AT命令
			AT_Mipsend(69);			
			expectingPrompt = true;  // 设置等待提示符标志
			currentTask = TASK_CARD_WAIT_PROMPT;
			lastCardTriggerTime = TIM2->CNT;
			g_SendDataTime = TIM2->CNT;
			g_SendLocationData = 0;
			g_SendDataNum++;
		}
		else if ((JudgeSomeDelayInProOk(lastCardTriggerTime, g_LocationAndSelfcheckDiffFlash.Diff * 5 * Delay1Sec) == true) && 
				(currentTask == TASK_NONE) && (!g_SendLocationDataToDiffService) )// 更新触发时间) && now - lastGGATriggerTime>= 100
		{
			//发送AT命令
			AT_Mipsend(69);			
			expectingPrompt = true;  // 设置等待提示符标志
			currentTask = TASK_CARD_WAIT_PROMPT;
			lastCardTriggerTime = TIM2->CNT;
			g_SendDataTime = TIM2->CNT;
			g_SendDataNum++;
		}
	}
}

//超时检查
void CheckTimeouts(void)
{
	if((g_WorkPlanReceiveFlag == 1) && (JudgeSomeDelayInProOk(g_WorkPlanTime, 15 * Delay1Sec) == true))
	{
		WT588E_PLAY(DZWLCG);
		g_WorkPlanReceiveFlag = 0;
	}
	
	//判断业务服务器是否断开
	if ((JudgeSomeDelayInProOk(ServiceAnsewerTime, 60 * Delay1Sec) == true) && (gprs_link_status == 1) && (g_SelfCheck.StartCharge == 0))
	{
		current_step = 0xAC;
		ServiceAnsewerTime = TIM2->CNT;
		currentTask = TASK_NONE;
		ggaPending = false;  // 取消GGA等待
		gprs_link_status = 0;
	}
	
	//判断差分服务器是否断开
	if ((JudgeSomeDelayInProOk(DiffServiceAnsewerTime, 60 * Delay1Sec) == true) && (diff_gprs_link_status == 1) && (g_SelfCheck.StartCharge == 0))
	{
		current_step = 0xAE;
		DiffServiceAnsewerTime = TIM2->CNT;
		currentTask = TASK_NONE;
		ggaPending = false;  // 取消GGA等待
		diff_gprs_link_status = 0;
	}
	
	//升级时，同一个包请求20次后，如果都没有回应就退出升级
	if (g_RemoteUpgradeSendNum > 20)
	{
		g_RemoteUpgradeSendNum = 0;
		g_RemoteUpdateGCFlag = 0;
		currentTask = TASK_NONE;
		g_SendLocationDataToDiffService = 0;
		//ggaPending = false;  // 取消GGA等待
		ReplaceVoicePackage = 0;
	}
	
	//服务器30S如果没有应答遥控器发来的数据，就不发了
	if ((JudgeSomeDelayInProOk(g_ReceivelLYDataTime, 30 * Delay1Sec) == true))
	{
		ReceivelLYData = 0;
	}
	 
	if ((JudgeSomeDelayInProOk(lastSendLyDataTime, 10 * Delay1Sec) == true) && (ReceivelLYData == 2))
	{
		ReceivelLYData = 1;
	}
	
	//处理等待尖括号
	if ((currentTask != TASK_NONE) && (g_SendDataNum > 0) && (JudgeSomeDelayInProOk(g_SendDataTime, Delay500mSec) == true))
	{
		currentTask = TASK_NONE;
		lastCardTriggerTime = TIM2->CNT;
		return;
	}
	
	//30s给差分服务器发一条定位数据
	//判断是否连接差分服务器正常
	if (diff_gprs_link_status == 1)
	{
		if (JudgeSomeDelayInProOk(lastSendLocationToDiffServiceTime, Delay30Sec) == true)
		{
			g_SendLocationDataToDiffService = 1;
			lastSendLocationToDiffServiceTime = TIM2->CNT;
		}
	}
}

//接收RTCM数据
_4GResult RTK_GetRTCMInfo(uint8_t *InBuf, uint16_t BufLen)
{
	uint16_t i = 0;
	uint16_t datlen = 0;
	uint16_t difflen = 0;
	uint8_t RTKDiffData[1600];

	memset(RTKDiffData, 0, sizeof(RTKDiffData));
	/* RTCM协议数据格式 数据头0xD3(1)+数据长度(2)不超过1023字节+数据体(N)+CRC24(3) */
	do
	{
		/* 检查RTCM协议头 0xD3 */
		if (InBuf[i] == 0xD3)
		{
			datlen = (uint16_t)InBuf[i+1] * 256 + (uint16_t)InBuf[i+2];	//计算数据长度
			if (datlen < 1023)
			{
				/* 检查剩余字节不够剩余数据长度 */
				if ((datlen + 6) > (BufLen - i))
				{
					/* 数据不够 退出循环 */
					break;
				}
				memcpy(RTKDiffData + difflen, InBuf + i, (datlen + 6));	//复制一条D3数据差分缓存
				difflen += datlen + 6;									//数据长度
				i += datlen + 6;										//数据指针向后挪动
			}
			else
			{
				/* 数据长度错误 查找下一个字节 */
				i++;
			}
		}
		else
		{
			/* 数据头错误 查找下一个字节 */
			i++;
		}
	} while (i < BufLen);
	
	if (difflen != 0)
	{
		UartSend(&g_Uart2,RTKDiffData,difflen);		        /*发送给RTK*/
		
		return _4GRET_OK;
	}
	
	return _4GRET_NODATA_ERR;
}

void DTUMode_ReInit(void)
{
	if(g_CtrlMsgFlag == 0 && g_NeedReInitDTU >0 && JudgeSomeDelayInProOk(g_ControlMsgDelay, Delay2Sec) == true)
	{
		current_step = 0xAC;
		gprs_link_status = 0;
		diff_gprs_link_status = 0;
		g_NeedReInitDTU = 0;
		g_SetIpAndPort = 1;	
	}
}
