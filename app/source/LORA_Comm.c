#if 0
/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: 人员定位
文件名称: LORA_Comm.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: LORA通信
其他说明:
功能列表:
******************************************************************/

#include "bsp.h"
#if LORATYPE               //=0移远E22，=1安信可,
  #include "radio.h"
  #include "sx126x_board.h"
  #include "sx126x_callback.h"
#elif LORATYPE == 2
  #include "ebyte_core.h"
#endif
#include "LORA_Comm.h"
//#include "radio.h"
//#include "sx126x_board.h"
//#include "sx126x_callback.h"

/********* 变量定义 **********/
LoraPara_t		g_LoraPara;

LoraRec_t       g_RecLora;

char g_RecLoraStr[8];
uint8_t g_RecLoraInit;
uint8_t g_LoraPktBuf[LORA_PKT_SIZE];		//LoRa数据包缓存
uint8_t g_LoraSendBuf[LORA_TX_BUFSIZE];		//LoRa发送缓存
uint8_t g_LoraRecvBuf[LORA_RX_BUFSIZE];		//LoRa接收缓存


/************************************** LORA通信函数 ************************************************/
/******************************************************************
函数名称: LoraResult LORA_SendMultiPkg
功能描述: 发送Lora数据(函数内拆包发送)
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: Byte0=总包数 Byte1=包序号 Byte2=当前包字节 Byte3=发送序号(预留) Byte4-Byte254=包数据 (共255字节)
******************************************************************/
LoraResult LORA_SendMultiPkg(LoraPara_t *LoraPara, uint8_t *OutBuf, uint16_t Size)
{
	uint16_t sendlen;
	
	/* 判断发送数据长度 */
	if (Size == 0)
	{
		return LORARET_LEN_ERR;
	}
	
	LoraPara->ucTxBufPos = 0;
	if ((Size % LORA_PKT_SIZE) == 0)
		LoraPara->TotalPktNum = Size / (LORA_PKT_SIZE - 4);
	else
		LoraPara->TotalPktNum = (Size / (LORA_PKT_SIZE - 4)) + 1;
	LoraPara->PktOrder = 0;
	
	/* 发送数据 */
	while (1)
	{
		memset(LoraPara->pPktBuf, 0x00, LORA_PKT_SIZE);
		/* 判断最后一包 */
		if (LoraPara->PktOrder == (LoraPara->TotalPktNum - 1))
		{
			sendlen = Size - LoraPara->ucTxBufPos;
		}
		/* 不是最后一包 */
		else
		{
			sendlen = (LORA_PKT_SIZE - 4);
		}
		
		LoraPara->RetryCnt = 0;
		LoraPara->pPktBuf[0] = LoraPara->TotalPktNum;
		LoraPara->pPktBuf[1] = LoraPara->PktOrder;
		LoraPara->pPktBuf[2] = sendlen;
		LoraPara->pPktBuf[3] = 0x00;					//预留数据序号
		memcpy(LoraPara->pPktBuf + 4, OutBuf + LoraPara->ucTxBufPos, sendlen);	//复制一包数据
		LoraPara->TxCpltFlag = 1;
//		Ebyte_RF.Send(LoraPara->pPktBuf, (uint8e_t)(sendlen + 4), 0);			//发送数据
//		Ebyte_RF.EnterReceiveMode(0);			//切换接收模式
		
		/* 根据发送状态返回值处理 */
		/* 2=发送已完成 */
		if (LoraPara->TxCpltFlag == 2)
		{
//			debug_info(LoraPara->pPktBuf, (sendlen + 4), "->TLORA");
			/* 判断最后一包 */
			if (LoraPara->PktOrder == (LoraPara->TotalPktNum - 1))
			{
				/* 发送结束退出发送 */
				break;
			}
			else
			{
				/* 发送下一包 */
				LoraPara->PktOrder++;
				LoraPara->ucTxBufPos += sendlen;
				LoraPara->TxCpltFlag = 0;
			}
		}
		/* 3=发送超时 */
		else if (LoraPara->TxCpltFlag == 3)
		{
			/* 发送超时 重发当前包 */
			LoraPara->TxCpltFlag = 0;
			LoraPara->RetryCnt++;
			if (LoraPara->RetryCnt > 1)
			{
				/* 重发发送失败 */
				LoraPara->TotalPktNum = 0;
				LoraPara->RetryCnt = 0;
				memset(LoraPara->pPktBuf, 0x00, LORA_PKT_SIZE);
				return LORARET_SEND_FAIL;
			}
		}
		/* 0xFF=其他错误 */
		else if (LoraPara->TxCpltFlag == 0xFF)
		{
			LoraPara->TxCpltFlag = 0;
			memset(LoraPara->pPktBuf, 0x00, LORA_PKT_SIZE);
			return LORARET_SEND_FAIL;
		}
	}
	
	LoraPara->TxCpltFlag = 0;		//全部数据发送完成 发送结束
	return LORARET_OK;
}

/******************************************************************
函数名称: LoraResult LORA_RecvMultiPkg
功能描述: 接收Lora数据(函数内拼包接收)
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: Byte0=总包数 Byte1=包序号 Byte2=当前包字节 Byte3=发送序号(预留) Byte4-Byte254=包数据 (共255字节)
******************************************************************/
LoraResult LORA_RecvMultiPkg(LoraPara_t *LoraPara, uint8_t *InBuf, uint16_t *Size, uint32_t Timeout)
{
	uint16_t rlen;
	uint16_t pos;
	uint16_t dlen;
	uint32_t tmrrecv, tdiff;
	uint8_t order;
	
	tmrrecv = uwTick;
	pos = 0;
	dlen = 0;
	
	/* 阻塞数据接收 */
	while (1)
	{
		/* 有数据接收 */
		if (LoraPara->RxCpltFlag == 2)
		{
			if (LoraPara->pPktBuf[0] != 0x00)
			{
				LoraPara->TotalPktNum = LoraPara->pPktBuf[0];	//获取总包数
				LoraPara->PktOrder = LoraPara->pPktBuf[1];		//获取包序号
				rlen = LoraPara->pPktBuf[2];					//获取当前包字节数
				order = LoraPara->pPktBuf[3];					//获取序号(预留)
				if (rlen > 4)
				{
					/* 获取Payload */
					pos = LoraPara->PktOrder * (LORA_PKT_SIZE - 4);
					memcpy(InBuf + pos, (LoraPara->pPktBuf + 4), rlen);
					dlen = pos + rlen;
					tmrrecv = uwTick;		//重置定时器
				}
				if (LoraPara->PktOrder == (LoraPara->TotalPktNum - 1))
					break;
			}
		}
		
		/* 判断超时 */
		tdiff = (uwTick >= tmrrecv) ? (uwTick - tmrrecv) : (0xFFFFFFFF - tmrrecv + uwTick);
		if (tdiff > Timeout)
		{
			memset(LoraPara->pPktBuf, 0x00, LORA_PKT_SIZE);
			LoraPara->PktOrder = 0;
			LoraPara->TotalPktNum = 0;
			return LORARET_TIMEOUT_ERR;
		}
		
		LoraPara->RxCpltFlag = 0;
//		Ebyte_RF.StartPollTask();						//检查数据
	}
	
	*Size = dlen;
	LoraPara->RxCpltFlag = 0;							//全部数据接收完成 接收结束
	memset(LoraPara->pPktBuf, 0x00, LORA_PKT_SIZE);		//清缓存
	return LORARET_OK;
}

/************************************** LORA数据处理函数 ************************************************/
/******************************************************************
函数名称: LoraResult LORA_SetupLoraBuffer
功能描述: 配置LoRa缓存指针
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_SetupLoraBuffer( LoraPara_t *LoraPara, uint8_t *PktBuf, uint8_t *TxBuf, uint8_t *RxBuf)
{
	/* 初始化缓存指针 */
	LoraPara->pPktBuf = PktBuf;
	LoraPara->pTxBuf = TxBuf;
	LoraPara->pRxBuf = RxBuf;
	
	/* 初始化变量 */
	LoraPara->ucTxBufPos = 0;
	LoraPara->ucRxBufPos = 0;
	LoraPara->TxCpltFlag = 0;
	LoraPara->RxCpltFlag = 0;
	
	LoraPara->TotalPktNum = 0;
	LoraPara->PktOrder = 0;
	LoraPara->RetryCnt = 0;
	
	return LORARET_OK;
}

/******************************************************************
函数名称: LoraResult LORA_InitLoraPara
功能描述: 初始化Lora参数
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_InitLoraPara(LoraPara_t *LoraPara)
{
	/* 初始化变量 */
	LoraPara->ucTxBufPos = 0;
	LoraPara->ucRxBufPos = 0;
	LoraPara->TxCpltFlag = 0;
	LoraPara->RxCpltFlag = 0;
	
	LoraPara->TotalPktNum = 0;
	LoraPara->PktOrder = 0;
	LoraPara->RetryCnt = 0;
	
	return LORARET_OK;
}

/************************************** 基站 数据处理函数 ************************************************/
/******************************************************************
函数名称: LoraResult LORA_Header2Arr
功能描述: 输出Lora数据头到数组
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Header2Arr(LoraHeader_t Header, uint8_t *Arr)
{
	Arr[0] = Header.Type;											//数据类型
	ConvUint16ToArray(Header.DestOrder, Arr + 1, M_LITTLE_ENDIAN);	//目标编号
	ConvUint16ToArray(Header.SrcOrder, Arr + 3, M_LITTLE_ENDIAN);	//源编号
	Arr[5] = Header.PositionSta;									//收敛标记
	Arr[6] = Header.WorkingSta;										//上下道标记
	Arr[7] = Header.TrainingSta;									//来车标记
	
	return LORARET_OK;
}

/******************************************************************
函数名称: LoraResult LORA_Arr2Header
功能描述: 从数组获取Lora数据头
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Arr2Header(uint8_t *Arr, LoraHeader_t *Header)
{
	Header->Type = Arr[0];											//类型
	Header->DestOrder = ConvArrayToUint16(Arr, 1, M_LITTLE_ENDIAN);	//目标编号
	Header->SrcOrder = ConvArrayToUint16(Arr, 3, M_LITTLE_ENDIAN);	//源编号
	Header->PositionSta = Arr[5];									//收敛标记
	Header->WorkingSta = Arr[6];									//上下道标记
	Header->TrainingSta = Arr[7];									//来车标记
	
	return LORARET_OK;
}

/******************************************************************
函数名称: LoraResult LORA_Arr2CardInfo
功能描述: 从数组获取定位卡信息
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Arr2CardInfo(uint8_t *Arr, CardData_t *CardInfo)
{
	memcpy(CardInfo->ID.array, Arr + 0, 7);		//卡ID 7字节
	CardInfo->Latitude = ConvArrayToUint32(Arr, 7, M_LITTLE_ENDIAN);	//卡纬度 单位0.0000001度 4字节
	CardInfo->Longitude = ConvArrayToUint32(Arr, 11, M_LITTLE_ENDIAN);	//卡经度 单位0.0000001度 4字节
	CardInfo->PosQuality = Arr[15];				//定位质量
	memcpy(CardInfo->Altitude, Arr + 16, 3);	//海拔高度 3字节
	CardInfo->Alram.all = Arr[19];				//报警标记
	CardInfo->BatVoltage = Arr[20];				//电池电量
	CardInfo->AlramType.all = Arr[21] + (Arr[22]<<8);
	CardInfo->FenceOrder = Arr[23];											//电子围栏编号
	CardInfo->CTimeOrder = Arr[24];											//施工时间编号	
	
	
	/* 剩余9字节预留 */
	
	return LORARET_OK;
}

/******************************************************************
函数名称: LoraResult LORA_CardInfo2Arr
功能描述: 从定位卡数据输出数组
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_CardInfo2Arr(CardData_t *CardInfo, uint8_t *Arr)
{
	memcpy(Arr + 0, CardInfo->ID.array, 7);
	ConvUint32ToArray(CardInfo->Latitude, Arr + 7, M_LITTLE_ENDIAN);
	ConvUint32ToArray(CardInfo->Longitude, Arr + 11, M_LITTLE_ENDIAN);
	Arr[15] = CardInfo->PosQuality;
	memcpy(Arr + 16, CardInfo->Altitude, 3);
	Arr[19] = CardInfo->Alram.all;
	Arr[20] = CardInfo->BatVoltage;
	Arr[21] = CardInfo->AlramType.all&0xFF;
	Arr[22] = (CardInfo->AlramType.all>>8)&0xFF;	
	Arr[23] = g_EFence.num;
	Arr[24] = g_WorkTime.num;
	Arr[23] = CardInfo->FenceOrder;											//电子围栏编号
	Arr[24] = CardInfo->CTimeOrder;											//施工时间编号
    Arr[25] = CardInfo->CWarnLEDOrder;										//指示灯配置编号   	
	/* 剩余9字节预留 */
	
	return LORARET_OK;
}

///******************************************************************
//函数名称: LoraResult LORA_Base_BoardcastFence
//功能描述: Lora基站广播电子围栏数据
//参数输入: 
//参数输出: 
//返 回 值: LoraResult 返回值
//其    他: 无
//******************************************************************/
//LoraResult LORA_Base_BoardcastFence(LoraPara_t *Para, LoraHeader_t Header, EFence_t FenceInfo)
//{
//	uint16_t slen;
//	uint16_t i, j, pos;
//	uint8_t chksum;
//	
//	/* 生成数据头 */
//	memset(Para->pTxBuf, 0x00, LORA_TX_BUFSIZE);
//	Para->pTxBuf[0] = LORA_HEADER;
//	Para->pTxBuf[1] = LORA_DOWNSTREAM;
//	Para->pTxBuf[2] = 124;						//数据内容长度 数据头(8)+电子围栏数据内容(116)=124
//	
//	/* 数据头转数组 */
//	LORA_Header2Arr(Header, Para->pTxBuf + 3);
//	Para->pTxBuf[3] = LORA_TYPE_EFENCE;
//	
//	/* 电子围栏数据 */
//	pos = 0;
//	for (i=0; i<3; i++)
//	{
//		for (j=0; j<4; j++)
//		{
//			pos = 11 + i * 32 + j * 8;
//			ConvUint32ToArray(FenceInfo.Area[i][j].Latitude, Para->pTxBuf + pos, M_LITTLE_ENDIAN);
//			ConvUint32ToArray(FenceInfo.Area[i][j].Longitude, Para->pTxBuf + (pos + 4), M_LITTLE_ENDIAN); 
//		}
//	}
//	
//	chksum = CheckSum(127, Para->pTxBuf);
//	Para->pTxBuf[127] = chksum;
//	
//	/* 发送数据 */
//	Para->TxCpltFlag = 1;
//	Para->ucTxBufPos = 128;
//	Ebyte_RF.Send(Para->pTxBuf, 128, 0);
//	Ebyte_RF.EnterReceiveMode(0);
//	/* 判断发送是否成功 */
//	if (Para->TxCpltFlag != 2)
//	{
//		return LORARET_SEND_FAIL;
//	}
//	
//	return LORARET_OK;
//}

/******************************************************************
函数名称: LoraResult LORA_Base_BoardcastDiffinfo
功能描述: Lora基站广播差分信息
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Base_BoardcastDiffInfo(LoraPara_t *Para, LoraHeader_t Header, uint8_t *DiffInfo, uint16_t DiffLen)
{
	uint16_t slen;
	uint16_t cplen;
	uint16_t pos;
	
	uint8_t chksum;
	
	/* 生成数据头 */
	memset(Para->pTxBuf, 0x00, LORA_TX_BUFSIZE);
	Para->pTxBuf[0] = LORA_HEADER;
	Para->pTxBuf[1] = LORA_DOWNSTREAM;
	Para->pTxBuf[2] = 251;						//数据内容长度 数据头(8)+数据内容(243)
	
	/* 数据头转数组 */
	LORA_Header2Arr(Header, Para->pTxBuf + 3);
	Para->pTxBuf[3] = LORA_TYPE_DIFFINFO;
	
	/* 生成分包数据 */
	if (DiffLen > 241)
	{
		if ((DiffLen % 241) != 0)
			Para->TotalPktNum = (DiffLen / 241) + 1;
		else
			Para->TotalPktNum = (DiffLen / 241);
	}
	else
		Para->TotalPktNum = 1;
	Para->PktOrder = 0;
	Para->RetryCnt = 0;
	pos = 0;
	cplen = 241;
	
	/* 发送数据 */
	while (1)
	{
		Para->pTxBuf[11] = Para->TotalPktNum;
		Para->pTxBuf[12] = Para->PktOrder;
		/* 是最后一包数据 */
		if (Para->PktOrder == (Para->TotalPktNum - 1))
		{
			cplen = DiffLen - pos;
		}
		else
		{
			cplen = 241;
		}
		memset(Para->pTxBuf + 13, 0x00, 241);
		memcpy(Para->pTxBuf + 13, DiffInfo + pos, cplen);
		chksum = CheckSum(254, Para->pTxBuf);
		Para->pTxBuf[254] = chksum;
		Para->ucTxBufPos = 255;
		Para->TxCpltFlag = 1;
//		Ebyte_RF.Send(Para->pTxBuf, 255, 0);
//		Ebyte_RF.EnterReceiveMode(0);			//切换接收模式
		
		/* 根据发送返回值处理 */
		/* 2=发送成功 */
		if (Para->TxCpltFlag == 2)
		{
			/* 判断最后一包 */
			if (Para->PktOrder == (Para->TotalPktNum - 1))
			{
				/* 所有发送完成 */
				break;
			}
			/* 不是最后一包 */
			Para->RetryCnt = 0;
			Para->PktOrder++;
			pos += cplen;
		}
		/* 其他=发送失败 重发机制 */
		else
		{
			Para->RetryCnt++;
			if (Para->RetryCnt > 1)				//重发一次
			{
				LORA_InitLoraPara(Para);		//发送失败 清除变量
				return LORARET_SEND_FAIL;
			}
		}
	}
	
	return LORARET_OK;
}

///******************************************************************
//函数名称: LoraResult LORA_Base_RecvCardFence
//功能描述: Lora基站接收定位卡围栏信息
//参数输入: 
//参数输出: 
//返 回 值: LoraResult 返回值
//其    他: 无
//******************************************************************/
//LoraResult LORA_Base_RecvCardFence(LoraPara_t *Para, LoraHeader_t *Header, CardData_t *CardInfo, EFence_t *FenceInfo)
//{
//	uint16_t rlen;
//	uint16_t pos, i, j;
//	uint8_t chksum;
//	
//	/* 未接收到数据 */
//	if (Para->RxCpltFlag != 2)
//	{
//		return LORARET_NODATA_ERR;
//	}
//	
//	/* 判断数据头 */
//	rlen = 0;
//	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_UPSTREAM))
//	{
//		rlen = Para->pRxBuf[2] + 4;		//获取到数据长度不包含包头(2)+长度(1)+校验和(1)
//		/* 检查校验和 */
//		chksum = CheckSum(rlen-1, Para->pRxBuf);
//		if (chksum == Para->pRxBuf[rlen-1])
//		{
//			/* 检查数据类型=围栏数据 */
//			if (Para->pRxBuf[3] == LORA_TYPE_EFENCE)
//			{
//				/* 获取定位卡数据头 */
//				LORA_Arr2Header(Para->pRxBuf + 3, Header);
//				
//				/* 获取定位卡数据 一组32字节 */
//				LORA_Arr2CardInfo(Para->pRxBuf + 11, CardInfo);
//				
//				/* 获取电子围栏信息 */
//				for (i=0; i<3; i++)
//				{
//					for (j=0; j<4; j++)
//					{
//						pos = 43 + i * 32 + j * 8;
//						FenceInfo->Area[i][j].Latitude = ConvArrayToUint32(Para->pRxBuf, pos, M_LITTLE_ENDIAN);
//						FenceInfo->Area[i][j].Longitude = ConvArrayToUint32(Para->pRxBuf, pos + 4, M_LITTLE_ENDIAN);
//					}
//				}
//			}
//			else
//			{
//				return LORARET_TYPE_ERR;
//			}
//		}
//		else
//		{
//			return LORARET_CHK_ERR;
//		}
//	}
//	else
//	{
//		return LORARET_KEYWORD_ERR;
//	}
//	
//	return LORARET_OK;
//}

/******************************************************************
函数名称: LoraResult LORA_Base_RecvCardPosition
功能描述: Lora基站接收定位卡定位信息
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Base_RecvCardPosition(LoraPara_t *Para, LoraHeader_t *Header, CardData_t *CardInfo[], uint8_t *CardNum)
{
	uint16_t rlen;
	uint16_t pos, i, j;
	uint8_t chksum;
	
	/* 未接收到数据 */
	if (Para->RxCpltFlag != 2)
	{
		return LORARET_NODATA_ERR;
	}
	
	/* 判断数据头 */
	rlen = 0;
	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_UPSTREAM))
	{
		rlen = Para->pRxBuf[2] + 4;
		/* 检查校验和 */
		chksum = CheckSum(rlen-1, Para->pRxBuf);
		if (chksum == Para->pRxBuf[rlen-1])
		{
			/* 检查数据类型=定位信息 */
			if (Para->pRxBuf[3] == LORA_TYPE_POSINFO)
			{
				/* 获取定位卡数据头 */
				LORA_Arr2Header(Para->pRxBuf + 3, Header);
				/* 获取定位卡数据 一组32字节 */
				i = 0;
				do
				{
					pos = 11 + i * 32;
					LORA_Arr2CardInfo(Para->pRxBuf + (pos + 0), (CardData_t*)&CardInfo[i]);					
					/* 检查是否有其他卡数据 */
					if (i < 3)
					{
						if ((Para->pRxBuf[pos + 36] == 0x00) && (Para->pRxBuf[pos + 37] == 0x00))
						{
							/* 没有其他卡数据 */
							break;
						}
					}
					i++;
				} while (i > 3);
			}
			else
			{
				return LORARET_TYPE_ERR;
			}
		}
		else
		{
			return LORARET_CHK_ERR;
		}
	}
	else
	{
		return LORARET_KEYWORD_ERR;
	}
	
	*CardNum = (i + 1);
	return LORARET_OK;
}

/************************************** 定位卡 数据处理函数 ************************************************/
/******************************************************************
函数名称: LoraResult LORA_Card_BoardcastWarnLEDSta
功能描述: Lora定位卡广播指示灯设置数据
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_BoardcastWarnLEDSta(LoraPara_t *Para, LoraHeader_t Header, CardData_t *SelfCardInfo,WLEDSta_t *WLEDSInfo)
{
	uint8_t chksum;
	
	/* 生成数据头 */
	memset(Para->pTxBuf, 0x00, LORA_TX_BUFSIZE);
	Para->pTxBuf[0] = LORA_HEADER;
	Para->pTxBuf[1] = LORA_UPSTREAM;
	
	Para->pTxBuf[2] = 67;						//数据长度 数据头(8)+本卡数据(32)+其他数据-时间(12)
	
	Header.PositionSta = 0x04;
	LORA_Header2Arr(Header, Para->pTxBuf + 3);
	
	/* 获取定位卡数据 一组32字节 */
	LORA_CardInfo2Arr(SelfCardInfo, Para->pTxBuf + 11);
	
	/* 施工时间数据 */
	memcpy(Para->pTxBuf + 43,WLEDSInfo->start,sizeof(uint8_t) * 12);	
	chksum = CheckSum(70, Para->pTxBuf);
	Para->pTxBuf[70] = chksum;
	
	/* 发送数据 */
	Para->TxCpltFlag = 1;
	Para->ucTxBufPos = 71;
	#if LORATYPE
		Radio.Send(Para->pTxBuf, 71);
		while (Para->TxCpltFlag == 1)
		{
			Radio.IrqProcess();
			HAL_Delay(1);
		}
		/* 判断发送是否成功 */
		if (Para->TxCpltFlag != 2)
		{
			return LORARET_SEND_FAIL;
		}
		Radio.Rx(0);	
	#elif LORATYPE == 2
		Ebyte_RF.Send(Para->pTxBuf, 71, 0);
		Ebyte_RF.EnterReceiveMode(0);
		if (Para->TxCpltFlag != 2)
		{
			return LORARET_SEND_FAIL;
		}		
	#endif

	
	return LORARET_OK;
}
/******************************************************************
函数名称: LoraResult LORA_Card_BoardcastWorkTime
功能描述: Lora定位卡广播施工时间数据
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_BoardcastWorkTime(LoraPara_t *Para, LoraHeader_t Header, CardData_t *SelfCardInfo, WorkTime_t *WTimeInfo)
{
	uint8_t chksum;
	
	/* 生成数据头 */
	memset(Para->pTxBuf, 0x00, LORA_TX_BUFSIZE);
	Para->pTxBuf[0] = LORA_HEADER;
	Para->pTxBuf[1] = LORA_UPSTREAM;
	
	Para->pTxBuf[2] = 52;						//数据长度 数据头(8)+本卡数据(32)+其他数据-时间(12)
	
	Header.PositionSta = 0x02;
	LORA_Header2Arr(Header, Para->pTxBuf + 3);
	
	/* 获取定位卡数据 一组32字节 */
	LORA_CardInfo2Arr(SelfCardInfo, Para->pTxBuf + 11);
	
	/* 施工时间数据 */
	memcpy(Para->pTxBuf + 43,WTimeInfo,sizeof(uint8_t) * 12);	
	chksum = CheckSum(55, Para->pTxBuf);
	Para->pTxBuf[55] = chksum;
	
	/* 发送数据 */
	Para->TxCpltFlag = 1;
	Para->ucTxBufPos = 56;
	#if LORATYPE
		Radio.Send(Para->pTxBuf, 56);
		while (Para->TxCpltFlag == 1)
		{
			Radio.IrqProcess();
			HAL_Delay(1);
		}
		/* 判断发送是否成功 */
		if (Para->TxCpltFlag != 2)
		{
			return LORARET_SEND_FAIL;
		}
		Radio.Rx(0);
	#elif LORATYPE == 2
		Ebyte_RF.Send(Para->pTxBuf, 56, 0);
		Ebyte_RF.EnterReceiveMode(0);
		if (Para->TxCpltFlag != 2)
		{
			return LORARET_SEND_FAIL;
		}		
    #endif		
	return LORARET_OK;	
//	uint8_t chksum;
//	
//	/* 生成数据头 */
//	memset(Para->pTxBuf, 0x00, LORA_TX_BUFSIZE);
//	Para->pTxBuf[0] = LORA_HEADER;
//	Para->pTxBuf[1] = LORA_UPSTREAM;
//	
//	Para->pTxBuf[2] = 52;						//数据长度 数据头(8)+本卡数据(32)+其他数据-时间(12)
//	
//	Header.PositionSta = 0x00;
//	LORA_Header2Arr(Header, Para->pTxBuf + 4);
//	
//	/* 获取定位卡数据 一组32字节 */
//	LORA_CardInfo2Arr(SelfCardInfo, Para->pTxBuf + 12);
//	
//	/* 施工时间数据 */
//	memcpy(Para->pTxBuf + 44,WTimeInfo,sizeof(uint8_t) * 12);	
//	chksum = CheckSum(56, Para->pTxBuf);
//	Para->pTxBuf[56] = chksum;
//	
//	/* 发送数据 */
//	Para->TxCpltFlag = 1;
//	Para->ucTxBufPos = 57;
//	LORA_SendMultiPkg(Para,Para->pTxBuf,57);
////	Ebyte_RF.Send(Para->pTxBuf, 56, 0);
//	Ebyte_RF.EnterReceiveMode(0);
//	if (Para->TxCpltFlag != 2)
//	{
//		return LORARET_SEND_FAIL;
//	}
//	
//	return LORARET_OK;
}
/******************************************************************
函数名称: LoraResult LORA_Card_BoardcastFence
功能描述: Lora定位卡广播电子围栏数据
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_BoardcastFence(LoraPara_t *Para, LoraHeader_t Header,EFence_t FenceInfo,uint8_t PackNum)
{
	uint16_t slen;
	uint16_t i, j, pos;
	uint8_t chksum;
	
	/* 生成数据头 */
	memset(Para->pTxBuf, 0x00, LORA_TX_BUFSIZE);
	Para->pTxBuf[0] = LORA_HEADER;
	Para->pTxBuf[1] = LORA_UPSTREAM;
	
	Para->pTxBuf[2] = 140;						//数据长度 数据头(8)+本卡数据(32)+其他数据-电子围栏(96)
	
	Header.PositionSta = 0x01;
	LORA_Header2Arr(Header, Para->pTxBuf + 3);
	
//	/* 获取定位卡数据 一组32字节 */
//	LORA_CardInfo2Arr(SelfCardInfo, Para->pTxBuf + 11);
	
//	/* 电子围栏数据 */
//	pos = 0;
//	for (i=0; i<3; i++)
//	{
//		for (j=0; j<4; j++)
//		{
//			pos = 43 + i * 32 + j * 8;
////			ConvUint32ToArray(FenceInfo.Area[i][j].Latitude, Para->pTxBuf + pos, M_LITTLE_ENDIAN);
////			ConvUint32ToArray(FenceInfo.Area[i][j].Longitude, Para->pTxBuf + (pos + 4), M_LITTLE_ENDIAN); 
//		}
//	}
    Para->pTxBuf[11] = PackNum;         /*包序号*/
	memcpy(Para->pTxBuf + 12, FenceInfo.EFence[PackNum] , 123);
	chksum = CheckSum(135, Para->pTxBuf);
	Para->pTxBuf[135] = chksum;
	
	/* 发送数据 */
	Para->TxCpltFlag = 1;
	Para->ucTxBufPos = 136;
    #if LORATYPE
		Radio.Send(Para->pTxBuf, 136);
		while (Para->TxCpltFlag == 1)
		{
			Radio.IrqProcess();
			HAL_Delay(1);
		}
		/* 判断发送是否成功 */
		if (Para->TxCpltFlag != 2)
		{
			return LORARET_SEND_FAIL;
		}
		Radio.Rx(0);		
	#elif LORATYPE == 2
		Ebyte_RF.Send(Para->pTxBuf, 136, 0);
		Ebyte_RF.EnterReceiveMode(0);
		if (Para->TxCpltFlag != 2)
		{
			return LORARET_SEND_FAIL;
		}		
    #endif
	return LORARET_OK;	
//	uint16_t slen;
//	uint16_t i, j, pos;
//	uint8_t chksum;
//	
//	/* 生成数据头 */
//	memset(Para->pTxBuf, 0x00, LORA_TX_BUFSIZE);
//	Para->pTxBuf[0] = LORA_HEADER;
//	Para->pTxBuf[1] = LORA_UPSTREAM;
//	Para->pTxBuf[2] = (16+123*g_RecLYEFenceData.Total)&0xFF;						//数据长度 数据头(8)+本卡数据(8)+其他数据-电子围栏(123*g_RecLYEFenceData.Total)
//	Para->pTxBuf[3] = ((16+123*g_RecLYEFenceData.Total)>>8)&0xFF;
//	Header.PositionSta = 0x00;
//	LORA_Header2Arr(Header, Para->pTxBuf + 4);
//	
//	/* 获取定位卡数据 一组32字节 */
//	memcpy(Para->pTxBuf + 12, SelfCardInfo, 7);	
//	Para->pTxBuf[19] = g_RecLYEFenceData.Total;
//	/* 电子围栏数据 */
//	pos = 0;
//	for (i=0; i<g_RecLYEFenceData.Total; i++)
//	{
//		pos = 20 + i*123;
//		memcpy(Para->pTxBuf + pos, FenceInfo.EFence[i] , 123);	
//	}
//	Para->pTxBuf[21 + g_RecLYEFenceData.Total*123] = g_EFence.num; 
//	Para->pTxBuf[22 + g_RecLYEFenceData.Total*123] = g_WorkTime.num;	
//	
//	
//	chksum = CheckSum(23 + g_RecLYEFenceData.Total*123, Para->pTxBuf);
//	Para->pTxBuf[23 + g_RecLYEFenceData.Total*123] = chksum;
//	
//	/* 发送数据 */
//	Para->TxCpltFlag = 1;
//	Para->ucTxBufPos = 24 + g_RecLYEFenceData.Total*123;
//	LORA_SendMultiPkg(Para,Para->pTxBuf,24 + g_RecLYEFenceData.Total*123);
////	Ebyte_RF.Send(Para->pTxBuf, 23 + g_RecLYEFenceData.Total*123, 0);
//	Ebyte_RF.EnterReceiveMode(0);
//	if (Para->TxCpltFlag != 2)
//	{
//		return LORARET_SEND_FAIL;
//	}
//	
//	return LORARET_OK;
}

/******************************************************************
函数名称: LoraResult LORA_Card_BoardcastCardInfo
功能描述: Lora定位卡广播卡定位信息
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_BoardcastCardInfo(LoraPara_t *Para, LoraHeader_t Header, CardData_t *SelfCardInfo, CardData_t *OtherCardInfo[], uint8_t CardNum)
{
	uint16_t slen;
	uint16_t cplen;
	uint16_t pos;
	uint8_t chksum;
	uint16_t i;
	
	/* 生成数据头 */
	memset(Para->pTxBuf, 0x00, LORA_TX_BUFSIZE);
	Para->pTxBuf[0] = LORA_HEADER;
	Para->pTxBuf[1] = LORA_UPSTREAM;
	Para->pTxBuf[2] = 40;
	
	Header.PositionSta = 0x00;
	LORA_Header2Arr(Header, Para->pTxBuf + 3);
	
	/* 获取自身定位卡数据 一组32字节 */
	LORA_CardInfo2Arr(SelfCardInfo, Para->pTxBuf + 11);
	/* 携带其他定位卡数据 一组32字节 */
	/* 有其他定位卡数据 */
//	if (CardNum != 0)
//	{
//		CardNum = (CardNum <= 3) ? CardNum : 3;
//		for (i=0; i<CardNum; i++)
//		{
//			pos = 43 + i * 32;
//			LORA_CardInfo2Arr(OtherCardInfo[i], Para->pTxBuf + pos);
//		}
//	}
	
	chksum = CheckSum(43, Para->pTxBuf);
	Para->pTxBuf[43] = chksum;
	
	/* 发送数据 */
	Para->TxCpltFlag = 1;
	Para->ucTxBufPos = 44;
	#if LORATYPE
		Radio.Send(Para->pTxBuf, 44);
		while (Para->TxCpltFlag == 1)
		{
			Radio.IrqProcess();
			HAL_Delay(1);
		}
		/* 判断发送是否成功 */
		if (Para->TxCpltFlag != 2)
		{
			return LORARET_SEND_FAIL;
		}
		Radio.Rx(0);	
	#elif LORATYPE == 2
		Ebyte_RF.Send(Para->pTxBuf, 44, 0);
		Ebyte_RF.EnterReceiveMode(0);
		if (Para->TxCpltFlag != 2)
		{
			return LORARET_SEND_FAIL;
		}		
	#endif

		
	return LORARET_OK;	
//	uint16_t slen;
//	uint16_t cplen;
//	uint16_t pos;
//	uint8_t chksum;
//	uint16_t i;
//	
//	/* 生成数据头 */
//	memset(Para->pTxBuf, 0x00, LORA_TX_BUFSIZE);
//	Para->pTxBuf[0] = LORA_HEADER;
//	Para->pTxBuf[1] = LORA_UPSTREAM;
//	Para->pTxBuf[2] = 136;
//	Para->pTxBuf[3] = 0;	
//	Header.PositionSta = 0x00;
//	LORA_Header2Arr(Header, Para->pTxBuf + 4);
//	
//	/* 获取自身定位卡数据 一组32字节 */
//	LORA_CardInfo2Arr(SelfCardInfo, Para->pTxBuf + 12);
//	/* 携带其他定位卡数据 一组32字节 */
//	/* 有其他定位卡数据 */
//	if (CardNum != 0)
//	{
//		CardNum = (CardNum <= 3) ? CardNum : 3;
//		for (i=0; i<CardNum; i++)
//		{
//			pos = 44 + i * 32;
//			LORA_CardInfo2Arr(OtherCardInfo[i], Para->pTxBuf + pos);
//		}
//	}
//	
//	chksum = CheckSum(140, Para->pTxBuf);
//	Para->pTxBuf[140] = chksum;
//	
//	/* 发送数据 */
//	Para->TxCpltFlag = 1;
//	Para->ucTxBufPos = 141;
//	LORA_SendMultiPkg(Para,Para->pTxBuf,141);
////	Ebyte_RF.Send(Para->pTxBuf, 140, 0);
//	Ebyte_RF.EnterReceiveMode(0);
//	if (Para->TxCpltFlag != 2)
//	{
//		return LORARET_SEND_FAIL;
//	}	
//	return LORARET_OK;
}

/******************************************************************
函数名称: LoraResult LORA_Card_BoardcastP2PInfo
功能描述: Lora定位卡广播申请点对点信息
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_BoardcastP2PInfo(LoraPara_t *Para, LoraHeader_t Header, CardData_t *SelfCardInfo,uint8_t Packages)
{
	uint16_t slen;
	uint16_t cplen;
	uint16_t pos;
	uint8_t chksum;
	uint16_t i;
	
	/* 生成数据头 */
	memset(Para->pTxBuf, 0x00, LORA_TX_BUFSIZE);
	Para->pTxBuf[0] = LORA_HEADER;
	Para->pTxBuf[1] = LORA_UPSTREAM;
	Para->pTxBuf[2] = 41;
	
	Header.PositionSta = 0x00;
	LORA_Header2Arr(Header, Para->pTxBuf + 3);
	
	/* 获取自身定位卡数据 一组32字节 */
	LORA_CardInfo2Arr(SelfCardInfo, Para->pTxBuf + 11);
    Para->pTxBuf[43] = Packages;    /*包数*/
	chksum = CheckSum(44, Para->pTxBuf);
	Para->pTxBuf[44] = chksum;
	
	/* 发送数据 */
	Para->TxCpltFlag = 1;
	Para->ucTxBufPos = 45;
	
    #if LORATYPE
		Radio.Send(Para->pTxBuf, 45);
		while (Para->TxCpltFlag == 1)
		{
			Radio.IrqProcess();
			HAL_Delay(1);
		}
		/* 判断发送是否成功 */
		if (Para->TxCpltFlag != 2)
		{
			return LORARET_SEND_FAIL;
		}
		Radio.Rx(0);	
	#elif LORATYPE == 2
		Ebyte_RF.Send(Para->pTxBuf, 45, 0);
		Ebyte_RF.EnterReceiveMode(0);
		if (Para->TxCpltFlag != 2)
		{
			return LORARET_SEND_FAIL;
		}
	#endif
	return LORARET_OK;	
}
///******************************************************************
//函数名称: LoraResult LORA_Card_RecvResP2P(LoraPara_t *Para, LoraHeader_t *Header)
//功能描述: Lora定位卡接收基站点对点应答
//参数输入: 
//参数输出: 
//返 回 值: LoraResult 返回值
//其    他: 无
//******************************************************************/
//LoraResult LORA_Card_RecvResP2P(LoraPara_t *Para, LoraHeader_t *Header)
//{
//	uint16_t rlen;
//	uint8_t chksum;
//	
//	/* 未接收到数据 */
//	if (Para->RxCpltFlag != 2)
//	{
//		return LORARET_NODATA_ERR;
//	}
//	
//	/* 判断数据头 */
//	rlen = 0;
//	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
//	{
//		rlen = Para->pRxBuf[2] + 4;
//		/* 检查校验和 */
//		chksum = CheckSum(rlen-1, Para->pRxBuf);
//		if (chksum == Para->pRxBuf[rlen-1])
//		{
//			/* 检查数据类型=应答点对点数据 */
//			if (Para->pRxBuf[3] == LORA_TYPE_P2P)
//			{
//				/* 获取基站数据头 */
//				LORA_Arr2Header(Para->pRxBuf + 3, Header);
//			}
//			else
//			{
//				return LORARET_TYPE_ERR;
//			}
//		}
//		else
//		{
//			return LORARET_CHK_ERR;
//		}
//	}
//	else
//	{
//		return LORARET_KEYWORD_ERR;
//	}
//	g_ResDataFromBaseFlag = 1;
//	g_SendDataOrerDelay = TIM6->CNT;
//	g_SendDataToBaseDelay = uwTick;
//	return LORARET_OK;
//}

/******************************************************************
函数名称: LoraResult LORA_Card_RecvResP2P(LoraPara_t *Para, LoraHeader_t *Header)
功能描述: Lora定位卡接收基站点对点应答
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_RecvResP2P(LoraPara_t *Para, LoraHeader_t *Header,uint8_t *PackNum)
{
	uint16_t rlen;
	uint8_t chksum;
	
	/* 未接收到数据 */
	if (Para->RxCpltFlag != 2)
	{
		return LORARET_NODATA_ERR;
	}
	
	/* 判断数据头 */
	rlen = 0;
	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
	{
		rlen = Para->pRxBuf[2] + 4;
		/* 检查校验和 */
		chksum = CheckSum(rlen-1, Para->pRxBuf);
		if (chksum == Para->pRxBuf[rlen-1])
		{
			/* 检查数据类型=应答点对点数据 */
			if (Para->pRxBuf[3] == LORA_TYPE_P2P)
			{
				/* 获取基站数据头 */
				LORA_Arr2Header(Para->pRxBuf + 3, Header);
				/*申请电子围栏序号*/
				*PackNum = Para->pRxBuf[11];
			}
			else
			{
				return LORARET_TYPE_ERR;
			}
		}
		else
		{
			return LORARET_CHK_ERR;
		}
	}
	else
	{
		return LORARET_KEYWORD_ERR;
	}
	g_ResDataFromBaseFlag = 1;
//	g_SendDataOrerDelay = TIM6->CNT;
	g_SendDataToBaseDelay = uwTick;
	return LORARET_OK;
}
/******************************************************************
函数名称: LoraResult LORA_Card_RecvResEFence(LoraPara_t *Para, LoraHeader_t *Header)
功能描述: Lora定位卡接收基站点对点应答
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_RecvResEFence(LoraPara_t *Para, LoraHeader_t *Header)
{
//	uint16_t rlen;
//	uint8_t chksum;
//	
//	/* 未接收到数据 */
//	if (Para->RxCpltFlag != 2)
//	{
//		return LORARET_NODATA_ERR;
//	}
//	
//	/* 判断数据头 */
//	rlen = 0;
//	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
//	{
//		rlen = Para->pRxBuf[2] + 4;
//		/* 检查校验和 */
//		chksum = CheckSum(rlen-1, Para->pRxBuf);
//		if (chksum == Para->pRxBuf[rlen-1])
//		{
//			/* 检查数据类型=应答定位卡电子围栏数据 */
//			if (Para->pRxBuf[3] == LORA_TYPE_RESEF)
//			{
//				/* 获取基站数据头 */
//				LORA_Arr2Header(Para->pRxBuf + 3, Header);
//			}
//			else
//			{
//				return LORARET_TYPE_ERR;
//			}
//		}
//		else
//		{
//			return LORARET_CHK_ERR;
//		}
//	}
//	else
//	{
//		return LORARET_KEYWORD_ERR;
//	}
//	g_ResDataFromBaseFlag = 1;
//	g_SendDataOrerDelay = TIM6->CNT;
//	g_SendDataToBaseDelay = uwTick;
//	return LORARET_OK;
}
/******************************************************************
函数名称: LoraResult LORA_Card_RecvBaseWorkTime
功能描述: Lora定位卡接收作业时间信息
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_RecvBaseWorkTime(LoraPara_t *Para, LoraHeader_t *Header,WorkTime_t *WTimeInfo)
{
	uint16_t rlen;
	uint8_t chksum;
	
	/* 未接收到数据 */
	if (Para->RxCpltFlag != 2)
	{
		return LORARET_NODATA_ERR;
	}
	
	/* 判断数据头 */
	rlen = 0;
	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
	{
		rlen = Para->pRxBuf[2] + 4;
		/* 检查校验和 */
		chksum = CheckSum(rlen-1, Para->pRxBuf);
		if (chksum == Para->pRxBuf[rlen-1])
		{
			/* 检查数据类型=施工时间数据 */
			if (Para->pRxBuf[3] == LORA_TYPE_WTIME)
			{
				/* 获取基站数据头 */
				LORA_Arr2Header(Para->pRxBuf + 3, Header);
				
				/* 获取施工时间信息 */
				memcpy(WTimeInfo,Para->pRxBuf+11,sizeof(uint8_t)*13);
				
			}
			else
			{
				return LORARET_TYPE_ERR;
			}
		}
		else
		{
			return LORARET_CHK_ERR;
		}
	}
	else
	{
		return LORARET_KEYWORD_ERR;
	}
	g_ResDataFromBaseFlag = 1;
	g_SendDataOrerDelay = TIM6->CNT;
	g_SendDataToBaseDelay = uwTick;
	return LORARET_OK;
}
/******************************************************************
函数名称: LoraResult LORA_Card_RecvBaseWarnLEDSta
功能描述: Lora定位卡接收指示灯配置信息
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_RecvBaseWarnLEDSta(LoraPara_t *Para, LoraHeader_t *Header,WLEDSta_t *WLEDSInfo)
{
	uint16_t rlen;
	uint8_t chksum;
	
	/* 未接收到数据 */
	if (Para->RxCpltFlag != 2)
	{
		return LORARET_NODATA_ERR;
	}
	
	/* 判断数据头 */
	rlen = 0;
	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
	{
		rlen = Para->pRxBuf[2] + 4;
		/* 检查校验和 */
		chksum = CheckSum(rlen-1, Para->pRxBuf);
		if (chksum == Para->pRxBuf[rlen-1])
		{
			/* 检查数据类型=施工时间数据 */
			if (Para->pRxBuf[3] == LORA_TYPE_LED)
			{
				/* 获取基站数据头 */
				LORA_Arr2Header(Para->pRxBuf + 3, Header);
				
				/* 获取施工时间信息 */
				memcpy(WLEDSInfo->start,Para->pRxBuf+11,sizeof(uint8_t)*12);
				WLEDSInfo->num = Para->pRxBuf[38];
			}
			else
			{
				return LORARET_TYPE_ERR;
			}
		}
		else
		{
			return LORARET_CHK_ERR;
		}
	}
	else
	{
		return LORARET_KEYWORD_ERR;
	}
	g_ResDataFromBaseFlag = 1;
	g_SendDataOrerDelay = TIM6->CNT;
	g_SendDataToBaseDelay = uwTick;
	return LORARET_OK;	
//	uint16_t rlen;
//	uint8_t chksum;
//	
//	/* 未接收到数据 */
//	if (Para->RxCpltFlag != 2)
//	{
//		return LORARET_NODATA_ERR;
//	}
//	
//	/* 判断数据头 */
//	rlen = 0;
//	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
//	{
//		rlen = Para->pRxBuf[2] + 4;
//		/* 检查校验和 */
//		chksum = CheckSum(rlen-1, Para->pRxBuf);
//		if (chksum == Para->pRxBuf[rlen-1])
//		{
//			/* 检查数据类型=施工时间数据 */
//			if (Para->pRxBuf[3] == LORA_TYPE_WTIME)
//			{
//				/* 获取基站数据头 */
//				LORA_Arr2Header(Para->pRxBuf + 3, Header);
//				
//				/* 获取施工时间信息 */
//				memcpy(WTimeInfo,Para->pRxBuf+11,sizeof(uint8_t)*13);
//				
//			}
//			else
//			{
//				return LORARET_TYPE_ERR;
//			}
//		}
//		else
//		{
//			return LORARET_CHK_ERR;
//		}
//	}
//	else
//	{
//		return LORARET_KEYWORD_ERR;
//	}
//	g_ResDataFromBaseFlag = 1;
//	g_SendDataOrerDelay = TIM6->CNT;
//	g_SendDataToBaseDelay = uwTick;
//	return LORARET_OK;
}
/******************************************************************
函数名称: LoraResult LORA_Card_RecvBaseFence
功能描述: Lora定位卡接收基站围栏信息
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_RecvBaseFence(LoraPara_t *Para, LoraHeader_t *Header, EFence_t *FenceInfo, uint32_t Timeout)
{
	uint16_t rlen;
	uint16_t pos, i, j;
	uint8_t chksum;
	uint32_t timer;	
	uint32_t tdiff;	
	
	/* 未接收到数据 */
	if (Para->RxCpltFlag != 2)
	{
		return LORARET_NODATA_ERR;
	}
	
	timer = uwTick;		//重置定时器
	
//	/* 未接收到数据 */
//	if (Para->RxCpltFlag != 2)
//	{
//		return LORARET_NODATA_ERR;
//	}
	while (1)
	{
		/* 接收到数据 */
		if (Para->RxCpltFlag == 2)
		{
			/* 判断数据头 */
			rlen = 0;
			if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
			{
				rlen = Para->pRxBuf[2] + 4;
				/* 检查校验和 */
				chksum = CheckSum(rlen-1, Para->pRxBuf);
				if (chksum == Para->pRxBuf[rlen-1])
				{
					/* 检查数据类型=电子围栏数据 */
					if (Para->pRxBuf[3] == LORA_TYPE_EFENCE)
					{
						timer = uwTick;		//重置定时
						
						/* 获取基站数据头 */
						LORA_Arr2Header(Para->pRxBuf + 3, Header);
												
						/* 获取差分数据信息 */
						Para->TotalPktNum = Para->pRxBuf[11];
						Para->PktOrder = Para->pRxBuf[12];
						memcpy(FenceInfo->EFence[Para->PktOrder], Para->pRxBuf + 13, 123);
						FenceInfo->total = Para->TotalPktNum;        /*总包数*/
						FenceInfo->num = Para->pRxBuf[136];          /*电子围栏*/						
						/* 最后一包数据 接收完成 */
						if (Para->PktOrder == (Para->TotalPktNum - 1))
							break;
					}
					else
					{
						return LORARET_TYPE_ERR;
					}
				}
				else
				{
					return LORARET_CHK_ERR;
				}
			}
			else
			{
				return LORARET_KEYWORD_ERR;
			}			
		}
		
		/* 判断超时 */
		tdiff = (uwTick >= timer) ? (uwTick - timer) : (0xFFFFFFFF - timer + uwTick);
		if (tdiff > Timeout)
		{
			Para->TotalPktNum = 0;
			Para->PktOrder = 0;
			memset(Para->pRxBuf, 0x00, LORA_RX_BUFSIZE);
			memset(tmpLoraRecvBuf, 0x00, 2048);			
			Para->ucRxBufPos = 0;
			Para->RxCpltFlag = 0;
			return LORARET_TIMEOUT_ERR;
		}
		Para->RxCpltFlag = 0;
		#if LORATYPE 	
			Radio.IrqProcess();
		#elif LORATYPE == 2
			Ebyte_RF.StartPollTask();		//检查数据
		#endif
	}

	g_ResDataFromBaseFlag = 1;
	g_SendDataOrerDelay = TIM6->CNT;
	g_SendDataToBaseDelay = uwTick;
    return LORARET_OK;	
	/* 判断数据头 */
//	rlen = 0;
//	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
//	{
//		rlen = Para->pRxBuf[2] + 4;
//		/* 检查校验和 */
//		chksum = CheckSum(rlen-1, Para->pRxBuf);
//		if (chksum == Para->pRxBuf[rlen-1])
//		{
//			/* 检查数据类型=围栏数据 */
//			if (Para->pRxBuf[3] == LORA_TYPE_EFENCE)
//			{
//				/* 获取基站数据头 */
//				LORA_Arr2Header(Para->pRxBuf + 3, Header);
//				
//				/* 获取电子围栏信息 */
//				for (i=0; i<3; i++)
//				{
//					for (j=0; j<4; j++)
//					{
//						pos = 11 + i * 32 + j * 8;
////						FenceInfo->Area[i][j].Latitude = ConvArrayToUint32(Para->pRxBuf, pos, M_LITTLE_ENDIAN);
////						FenceInfo->Area[i][j].Longitude = ConvArrayToUint32(Para->pRxBuf, pos + 4, M_LITTLE_ENDIAN);
//					}
//				}
//				FenceInfo->num = Para->pRxBuf[107];
//			}
//			else
//			{
//				return LORARET_TYPE_ERR;
//			}
//		}
//		else
//		{
//			return LORARET_CHK_ERR;
//		}
//	}
//	else
//	{
//		return LORARET_KEYWORD_ERR;
//	}
//	g_ResDataFromBaseFlag = 1;
//	g_SendDataOrerDelay = TIM6->CNT;
//	g_SendDataToBaseDelay = uwTick;
//	return LORARET_OK;
//	uint16_t rlen;
//	uint16_t pos, i, j;
//	uint8_t chksum;
//	
//	/* 未接收到数据 */
//	if (Para->RxCpltFlag != 2)
//	{
//		return LORARET_NODATA_ERR;
//	}
//	
//	/* 判断数据头 */
//	rlen = 0;
//	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
//	{
//		rlen = Para->pRxBuf[2] + 4;
//		/* 检查校验和 */
//		chksum = CheckSum(rlen-1, Para->pRxBuf);
//		if (chksum == Para->pRxBuf[rlen-1])
//		{
//			/* 检查数据类型=围栏数据 */
//			if (Para->pRxBuf[3] == LORA_TYPE_EFENCE)
//			{
//				/* 获取基站数据头 */
//				LORA_Arr2Header(Para->pRxBuf + 3, Header);				
//				FenceInfo->total = Para->pRxBuf[11];
//				/* 获取电子围栏信息 */
//				for (i=0; i<FenceInfo->total; i++)
//				{
//					memcpy(FenceInfo,Para->pRxBuf+12+i*123,sizeof(uint8_t)*123);
//				}
//				FenceInfo->num = Para->pRxBuf[13+FenceInfo->total*123];
//			}
//			else
//			{
//				return LORARET_TYPE_ERR;
//			}
//		}
//		else
//		{
//			return LORARET_CHK_ERR;
//		}
//	}
//	else
//	{
//		return LORARET_KEYWORD_ERR;
//	}
//	g_ResDataFromBaseFlag = 1;
//	g_SendDataOrerDelay = TIM6->CNT;
//	g_SendDataToBaseDelay = uwTick;
//	return LORARET_OK;
}

/******************************************************************
函数名称: LoraResult LORA_Card_RecvBaseDiffInfo
功能描述: Lora定位卡接收基站差分信息
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_RecvBaseDiffInfo(LoraPara_t *Para, LoraHeader_t *Header, uint8_t *DiffInfo, uint16_t *DiffLen)
{
	uint16_t rlen;
//	uint16_t dlen;
	uint8_t chksum;
	
	/* 未接收到数据 */
	if (Para->RxCpltFlag != 2)
	{
		return LORARET_NODATA_ERR;
	}
	
	/* 判断数据头 */
	rlen = 0;
	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
	{
		rlen = Para->pRxBuf[2] + 4;
		/* 检查校验和 */
		chksum = CheckSum(rlen-1, Para->pRxBuf);
		if (chksum == Para->pRxBuf[rlen-1])
		{
			/* 检查数据类型=差分数据 */
			if (Para->pRxBuf[3] == LORA_TYPE_DIFFINFO)
			{
				/* 获取基站数据头 */
				LORA_Arr2Header(Para->pRxBuf + 3, Header);
				
				/* 获取差分数据 */
				Para->TotalPktNum = Para->pRxBuf[11];
				Para->PktOrder = Para->pRxBuf[12];
				/* 判断是最后一包数据 */
				if (Para->TotalPktNum == (Para->TotalPktNum - 1))
				{
					memcpy(DiffInfo, Para->pRxBuf + 13, 241);
					*DiffLen = 241;
					return LORARET_OK;
				}
				else
				{
					memcpy(DiffInfo, Para->pRxBuf + 13, 241);
					*DiffLen = 241;
					return LORARET_INCOMPLETE_ERR;
				}
			}
			else
			{
				return LORARET_TYPE_ERR;
			}
		}
		else
		{
			return LORARET_CHK_ERR;
		}
	}
	else
	{
		return LORARET_KEYWORD_ERR;
	}
	
//	return LORARET_OK;	
//	uint16_t rlen;
//	uint16_t dlen;
//	uint8_t chksum;
//	
//	/* 未接收到数据 */
//	if (Para->RxCpltFlag != 2)
//	{
//		return LORARET_NODATA_ERR;
//	}
//	
//	/* 判断数据头 */
//	rlen = 0;
//	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
//	{
//		rlen = Para->pRxBuf[2] + 4;
//		/* 检查校验和 */
//		chksum = CheckSum(rlen-1, Para->pRxBuf);
//		if (chksum == Para->pRxBuf[rlen-1])
//		{
//			/* 检查数据类型=差分数据 */
//			if (Para->pRxBuf[3] == LORA_TYPE_DIFFINFO)
//			{
//				/* 获取基站数据头 */
//				LORA_Arr2Header(Para->pRxBuf + 3, Header);
//				
//				/* 获取差分数据 */
//				Para->TotalPktNum = Para->pRxBuf[11];
//				Para->PktOrder = Para->pRxBuf[12];
//				/* 判断是最后一包数据 */
//				if (Para->TotalPktNum == (Para->TotalPktNum - 1))
//				{
//					memcpy(DiffInfo, Para->pRxBuf + 13, 241);
//					*DiffLen = 241;
//					return LORARET_OK;
//				}
//				else
//				{
//					memcpy(DiffInfo, Para->pRxBuf + 13, 241);
//					*DiffLen = 241;
//					return LORARET_INCOMPLETE_ERR;
//				}
//			}
//			else
//			{
//				return LORARET_TYPE_ERR;
//			}
//		}
//		else
//		{
//			return LORARET_CHK_ERR;
//		}
//	}
//	else
//	{
//		return LORARET_KEYWORD_ERR;
//	}
//	
//	return LORARET_OK;
}
uint8_t tmpLoraRecvBuf[2048];
/******************************************************************
函数名称: LoraResult LORA_Card_RecvBaseDiffByBlock
功能描述: Lora定位卡接收基站差分信息(阻塞)
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_RecvBaseDiffByBlock(LoraPara_t *Para, LoraHeader_t *Header, uint8_t *DiffInfo, uint16_t *DiffLen, uint32_t Timeout)
{
	uint8_t rlen;
	uint16_t pos;
	uint8_t chksum;
	uint32_t timer;
	
	uint32_t tdiff;	
	uint16_t i;

	uint16_t dlen;
	uint16_t datlen;
	uint16_t datpos;
	uint16_t dbufpos;
	
	/* 未接收到数据 */
	if (Para->RxCpltFlag != 2)
	{
		return LORARET_NODATA_ERR;
	}
	
	timer = uwTick;		//重置定时器
	dlen = 0;
	/* 差分定位数据有没有收全 */
	if((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
	{
		if(g_RecLora.Total < 255)
			g_RecLora.Total ++;
		else
			g_RecLora.Total = 0;
	}
	/* 接收定位数据 */
	while (1)
	{
		/* 接收到数据 */
		if (Para->RxCpltFlag == 2)
		{
			/* 判断数据头 */
			rlen = 0;
			if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
			{
				rlen = Para->pRxBuf[2] + 4;
				/* 检查校验和 */
				chksum = CheckSum(rlen-1, Para->pRxBuf);
				if (chksum == Para->pRxBuf[rlen-1])
				{
					/* 检查数据类型=差分数据 */
					if (Para->pRxBuf[3] == LORA_TYPE_DIFFINFO)
					{
						timer = uwTick;		//重置定时
						
						/* 获取基站数据头 */
						LORA_Arr2Header(Para->pRxBuf + 3, Header);
						
						if(Para->pRxBuf[6] <= 255 )						
							g_RecLora.BaseTotal = Para->pRxBuf[6];         /*基站lora发送数据总数*/
						if(Para->pRxBuf[7] <=  255)						
							g_RecLora.BaseSucNum = Para->pRxBuf[7];         /*基站lora发送的成功数据条数*/						
						if(g_RecLoraInit == 1)
						{
							g_RecLora.Total = g_RecLora.BaseTotal;
							g_RecLora.SucNum = g_RecLora.BaseSucNum;
							g_RecLoraInit = 0;
						}
						
						/* 获取差分数据信息 */
						Para->TotalPktNum = Para->pRxBuf[11];
						Para->PktOrder = Para->pRxBuf[12];
						pos = Para->PktOrder * 241;
						memcpy(tmpLoraRecvBuf + pos, Para->pRxBuf + 13, 241);
						dlen += 241;
						
						/* 最后一包数据 接收完成 */
						if (Para->PktOrder == (Para->TotalPktNum - 1))
							break;
					}
					else
					{
						return LORARET_TYPE_ERR;
					}
				}
				else
				{
					return LORARET_CHK_ERR;
				}
			}
			else
			{
				return LORARET_KEYWORD_ERR;
			}			
		}
		
		/* 判断超时 */
		tdiff = (uwTick >= timer) ? (uwTick - timer) : (0xFFFFFFFF - timer + uwTick);
		if (tdiff > Timeout)
		{
			Para->TotalPktNum = 0;
			Para->PktOrder = 0;
			memset(Para->pRxBuf, 0x00, LORA_RX_BUFSIZE);
			memset(tmpLoraRecvBuf, 0x00, 2048);			
			Para->ucRxBufPos = 0;
			Para->RxCpltFlag = 0;
			return LORARET_TIMEOUT_ERR;
		}
		Para->RxCpltFlag = 0;
		#if LORATYPE 	
			Radio.IrqProcess();
		#elif LORATYPE == 2
			Ebyte_RF.StartPollTask();		//检查数据
		#endif		
	}

	/* 计算差分数据长度 */
	i = 0;
	datlen = 0;
	dbufpos = 0;
	do
	{
		if(tmpLoraRecvBuf[i] == 0x5A && tmpLoraRecvBuf[i+1] == 0xA5 && tmpLoraRecvBuf[i+2] == 0xA5 && tmpLoraRecvBuf[i+3] == 0x5A)
		{
			dbufpos = i;
			break;
		}
		else
		{
			i++;
		}
	} while (i < dlen);
	if (i == dlen) dbufpos = i;
	if(dbufpos <2000)
	{
		*DiffLen = dbufpos;
		memcpy(DiffInfo, tmpLoraRecvBuf, dbufpos);
		memset(tmpLoraRecvBuf, 0x00, 2048);
		g_ResDataFromBaseFlag = 1;
		g_SendDataOrerDelay = TIM6->CNT;
		g_SendDataToBaseDelay = uwTick;		
	}
	return LORARET_OK;	
//	uint8_t rlen;
//	uint16_t pos;
//	uint8_t chksum;
//	uint32_t timer;
//	
//	uint32_t tdiff;	
//	uint16_t i;

//	uint16_t dlen;
//	uint16_t datlen;
//	uint16_t datpos;
//	uint16_t dbufpos;
//	
//	/* 未接收到数据 */
//	if (Para->RxCpltFlag != 2)
//	{
//		return LORARET_NODATA_ERR;
//	}
//	
//	timer = uwTick;		//重置定时器
//	dlen = 0;
//	/* 差分定位数据有没有收全 */
//	if((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
//	{
//		if(g_RecLora.Total < 255)
//			g_RecLora.Total ++;
//		else
//			g_RecLora.Total = 0;
//	}
//	/* 接收定位数据 */
////	while (1)
//	{
//		/* 接收到数据 */
//		if (Para->RxCpltFlag == 2)
//		{
//			/* 判断数据头 */
//			rlen = 0;
//			if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
//			{
//				rlen = Para->pRxBuf[2] + 4;
//				/* 检查校验和 */
//				chksum = CheckSum(rlen-1, Para->pRxBuf);
//				if (chksum == Para->pRxBuf[rlen-1])
//				{
//					/* 检查数据类型=差分数据 */
//					if (Para->pRxBuf[3] == LORA_TYPE_DIFFINFO)
//					{
//						timer = uwTick;		//重置定时
//						
//						/* 获取基站数据头 */
//						LORA_Arr2Header(Para->pRxBuf + 3, Header);
//						
//						if(Para->pRxBuf[6] <= 255 )						
//							g_RecLora.BaseTotal = Para->pRxBuf[6];         /*基站lora发送数据总数*/
//						if(Para->pRxBuf[7] <=  255)						
//							g_RecLora.BaseSucNum = Para->pRxBuf[7];         /*基站lora发送的成功数据条数*/						
//						if(g_RecLoraInit == 1)
//						{
//							g_RecLora.Total = g_RecLora.BaseTotal;
//							g_RecLora.SucNum = g_RecLora.BaseSucNum;
//							g_RecLoraInit = 0;
//						}
//						
//						/* 获取差分数据信息 */
////						Para->TotalPktNum = Para->pRxBuf[11];
////						Para->PktOrder = Para->pRxBuf[12];
////						pos = Para->PktOrder * 241;
//						memcpy(tmpLoraRecvBuf, Para->pRxBuf + 11, rlen-8);
//						dbufpos = rlen-12;
////						dlen += 241;
//						
////						/* 最后一包数据 接收完成 */
////						if (Para->PktOrder == (Para->TotalPktNum - 1))
////							break;
//					}
//					else
//					{
//						return LORARET_TYPE_ERR;
//					}
//				}
//				else
//				{
//					return LORARET_CHK_ERR;
//				}
//			}
//			else
//			{
//				return LORARET_KEYWORD_ERR;
//			}			
//		}
//		
//		/* 判断超时 */
////		tdiff = (uwTick >= timer) ? (uwTick - timer) : (0xFFFFFFFF - timer + uwTick);
////		if (tdiff > Timeout)
////		{
////			Para->TotalPktNum = 0;
////			Para->PktOrder = 0;
////			memset(Para->pRxBuf, 0x00, LORA_RX_BUFSIZE);
////			memset(tmpLoraRecvBuf, 0x00, 2048);			
////			Para->ucRxBufPos = 0;
////			Para->RxCpltFlag = 0;
////			return LORARET_TIMEOUT_ERR;
////		}
//		Para->RxCpltFlag = 0;
//		Ebyte_RF.StartPollTask();		//检查数据
//	}

////	/* 计算差分数据长度 */
////	i = 0;
////	datlen = 0;
////	dbufpos = 0;
////	do
////	{
////		if(tmpLoraRecvBuf[i] == 0x5A && tmpLoraRecvBuf[i+1] == 0xA5 && tmpLoraRecvBuf[i+2] == 0xA5 && tmpLoraRecvBuf[i+3] == 0x5A)
////		{
////			dbufpos = i;
////			break;
////		}
////		else
////		{
////			i++;
////		}
////	} while (i < dlen);
////	if (i == dlen) dbufpos = i;
//	if(dbufpos <1400)
//	{
//		*DiffLen = dbufpos;
//		memcpy(DiffInfo, tmpLoraRecvBuf, dbufpos);
//		memset(tmpLoraRecvBuf, 0x00, 2048);
//		g_ResDataFromBaseFlag = 1;
//		g_SendDataOrerDelay = TIM6->CNT;
//		g_SendDataToBaseDelay = uwTick;		
//	}
//	return LORARET_OK;
}


/***********************接收板*******************************/
uint8_t g_RecIdleFlag = 0;
uint8_t g_RecWarnFlag = 0;
uint8_t g_RecDataFlag = 0;




#endif