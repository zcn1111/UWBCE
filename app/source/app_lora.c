#if 0
	

#include "bsp.h"
#include "main.h"

#include "bsp.h"
#if LORATYPE                //=0移远E22，=1安信可,
  #include "ebyte_core.h"
#elif LORATYPE == 2
  #include "radio.h"
//  #include "sx126x_board.h"
//  #include "sx126x_callback.h"

//LoraHeader_t	g_CardHeader;		//定位卡数据头
//CardData_t		g_CardData[4];		//定位卡数据组


//LoraHeader_t     g_CardLora;
//CardData_t       g_CardLocaData;
//CardData_t       g_OtherCardData[2];
//EFence_t         g_EFenceInf; 
//WorkTime_t       g_WorkTimeInf;
//WLEDSta_t        g_WLEDStaInf;
//LoraHeader_t     g_LoraFromBaseSta;

//DataType         g_LoraDataSta;     /*设备状态*/

//CardInfoTable_t	 g_CardTable;		//定位卡信息表

//uint8_t          g_RTKDiffData[1400];  /*差分数据*/
//uint16_t         g_RTKDiffNum;         /*差分数据长度*/


//uint32_t g_SendDataToBaseDelay;    /*给基站发送数据时间*/
//uint32_t g_SendDataOrerDelay;
//uint8_t  g_ResDataFromBaseFlag;    /*接收到基站数据标记*/

/*卡ID*/
uint16_t g_CareIDNum;                /*设备最后一个ID*/
uint8_t g_LYSendUplinkNum;       /*发送上道次数*/
uint8_t g_LYSendTrainNum;        /*来车下发次数*/

/**/
uint8_t g_LocalAstrngFlag;         /*收敛标记*/
uint8_t LowPowerWarnFlag;          /*低功耗报警 =1电量小于50%,=2电量小于80%  =0 电量大于等于80% */

uint8_t  g_StrSelfCheck_Lora;      /*lora自检标记*/

uint8_t  g_P2PModeEnableMark;      /*点对点模式开启标记*/
uint8_t  g_BaseAskEFPackage;       /*基站索要包序号*/
uint8_t			g_LoraSendCnt;		//Lora发送次数
uint8_t			g_LoraSendOkCnt;	//Lora发送成功次数

void LoraInit(void)
{
	g_P2PModeEnableMark  = 0; 
	g_LYSendUplinkNum = 0;       /*发送上道次数*/
	g_LYSendTrainNum = 0;
	g_LocalAstrngFlag=0;        /*收敛标记*/
	g_SendDataToBaseDelay = uwTick;
	g_SendDataOrerDelay = TIM6->CNT;
	g_NoDWDelay =  TIM6->CNT;
	LowPowerWarnFlag = 0;
	g_ResDataFromBaseFlag = 0;
	g_StrSelfCheck_Lora = 0;
	g_BaseAskEFPackage = 0;
	memset((uint8_t*)&g_NeedSetEFence,0,sizeof(g_NeedSetEFence));
}



/******************************************************************
函数名称: LoraProcess
功能描述: Lora定位卡广播电子围栏数据
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
void LoraProcess(void)
{
	
	uint8_t		cardnum;	
	uint8_t		j;
	uint8_t     ResRecRes;
	
	if(g_ResDataFromBaseFlag == 1 && JudgeSomeDelayInProOk(g_SendDataOrerDelay,Delay500mSec*g_CareIDNum+Delay50mSec)) /*超时g_ResDataFromBaseFlag == 1 && Delay100mSec*g_CareIDNum+Delay50mSec*/
	{
		LORA_BANDWIDTH = LORA_BANDWIDTH_SEND;
		LORA_SPREADING_FACTOR = LORA_SPREADING_FACTOR_SEND;
		LORA_CODINGRATE = LORA_CODINGRATE_SEND;		
		SX126x_SetupRadioPara(&SX126xRadioEvents);				//配置参数
//		if(g_RecLYDataType.ElecFence == 1)   /*蓝牙下发电子围栏，下发三次*/
//		{
//			g_CardLora.DestOrder = 0;   //目标编号 0=广播
//			g_CardLora.SrcOrder = 0;    //源编号
//			g_CardLora.Type = 1;
//			if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
//				Line_2A_WT588F(QXD);
//			g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
//			if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
//				Line_2A_WT588F(LCXD);
//			g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

//			memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
//			g_CardLocaData.BatVoltage = g_BatVal; 
//			if(g_CarWarnFlag.all > 0)
//				g_CardLocaData.Alram.bit.Alram= 1;
//			g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
//			g_CardLocaData.Alram.bit.SOS=sos ;
//			if(g_ManualPunFlag == 2)
//			{
//				g_CardLocaData.Alram.bit.Bribe = 0x01;
//				g_ManualPunFlag = 0;
//			}
//			else
//				g_CardLocaData.Alram.bit.Bribe = 0;
//			g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
//			g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
//			g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
//			g_CardLocaData.PosQuality = g_PosQuality;
//			g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
//			g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
//			g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;				
//			memcpy((uint8_t*)&g_EFenceInf, (uint8_t*)&LYElecFenceData ,sizeof(g_EFenceInf)-1);
//			if(LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,&g_CardLocaData,g_EFenceInf) == LORARET_SEND_FAIL)
//			{
//				LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,&g_CardLocaData,g_EFenceInf);
//			}
//			memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
//			memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
//			memset((uint8_t*)&g_EFenceInf,0,sizeof(g_EFenceInf));
//		}
		
		if(g_RecLYDataType.ElecFence == 1 && g_P2PModeEnableMark == 0 )   /*点对点申请*/
		{
			/*请求点对点通信*/
			g_CardLora.DestOrder = 0;   //目标编号 0=广播
			g_CardLora.SrcOrder = 0;    //源编号
			g_CardLora.Type = 3;
//			if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
//				Line_2A_WT588F(QXD);
			g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
//			if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
//				Line_2A_WT588F(LCXD);
			g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

			memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
			g_CardLocaData.BatVoltage = g_BatVal; 
			if(g_CarWarnFlag.all > 0)
				g_CardLocaData.Alram.bit.Alram= 1;
			g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
			g_CardLocaData.Alram.bit.SOS=sos ;
			if(g_ManualPunFlag == 2)
			{
				g_CardLocaData.Alram.bit.Bribe = 0x01;
				g_ManualPunFlag = 0;
			}
			else
				g_CardLocaData.Alram.bit.Bribe = 0;	
			g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
			g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
			g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
			g_CardLocaData.PosQuality = g_PosQuality;
			g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>16)&0xFF;
			g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>8)&0xFF;
			g_CardLocaData.Altitude[2] = (g_tGPS.Altitude)&0xFF;				
			if(LORA_Card_BoardcastP2PInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,g_RecLYEFenceData.Total) == LORARET_SEND_FAIL)
			{
				LORA_Card_BoardcastP2PInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,g_RecLYEFenceData.Total);
			}
			g_P2PModeEnableMark  = 1;				
			memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
			memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));	
		}			
		else if(g_RecLYDataType.ElecFence == 2 && g_P2PModeEnableMark  == 1)   /*发送电子围栏数据*/
		{
			g_CardLora.DestOrder = 0;   //目标编号 0=广播
			g_CardLora.SrcOrder = 0;    //源编号
			g_CardLora.Type = 0x81;
//			if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
//				Line_2A_WT588F(QXD);
			g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
//			if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
//				Line_2A_WT588F(LCXD);
			g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车
					
			memcpy((uint8_t*)&g_EFenceInf, (uint8_t*)&LYElecFenceData ,sizeof(LYElecFenceData));
			if(LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,g_EFenceInf,g_BaseAskEFPackage) == LORARET_SEND_FAIL)
			{
				LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,g_EFenceInf,g_BaseAskEFPackage);
			}

			memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
			memset((uint8_t*)&g_EFenceInf,0,sizeof(g_EFenceInf));		
			g_RecLYDataType.ElecFence = 1;			
		}					
		else if(g_RecLYDataType.ConstrTime == 1)   /*发送作业时间数据*/
		{
//			if(g_LYSendConstrTimeNum == 3)
				WriteWorkTimeToFLASH();
			g_CardLora.DestOrder = 0;   //目标编号 0=广播
			g_CardLora.SrcOrder = 0;    //源编号
			g_CardLora.Type = 2;
//			if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
//				Line_2A_WT588F(QXD);
			g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
//			if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
//				Line_2A_WT588F(LCXD);
			g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

			memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
			g_CardLocaData.BatVoltage = g_BatVal; 
			if(g_CarWarnFlag.all > 0)
				g_CardLocaData.Alram.bit.Alram= 1;
			g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
			g_CardLocaData.Alram.bit.SOS=sos ;
			if(g_ManualPunFlag == 2)
			{
				g_CardLocaData.Alram.bit.Bribe = 0x01;
				g_ManualPunFlag = 0;
			}
			else
				g_CardLocaData.Alram.bit.Bribe = 0;
			g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
			g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
			g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
			g_CardLocaData.PosQuality = g_PosQuality;
			g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>16)&0xFF;
			g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>8)&0xFF;
			g_CardLocaData.Altitude[2] = (g_tGPS.Altitude)&0xFF;		
			memcpy((uint8_t*)&g_WorkTimeInf, (uint8_t*)&g_WorkTime ,sizeof(g_WorkTimeInf));
			if(LORA_Card_BoardcastWorkTime(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WorkTimeInf) == LORARET_SEND_FAIL)
			{
				LORA_Card_BoardcastWorkTime(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WorkTimeInf);
			}
			memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
			memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
			memset((uint8_t*)&g_WorkTimeInf,0,sizeof(g_WorkTimeInf));
		}
		else if(g_RecLYDataType.WarnLEDSta > 0)
		{
//			WriteIndicatorStaToFLASH();
			g_CardLora.DestOrder = 0;   //目标编号 0=广播
			g_CardLora.SrcOrder = 0;    //源编号
			g_CardLora.Type = 4;
//			if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
//				Line_2A_WT588F(QXD);
			g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
//			if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
//				Line_2A_WT588F(LCXD);
			g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

			memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
			g_CardLocaData.BatVoltage = g_BatVal; 
			if(g_CarWarnFlag.all > 0)
				g_CardLocaData.Alram.bit.Alram= 1;
			g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
			g_CardLocaData.Alram.bit.SOS=sos ;
			if(g_ManualPunFlag == 2)
			{
				g_CardLocaData.Alram.bit.Bribe = 0x01;
				g_ManualPunFlag = 0;
			}
			else
				g_CardLocaData.Alram.bit.Bribe = 0;
			g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
			g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
			g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
			g_CardLocaData.PosQuality = g_PosQuality;
			g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>16)&0xFF;
			g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>8)&0xFF;
			g_CardLocaData.Altitude[2] = (g_tGPS.Altitude)&0xFF;		
			memcpy((uint8_t*)&g_WLEDStaInf.start, (uint8_t*)&g_WarnLedSta ,sizeof(g_WLEDStaInf.start));
			if(LORA_Card_BoardcastWarnLEDSta(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WLEDStaInf) == LORARET_SEND_FAIL)
			{
				LORA_Card_BoardcastWarnLEDSta(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WLEDStaInf);
			}
			memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
			memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
			memset((uint8_t*)&g_WLEDStaInf,0,sizeof(g_WLEDStaInf));			
		}
		else
		{

			g_CardLora.DestOrder = 0;   //目标编号 0=广播
			g_CardLora.SrcOrder = 0;    //源编号
			g_CardLora.Type = 0;
//			if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
//				Line_2A_WT588F(QXD);
			g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
//			if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
//				Line_2A_WT588F(LCXD);
			g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

			memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
			g_CardLocaData.BatVoltage = g_BatVal; 
			if(g_CarWarnFlag.all > 0)
				g_CardLocaData.Alram.bit.Alram= 1;
			g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
			g_CardLocaData.Alram.bit.SOS=sos ;
			if(g_ManualPunFlag == 2)
			{
				g_CardLocaData.Alram.bit.Bribe = 0x01;
				g_ManualPunFlag = 0;
			}
			else
				g_CardLocaData.Alram.bit.Bribe = 0;	
			g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
			g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
			g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
			g_CardLocaData.PosQuality = g_PosQuality;
			g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>16)&0xFF;
			g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>8)&0xFF;
			g_CardLocaData.Altitude[2] = (g_tGPS.Altitude)&0xFF;

			g_CardLocaData.FenceOrder= g_EFence.num;                //电子围栏编号
			g_CardLocaData.CTimeOrder = g_WorkTime.num;             //施工时间编号
			g_CardLocaData.CWarnLEDOrder = g_WarnLedSta.num;        //指示灯配置编号
			if(LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0) == LORARET_SEND_FAIL)
			{
				LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0);
			}	
			memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
			memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
		}
		g_ResDataFromBaseFlag = 0;
		g_SendDataToBaseDelay = uwTick;
		g_SendDataOrerDelay = TIM6->CNT;
		LORA_BANDWIDTH = LORA_BANDWIDTH_REC;
		LORA_SPREADING_FACTOR = LORA_SPREADING_FACTOR_REC;
		LORA_CODINGRATE = LORA_CODINGRATE_REC;		
		SX126x_SetupRadioPara(&SX126xRadioEvents);				//配置参数		
		
	}	
    #if LORATYPE
		Radio.IrqProcess();	
	#elif LORATYPE == 2
		Ebyte_RF.StartPollTask();		//检查数据	
	#endif
	ResRecRes = LORA_Card_RecvBaseWorkTime(&g_LoraPara,&g_LoraFromBaseSta,&g_WorkTimeInf);           /*接收施工时间到数据*/
	switch(ResRecRes)
	{
		case LORARET_OK:             /* 0 = 成功   收到数据 */				
		    if(memcmp((uint8_t*)&g_WorkTimeInf,(uint8_t*)&g_WorkTime,sizeof(g_WorkTimeInf)) != 0)            
			{
				g_CarWarnFlag.all = 0x00;
				WarnLED_Off();
				memcpy((uint8_t*)&g_WorkTime, (uint8_t*)&g_WorkTimeInf ,sizeof(g_WorkTimeInf));		
				WriteWorkTimeToFLASH();		
			}		
			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
			{
				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
			}
			
			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
			{
				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
				{
					g_LYSendTrainNum = 0;
					g_RecLYDataType.TrainAppro = 0;
				}
				if(g_StaJudeNum.g_Train < JudgStaNum)
				{
					if(g_CarWarnFlag.bit.TraDownRoad == 0)
						WarnLED_Off();					
					g_CarWarnFlag.bit.TraDownRoad = g_LoraFromBaseSta.TrainingSta;								
				}	
//				g_CarWarnFlag.bit.TraDownRoad = 1;				
//                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
			}
			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
			{
				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
				{
					g_LYSendUplinkNum = 0;
					g_RecLYDataType.Uplink = 0;
				}
				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
				if(g_LoraDataSta.Uplink == 1)
					g_CarWarnFlag.bit.OnRoad = 1;
				else if(g_LoraDataSta.Uplink == 2)
					g_CarWarnFlag.bit.DownRoad = 1;	
				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
			}
			if( g_RecLYDataType.ConstrTime > 0)
			{
				g_RecLYDataType.ConstrTime = 0;
			}
            SendYYWarn(ZYSJCG);				
			break;
		case LORARET_NODATA_ERR:     /*6 = 无数据*/
			break;
		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
			break;
		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
			break;
		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
			break;
	}	
//	ResRecRes = LORA_Card_RecvBaseFence(&g_LoraPara,&g_LoraFromBaseSta,&g_EFenceInf);                /*接收电子围栏数据*/
//	switch(ResRecRes)
//	{
//		case LORARET_OK:             /* 0 = 成功   收到数据 */			
//			g_CarWarnFlag.all = 0x00;
//			led_off(LED_RGB_R);
//			led_off(LED_RGB_B);
//		    memcpy((uint8_t*)&g_EFence, (uint8_t*)&g_EFenceInf ,97);
//		    WriteEFenceToFLASH();
//			if(memcmp((uint8_t*)&ElecFenceData,(uint8_t*)&g_EFenceInf,32) != 0)            /*0电子围栏接收到数据*/
//			{
//				memcpy((uint8_t*)&ElecFenceData, (uint8_t*)&g_EFenceInf ,32);		
//				g_NeedSetEFence.NeedSetEFence0 = 1;
//				g_NeedSetEFence.SendOrderFlag = 1;		
//			}
//			if(memcmp((uint8_t*)&ElecFenceData+32,(uint8_t*)&g_EFenceInf+32,32) != 0)
//			{
//				memcpy((uint8_t*)&ElecFenceData+32, (uint8_t*)&g_EFenceInf+32 ,32);		
//				g_NeedSetEFence.NeedSetEFence1 = 1;
//				g_NeedSetEFence.SendOrderFlag = 1;				
//			}
//			if(memcmp((uint8_t*)&ElecFenceData+64,(uint8_t*)&g_EFenceInf+64,32) != 0)
//			{
//				memcpy((uint8_t*)&ElecFenceData+64, (uint8_t*)&g_EFenceInf+64,32);		
//				g_NeedSetEFence.NeedSetEFence2 = 1;
//				g_NeedSetEFence.SendOrderFlag = 1;				
//			}			
//			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
//			{
//				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
//			}
//			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
//			{
//				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
//				{
//					g_LYSendTrainNum = 0;
//					g_RecLYDataType.TrainAppro = 0;
//				}
//				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
//				g_CarWarnFlag.bit.TraDownRoad = 1;
//                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
//			}
//			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
//			{
//				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
//				{
//					g_LYSendUplinkNum = 0;
//					g_RecLYDataType.Uplink = 0;
//				}
//				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
//				if(g_LoraDataSta.Uplink == 1)
//					g_CarWarnFlag.bit.OnRoad = 1;
//				else if(g_LoraDataSta.Uplink == 2)
//					g_CarWarnFlag.bit.DownRoad = 1;					
//				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
//			}
//			if(g_RecLYDataType.ElecFence > 0)
//			{
//				g_RecLYDataType.ElecFence = 0;
//			}
//			break;
//		case LORARET_NODATA_ERR:     /*6 = 无数据*/
//			break;
//		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
//			break;
//		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
//			break;
//		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
//			break;
//	}
	ResRecRes = LORA_Card_RecvBaseFence(&g_LoraPara,&g_LoraFromBaseSta,&g_EFenceInf,RTKDiffTimeOut);                /*接收新电子围栏数据*/
	switch(ResRecRes)
	{
		case LORARET_OK:             /* 0 = 成功   收到数据 */			
			g_CarWarnFlag.all = 0x00;
			WarnLED_Off();
            memcpy(g_EFence.data,g_EFenceInf.EFence,sizeof(g_EFenceInf.EFence));
            g_EFence.total = g_EFenceInf.total;
			g_EFence.num = g_EFenceInf.num;
			WriteEFenceToFLASH();
			ReadEFenceFomFlash();	
			g_P2PModeEnableMark  = 0; 	
			g_CarWarnFlag.all = 0x00;
			WarnLED_Off();
			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
			{
				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
			}
			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
			{
				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
				{
					g_LYSendTrainNum = 0;
					g_RecLYDataType.TrainAppro = 0;
				}
				if(g_StaJudeNum.g_Train < JudgStaNum)
				{
					if(g_CarWarnFlag.bit.TraDownRoad == 0)
						WarnLED_Off();					
					g_CarWarnFlag.bit.TraDownRoad = g_LoraFromBaseSta.TrainingSta;								
				}	
//				if(g_CarWarnFlag.bit.TraDownRoad == 0)
//					WarnLED_Off();				
//				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
//				g_CarWarnFlag.bit.TraDownRoad = 1;
//                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
			}
			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
			{
				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
				{
					g_LYSendUplinkNum = 0;
					g_RecLYDataType.Uplink = 0;
				}
				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
				if(g_LoraDataSta.Uplink == 1)
					g_CarWarnFlag.bit.OnRoad = 1;
				else if(g_LoraDataSta.Uplink == 2)
					g_CarWarnFlag.bit.DownRoad = 1;					
				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
			}
			if(g_RecLYDataType.ElecFence > 0)
			{
				g_P2PModeEnableMark = 0;
				g_RecLYDataType.ElecFence = 0;
				memset(LYElecFenceData,0,sizeof(LYElecFenceData));
				memset((uint8_t*)&g_RecLYEFenceData,0,sizeof(g_RecLYEFenceData));					
			}
            SendYYWarn(DZWLCG);			
			break;
		case LORARET_NODATA_ERR:     /*6 = 无数据*/
			break;
		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
			break;
		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
			break;
		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
			break;
	}
	if(g_P2PModeEnableMark > 0) 
	{
		ResRecRes = LORA_Card_RecvResP2P(&g_LoraPara,&g_LoraFromBaseSta,&g_BaseAskEFPackage);                /*接收索要点对点数据*/
		switch(ResRecRes)
		{
			case LORARET_OK:             /* 0 = 成功   收到数据 */			
				g_CarWarnFlag.all = 0x00;
				WarnLED_Off();	
				g_RecLYDataType.ElecFence = 2;
				if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
				{
					g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
				}
				
				if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
				{
					if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
					{
						g_LYSendTrainNum = 0;
						g_RecLYDataType.TrainAppro = 0;
					}
					if(g_StaJudeNum.g_Train < JudgStaNum)
					{
						if(g_CarWarnFlag.bit.TraDownRoad == 0)
							WarnLED_Off();					
						g_CarWarnFlag.bit.TraDownRoad = g_LoraFromBaseSta.TrainingSta;								
					}
//					if(g_CarWarnFlag.bit.TraDownRoad == 0)
//						WarnLED_Off();					
//					g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
//					g_CarWarnFlag.bit.TraDownRoad = 1;
//					g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
				}
				if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
				{
					if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
					{
						g_LYSendUplinkNum = 0;
						g_RecLYDataType.Uplink = 0;
					}
					g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
					if(g_LoraDataSta.Uplink == 1)
						g_CarWarnFlag.bit.OnRoad = 1;
					else if(g_LoraDataSta.Uplink == 2)
						g_CarWarnFlag.bit.DownRoad = 1;	
					g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
				}		
				break;
			case LORARET_NODATA_ERR:     /*6 = 无数据*/
				break;
			case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
				break;
			case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
				break;
			case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
				break;
		}		
	}	
    ResRecRes = LORA_Card_RecvBaseDiffByBlock(&g_LoraPara,&g_LoraFromBaseSta,g_RTKDiffData,&g_RTKDiffNum,RTKDiffTimeOut); /*接收差分信息*/
	switch(ResRecRes)
	{
		case LORARET_OK:             /* 0 = 成功   收到数据 */
//			g_LoraDelay = uwTick;
			g_P2PModeEnableMark = 0;		
			if(g_RecLora.SucNum < 255)
				g_RecLora.SucNum ++;
			else
				g_RecLora.SucNum = 0;
			#if SWITCHTYPE
			UartSend(&g_Uart1,g_RTKDiffData,g_RTKDiffNum);		        /*发送给RTK*/
				Arr2AscStr((uint8_t*)&g_RecLora,4,0,g_RecLoraStr);
				UartSend(&g_Uart1,(uint8_t*)&g_RecLora,sizeof(g_RecLora));	
			#endif
			if(g_RTKDiffNum > 0)
				UartSend(&g_Uart2,g_RTKDiffData,g_RTKDiffNum);		        /*发送给RTK*/	
			
			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
			{
				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
			}
			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
			{
				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
				{
					g_LYSendTrainNum = 0;
					g_RecLYDataType.TrainAppro = 0;
				}
				if(g_StaJudeNum.g_Train < JudgStaNum)
				{
					if(g_CarWarnFlag.bit.TraDownRoad == 0)
						WarnLED_Off();					
					g_CarWarnFlag.bit.TraDownRoad = g_LoraFromBaseSta.TrainingSta;								
				}				
//				if(g_CarWarnFlag.bit.TraDownRoad == 0)
//					WarnLED_Off();				
//				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
//				g_CarWarnFlag.bit.TraDownRoad = 1;
			}
			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
			{
				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
				{
					g_LYSendUplinkNum = 0;
					g_RecLYDataType.Uplink = 0;
				}
				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
				if(g_LoraDataSta.Uplink == 1)
					g_CarWarnFlag.bit.OnRoad = 1;
				else if(g_LoraDataSta.Uplink == 2)
					g_CarWarnFlag.bit.DownRoad = 1;					
			}
//			if(g_LYSendEFenceNum > 0 || g_RecLYDataType.ElecFence > 0)
//			{
//				g_LYSendEFenceNum = 0;
//				g_RecLYDataType.ElecFence = 0;
//			}					
			break;
		case LORARET_NODATA_ERR:     /*6 = 无数据*/
			break;
		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
			break;
		case LORARET_TIMEOUT_ERR:    /* 10 = 超时 */
			break;
		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
			break;
	}
	ResRecRes = LORA_Card_RecvBaseWarnLEDSta(&g_LoraPara,&g_LoraFromBaseSta,&g_WLEDStaInf);           /*接收广播的指示灯配置数据*/
	switch(ResRecRes)
	{
		case LORARET_OK:             /* 0 = 成功   收到数据 */				
		    if(memcmp((uint8_t*)&g_WLEDStaInf,(uint8_t*)&g_WarnLedSta,sizeof(g_WLEDStaInf)) != 0)            
			{
				g_CarWarnFlag.all = 0x00;
				WarnLED_Off();
				memcpy((uint8_t*)&g_WarnLedSta, (uint8_t*)&g_WLEDStaInf ,sizeof(g_WLEDStaInf));		
				WriteIndicatorStaToFLASH();		
			}		
			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
			{
				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
			}
			
			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
			{
				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
				{
					g_LYSendTrainNum = 0;
					g_RecLYDataType.TrainAppro = 0;
				}
				if(g_StaJudeNum.g_Train < JudgStaNum)
				{
					if(g_CarWarnFlag.bit.TraDownRoad == 0)
						WarnLED_Off();					
					g_CarWarnFlag.bit.TraDownRoad = g_LoraFromBaseSta.TrainingSta;								
				}
//				if(g_CarWarnFlag.bit.TraDownRoad == 0)
//					WarnLED_Off();
//				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
//				g_CarWarnFlag.bit.TraDownRoad = 1;
//                g_StaJudeNum.g_Train = 0; 				/*判断来车后是否下道标记*/
			}
			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
			{
				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
				{
					g_LYSendUplinkNum = 0;
					g_RecLYDataType.Uplink = 0;
				}
				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
				if(g_LoraDataSta.Uplink == 1)
					g_CarWarnFlag.bit.OnRoad = 1;
				else if(g_LoraDataSta.Uplink == 2)
					g_CarWarnFlag.bit.DownRoad = 1;	
				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
			}
			if( g_RecLYDataType.WarnLEDSta > 0)
			{
				g_RecLYDataType.WarnLEDSta = 0;
			}			
			break;
		case LORARET_NODATA_ERR:     /*6 = 无数据*/
			break;
		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
			break;
		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
			break;
		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
			break;
	}
	cardnum = 0;
    ResRecRes = LORA_Base_RecvCardPosition(&g_LoraPara, &g_CardHeader, (CardData_t**)&g_CardData, &cardnum); /*接收其他定位卡定位信息*/
	switch(ResRecRes)
	{
		/* 接收到定位卡数据 保存卡数据 */
		case LORARET_OK: 
			if (cardnum != 0)
			{
				j = 0;
				do
				{					
					RYDW_UpdateCardInfoTable(g_CardData[j], g_LoraSendOkCnt, &g_CardTable);	//更新定位卡信息表
					j++;
				}
				while (j < (cardnum - 1)); 
			}
            break;			
		case LORARET_NODATA_ERR:     /*6 = 无数据*/
			break;
		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
			break;
		case LORARET_TIMEOUT_ERR:    /* 10 = 超时 */
			break;
		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
			break;		
	}
	memset(g_LoraPara.pRxBuf, 0x00, LORA_RX_BUFSIZE);					//重置Lora接收数组
	memset((uint8_t*)&g_LoraFromBaseSta,0,sizeof(g_LoraFromBaseSta));
	/* Lora发送结束重置发送标记 */
	g_LoraPara.TxCpltFlag = 0;
    g_LoraPara.RxCpltFlag = 0;		
	if(JudgeSomeDelayInProOk(g_SendDataToBaseDelay,Delay10Min) == true)
	{
		LoraInit();
		led_on(LED_LORA); 
		g_SendDataToBaseDelay = uwTick;
	}
	
	
}













///******************************************************************
//函数名称: LoraProcess
//功能描述: Lora定位卡广播电子围栏数据
//参数输入: 
//参数输出: 
//返 回 值: LoraResult 返回值
//其    他: 无
//******************************************************************/
//void LoraProcess(void)
//{
//	uint8_t ResRecRes;
//	
//	
//	uint8_t chksum,i;
//	uint16_t rlen,tmpLoraRecvBufSize= 0;
//	
//	
////	/*解析接收Lora数据*/
////	if(LORA_RecvMultiPkg(&g_LoraPara, tmpLoraRecvBuf,&tmpLoraRecvBufSize , RTKDiffTimeOut) == LORARET_OK)
////	{
////		if (g_LoraRecvBuf[0] == LORA_HEADER && g_LoraRecvBuf[1] == LORA_DOWNSTREAM)
////		{
////			rlen = g_LoraRecvBuf[2] + (g_LoraRecvBuf[3] << 8)+ 5;
////			/* 检查校验和 */
////			chksum = CheckSum(rlen-1, g_LoraRecvBuf);
////			if (chksum == g_LoraRecvBuf[rlen-1])
////			{
////				/* 获取基站数据头 */
////				LORA_Arr2Header(g_LoraRecvBuf+ 4, &g_LoraFromBaseSta);	
////				if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
////					g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;	
////				if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
////				{
////					if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
////					{
////						g_LYSendTrainNum = 0;
////						g_RecLYDataType.TrainAppro = 0;
////					}
////					g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
////					g_CarWarnFlag.bit.TraDownRoad = 1;
////					g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
////				}
////				if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
////				{
////					if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
////					{
////						g_LYSendUplinkNum = 0;
////						g_RecLYDataType.Uplink = 0;
////					}
////					g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
////					if(g_LoraDataSta.Uplink == 1)
////						g_CarWarnFlag.bit.OnRoad = 1;
////					else if(g_LoraDataSta.Uplink == 2)
////						g_CarWarnFlag.bit.DownRoad = 1;	
////					g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
////				}
////				g_ResDataFromBaseFlag = 1;
////				g_SendDataToBaseDelay = uwTick;						
////				switch(g_LoraRecvBuf[3])
////				{
////					case LORA_TYPE_P2P:      /* 检查数据类型=应答点对点数据 */
////						g_P2PModeEnableMark  = 1; 				
////						g_RecLYDataType.ElecFence = 2;				
////						break;					
////					case LORA_TYPE_RESEF:    /* 检查数据类型=应答定位卡电子围栏数据 */
////						g_P2PModeEnableMark  = 0; 				
////						g_RecLYDataType.ElecFence = 0;
////						memset(LYElecFenceData,0,sizeof(LYElecFenceData));
////						memset((uint8_t*)&g_RecLYEFenceData,0,sizeof(g_RecLYEFenceData));								
////						break;	
////					case LORA_TYPE_WTIME:    /* 检查数据类型=施工时间数据 */
////						memcpy((uint8_t*)&g_WorkTime,g_LoraRecvBuf+12,sizeof(uint8_t)*13);
////						if(g_P2PModeEnableMark == 1 && g_RecLYDataType.ElecFence == 0)		
////							g_P2PModeEnableMark  = 0;
////						else if(g_RecLYDataType.ElecFence > 0)
////						{
////							g_RecLYDataType.ElecFence = 1;
////							g_P2PModeEnableMark  = 1;
////						}
////						g_CarWarnFlag.all = 0x00;
////						led_off(LED_RGB_R);
////						led_off(LED_RGB_B);	
////						WriteWorkTimeToFLASH();			
////						if(g_RecLYDataType.ConstrTime > 0)
////							g_RecLYDataType.ConstrTime = 0;		
////						g_SendDataOrerDelay = TIM6->CNT;
////						break;	
////					case LORA_TYPE_EFENCE:   /* 检查数据类型=围栏数据 */
////                        g_SendDataOrerDelay = TIM6->CNT;
////						g_EFence.total = g_LoraRecvBuf[12];
////						/* 获取电子围栏信息 */
////						for (i=0; i<g_EFence.total; i++)
////						{
////							memcpy(g_EFence.data,g_LoraRecvBuf+13+i*123,sizeof(uint8_t)*123);
////						}
////						g_EFence.num = g_LoraRecvBuf[14+g_EFence.total*123];
////						WriteEFenceToFLASH();
////						ReadEFenceFomFlash();	
////						g_P2PModeEnableMark  = 0; 	
////						g_CarWarnFlag.all = 0x00;
////						led_off(LED_RGB_R);
////						led_off(LED_RGB_B);
////	
////						if(g_RecLYDataType.ElecFence > 0)
////						{
////							g_RecLYDataType.ElecFence = 0;
////							memset(LYElecFenceData,0,sizeof(LYElecFenceData));
////							memset((uint8_t*)&g_RecLYEFenceData,0,sizeof(g_RecLYEFenceData));					
////						}						
////						break;
////					case LORA_TYPE_DIFFINFO: /* 检查数据类型=差分数据 */
////						if(g_P2PModeEnableMark == 1 && g_RecLYDataType.ElecFence == 0)		
////							g_P2PModeEnableMark  = 0;
////						else if(g_RecLYDataType.ElecFence > 0)
////						{
////							g_RecLYDataType.ElecFence = 1;
////							g_P2PModeEnableMark  = 1;				
////						}
////						if(g_LoraRecvBuf[rlen-5] == 0x5A && g_LoraRecvBuf[rlen-4] == 0xA5 && g_LoraRecvBuf[rlen-3] == 0xA5 && g_LoraRecvBuf[rlen-2] == 0x5A)
////						{
////							 memcpy(g_RTKDiffData,g_LoraRecvBuf+12,rlen-13);    /*差分数据拷贝*/
////							 if(g_RTKDiffNum > 0)
////								UartSend(&g_Uart2,g_RTKDiffData,rlen-13);		        /*发送给RTK*/	
////						}
////						g_SendDataOrerDelay = TIM6->CNT;
////						break;						
////				}								
////			}
////		}
////		else if(g_LoraRecvBuf[0] == LORA_HEADER && g_LoraRecvBuf[1] == LORA_UPSTREAM)      /*解析其他定位卡数据*/
////		{
////			rlen = g_LoraRecvBuf[2] + (g_LoraRecvBuf[3] << 8)+ 5;
////			/* 检查校验和 */
////			chksum = CheckSum(rlen-1, g_LoraRecvBuf);
////			if (chksum == g_LoraRecvBuf[rlen-1])
////			{
////				if(g_LoraRecvBuf[4] == LORA_TYPE_POSINFO)     /*定位卡定位信息*/
////				{
////					
////				}
////			
////			}


////		}

////		
////	}
////	/*发送数据*/
////	if(g_ResDataFromBaseFlag == 1 && JudgeSomeDelayInProOk(g_SendDataOrerDelay,Delay500mSec*g_CareIDNum)) /*超时*/
////	{
////			if(g_RecLYDataType.ElecFence == 1)   /*请求点对点申请*/
////			{				 
////				memcpy(g_LoraSendBuf+12, (uint8_t*)&g_CardID ,sizeof(g_CardID)); /*卡定位信息*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;					
////				
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 3;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;	
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////		
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;	
////				if(LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0);
////				}	
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));	
////			}
////			if(g_RecLYDataType.ElecFence == 1)   /*点对点申请*/
////			{
////				/*请求点对点通信*/
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 3;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;	
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;				
////				if(LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0);
////				}	
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));	
////			}			
////			else if(g_RecLYDataType.ElecFence == 2)   /*发送电子围栏数据*/
////			{
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 1;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));						
////				memcpy((uint8_t*)&g_EFenceInf, (uint8_t*)&LYElecFenceData ,sizeof(LYElecFenceData));
////				if(LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,&g_CardLocaData,g_EFenceInf) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,&g_CardLocaData,g_EFenceInf);
////				}

////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
////				memset((uint8_t*)&g_EFenceInf,0,sizeof(g_EFenceInf));		
////							
////			}		
////			else if(g_RecLYDataType.ConstrTime == 1)   /*发送作业时间数据*/
////			{
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 2;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;		
////				memcpy((uint8_t*)&g_WorkTimeInf, (uint8_t*)&g_WorkTime ,sizeof(g_WorkTimeInf));
////				if(LORA_Card_BoardcastWorkTime(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WorkTimeInf) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastWorkTime(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WorkTimeInf);
////				}
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
////				memset((uint8_t*)&g_WorkTimeInf,0,sizeof(g_WorkTimeInf));
////			}
////			else if(g_P2PModeEnableMark == 0)	
////			{

////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 0;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;	
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;				
////				if(LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0);
////				}	
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
////			}		
////		
////	}		
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
////	if(g_ResDataFromBaseFlag == 1 && JudgeSomeDelayInProOk(g_SendDataOrerDelay,Delay500mSec*g_CareIDNum)) /*超时*/
////	{
////			if(g_RecLYDataType.ElecFence == 1)   /*点对点申请*/
////			{
////				/*请求点对点通信*/
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 3;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;	
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;				
////				if(LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0);
////				}	
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));	
////			}			
////			else if(g_RecLYDataType.ElecFence == 2)   /*发送电子围栏数据*/
////			{
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 1;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));						
////				memcpy((uint8_t*)&g_EFenceInf, (uint8_t*)&LYElecFenceData ,sizeof(LYElecFenceData));
////				if(LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,&g_CardLocaData,g_EFenceInf) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,&g_CardLocaData,g_EFenceInf);
////				}

////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
////				memset((uint8_t*)&g_EFenceInf,0,sizeof(g_EFenceInf));		
////							
////			}		
////			else if(g_RecLYDataType.ConstrTime == 1)   /*发送作业时间数据*/
////			{
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 2;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;		
////				memcpy((uint8_t*)&g_WorkTimeInf, (uint8_t*)&g_WorkTime ,sizeof(g_WorkTimeInf));
////				if(LORA_Card_BoardcastWorkTime(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WorkTimeInf) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastWorkTime(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WorkTimeInf);
////				}
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
////				memset((uint8_t*)&g_WorkTimeInf,0,sizeof(g_WorkTimeInf));
////			}
////			else if(g_P2PModeEnableMark == 0)	
////			{

////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 0;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;	
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;				
////				if(LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0);
////				}	
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
////			}			
////		g_ResDataFromBaseFlag = 0;
////		g_SendDataToBaseDelay = uwTick;
////		g_SendDataOrerDelay = TIM6->CNT;
////	}
////	Ebyte_RF.StartPollTask();		//检查数据	
////	
//////	ResRecRes = LORA_RecvMultiPkg(&g_LoraPara,g_OutBufInf,g_OutBufLen,RTKDiffTimeOut)
//////	switch(ResRecRes)
//////	{
//////		case LORARET_OK:             /* 0 = 成功   收到数据 */
//////			if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
//////			{
//////				rlen = Para->pRxBuf[2] + 4;
//////				/* 检查校验和 */
//////				chksum = CheckSum(rlen-1, Para->pRxBuf);
//////				if (chksum == Para->pRxBuf[rlen-1])
//////				{
//////					/* 检查数据类型=应答点对点数据 */
//////					if (Para->pRxBuf[3] == LORA_TYPE_P2P)
//////					{
//////						/* 获取基站数据头 */
//////						LORA_Arr2Header(Para->pRxBuf + 3, Header);
//////					}
//////				}
//////		
//////			break;
//////		case LORARET_NODATA_ERR:     /*6 = 无数据*/
//////			break;
//////		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
//////			break;
//////		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
//////			break;
//////		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
//////			break;	
//////	}		
////	
////	ResRecRes = LORA_Card_RecvResP2P(&g_LoraPara,&g_LoraFromBaseSta);   /*点对点应答数据*/
////	switch(ResRecRes)
////	{
////		case LORARET_OK:             /* 0 = 成功   收到数据 */		
////            g_P2PModeEnableMark  = 1; 				
////			g_RecLYDataType.ElecFence = 2;
////			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
////			{
////				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
////			}
////			
////			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
////			{
////				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
////				{
////					g_LYSendTrainNum = 0;
////					g_RecLYDataType.TrainAppro = 0;
////				}
////				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
////				g_CarWarnFlag.bit.TraDownRoad = 1;
////                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
////			}
////			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
////			{
////				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
////				{
////					g_LYSendUplinkNum = 0;
////					g_RecLYDataType.Uplink = 0;
////				}
////				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
////				if(g_LoraDataSta.Uplink == 1)
////					g_CarWarnFlag.bit.OnRoad = 1;
////				else if(g_LoraDataSta.Uplink == 2)
////					g_CarWarnFlag.bit.DownRoad = 1;	
////				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
////			}		
////			break;
////		case LORARET_NODATA_ERR:     /*6 = 无数据*/
////			break;
////		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
////			break;
////		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
////			break;
////		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
////			break;	
////	}
////	ResRecRes = LORA_Card_RecvResEFence(&g_LoraPara,&g_LoraFromBaseSta);   /*电子围栏应答数据*/
////	switch(ResRecRes)
////	{
////		case LORARET_OK:             /* 0 = 成功   收到数据 */	
////            g_P2PModeEnableMark  = 0; 				
////			g_RecLYDataType.ElecFence = 0;
////			memset(LYElecFenceData,0,sizeof(LYElecFenceData));
////		    memset((uint8_t*)&g_RecLYEFenceData,0,sizeof(g_RecLYEFenceData));		
////			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
////				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
////			
////			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
////			{
////				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
////				{
////					g_LYSendTrainNum = 0;
////					g_RecLYDataType.TrainAppro = 0;
////				}
////				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
////				g_CarWarnFlag.bit.TraDownRoad = 1;
////                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
////			}
////			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
////			{
////				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
////				{
////					g_LYSendUplinkNum = 0;
////					g_RecLYDataType.Uplink = 0;
////				}
////				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
////				if(g_LoraDataSta.Uplink == 1)
////					g_CarWarnFlag.bit.OnRoad = 1;
////				else if(g_LoraDataSta.Uplink == 2)
////					g_CarWarnFlag.bit.DownRoad = 1;	
////				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
////			}		
////			break;
////		case LORARET_NODATA_ERR:     /*6 = 无数据*/
////			break;
////		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
////			break;
////		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
////			break;
////		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
////			break;	
////	}	
////	ResRecRes = LORA_Card_RecvBaseWorkTime(&g_LoraPara,&g_LoraFromBaseSta,&g_WorkTimeInf);    /*作业时间*/
////	switch(ResRecRes)
////	{
////		case LORARET_OK:             /* 0 = 成功   收到数据 */
////			if(g_P2PModeEnableMark == 1 && g_RecLYDataType.ElecFence == 0)		
////				g_P2PModeEnableMark  = 0;
//// 			else if(g_RecLYDataType.ElecFence > 0)
////			{
////				g_RecLYDataType.ElecFence = 1;
////				g_P2PModeEnableMark  = 1;
////			}
////		    if(memcmp((uint8_t*)&g_WorkTimeInf,(uint8_t*)&g_WorkTime,sizeof(g_WorkTimeInf)) != 0)            /*接收施工时间到数据*/
////			{
////				g_CarWarnFlag.all = 0x00;
////				led_off(LED_RGB_R);
////				led_off(LED_RGB_B);
////				memcpy((uint8_t*)&g_WorkTime, (uint8_t*)&g_WorkTimeInf ,sizeof(g_WorkTimeInf));		
////				WriteWorkTimeToFLASH();		
////			}		
////			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
////			{
////				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
////			}
////			
////			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
////			{
////				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
////				{
////					g_LYSendTrainNum = 0;
////					g_RecLYDataType.TrainAppro = 0;
////				}
////				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
////				g_CarWarnFlag.bit.TraDownRoad = 1;
////                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
////			}
////			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
////			{
////				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
////				{
////					g_LYSendUplinkNum = 0;
////					g_RecLYDataType.Uplink = 0;
////				}
////				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
////				if(g_LoraDataSta.Uplink == 1)
////					g_CarWarnFlag.bit.OnRoad = 1;
////				else if(g_LoraDataSta.Uplink == 2)
////					g_CarWarnFlag.bit.DownRoad = 1;	
////				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
////			}
////			if(g_RecLYDataType.ConstrTime > 0)
////			{
////				g_RecLYDataType.ConstrTime = 0;
////			}			
////			break;
////		case LORARET_NODATA_ERR:     /*6 = 无数据*/
////			break;
////		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
////			break;
////		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
////			break;
////		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
////			break;
////	}	
////	ResRecRes = LORA_Card_RecvBaseFence(&g_LoraPara,&g_LoraFromBaseSta,&g_EFenceInf);                               /*接收电子围栏数据*/
////	switch(ResRecRes)
////	{
////		case LORARET_OK:             /* 0 = 成功   收到数据 */			
////			g_P2PModeEnableMark  = 0; 	
////			g_CarWarnFlag.all = 0x00;
////			led_off(LED_RGB_R);
////			led_off(LED_RGB_B);
////		    memcpy((uint8_t*)&g_EFence, (uint8_t*)&g_EFenceInf ,sizeof(g_EFenceInf));
////		    WriteEFenceToFLASH();
////	        ReadEFenceFomFlash();		
////			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
////			{
////				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
////			}
////			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
////			{
////				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
////				{
////					g_LYSendTrainNum = 0;
////					g_RecLYDataType.TrainAppro = 0;
////				}
////				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
////				g_CarWarnFlag.bit.TraDownRoad = 1;
////                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
////			}
////			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
////			{
////				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
////				{
////					g_LYSendUplinkNum = 0;
////					g_RecLYDataType.Uplink = 0;
////				}
////				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
////				if(g_LoraDataSta.Uplink == 1)
////					g_CarWarnFlag.bit.OnRoad = 1;
////				else if(g_LoraDataSta.Uplink == 2)
////					g_CarWarnFlag.bit.DownRoad = 1;					
////				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
////			}
////			if(g_RecLYDataType.ElecFence > 0)
////			{
////				g_RecLYDataType.ElecFence = 0;
////				memset(LYElecFenceData,0,sizeof(LYElecFenceData));
////				memset((uint8_t*)&g_RecLYEFenceData,0,sizeof(g_RecLYEFenceData));					
////			}
////			break;
////		case LORARET_NODATA_ERR:     /*6 = 无数据*/
////			break;
////		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
////			break;
////		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
////			break;
////		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
////			break;
////	}
////    ResRecRes = LORA_Card_RecvBaseDiffByBlock(&g_LoraPara,&g_LoraFromBaseSta,g_RTKDiffData,&g_RTKDiffNum,RTKDiffTimeOut);                           /*接收差分信息*/
////	switch(ResRecRes)
////	{
////		case LORARET_OK:             /* 0 = 成功   收到数据 */
//////			g_LoraDelay = uwTick;
////			if(g_P2PModeEnableMark == 1 && g_RecLYDataType.ElecFence == 0)		
////				g_P2PModeEnableMark  = 0;
//// 			else if(g_RecLYDataType.ElecFence > 0)
////			{
////				g_RecLYDataType.ElecFence = 1;
////                g_P2PModeEnableMark  = 1;				
////			}
////			if(g_RecLora.SucNum < 255)
////				g_RecLora.SucNum ++;
////			else
////				g_RecLora.SucNum = 0;
////			if(g_RTKDiffNum > 0)
////				UartSend(&g_Uart2,g_RTKDiffData,g_RTKDiffNum);		        /*发送给RTK*/		
////			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
////			{
////				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
////			}
////			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
////			{
////				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
////				{
////					g_LYSendTrainNum = 0;
////					g_RecLYDataType.TrainAppro = 0;
////				}
////				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
////				g_CarWarnFlag.bit.TraDownRoad = 1;
////			}
////			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
////			{
////				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
////				{
////					g_LYSendUplinkNum = 0;
////					g_RecLYDataType.Uplink = 0;
////				}
////				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
////				if(g_LoraDataSta.Uplink == 1)
////					g_CarWarnFlag.bit.OnRoad = 1;
////				else if(g_LoraDataSta.Uplink == 2)
////					g_CarWarnFlag.bit.DownRoad = 1;					
////			}					
////			break;
////		case LORARET_NODATA_ERR:     /*6 = 无数据*/
////			break;
////		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
////			break;
////		case LORARET_TIMEOUT_ERR:    /* 10 = 超时 */
////			break;
////		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
////			break;
////	}
//	memset(g_LoraPara.pRxBuf, 0x00, LORA_RX_BUFSIZE);					//重置Lora接收数组
//	memset((uint8_t*)&g_LoraFromBaseSta,0,sizeof(g_LoraFromBaseSta));
//		
//	if(JudgeMilSecondDelayInPro(g_SendDataToBaseDelay,Delay10Min) == true)
//	{
//		LoraInit();
//		led_on(LED_LORA); 
//		g_SendDataToBaseDelay = uwTick;
//	}	
//}





#endif
#include "bsp.h"
#include "main.h"

#include "bsp.h"
#if LORATYPE                //=0移远E22，=1安信可,
  #include "ebyte_core.h"
#elif LORATYPE == 2
  #include "radio.h"
  #include "sx126x_board.h"
  #include "sx126x_callback.h"
#endif
LoraHeader_t	g_CardHeader;		//定位卡数据头
CardData_t		g_CardData[4];		//定位卡数据组


LoraHeader_t     g_CardLora;
CardData_t       g_CardLocaData;
CardData_t       g_OtherCardData[2];
EFence_t         g_EFenceInf; 
WorkTime_t       g_WorkTimeInf;
WLEDSta_t        g_WLEDStaInf;
LoraHeader_t     g_LoraFromBaseSta;

DataType         g_LoraDataSta;     /*设备状态*/

CardInfoTable_t	 g_CardTable;		//定位卡信息表

uint8_t          g_RTKDiffData[1400];  /*差分数据*/
uint16_t         g_RTKDiffNum;         /*差分数据长度*/


uint32_t g_SendDataToBaseDelay;    /*给基站发送数据时间*/
uint32_t g_SendDataOrerDelay;
uint8_t  g_ResDataFromBaseFlag;    /*接收到基站数据标记*/

/*卡ID*/
uint16_t g_CareIDNum;                /*设备最后一个ID*/
uint8_t g_LYSendUplinkNum;       /*发送上道次数*/
uint8_t g_LYSendTrainNum;        /*来车下发次数*/

/**/
uint8_t g_LocalAstrngFlag;         /*收敛标记*/
uint8_t LowPowerWarnFlag;          /*低功耗报警 =1电量小于50%,=2电量小于80%  =0 电量大于等于80% */

uint8_t  g_StrSelfCheck_Lora;      /*lora自检标记*/

uint8_t  g_P2PModeEnableMark;      /*点对点模式开启标记*/
uint8_t  g_BaseAskEFPackage;       /*基站索要包序号*/
uint8_t			g_LoraSendCnt;		//Lora发送次数
uint8_t			g_LoraSendOkCnt;	//Lora发送成功次数

void LoraInit(void)
{
	g_P2PModeEnableMark  = 0; 
	g_LYSendUplinkNum = 0;       /*发送上道次数*/
	g_LYSendTrainNum = 0;
	g_LocalAstrngFlag=0;        /*收敛标记*/
	g_SendDataToBaseDelay = uwTick;
	g_SendDataOrerDelay = TIM6->CNT;
	g_NoDWDelay =  TIM6->CNT;
	LowPowerWarnFlag = 0;
	g_ResDataFromBaseFlag = 0;
	g_StrSelfCheck_Lora = 0;
	g_BaseAskEFPackage = 0;
	memset((uint8_t*)&g_NeedSetEFence,0,sizeof(g_NeedSetEFence));
}



/******************************************************************
函数名称: LoraProcess
功能描述: Lora定位卡广播电子围栏数据
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
void LoraProcess(void)
{
	
	uint8_t		cardnum;	
	uint8_t		j;
	uint8_t     ResRecRes;
	
	if(g_ResDataFromBaseFlag == 1 && JudgeSomeDelayInProOk(g_SendDataOrerDelay,Delay500mSec*g_CareIDNum+Delay50mSec)) /*超时g_ResDataFromBaseFlag == 1 && Delay100mSec*g_CareIDNum+Delay50mSec*/
	{
		LORA_BANDWIDTH = LORA_BANDWIDTH_SEND;
		LORA_SPREADING_FACTOR = LORA_SPREADING_FACTOR_SEND;
		LORA_CODINGRATE = LORA_CODINGRATE_SEND;		
		SX126x_SetupRadioPara(&SX126xRadioEvents);				//配置参数
//		if(g_RecLYDataType.ElecFence == 1)   /*蓝牙下发电子围栏，下发三次*/
//		{
//			g_CardLora.DestOrder = 0;   //目标编号 0=广播
//			g_CardLora.SrcOrder = 0;    //源编号
//			g_CardLora.Type = 1;
//			if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
//				Line_2A_WT588F(QXD);
//			g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
//			if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
//				Line_2A_WT588F(LCXD);
//			g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

//			memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
//			g_CardLocaData.BatVoltage = g_BatVal; 
//			if(g_CarWarnFlag.all > 0)
//				g_CardLocaData.Alram.bit.Alram= 1;
//			g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
//			g_CardLocaData.Alram.bit.SOS=sos ;
//			if(g_ManualPunFlag == 2)
//			{
//				g_CardLocaData.Alram.bit.Bribe = 0x01;
//				g_ManualPunFlag = 0;
//			}
//			else
//				g_CardLocaData.Alram.bit.Bribe = 0;
//			g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
//			g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
//			g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
//			g_CardLocaData.PosQuality = g_PosQuality;
//			g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
//			g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
//			g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;				
//			memcpy((uint8_t*)&g_EFenceInf, (uint8_t*)&LYElecFenceData ,sizeof(g_EFenceInf)-1);
//			if(LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,&g_CardLocaData,g_EFenceInf) == LORARET_SEND_FAIL)
//			{
//				LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,&g_CardLocaData,g_EFenceInf);
//			}
//			memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
//			memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
//			memset((uint8_t*)&g_EFenceInf,0,sizeof(g_EFenceInf));
//		}
		
		if(g_RecLYDataType.ElecFence == 1 && g_P2PModeEnableMark == 0 )   /*点对点申请*/
		{
			/*请求点对点通信*/
			g_CardLora.DestOrder = 0;   //目标编号 0=广播
			g_CardLora.SrcOrder = 0;    //源编号
			g_CardLora.Type = 3;
//			if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
//				Line_2A_WT588F(QXD);
			g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
//			if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
//				Line_2A_WT588F(LCXD);
			g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

			memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
			g_CardLocaData.BatVoltage = g_BatVal; 
			if(g_CarWarnFlag.all > 0)
				g_CardLocaData.Alram.bit.Alram= 1;
			g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
			g_CardLocaData.Alram.bit.SOS=sos ;
			if(g_ManualPunFlag == 2)
			{
				g_CardLocaData.Alram.bit.Bribe = 0x01;
				g_ManualPunFlag = 0;
			}
			else
				g_CardLocaData.Alram.bit.Bribe = 0;	
			g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
			g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
			g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
			g_CardLocaData.PosQuality = g_PosQuality;
			g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>16)&0xFF;
			g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>8)&0xFF;
			g_CardLocaData.Altitude[2] = (g_tGPS.Altitude)&0xFF;				
			if(LORA_Card_BoardcastP2PInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,g_RecLYEFenceData.Total) == LORARET_SEND_FAIL)
			{
				LORA_Card_BoardcastP2PInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,g_RecLYEFenceData.Total);
			}
			g_P2PModeEnableMark  = 1;				
			memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
			memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));	
		}			
		else if(g_RecLYDataType.ElecFence == 2 && g_P2PModeEnableMark  == 1)   /*发送电子围栏数据*/
		{
			g_CardLora.DestOrder = 0;   //目标编号 0=广播
			g_CardLora.SrcOrder = 0;    //源编号
			g_CardLora.Type = 0x81;
//			if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
//				Line_2A_WT588F(QXD);
			g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
//			if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
//				Line_2A_WT588F(LCXD);
			g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车
					
			memcpy((uint8_t*)&g_EFenceInf, (uint8_t*)&LYElecFenceData ,sizeof(LYElecFenceData));
			if(LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,g_EFenceInf,g_BaseAskEFPackage) == LORARET_SEND_FAIL)
			{
				LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,g_EFenceInf,g_BaseAskEFPackage);
			}

			memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
			memset((uint8_t*)&g_EFenceInf,0,sizeof(g_EFenceInf));		
			g_RecLYDataType.ElecFence = 1;			
		}					
		else if(g_RecLYDataType.ConstrTime == 1)   /*发送作业时间数据*/
		{
//			if(g_LYSendConstrTimeNum == 3)
				WriteWorkTimeToFLASH();
			g_CardLora.DestOrder = 0;   //目标编号 0=广播
			g_CardLora.SrcOrder = 0;    //源编号
			g_CardLora.Type = 2;
//			if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
//				Line_2A_WT588F(QXD);
			g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
//			if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
//				Line_2A_WT588F(LCXD);
			g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

			memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
			g_CardLocaData.BatVoltage = g_BatVal; 
			if(g_CarWarnFlag.all > 0)
				g_CardLocaData.Alram.bit.Alram= 1;
			g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
			g_CardLocaData.Alram.bit.SOS=sos ;
			if(g_ManualPunFlag == 2)
			{
				g_CardLocaData.Alram.bit.Bribe = 0x01;
				g_ManualPunFlag = 0;
			}
			else
				g_CardLocaData.Alram.bit.Bribe = 0;
			g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
			g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
			g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
			g_CardLocaData.PosQuality = g_PosQuality;
			g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>16)&0xFF;
			g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>8)&0xFF;
			g_CardLocaData.Altitude[2] = (g_tGPS.Altitude)&0xFF;		
			memcpy((uint8_t*)&g_WorkTimeInf, (uint8_t*)&g_WorkTime ,sizeof(g_WorkTimeInf));
			if(LORA_Card_BoardcastWorkTime(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WorkTimeInf) == LORARET_SEND_FAIL)
			{
				LORA_Card_BoardcastWorkTime(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WorkTimeInf);
			}
			memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
			memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
			memset((uint8_t*)&g_WorkTimeInf,0,sizeof(g_WorkTimeInf));
		}
		else if(g_RecLYDataType.WarnLEDSta > 0)
		{
//			WriteIndicatorStaToFLASH();
			g_CardLora.DestOrder = 0;   //目标编号 0=广播
			g_CardLora.SrcOrder = 0;    //源编号
			g_CardLora.Type = 4;
//			if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
//				Line_2A_WT588F(QXD);
			g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
//			if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
//				Line_2A_WT588F(LCXD);
			g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

			memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
			g_CardLocaData.BatVoltage = g_BatVal; 
			if(g_CarWarnFlag.all > 0)
				g_CardLocaData.Alram.bit.Alram= 1;
			g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
			g_CardLocaData.Alram.bit.SOS=sos ;
			if(g_ManualPunFlag == 2)
			{
				g_CardLocaData.Alram.bit.Bribe = 0x01;
				g_ManualPunFlag = 0;
			}
			else
				g_CardLocaData.Alram.bit.Bribe = 0;
			g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
			g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
			g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
			g_CardLocaData.PosQuality = g_PosQuality;
			g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>16)&0xFF;
			g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>8)&0xFF;
			g_CardLocaData.Altitude[2] = (g_tGPS.Altitude)&0xFF;		
			memcpy((uint8_t*)&g_WLEDStaInf.start, (uint8_t*)&g_WarnLedSta ,sizeof(g_WLEDStaInf.start));
			if(LORA_Card_BoardcastWarnLEDSta(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WLEDStaInf) == LORARET_SEND_FAIL)
			{
				LORA_Card_BoardcastWarnLEDSta(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WLEDStaInf);
			}
			memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
			memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
			memset((uint8_t*)&g_WLEDStaInf,0,sizeof(g_WLEDStaInf));			
		}
		else
		{

			g_CardLora.DestOrder = 0;   //目标编号 0=广播
			g_CardLora.SrcOrder = 0;    //源编号
			g_CardLora.Type = 0;
//			if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
//				Line_2A_WT588F(QXD);
			g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
//			if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
//				Line_2A_WT588F(LCXD);
			g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

			memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
			g_CardLocaData.BatVoltage = g_BatVal; 
			if(g_CarWarnFlag.all > 0)
				g_CardLocaData.Alram.bit.Alram= 1;
			g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
			g_CardLocaData.Alram.bit.SOS=sos ;
			if(g_ManualPunFlag == 2)
			{
				g_CardLocaData.Alram.bit.Bribe = 0x01;
				g_ManualPunFlag = 0;
			}
			else
				g_CardLocaData.Alram.bit.Bribe = 0;	
			g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
			g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
			g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
			g_CardLocaData.PosQuality = g_PosQuality;
			g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>16)&0xFF;
			g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>8)&0xFF;
			g_CardLocaData.Altitude[2] = (g_tGPS.Altitude)&0xFF;

			g_CardLocaData.FenceOrder= g_EFence.num;                //电子围栏编号
			g_CardLocaData.CTimeOrder = g_WorkTime.num;             //施工时间编号
			g_CardLocaData.CWarnLEDOrder = g_WarnLedSta.num;        //指示灯配置编号
			if(LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0) == LORARET_SEND_FAIL)
			{
				LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0);
			}	
			memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
			memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
		}
		g_ResDataFromBaseFlag = 0;
		g_SendDataToBaseDelay = uwTick;
		g_SendDataOrerDelay = TIM6->CNT;
		LORA_BANDWIDTH = LORA_BANDWIDTH_REC;
		LORA_SPREADING_FACTOR = LORA_SPREADING_FACTOR_REC;
		LORA_CODINGRATE = LORA_CODINGRATE_REC;		
		SX126x_SetupRadioPara(&SX126xRadioEvents);				//配置参数		
		
	}	
    #if LORATYPE
		Radio.IrqProcess();	
	#elif LORATYPE == 2
		Ebyte_RF.StartPollTask();		//检查数据	
	#endif
	ResRecRes = LORA_Card_RecvBaseWorkTime(&g_LoraPara,&g_LoraFromBaseSta,&g_WorkTimeInf);           /*接收施工时间到数据*/
	switch(ResRecRes)
	{
		case LORARET_OK:             /* 0 = 成功   收到数据 */				
		    if(memcmp((uint8_t*)&g_WorkTimeInf,(uint8_t*)&g_WorkTime,sizeof(g_WorkTimeInf)) != 0)            
			{
				g_CarWarnFlag.all = 0x00;
				WarnLED_Off();
				memcpy((uint8_t*)&g_WorkTime, (uint8_t*)&g_WorkTimeInf ,sizeof(g_WorkTimeInf));		
				WriteWorkTimeToFLASH();		
			}		
			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
			{
				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
			}
			
			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
			{
				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
				{
					g_LYSendTrainNum = 0;
					g_RecLYDataType.TrainAppro = 0;
				}
				if(g_StaJudeNum.g_Train < JudgStaNum)
				{
					if(g_CarWarnFlag.bit.TraDownRoad == 0)
						WarnLED_Off();					
					g_CarWarnFlag.bit.TraDownRoad = g_LoraFromBaseSta.TrainingSta;								
				}	
//				g_CarWarnFlag.bit.TraDownRoad = 1;				
//                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
			}
			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
			{
				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
				{
					g_LYSendUplinkNum = 0;
					g_RecLYDataType.Uplink = 0;
				}
				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
				if(g_LoraDataSta.Uplink == 1)
					g_CarWarnFlag.bit.OnRoad = 1;
				else if(g_LoraDataSta.Uplink == 2)
					g_CarWarnFlag.bit.DownRoad = 1;	
				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
			}
			if( g_RecLYDataType.ConstrTime > 0)
			{
				g_RecLYDataType.ConstrTime = 0;
			}
            SendYYWarn(ZYSJCG);				
			break;
		case LORARET_NODATA_ERR:     /*6 = 无数据*/
			break;
		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
			break;
		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
			break;
		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
			break;
	}	
//	ResRecRes = LORA_Card_RecvBaseFence(&g_LoraPara,&g_LoraFromBaseSta,&g_EFenceInf);                /*接收电子围栏数据*/
//	switch(ResRecRes)
//	{
//		case LORARET_OK:             /* 0 = 成功   收到数据 */			
//			g_CarWarnFlag.all = 0x00;
//			led_off(LED_RGB_R);
//			led_off(LED_RGB_B);
//		    memcpy((uint8_t*)&g_EFence, (uint8_t*)&g_EFenceInf ,97);
//		    WriteEFenceToFLASH();
//			if(memcmp((uint8_t*)&ElecFenceData,(uint8_t*)&g_EFenceInf,32) != 0)            /*0电子围栏接收到数据*/
//			{
//				memcpy((uint8_t*)&ElecFenceData, (uint8_t*)&g_EFenceInf ,32);		
//				g_NeedSetEFence.NeedSetEFence0 = 1;
//				g_NeedSetEFence.SendOrderFlag = 1;		
//			}
//			if(memcmp((uint8_t*)&ElecFenceData+32,(uint8_t*)&g_EFenceInf+32,32) != 0)
//			{
//				memcpy((uint8_t*)&ElecFenceData+32, (uint8_t*)&g_EFenceInf+32 ,32);		
//				g_NeedSetEFence.NeedSetEFence1 = 1;
//				g_NeedSetEFence.SendOrderFlag = 1;				
//			}
//			if(memcmp((uint8_t*)&ElecFenceData+64,(uint8_t*)&g_EFenceInf+64,32) != 0)
//			{
//				memcpy((uint8_t*)&ElecFenceData+64, (uint8_t*)&g_EFenceInf+64,32);		
//				g_NeedSetEFence.NeedSetEFence2 = 1;
//				g_NeedSetEFence.SendOrderFlag = 1;				
//			}			
//			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
//			{
//				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
//			}
//			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
//			{
//				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
//				{
//					g_LYSendTrainNum = 0;
//					g_RecLYDataType.TrainAppro = 0;
//				}
//				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
//				g_CarWarnFlag.bit.TraDownRoad = 1;
//                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
//			}
//			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
//			{
//				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
//				{
//					g_LYSendUplinkNum = 0;
//					g_RecLYDataType.Uplink = 0;
//				}
//				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
//				if(g_LoraDataSta.Uplink == 1)
//					g_CarWarnFlag.bit.OnRoad = 1;
//				else if(g_LoraDataSta.Uplink == 2)
//					g_CarWarnFlag.bit.DownRoad = 1;					
//				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
//			}
//			if(g_RecLYDataType.ElecFence > 0)
//			{
//				g_RecLYDataType.ElecFence = 0;
//			}
//			break;
//		case LORARET_NODATA_ERR:     /*6 = 无数据*/
//			break;
//		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
//			break;
//		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
//			break;
//		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
//			break;
//	}
	ResRecRes = LORA_Card_RecvBaseFence(&g_LoraPara,&g_LoraFromBaseSta,&g_EFenceInf,RTKDiffTimeOut);                /*接收新电子围栏数据*/
	switch(ResRecRes)
	{
		case LORARET_OK:             /* 0 = 成功   收到数据 */			
			g_CarWarnFlag.all = 0x00;
			WarnLED_Off();
            memcpy(g_EFence.data,g_EFenceInf.EFence,sizeof(g_EFenceInf.EFence));
            g_EFence.total = g_EFenceInf.total;
			g_EFence.num = g_EFenceInf.num;
			WriteEFenceToFLASH();
			ReadEFenceFomFlash();	
			g_P2PModeEnableMark  = 0; 	
			g_CarWarnFlag.all = 0x00;
			WarnLED_Off();
			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
			{
				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
			}
			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
			{
				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
				{
					g_LYSendTrainNum = 0;
					g_RecLYDataType.TrainAppro = 0;
				}
				if(g_StaJudeNum.g_Train < JudgStaNum)
				{
					if(g_CarWarnFlag.bit.TraDownRoad == 0)
						WarnLED_Off();					
					g_CarWarnFlag.bit.TraDownRoad = g_LoraFromBaseSta.TrainingSta;								
				}	
//				if(g_CarWarnFlag.bit.TraDownRoad == 0)
//					WarnLED_Off();				
//				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
//				g_CarWarnFlag.bit.TraDownRoad = 1;
//                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
			}
			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
			{
				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
				{
					g_LYSendUplinkNum = 0;
					g_RecLYDataType.Uplink = 0;
				}
				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
				if(g_LoraDataSta.Uplink == 1)
					g_CarWarnFlag.bit.OnRoad = 1;
				else if(g_LoraDataSta.Uplink == 2)
					g_CarWarnFlag.bit.DownRoad = 1;					
				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
			}
			if(g_RecLYDataType.ElecFence > 0)
			{
				g_P2PModeEnableMark = 0;
				g_RecLYDataType.ElecFence = 0;
				memset(LYElecFenceData,0,sizeof(LYElecFenceData));
				memset((uint8_t*)&g_RecLYEFenceData,0,sizeof(g_RecLYEFenceData));					
			}
            SendYYWarn(DZWLCG);			
			break;
		case LORARET_NODATA_ERR:     /*6 = 无数据*/
			break;
		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
			break;
		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
			break;
		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
			break;
	}
	if(g_P2PModeEnableMark > 0) 
	{
		ResRecRes = LORA_Card_RecvResP2P(&g_LoraPara,&g_LoraFromBaseSta,&g_BaseAskEFPackage);                /*接收索要点对点数据*/
		switch(ResRecRes)
		{
			case LORARET_OK:             /* 0 = 成功   收到数据 */			
				g_CarWarnFlag.all = 0x00;
				WarnLED_Off();		
				g_RecLYDataType.ElecFence = 2;
				if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
				{
					g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
				}
				
				if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
				{
					if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
					{
						g_LYSendTrainNum = 0;
						g_RecLYDataType.TrainAppro = 0;
					}
					if(g_StaJudeNum.g_Train < JudgStaNum)
					{
						if(g_CarWarnFlag.bit.TraDownRoad == 0)
							WarnLED_Off();					
						g_CarWarnFlag.bit.TraDownRoad = g_LoraFromBaseSta.TrainingSta;								
					}
//					if(g_CarWarnFlag.bit.TraDownRoad == 0)
//						WarnLED_Off();					
//					g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
//					g_CarWarnFlag.bit.TraDownRoad = 1;
//					g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
				}
				if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
				{
					if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
					{
						g_LYSendUplinkNum = 0;
						g_RecLYDataType.Uplink = 0;
					}
					g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
					if(g_LoraDataSta.Uplink == 1)
						g_CarWarnFlag.bit.OnRoad = 1;
					else if(g_LoraDataSta.Uplink == 2)
						g_CarWarnFlag.bit.DownRoad = 1;	
					g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
				}		
				break;
			case LORARET_NODATA_ERR:     /*6 = 无数据*/
				break;
			case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
				break;
			case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
				break;
			case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
				break;
		}		
	}	
    ResRecRes = LORA_Card_RecvBaseDiffByBlock(&g_LoraPara,&g_LoraFromBaseSta,g_RTKDiffData,&g_RTKDiffNum,RTKDiffTimeOut); /*接收差分信息*/
	switch(ResRecRes)
	{
		case LORARET_OK:             /* 0 = 成功   收到数据 */
//			g_LoraDelay = uwTick;
			g_P2PModeEnableMark = 0;		
			if(g_RecLora.SucNum < 255)
				g_RecLora.SucNum ++;
			else
				g_RecLora.SucNum = 0;
			#if SWITCHTYPE
			UartSend(&g_Uart1,g_RTKDiffData,g_RTKDiffNum);		        /*发送给RTK*/
				Arr2AscStr((uint8_t*)&g_RecLora,4,0,g_RecLoraStr);
				UartSend(&g_Uart1,(uint8_t*)&g_RecLora,sizeof(g_RecLora));	
			#endif
			if(g_RTKDiffNum > 0)
				UartSend(&g_Uart2,g_RTKDiffData,g_RTKDiffNum);		        /*发送给RTK*/	
			
			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
			{
				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
			}
			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
			{
				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
				{
					g_LYSendTrainNum = 0;
					g_RecLYDataType.TrainAppro = 0;
				}
				if(g_StaJudeNum.g_Train < JudgStaNum)
				{
					if(g_CarWarnFlag.bit.TraDownRoad == 0)
						WarnLED_Off();					
					g_CarWarnFlag.bit.TraDownRoad = g_LoraFromBaseSta.TrainingSta;								
				}				
//				if(g_CarWarnFlag.bit.TraDownRoad == 0)
//					WarnLED_Off();				
//				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
//				g_CarWarnFlag.bit.TraDownRoad = 1;
			}
			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
			{
				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
				{
					g_LYSendUplinkNum = 0;
					g_RecLYDataType.Uplink = 0;
				}
				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
				if(g_LoraDataSta.Uplink == 1)
					g_CarWarnFlag.bit.OnRoad = 1;
				else if(g_LoraDataSta.Uplink == 2)
					g_CarWarnFlag.bit.DownRoad = 1;					
			}
//			if(g_LYSendEFenceNum > 0 || g_RecLYDataType.ElecFence > 0)
//			{
//				g_LYSendEFenceNum = 0;
//				g_RecLYDataType.ElecFence = 0;
//			}					
			break;
		case LORARET_NODATA_ERR:     /*6 = 无数据*/
			break;
		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
			break;
		case LORARET_TIMEOUT_ERR:    /* 10 = 超时 */
			break;
		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
			break;
	}
	ResRecRes = LORA_Card_RecvBaseWarnLEDSta(&g_LoraPara,&g_LoraFromBaseSta,&g_WLEDStaInf);           /*接收广播的指示灯配置数据*/
	switch(ResRecRes)
	{
		case LORARET_OK:             /* 0 = 成功   收到数据 */				
		    if(memcmp((uint8_t*)&g_WLEDStaInf,(uint8_t*)&g_WarnLedSta,sizeof(g_WLEDStaInf)) != 0)            
			{
				g_CarWarnFlag.all = 0x00;
				WarnLED_Off();
				memcpy((uint8_t*)&g_WarnLedSta, (uint8_t*)&g_WLEDStaInf ,sizeof(g_WLEDStaInf));		
				WriteIndicatorStaToFLASH();		
			}		
			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
			{
				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
			}
			
			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
			{
				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
				{
					g_LYSendTrainNum = 0;
					g_RecLYDataType.TrainAppro = 0;
				}
				if(g_StaJudeNum.g_Train < JudgStaNum)
				{
					if(g_CarWarnFlag.bit.TraDownRoad == 0)
						WarnLED_Off();					
					g_CarWarnFlag.bit.TraDownRoad = g_LoraFromBaseSta.TrainingSta;								
				}
//				if(g_CarWarnFlag.bit.TraDownRoad == 0)
//					WarnLED_Off();
//				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
//				g_CarWarnFlag.bit.TraDownRoad = 1;
//                g_StaJudeNum.g_Train = 0; 				/*判断来车后是否下道标记*/
			}
			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
			{
				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
				{
					g_LYSendUplinkNum = 0;
					g_RecLYDataType.Uplink = 0;
				}
				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
				if(g_LoraDataSta.Uplink == 1)
					g_CarWarnFlag.bit.OnRoad = 1;
				else if(g_LoraDataSta.Uplink == 2)
					g_CarWarnFlag.bit.DownRoad = 1;	
				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
			}
			if( g_RecLYDataType.WarnLEDSta > 0)
			{
				g_RecLYDataType.WarnLEDSta = 0;
			}			
			break;
		case LORARET_NODATA_ERR:     /*6 = 无数据*/
			break;
		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
			break;
		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
			break;
		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
			break;
	}
	cardnum = 0;
    ResRecRes = LORA_Base_RecvCardPosition(&g_LoraPara, &g_CardHeader, (CardData_t**)&g_CardData, &cardnum); /*接收其他定位卡定位信息*/
	switch(ResRecRes)
	{
		/* 接收到定位卡数据 保存卡数据 */
		case LORARET_OK: 
			if (cardnum != 0)
			{
				j = 0;
				do
				{					
					RYDW_UpdateCardInfoTable(g_CardData[j], g_LoraSendOkCnt, &g_CardTable);	//更新定位卡信息表
					j++;
				}
				while (j < (cardnum - 1)); 
			}
            break;			
		case LORARET_NODATA_ERR:     /*6 = 无数据*/
			break;
		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
			break;
		case LORARET_TIMEOUT_ERR:    /* 10 = 超时 */
			break;
		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
			break;		
	}
	memset(g_LoraPara.pRxBuf, 0x00, LORA_RX_BUFSIZE);					//重置Lora接收数组
	memset((uint8_t*)&g_LoraFromBaseSta,0,sizeof(g_LoraFromBaseSta));
	/* Lora发送结束重置发送标记 */
	g_LoraPara.TxCpltFlag = 0;
    g_LoraPara.RxCpltFlag = 0;		
	if(JudgeSomeDelayInProOk(g_SendDataToBaseDelay,Delay10Min) == true)
	{
		LoraInit();
		led_on(LED_LORA); 
		g_SendDataToBaseDelay = uwTick;
	}
	
	
}













///******************************************************************
//函数名称: LoraProcess
//功能描述: Lora定位卡广播电子围栏数据
//参数输入: 
//参数输出: 
//返 回 值: LoraResult 返回值
//其    他: 无
//******************************************************************/
//void LoraProcess(void)
//{
//	uint8_t ResRecRes;
//	
//	
//	uint8_t chksum,i;
//	uint16_t rlen,tmpLoraRecvBufSize= 0;
//	
//	
////	/*解析接收Lora数据*/
////	if(LORA_RecvMultiPkg(&g_LoraPara, tmpLoraRecvBuf,&tmpLoraRecvBufSize , RTKDiffTimeOut) == LORARET_OK)
////	{
////		if (g_LoraRecvBuf[0] == LORA_HEADER && g_LoraRecvBuf[1] == LORA_DOWNSTREAM)
////		{
////			rlen = g_LoraRecvBuf[2] + (g_LoraRecvBuf[3] << 8)+ 5;
////			/* 检查校验和 */
////			chksum = CheckSum(rlen-1, g_LoraRecvBuf);
////			if (chksum == g_LoraRecvBuf[rlen-1])
////			{
////				/* 获取基站数据头 */
////				LORA_Arr2Header(g_LoraRecvBuf+ 4, &g_LoraFromBaseSta);	
////				if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
////					g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;	
////				if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
////				{
////					if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
////					{
////						g_LYSendTrainNum = 0;
////						g_RecLYDataType.TrainAppro = 0;
////					}
////					g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
////					g_CarWarnFlag.bit.TraDownRoad = 1;
////					g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
////				}
////				if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
////				{
////					if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
////					{
////						g_LYSendUplinkNum = 0;
////						g_RecLYDataType.Uplink = 0;
////					}
////					g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
////					if(g_LoraDataSta.Uplink == 1)
////						g_CarWarnFlag.bit.OnRoad = 1;
////					else if(g_LoraDataSta.Uplink == 2)
////						g_CarWarnFlag.bit.DownRoad = 1;	
////					g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
////				}
////				g_ResDataFromBaseFlag = 1;
////				g_SendDataToBaseDelay = uwTick;						
////				switch(g_LoraRecvBuf[3])
////				{
////					case LORA_TYPE_P2P:      /* 检查数据类型=应答点对点数据 */
////						g_P2PModeEnableMark  = 1; 				
////						g_RecLYDataType.ElecFence = 2;				
////						break;					
////					case LORA_TYPE_RESEF:    /* 检查数据类型=应答定位卡电子围栏数据 */
////						g_P2PModeEnableMark  = 0; 				
////						g_RecLYDataType.ElecFence = 0;
////						memset(LYElecFenceData,0,sizeof(LYElecFenceData));
////						memset((uint8_t*)&g_RecLYEFenceData,0,sizeof(g_RecLYEFenceData));								
////						break;	
////					case LORA_TYPE_WTIME:    /* 检查数据类型=施工时间数据 */
////						memcpy((uint8_t*)&g_WorkTime,g_LoraRecvBuf+12,sizeof(uint8_t)*13);
////						if(g_P2PModeEnableMark == 1 && g_RecLYDataType.ElecFence == 0)		
////							g_P2PModeEnableMark  = 0;
////						else if(g_RecLYDataType.ElecFence > 0)
////						{
////							g_RecLYDataType.ElecFence = 1;
////							g_P2PModeEnableMark  = 1;
////						}
////						g_CarWarnFlag.all = 0x00;
////						led_off(LED_RGB_R);
////						led_off(LED_RGB_B);
////						WriteWorkTimeToFLASH();			
////						if(g_RecLYDataType.ConstrTime > 0)
////							g_RecLYDataType.ConstrTime = 0;		
////						g_SendDataOrerDelay = TIM6->CNT;
////						break;	
////					case LORA_TYPE_EFENCE:   /* 检查数据类型=围栏数据 */
////                        g_SendDataOrerDelay = TIM6->CNT;
////						g_EFence.total = g_LoraRecvBuf[12];
////						/* 获取电子围栏信息 */
////						for (i=0; i<g_EFence.total; i++)
////						{
////							memcpy(g_EFence.data,g_LoraRecvBuf+13+i*123,sizeof(uint8_t)*123);
////						}
////						g_EFence.num = g_LoraRecvBuf[14+g_EFence.total*123];
////						WriteEFenceToFLASH();
////						ReadEFenceFomFlash();	
////						g_P2PModeEnableMark  = 0; 	
////						g_CarWarnFlag.all = 0x00;
////						led_off(LED_RGB_R);
////						led_off(LED_RGB_B);
////	
////						if(g_RecLYDataType.ElecFence > 0)
////						{
////							g_RecLYDataType.ElecFence = 0;
////							memset(LYElecFenceData,0,sizeof(LYElecFenceData));
////							memset((uint8_t*)&g_RecLYEFenceData,0,sizeof(g_RecLYEFenceData));					
////						}						
////						break;
////					case LORA_TYPE_DIFFINFO: /* 检查数据类型=差分数据 */
////						if(g_P2PModeEnableMark == 1 && g_RecLYDataType.ElecFence == 0)		
////							g_P2PModeEnableMark  = 0;
////						else if(g_RecLYDataType.ElecFence > 0)
////						{
////							g_RecLYDataType.ElecFence = 1;
////							g_P2PModeEnableMark  = 1;				
////						}
////						if(g_LoraRecvBuf[rlen-5] == 0x5A && g_LoraRecvBuf[rlen-4] == 0xA5 && g_LoraRecvBuf[rlen-3] == 0xA5 && g_LoraRecvBuf[rlen-2] == 0x5A)
////						{
////							 memcpy(g_RTKDiffData,g_LoraRecvBuf+12,rlen-13);    /*差分数据拷贝*/
////							 if(g_RTKDiffNum > 0)
////								UartSend(&g_Uart2,g_RTKDiffData,rlen-13);		        /*发送给RTK*/	
////						}
////						g_SendDataOrerDelay = TIM6->CNT;
////						break;						
////				}								
////			}
////		}
////		else if(g_LoraRecvBuf[0] == LORA_HEADER && g_LoraRecvBuf[1] == LORA_UPSTREAM)      /*解析其他定位卡数据*/
////		{
////			rlen = g_LoraRecvBuf[2] + (g_LoraRecvBuf[3] << 8)+ 5;
////			/* 检查校验和 */
////			chksum = CheckSum(rlen-1, g_LoraRecvBuf);
////			if (chksum == g_LoraRecvBuf[rlen-1])
////			{
////				if(g_LoraRecvBuf[4] == LORA_TYPE_POSINFO)     /*定位卡定位信息*/
////				{
////					
////				}
////			
////			}


////		}

////		
////	}
////	/*发送数据*/
////	if(g_ResDataFromBaseFlag == 1 && JudgeSomeDelayInProOk(g_SendDataOrerDelay,Delay500mSec*g_CareIDNum)) /*超时*/
////	{
////			if(g_RecLYDataType.ElecFence == 1)   /*请求点对点申请*/
////			{				 
////				memcpy(g_LoraSendBuf+12, (uint8_t*)&g_CardID ,sizeof(g_CardID)); /*卡定位信息*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;					
////				
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 3;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;	
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////		
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;	
////				if(LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0);
////				}	
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));	
////			}
////			if(g_RecLYDataType.ElecFence == 1)   /*点对点申请*/
////			{
////				/*请求点对点通信*/
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 3;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;	
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;				
////				if(LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0);
////				}	
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));	
////			}			
////			else if(g_RecLYDataType.ElecFence == 2)   /*发送电子围栏数据*/
////			{
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 1;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));						
////				memcpy((uint8_t*)&g_EFenceInf, (uint8_t*)&LYElecFenceData ,sizeof(LYElecFenceData));
////				if(LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,&g_CardLocaData,g_EFenceInf) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,&g_CardLocaData,g_EFenceInf);
////				}

////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
////				memset((uint8_t*)&g_EFenceInf,0,sizeof(g_EFenceInf));		
////							
////			}		
////			else if(g_RecLYDataType.ConstrTime == 1)   /*发送作业时间数据*/
////			{
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 2;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;		
////				memcpy((uint8_t*)&g_WorkTimeInf, (uint8_t*)&g_WorkTime ,sizeof(g_WorkTimeInf));
////				if(LORA_Card_BoardcastWorkTime(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WorkTimeInf) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastWorkTime(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WorkTimeInf);
////				}
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
////				memset((uint8_t*)&g_WorkTimeInf,0,sizeof(g_WorkTimeInf));
////			}
////			else if(g_P2PModeEnableMark == 0)	
////			{

////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 0;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;	
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;				
////				if(LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0);
////				}	
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
////			}		
////		
////	}		
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
//	
////	if(g_ResDataFromBaseFlag == 1 && JudgeSomeDelayInProOk(g_SendDataOrerDelay,Delay500mSec*g_CareIDNum)) /*超时*/
////	{
////			if(g_RecLYDataType.ElecFence == 1)   /*点对点申请*/
////			{
////				/*请求点对点通信*/
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 3;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;	
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;				
////				if(LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0);
////				}	
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));	
////			}			
////			else if(g_RecLYDataType.ElecFence == 2)   /*发送电子围栏数据*/
////			{
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 1;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));						
////				memcpy((uint8_t*)&g_EFenceInf, (uint8_t*)&LYElecFenceData ,sizeof(LYElecFenceData));
////				if(LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,&g_CardLocaData,g_EFenceInf) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastFence(&g_LoraPara,g_CardLora,&g_CardLocaData,g_EFenceInf);
////				}

////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
////				memset((uint8_t*)&g_EFenceInf,0,sizeof(g_EFenceInf));		
////							
////			}		
////			else if(g_RecLYDataType.ConstrTime == 1)   /*发送作业时间数据*/
////			{
////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 2;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;		
////				memcpy((uint8_t*)&g_WorkTimeInf, (uint8_t*)&g_WorkTime ,sizeof(g_WorkTimeInf));
////				if(LORA_Card_BoardcastWorkTime(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WorkTimeInf) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastWorkTime(&g_LoraPara,g_CardLora,&g_CardLocaData,&g_WorkTimeInf);
////				}
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
////				memset((uint8_t*)&g_WorkTimeInf,0,sizeof(g_WorkTimeInf));
////			}
////			else if(g_P2PModeEnableMark == 0)	
////			{

////				g_CardLora.DestOrder = 0;   //目标编号 0=广播
////				g_CardLora.SrcOrder = 0;    //源编号
////				g_CardLora.Type = 0;
////				if(g_CardLora.WorkingSta != 2 && g_RecLYDataType.Uplink == 2)
////					Line_2A_WT588F(QXD);
////				g_CardLora.WorkingSta =g_RecLYDataType.Uplink;   //上下道标记 0=无;1=上道;2=下道		
////				if(g_CardLora.TrainingSta == 0 && g_RecLYDataType.TrainAppro == 1)
////					Line_2A_WT588F(LCXD);
////				g_CardLora.TrainingSta = g_RecLYDataType.TrainAppro;  //来车标记 0=无;1=来车

////				memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&g_CardID ,sizeof(g_CardID));			
////				g_CardLocaData.BatVoltage = g_BatVal; 
////				if(g_CarWarnFlag.all > 0)
////					g_CardLocaData.Alram.bit.Alram= 1;
////				g_CardLocaData.Alram.bit.LowPower= LowPowerWarnFlag & 0x01;
////				g_CardLocaData.Alram.bit.SOS=sos ;
////				if(g_ManualPunFlag == 2)
////				{
////					g_CardLocaData.Alram.bit.Bribe = 0x01;
////					g_ManualPunFlag = 0;
////				}
////				else
////					g_CardLocaData.Alram.bit.Bribe = 0;	
////				g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
////				g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
////				g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
////				g_CardLocaData.PosQuality = g_PosQuality;
////				g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>24)&0xFF;
////				g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>16)&0xFF;
////				g_CardLocaData.Altitude[2] = (g_tGPS.Altitude>>8)&0xFF;				
////				if(LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0) == LORARET_SEND_FAIL)
////				{
////					LORA_Card_BoardcastCardInfo(&g_LoraPara,g_CardLora,&g_CardLocaData,(CardData_t **)&g_OtherCardData[1],0);
////				}	
////				memset((uint8_t*)&g_CardLora,0,sizeof(g_CardLora));
////				memset((uint8_t*)&g_CardLocaData,0,sizeof(g_CardLocaData));
////			}			
////		g_ResDataFromBaseFlag = 0;
////		g_SendDataToBaseDelay = uwTick;
////		g_SendDataOrerDelay = TIM6->CNT;
////	}
////	Ebyte_RF.StartPollTask();		//检查数据	
////	
//////	ResRecRes = LORA_RecvMultiPkg(&g_LoraPara,g_OutBufInf,g_OutBufLen,RTKDiffTimeOut)
//////	switch(ResRecRes)
//////	{
//////		case LORARET_OK:             /* 0 = 成功   收到数据 */
//////			if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
//////			{
//////				rlen = Para->pRxBuf[2] + 4;
//////				/* 检查校验和 */
//////				chksum = CheckSum(rlen-1, Para->pRxBuf);
//////				if (chksum == Para->pRxBuf[rlen-1])
//////				{
//////					/* 检查数据类型=应答点对点数据 */
//////					if (Para->pRxBuf[3] == LORA_TYPE_P2P)
//////					{
//////						/* 获取基站数据头 */
//////						LORA_Arr2Header(Para->pRxBuf + 3, Header);
//////					}
//////				}
//////		
//////			break;
//////		case LORARET_NODATA_ERR:     /*6 = 无数据*/
//////			break;
//////		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
//////			break;
//////		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
//////			break;
//////		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
//////			break;	
//////	}		
////	
////	ResRecRes = LORA_Card_RecvResP2P(&g_LoraPara,&g_LoraFromBaseSta);   /*点对点应答数据*/
////	switch(ResRecRes)
////	{
////		case LORARET_OK:             /* 0 = 成功   收到数据 */		
////            g_P2PModeEnableMark  = 1; 				
////			g_RecLYDataType.ElecFence = 2;
////			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
////			{
////				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
////			}
////			
////			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
////			{
////				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
////				{
////					g_LYSendTrainNum = 0;
////					g_RecLYDataType.TrainAppro = 0;
////				}
////				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
////				g_CarWarnFlag.bit.TraDownRoad = 1;
////                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
////			}
////			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
////			{
////				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
////				{
////					g_LYSendUplinkNum = 0;
////					g_RecLYDataType.Uplink = 0;
////				}
////				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
////				if(g_LoraDataSta.Uplink == 1)
////					g_CarWarnFlag.bit.OnRoad = 1;
////				else if(g_LoraDataSta.Uplink == 2)
////					g_CarWarnFlag.bit.DownRoad = 1;	
////				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
////			}		
////			break;
////		case LORARET_NODATA_ERR:     /*6 = 无数据*/
////			break;
////		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
////			break;
////		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
////			break;
////		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
////			break;	
////	}
////	ResRecRes = LORA_Card_RecvResEFence(&g_LoraPara,&g_LoraFromBaseSta);   /*电子围栏应答数据*/
////	switch(ResRecRes)
////	{
////		case LORARET_OK:             /* 0 = 成功   收到数据 */	
////            g_P2PModeEnableMark  = 0; 				
////			g_RecLYDataType.ElecFence = 0;
////			memset(LYElecFenceData,0,sizeof(LYElecFenceData));
////		    memset((uint8_t*)&g_RecLYEFenceData,0,sizeof(g_RecLYEFenceData));		
////			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
////				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
////			
////			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
////			{
////				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
////				{
////					g_LYSendTrainNum = 0;
////					g_RecLYDataType.TrainAppro = 0;
////				}
////				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
////				g_CarWarnFlag.bit.TraDownRoad = 1;
////                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
////			}
////			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
////			{
////				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
////				{
////					g_LYSendUplinkNum = 0;
////					g_RecLYDataType.Uplink = 0;
////				}
////				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
////				if(g_LoraDataSta.Uplink == 1)
////					g_CarWarnFlag.bit.OnRoad = 1;
////				else if(g_LoraDataSta.Uplink == 2)
////					g_CarWarnFlag.bit.DownRoad = 1;	
////				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
////			}		
////			break;
////		case LORARET_NODATA_ERR:     /*6 = 无数据*/
////			break;
////		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
////			break;
////		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
////			break;
////		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
////			break;	
////	}	
////	ResRecRes = LORA_Card_RecvBaseWorkTime(&g_LoraPara,&g_LoraFromBaseSta,&g_WorkTimeInf);    /*作业时间*/
////	switch(ResRecRes)
////	{
////		case LORARET_OK:             /* 0 = 成功   收到数据 */
////			if(g_P2PModeEnableMark == 1 && g_RecLYDataType.ElecFence == 0)		
////				g_P2PModeEnableMark  = 0;
//// 			else if(g_RecLYDataType.ElecFence > 0)
////			{
////				g_RecLYDataType.ElecFence = 1;
////				g_P2PModeEnableMark  = 1;
////			}
////		    if(memcmp((uint8_t*)&g_WorkTimeInf,(uint8_t*)&g_WorkTime,sizeof(g_WorkTimeInf)) != 0)            /*接收施工时间到数据*/
////			{
////				g_CarWarnFlag.all = 0x00;
////				led_off(LED_RGB_R);
////				led_off(LED_RGB_B);
////				memcpy((uint8_t*)&g_WorkTime, (uint8_t*)&g_WorkTimeInf ,sizeof(g_WorkTimeInf));		
////				WriteWorkTimeToFLASH();		
////			}		
////			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
////			{
////				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
////			}
////			
////			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
////			{
////				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
////				{
////					g_LYSendTrainNum = 0;
////					g_RecLYDataType.TrainAppro = 0;
////				}
////				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
////				g_CarWarnFlag.bit.TraDownRoad = 1;
////                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
////			}
////			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
////			{
////				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
////				{
////					g_LYSendUplinkNum = 0;
////					g_RecLYDataType.Uplink = 0;
////				}
////				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
////				if(g_LoraDataSta.Uplink == 1)
////					g_CarWarnFlag.bit.OnRoad = 1;
////				else if(g_LoraDataSta.Uplink == 2)
////					g_CarWarnFlag.bit.DownRoad = 1;	
////				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
////			}
////			if(g_RecLYDataType.ConstrTime > 0)
////			{
////				g_RecLYDataType.ConstrTime = 0;
////			}			
////			break;
////		case LORARET_NODATA_ERR:     /*6 = 无数据*/
////			break;
////		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
////			break;
////		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
////			break;
////		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
////			break;
////	}	
////	ResRecRes = LORA_Card_RecvBaseFence(&g_LoraPara,&g_LoraFromBaseSta,&g_EFenceInf);                               /*接收电子围栏数据*/
////	switch(ResRecRes)
////	{
////		case LORARET_OK:             /* 0 = 成功   收到数据 */			
////			g_P2PModeEnableMark  = 0; 	
////			g_CarWarnFlag.all = 0x00;
////			led_off(LED_RGB_R);
////			led_off(LED_RGB_B);
////		    memcpy((uint8_t*)&g_EFence, (uint8_t*)&g_EFenceInf ,sizeof(g_EFenceInf));
////		    WriteEFenceToFLASH();
////	        ReadEFenceFomFlash();		
////			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
////			{
////				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
////			}
////			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
////			{
////				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
////				{
////					g_LYSendTrainNum = 0;
////					g_RecLYDataType.TrainAppro = 0;
////				}
////				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
////				g_CarWarnFlag.bit.TraDownRoad = 1;
////                g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
////			}
////			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
////			{
////				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
////				{
////					g_LYSendUplinkNum = 0;
////					g_RecLYDataType.Uplink = 0;
////				}
////				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
////				if(g_LoraDataSta.Uplink == 1)
////					g_CarWarnFlag.bit.OnRoad = 1;
////				else if(g_LoraDataSta.Uplink == 2)
////					g_CarWarnFlag.bit.DownRoad = 1;					
////				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
////			}
////			if(g_RecLYDataType.ElecFence > 0)
////			{
////				g_RecLYDataType.ElecFence = 0;
////				memset(LYElecFenceData,0,sizeof(LYElecFenceData));
////				memset((uint8_t*)&g_RecLYEFenceData,0,sizeof(g_RecLYEFenceData));					
////			}
////			break;
////		case LORARET_NODATA_ERR:     /*6 = 无数据*/
////			break;
////		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
////			break;
////		case LORARET_CHK_ERR:        /* 4 = 校验和错误 */
////			break;
////		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
////			break;
////	}
////    ResRecRes = LORA_Card_RecvBaseDiffByBlock(&g_LoraPara,&g_LoraFromBaseSta,g_RTKDiffData,&g_RTKDiffNum,RTKDiffTimeOut);                           /*接收差分信息*/
////	switch(ResRecRes)
////	{
////		case LORARET_OK:             /* 0 = 成功   收到数据 */
//////			g_LoraDelay = uwTick;
////			if(g_P2PModeEnableMark == 1 && g_RecLYDataType.ElecFence == 0)		
////				g_P2PModeEnableMark  = 0;
//// 			else if(g_RecLYDataType.ElecFence > 0)
////			{
////				g_RecLYDataType.ElecFence = 1;
////                g_P2PModeEnableMark  = 1;				
////			}
////			if(g_RecLora.SucNum < 255)
////				g_RecLora.SucNum ++;
////			else
////				g_RecLora.SucNum = 0;
////			if(g_RTKDiffNum > 0)
////				UartSend(&g_Uart2,g_RTKDiffData,g_RTKDiffNum);		        /*发送给RTK*/		
////			if(g_LoraFromBaseSta.PositionSta > 0)    //收敛标记 0=未收敛;1=正在收敛;2=已收敛
////			{
////				g_LocalAstrngFlag  = g_LoraFromBaseSta.PositionSta;
////			}
////			if(g_LoraFromBaseSta.TrainingSta > 0)    //来车标记 0=无;1=来车
////			{
////				if(g_LYSendTrainNum > 0 || g_RecLYDataType.TrainAppro > 0	)
////				{
////					g_LYSendTrainNum = 0;
////					g_RecLYDataType.TrainAppro = 0;
////				}
////				g_LoraDataSta.TrainAppro = g_LoraFromBaseSta.TrainingSta;
////				g_CarWarnFlag.bit.TraDownRoad = 1;
////			}
////			if(g_LoraFromBaseSta.WorkingSta > 0)    //上下道标记 0=无;1=上道;2=下道
////			{
////				if(g_LYSendUplinkNum > 0 || g_RecLYDataType.Uplink > 0	)
////				{
////					g_LYSendUplinkNum = 0;
////					g_RecLYDataType.Uplink = 0;
////				}
////				g_LoraDataSta.Uplink = g_LoraFromBaseSta.WorkingSta;
////				if(g_LoraDataSta.Uplink == 1)
////					g_CarWarnFlag.bit.OnRoad = 1;
////				else if(g_LoraDataSta.Uplink == 2)
////					g_CarWarnFlag.bit.DownRoad = 1;					
////			}					
////			break;
////		case LORARET_NODATA_ERR:     /*6 = 无数据*/
////			break;
////		case LORARET_TYPE_ERR:       /* 8 = 类型错误 */
////			break;
////		case LORARET_TIMEOUT_ERR:    /* 10 = 超时 */
////			break;
////		case LORARET_KEYWORD_ERR:    /* 7 = 关键字错误 */
////			break;
////	}
//	memset(g_LoraPara.pRxBuf, 0x00, LORA_RX_BUFSIZE);					//重置Lora接收数组
//	memset((uint8_t*)&g_LoraFromBaseSta,0,sizeof(g_LoraFromBaseSta));
//		
//	if(JudgeMilSecondDelayInPro(g_SendDataToBaseDelay,Delay10Min) == true)
//	{
//		LoraInit();
//		led_on(LED_LORA); 
//		g_SendDataToBaseDelay = uwTick;
//	}	
//}


#endif

