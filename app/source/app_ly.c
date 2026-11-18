#include "bsp.h"
#include "main.h"
#include "bsp_eeprom.h"
#include <stdint.h>
uint32_t   g_LYInitDelay; 
uint32_t	g_LYSendLocalDelay;        /*给蓝牙发送定位数据的间隔时间*/
uint8_t g_LYLink = 0;
//1下行解除   2上行解除 3下行预告  4上行预告
uint8_t  RemoteControl_data[3];  //[0] 事件类型（首次连接事件，按键触发事件） [1]按键编号 [3] 电池电量百分比
uint8_t ReceivelLYData = 0;
uint32_t g_ReceivelLYDataTime = 0;

void LYParam_Init(void)
{
	char *q;
	uint8_t FindEquipmentNum[EEP_UNIT_SIZE] = {0};
	if(g_Uart1.usRxCount > 0)
	{
		switch(g_LYInitPro)
		{
			case 2:
				if(strstr((const char*)g_RxBuf1,btok) != NULL)
				{
					g_RecLYOkFlag = 1;
					g_LYInitErr = 0;
				}
				else
				{
				    if(g_LYInitErr < 10)
					{
						g_LYInitErr++;
						g_LYInitPro = 1;
					}
					else
					{
						g_LYInitPro = 0;
						g_LYInitErr = 0;
					}
				}
				break;
			case 3:
				g_LYInitErr = 0;

				//如果有"ZRDW"则重新命名
				if (strstr((const char*)g_RxBuf1,(const char*)"ZRDW"))
				{
					g_RecLYOkFlag = 1;
					
					break;
				}	
				else
				{
					uint8_t BuffFF[EEP_UNIT_SIZE];
					memset(BuffFF, 0xFF, EEP_UNIT_SIZE);
					memset(FindEquipmentNum, 0X00, EEP_UNIT_SIZE);
					uint8_t ret = FindStrCopy((const char*)EquipmentNum, (char *)FindEquipmentNum, EEP_UNIT_SIZE, "DWK");
					//如果在设备号里面未好到"DWK"，则重新命名
					if (ret == 1)
					{
						g_RecLYOkFlag = 1;
					
						break;
					}
					else
					{
						if (!strstr((const char*)g_RxBuf1,(const char*)FindEquipmentNum))
						{
							g_RecLYOkFlag = 1;
					
							break;
						}
						else if (!strstr((const char*)g_RxBuf1,(const char*)BuffFF))
						{
							g_RecLYOkFlag = 1;
					
							break;
						}
						else
						{
							g_LYInitPro = 5;
							g_RecLYOkFlag = 1;																																																																																																																				 
					
							break;
						}
					}
				}
			case 4:			
				if(strstr((const char*)g_RxBuf1,btok) != NULL)
			    {
					g_RecLYOkFlag = 1;
                    g_LYInitErr = 0;					
				}
				else
				{
				    if(g_LYInitErr < 10)
					{
						g_LYInitErr++;
						g_LYInitPro = 4;
						g_RecLYOkFlag = 1;
					}
					else
					{
						g_LYInitPro = 0;
						g_LYInitErr = 0;
					}
				}
				break;	
			case 5:			
				if(strstr((const char*)g_RxBuf1,btok) != NULL)
			    {
					g_RecLYOkFlag = 1;
                    g_LYInitErr = 0;					
				}
				else
				{
				    if(g_LYInitErr < 10)
					{
						g_LYInitErr++;
						g_LYInitPro = 5;
						g_RecLYOkFlag = 1;
					}
					else
					{
						g_LYInitPro = 0;
						g_LYInitErr = 0;
					}
				}
				break;
			case 6:			
				if(strstr((const char*)g_RxBuf1,btok) != NULL)
			    {
					g_RecLYOkFlag = 1;
                    g_LYInitErr = 0;					
				}
				else
				{
				    if(g_LYInitErr < 10)
					{
						g_LYInitErr++;
						g_LYInitPro = 6;
						g_RecLYOkFlag = 1;
					}
					else
					{
						g_LYInitPro = 0;
						g_LYInitErr = 0;
					}
				}
				break;
			case 7:			
				if(strstr((const char*)g_RxBuf1,btok) != NULL)
			    {
					g_RecLYOkFlag = 1;
                    g_LYInitErr = 0;					
				}
				else
				{
				    if(g_LYInitErr < 10)
					{
						g_LYInitErr++;
						g_LYInitPro = 7;
						g_RecLYOkFlag = 1;
					}
					else
					{
						g_LYInitPro = 0;
						g_LYInitErr = 0;
					}
				}
				break;
			case 8:			
				if(strstr((const char*)g_RxBuf1,btok) != NULL)
			    {
					g_RecLYOkFlag = 1;
                    g_LYInitErr = 0;					
				}
				else
				{
				    if(g_LYInitErr < 10)
					{
						g_LYInitErr++;
						g_LYInitPro = 8;
						g_RecLYOkFlag = 1;
					}
					else
					{
						g_LYInitPro = 0;
						g_LYInitErr = 0;
					}
				}
				break;
            default:
				break;
		}
		g_LYInitDelay = TIM2->CNT;
        g_Uart1.usRxCount = 0;
	}
	if(g_LYInitPro == 1 && JudgeSomeDelayInProOk(g_LYInitDelay,Delay200mSec) == true)
	{		
		g_LYInitPro = 2;
		g_RecLYOkFlag = 0;	
		g_LYInitDelay = TIM2->CNT;
		BTSendAT(enspp);                    /*使能SPP*/
		//BTSendAT(enble);
	}
	else if(g_RecLYOkFlag == 1 && g_LYInitPro == 2 && JudgeSomeDelayInProOk(g_LYInitDelay,Delay200mSec) == true)
	{
		g_LYInitPro = 3;
		g_RecLYOkFlag = 0;
        g_LYInitDelay = TIM2->CNT;
        BTSendAT(checkname_spp);   /*读取蓝牙名称*/		
	}
	else if(g_RecLYOkFlag == 1 && g_LYInitPro == 3 && JudgeSomeDelayInProOk(g_LYInitDelay,Delay200mSec) == true)
	{			        
		char str[40];
		uint8_t BuffFF[EEP_UNIT_SIZE];
		uint8_t Buff00[EEP_UNIT_SIZE];
		
		g_LYInitPro = 4;
		g_RecLYOkFlag = 0;
        g_LYInitDelay = TIM2->CNT;	
		memset(str,0,sizeof(str));	

		memset(BuffFF, 0xFF, EEP_UNIT_SIZE);
		memset(Buff00, 0x00, EEP_UNIT_SIZE);
		
		if(memcmp(EquipmentNum,BuffFF,EEP_UNIT_SIZE) == 0 || memcmp(EquipmentNum,Buff00,EEP_UNIT_SIZE) == 0)
		{//若没有设备编号,使用ID号
			snprintf(str, sizeof(str), "%s%s%s\r\n", setname_spp, "DWK-", g_LYID);			
		}		
		else
		{//写入设备编号
			memset(FindEquipmentNum, 0X00, EEP_UNIT_SIZE);
			FindStrCopy((const char*)EquipmentNum, (char *)FindEquipmentNum, EEP_UNIT_SIZE, "DWK");
			snprintf(str, sizeof(str), "%s%s\r\n", setname_spp, FindEquipmentNum);	
		}
		
		BTSendAT(str);           /*修改蓝牙名称*/		
	}		
	else if(g_RecLYOkFlag == 1 && g_LYInitPro == 4 && JudgeSomeDelayInProOk(g_LYInitDelay,Delay200mSec) == true)
	{
		char str[40];
		uint8_t BuffFF[EEP_UNIT_SIZE];
		uint8_t Buff00[EEP_UNIT_SIZE];
		
		g_LYInitPro = 5;
		g_RecLYOkFlag = 0;
        g_LYInitDelay = TIM2->CNT;		

		memset(BuffFF, 0xFF, EEP_UNIT_SIZE);
		memset(Buff00, 0x00, EEP_UNIT_SIZE);
		
		if(memcmp(EquipmentNum,BuffFF,EEP_UNIT_SIZE) == 0 || memcmp(EquipmentNum,Buff00,EEP_UNIT_SIZE) == 0)
		{//若没有设备编号,使用ID号
			snprintf(str, sizeof(str), "%s%s%s\r\n", setname_ble, "DWK-", g_LYID);			
		}		
		else
		{//写入设备编号
			memset(FindEquipmentNum, 0X00, EEP_UNIT_SIZE);
			FindStrCopy((const char*)EquipmentNum, (char *)FindEquipmentNum, EEP_UNIT_SIZE, "DWK");
			snprintf(str, sizeof(str), "%s%s\r\n", setname_ble, FindEquipmentNum);	
		}

		BTSendAT(str);           /*修改蓝牙名称*/				
	}
	else if(g_RecLYOkFlag == 1 && g_LYInitPro == 5 && JudgeSomeDelayInProOk(g_LYInitDelay,Delay200mSec) == true)
	{
		g_LYInitPro = 6;
		g_RecLYOkFlag = 0;
        g_LYInitDelay = TIM2->CNT;	
		BTSendAT(setuuidser);           /*设置UUID:F000*/
	}	
	else if(g_RecLYOkFlag == 1 && g_LYInitPro == 6 && JudgeSomeDelayInProOk(g_LYInitDelay,Delay200mSec) == true)
	{
		g_LYInitPro = 7;
		g_RecLYOkFlag = 0;
        g_LYInitDelay = TIM2->CNT;
		BTSendAT(setuuidwrite);           /*设置UUID:F000*/
	}		
	else if(g_RecLYOkFlag == 1 && g_LYInitPro == 7 && JudgeSomeDelayInProOk(g_LYInitDelay,Delay200mSec) == true)
	{		
		g_LYInitPro = 8;
		g_RecLYOkFlag = 0;
        g_LYInitDelay = TIM2->CNT;	
		BTSendAT(setuuidread);           /*设置UUID:F000*/
	}	
	else if(g_RecLYOkFlag == 1 && g_LYInitPro == 8 && JudgeSomeDelayInProOk(g_LYInitDelay,Delay200mSec) == true)
	{		
		g_LYInitPro = 0;
		g_RecLYOkFlag = 0;	
        g_LYInitDelay = TIM2->CNT;
		BTSendAT(ResLY);           /*复位*/
		
		//蓝牙已连接上
		g_LYLink = 1;
	}
	
	if(g_LYInitPro > 0 && JudgeSomeDelayInProOk(g_LYInitDelay,Delay3Sec) == true)
	{
		g_LYInitDelay = TIM2->CNT;
		if(g_LYInitErr++ < 10)
			g_LYInitPro-=1;
		else
		{
			g_LYInitPro=0;
			g_LYInitErr = 0;
			 /*后期增加故障代码*/
		}
	}	
}

/******************************************************************
函数名称: void RecLYData(void)
功能描述: 
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RecLYData(uint8_t *bPtr,int32_t len)
{
	uint16_t i;
	for(i = 0; i < len; i++)
	{
		if(bPtr[i] == 0x5A && bPtr[i+1] == 0xA5 && bPtr[i+2] == 0x00 )
		{
			if(bPtr[i+3] == 0x01 && bPtr[i+127] == CheckSum(127, bPtr+i))                   /*电子围栏数据*/
			{
				if(bPtr[i+5] == 0x00) /*区域编号*/
				{				
				}
				g_RecLYEFenceData.AreaType = bPtr[i+4];
				g_RecLYEFenceData.AreaNum = bPtr[i+5];
				g_RecLYEFenceData.Total = bPtr[i+6];		
                g_LYRturnType.ElecFence = 1;				
				if(g_RecLYEFenceData.AreaNum  ==  g_RecLYEFenceData.Total-1  )     /*是否数据收全*/	
				{
//					g_RecLYDataType.ElecFence = 1;			
				}					
				i+=127; 				
			}
			else if(bPtr[i+3] == 0x02 && bPtr[i+15] == CheckSum(15, bPtr+i))              /*上下道数据*/
			{
				g_DataSta.Uplink = bPtr[i+4];                        /*判断设备是否完成上下道*/
				if(g_DataSta.Uplink == 1)
//					g_CarWarnFlag.bit.OnRoad = 1;
				;
				else if(g_DataSta.Uplink == 2)
					//g_CarWarnFlag.bit.DownRoad = 1;				
//				g_RecLYDataType.Uplink = bPtr[i+4];					   /*接到到蓝牙上下道标记*/
				g_LYRturnType.Uplink = bPtr[i+4];                        /*应答蓝牙标记*/ 
				g_StaJudeNum.g_Uplink = 0;                                   /*判断是否上下道次数*/
				i+=15; 
			}
			else if(bPtr[i+3] == 0x03 && bPtr[i+15] == CheckSum(15, bPtr+i) )              /*来车标记*/
			{		
				g_LYRturnType.TrainAppro = bPtr[i+4];
				
				if(g_StaJudeNum.g_Train < JudgStaNum)
				{
					WarnLED_OFF();					
					g_CarWarnFlag.bit.TrainForeshow = bPtr[i+4];								
				}				
				/*判断来车后是否下道标记*/
//				WarnLED_OFF();
				i+=15; 
			}
			else if(bPtr[i+3] == 0x04 && bPtr[i+31] == CheckSum(31, bPtr+i) )              /*施工时间标记*/
			{
				/*施工时间*/
                g_LYRturnType.ConstrTime = 1;		 /*应答蓝牙标记*/						
				if(JudgTimeOrder(g_SysTime,g_WorkTime.stop,0) == BEFORE)
				{
				}					
				i+=31; 
			}
			else if(bPtr[i+3] == 0x05 && bPtr[i+15] == CheckSum(15, bPtr+i) )              /*蓝牙开始接收定位标记*/
			{
				/*蓝牙开始接收定位数据标记*/
				g_LYRecLocalDataFlag = 1;
				i+=15; 
			}
			else if(bPtr[i+3] == 0x06 && bPtr[i+15] == CheckSum(15, bPtr+i) )              /*应答定位数据标记*/
			{
				/*应答定位数据标记*/
                g_LYAnsDataNum = 0; 
				i+=15; 
			}
			else if(bPtr[i+3] == 0x07 && bPtr[i+31] == CheckSum(31, bPtr+i) )              /*指示灯配置*/
			{
				/*应答定位数据标记*/     
//				g_RecLYDataType.WarnLEDSta = 1;      /*接收到蓝牙下发施工时间标记*/
				g_LYRturnType.WarnLEDSta = 1;		 /*应答蓝牙标记*/				
				memcpy((uint8_t*)&g_WarnLedSta,bPtr+i+4,sizeof(g_WarnLedSta)-3);							
				i+=31; 				
			}
			else if(bPtr[i+3] == 0x08 && bPtr[i+7] == CheckSum(7, bPtr+i) )              /*遥控器数据接收*/
			{
				/*应答定位数据标记*/     
				g_LYRturnType.RemoteControl = 1;		 /*应答蓝牙标记*/				
				RemoteControl_data[0] = bPtr[i + 4];   // 直接读取第一个字节   首次连接成功/按键事件/5min心跳包
				RemoteControl_data[1] = bPtr[i + 5];   // 直接读取第二个字节   按键按下编号
				RemoteControl_data[2] = bPtr[i + 6];   // 直接读取第二个字节   电量百分比			
				i+=7; 	

				ReceivelLYData = 1;
				
				g_ReceivelLYDataTime = TIM2->CNT;
			}
		}
	}
}

/******************************************************************
函数名称: void ReturnEFenceToLY(void)
功能描述: 卡片给蓝牙发送电子围栏应答
参数输入: 无
参数输出: 
返  回  值:i+=16; i+=16;  无
编写者：
******************************************************************/
void ReturnEFenceToLY(void)
{
    memset(g_SecTxBuf1,0,sizeof(g_SecTxBuf1));
    g_SecTxBuf1[0] =0x5A; 
	g_SecTxBuf1[1] =0xA5; 
	g_SecTxBuf1[2] =0x01;
	g_SecTxBuf1[3] =0x01;
	g_SecTxBuf1[4] = g_RecLYEFenceData.AreaType;
	g_SecTxBuf1[5] = g_RecLYEFenceData.AreaNum;
	g_SecTxBuf1[6] = g_RecLYEFenceData.Total;
	g_SecTxBuf1[15] = CheckSum(15, g_SecTxBuf1);	
	UartSend(&g_Uart1,g_SecTxBuf1,16);
}
/******************************************************************
函数名称: void ReturnTrainToLY(void)
功能描述: 卡片给蓝牙发送来车应答
参数输入: 无
参数输出: 
返  回  值: 无
编写者：
******************************************************************/
void ReturnTrainToLY(void)
{
    memset(g_SecTxBuf1,0,sizeof(g_SecTxBuf1));
    g_SecTxBuf1[0] =0x5A; 
	g_SecTxBuf1[1] =0xA5; 
	g_SecTxBuf1[2] =0x01;
	g_SecTxBuf1[3] =0x03;
	g_SecTxBuf1[15] = CheckSum(15, g_SecTxBuf1);
	UartSend(&g_Uart1,g_SecTxBuf1,16);	
}
/******************************************************************
函数名称: void ReturnTrainToLY(void)
功能描述: 卡片给蓝牙发送上下道应答
参数输入: 无
参数输出: 
返  回  值: 无
编写者：
******************************************************************/
void ReturnUplinkToLY(void)
{
    memset(g_SecTxBuf1,0,sizeof(g_SecTxBuf1));
    g_SecTxBuf1[0] =0x5A; 
	g_SecTxBuf1[1] =0xA5; 
	g_SecTxBuf1[2] =0x01;
	g_SecTxBuf1[3] =0x02;
	g_SecTxBuf1[4] =g_LYRturnType.Uplink;
	g_SecTxBuf1[15] = CheckSum(15, g_SecTxBuf1);	
	UartSend(&g_Uart1,g_SecTxBuf1,16);
}
/******************************************************************
函数名称: void ReturnConstrTimeToLY(void)
功能描述: 卡片给蓝牙发送收到施工时间应答
参数输入: 无
参数输出: 
返  回  值: 无
编写者：
******************************************************************/
void ReturnConstrTimeToLY(void)
{
    memset(g_SecTxBuf1,0,sizeof(g_SecTxBuf1));
    g_SecTxBuf1[0] =0x5A; 
	g_SecTxBuf1[1] =0xA5; 
	g_SecTxBuf1[2] =0x01;
	g_SecTxBuf1[3] =0x04;
	g_SecTxBuf1[15] = CheckSum(15, g_SecTxBuf1);
	UartSend(&g_Uart1,g_SecTxBuf1,16);	
}

/******************************************************************
函数名称: void ReturnWarnLEDStaToLY(void)
功能描述: 卡片给蓝牙发送收到指示灯配置应答
参数输入: 无
参数输出: 
返  回  值: 无
编写者：
******************************************************************/
void ReturnWarnLEDStaToLY(void)
{
    memset(g_SecTxBuf1,0,sizeof(g_SecTxBuf1));
    g_SecTxBuf1[0] =0x5A; 
	g_SecTxBuf1[1] =0xA5; 
	g_SecTxBuf1[2] =0x01;
	g_SecTxBuf1[3] =0x07;
	g_SecTxBuf1[15] = CheckSum(15, g_SecTxBuf1);
	UartSend(&g_Uart1,g_SecTxBuf1,16);	
}

/******************************************************************
函数名称: void ReturnWarnLEDStaToLY(void)
功能描述: 卡片给蓝牙发送收到遥控器数据应答
参数输入: 无
参数输出: 
返  回  值: 无
编写者：
******************************************************************/
//[0] 事件类型（首次连接事件，按键触发事件） [1]按键编号 [3] 电池电量百分比
void ReturnCardControlToLY(void)
{
    // 检查事件类型
    if (RemoteControl_data[0] == 0x00) 
	{
        // 首次连接事件，不进行数据应答回复
		WT588E_PLAY(0x1A);
		
        return;
    }
    else if (RemoteControl_data[0] == 0x01) 
	{
        // 按键触发事件，进行card回复
        memset(g_SecTxBuf1, 0, sizeof(g_SecTxBuf1));
        // 协议头
        g_SecTxBuf1[0] = 0x5A; 
        g_SecTxBuf1[1] = 0xA5; 
        g_SecTxBuf1[2] = 0x01;
        g_SecTxBuf1[3] = 0x08;
		g_SecTxBuf1[4] = 0x00;	//card
		g_SecTxBuf1[5] = RemoteControl_data[1];
			
        // 计算校验和（前7个字节）
        g_SecTxBuf1[7] = CheckSum(7, g_SecTxBuf1);
        // 发送数据
        UartSend(&g_Uart1, g_SecTxBuf1, 8);
			
		switch (RemoteControl_data[1]) 
		{
			case 0x01:  //下行解除
				//处理下行解除逻辑
				//g_YYPlay.ly |= 0x01;
				WT588E_PLAY(XXJC);
				break;
        
			case 0x02:  //上行解除
				//处理上行解除逻辑
				//g_YYPlay.ly |= 0x02;
				WT588E_PLAY(SXJC);
				break;
        
			case 0x03:  //下行预告
				//处理下行预告逻辑
				//g_YYPlay.ly |= 0x04;
				WT588E_PLAY(XXYG);
				break;
       
			case 0x04:  //上行预告
			//处理上行预告逻辑
				//g_YYPlay.ly |= 0x08;
				WT588E_PLAY(SXYG);
				break;
        
			default:
			//处理未知指令
				break;
		}
	}
	else if (RemoteControl_data[0] == 0x02)
	{
		// 按键触发事件，进行card回复
        memset(g_SecTxBuf1, 0, sizeof(g_SecTxBuf1));
        // 协议头
        g_SecTxBuf1[0] = 0x5A; 
        g_SecTxBuf1[1] = 0xA5; 
        g_SecTxBuf1[2] = 0x01;
        g_SecTxBuf1[3] = 0x08;
		g_SecTxBuf1[4] = 0x00;	//card
		g_SecTxBuf1[5] = 0x00;
			
        // 计算校验和（前7个字节）
        g_SecTxBuf1[7] = CheckSum(7, g_SecTxBuf1);
        // 发送数据
        UartSend(&g_Uart1, g_SecTxBuf1, 8);
	}
}

//给蓝牙发送服务器的应答
void ReturnServiceControlToLY(void)
{
	//按键触发事件，进行service 回复
    memset(g_SecTxBuf1, 0, sizeof(g_SecTxBuf1));
    //协议头
    g_SecTxBuf1[0] = 0x5A; 
    g_SecTxBuf1[1] = 0xA5; 
    g_SecTxBuf1[2] = 0x01;
    g_SecTxBuf1[3] = 0x08;
	g_SecTxBuf1[4] = 0x01;//service
	
	if (g_ServiceResponseLY == 0x01)
	{
		///g_SecTxBuf1[5] = 0x00;
	}
	else if (g_ServiceResponseLY == 0x02)
	{
		g_SecTxBuf1[5] = RemoteControl_data[1];
	}
	else 
	{
		return;
	}
	
    //计算校验和（前7个字节）
    g_SecTxBuf1[7] = CheckSum(7, g_SecTxBuf1);
    //发送数据
    UartSend(&g_Uart1, g_SecTxBuf1, 8);
}

/******************************************************************
函数名称: void RYDW_GenerateCardRecord
功能描述: 生成定位卡数据记录
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_GenerateCardRecord(CardInfoTable_t CardTable, uint8_t *RecordBuf, uint16_t *RecordLen)
{
	uint16_t i, ord;
	uint16_t pos;
	uint16_t reclen = 0;
//	uint8_t chksum;
	uint8_t chksum_t;
	
	/* 生成记录数据头 */
	RecordBuf[0] = 0x5A;
	RecordBuf[1] = 0xA5;
	RecordBuf[2] = 0x01;
	RecordBuf[3] = 0x06;     /*接收定位数据*/

	memcpy(RecordBuf + 4, (uint8_t*)&g_SysTime, 6);
    reclen += 15;
	/* 生成卡数据 */
	ord = 0;
	for (i=0; i<CardTable.CardNbr; i++)
	{
		/* 在线的定位卡发送 */
		if (CardTable.Info[i].Alram.bit.Online == 1)
		{
			pos = 15 + ord * 32;
			memcpy(RecordBuf + (pos + 0), CardTable.Info[i].ID.array, 5);
			RecordBuf[pos + 5] = (uint8_t)(CardTable.Info[i].ID.byte.UserID & 0xFF);						//用户ID按小端模式上传
			RecordBuf[pos + 6] = (uint8_t)(CardTable.Info[i].ID.byte.UserID >> 8);
			ConvUint32ToArray(CardTable.Info[i].Latitude, RecordBuf + (pos + 7), M_LITTLE_ENDIAN);
			ConvUint32ToArray(CardTable.Info[i].Longitude, RecordBuf + (pos + 11), M_LITTLE_ENDIAN);
			RecordBuf[(pos + 15)] = CardTable.Info[i].PosQuality;
			memcpy(RecordBuf + (pos + 16), CardTable.Info[i].Altitude, 3);
			RecordBuf[(pos + 19)] = CardTable.Info[i].Alram.all;
			RecordBuf[(pos + 20)] = CardTable.Info[i].BatVoltage;
			ConvUint16ToArray(CardTable.Info[i].AlramType.all, RecordBuf + (pos + 21), M_LITTLE_ENDIAN);
			RecordBuf[(pos + 23)] = CardTable.Info[i].FenceOrder;
			RecordBuf[(pos + 24)] = CardTable.Info[i].CTimeOrder;
//			RecordBuf[(pos + 25)] = CardTable.Info[i].CWarnLEDOrder;
			RecordBuf[(pos + 25)] = CardTable.Info[i].Reserver[0];										//预留
			RecordBuf[(pos + 26)] = CardTable.Info[i].ViewNumber;                                         //星数
			RecordBuf[(pos + 27)] = CardTable.Info[i].AreaType;                                           //区域类型			
			memcpy(RecordBuf + (pos + 28), CardTable.Info[i].Reserver, 4);		//其他7个预留字节
			reclen += 32;
			ord++;
		}
	}
	
	RecordBuf[10] = (ord*32)&0xFF;             /*定位卡数据长度*/
	RecordBuf[11] = (ord*32>>8)&0xFF;	
	chksum_t = CheckSum(reclen, RecordBuf);
	RecordBuf[reclen] = chksum_t;
	
	*RecordLen = reclen + 1;
}

void LYProcess(void)
{
	uint16_t g_UploadRecordLen = 0;
	if(g_LYRturnType.ElecFence == 1)
	{
		ReturnEFenceToLY();		
		g_LYRturnType.ElecFence = 0;
	}
	else if(g_LYRturnType.TrainAppro > 0)
	{
		ReturnTrainToLY();
		g_LYRturnType.TrainAppro = 0;			
	}
	else if(g_LYRturnType.Uplink > 0)
	{		
		ReturnUplinkToLY();
		g_LYRturnType.Uplink = 0;			
	}
	else if(g_LYRturnType.ConstrTime > 0) /*应答*/
	{
		ReturnConstrTimeToLY(); 
		g_LYRturnType.ConstrTime = 0;		
	}
	else if(g_LYRturnType.WarnLEDSta > 0)
	{
		ReturnWarnLEDStaToLY(); 
		g_LYRturnType.WarnLEDSta = 0;
	}
	else if(g_LYRturnType.RemoteControl > 0)
	{
		ReturnCardControlToLY(); 
		g_LYRturnType.RemoteControl = 0;
	}
	else
	{
		if(g_LYRecLocalDataFlag == 1 &&JudgeSomeDelayInProOk(g_LYSendLocalDelay,LYSendLocalTime) == true)   /*间隔时间*/
		{
			memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&(g_IDInFlash.IDnum),sizeof(g_IDInFlash.IDnum));			
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
			g_CardLocaData.AlramType = g_CarWarnFlag;          /*报警类型*/
			g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
			g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;			
			g_CardLocaData.PosQuality = g_PosQuality;
			g_CardLocaData.Altitude[0] = (g_tGPS.Altitude>>16)&0xFF;
			g_CardLocaData.Altitude[1] = (g_tGPS.Altitude>>8)&0xFF;
			g_CardLocaData.Altitude[2] = (g_tGPS.Altitude)&0xFF;
			g_CardLocaData.FenceOrder= 0;                //电子围栏编号
			g_CardLocaData.CTimeOrder = 0;             //施工时间编号	
			g_CardLocaData.ViewNumber = g_tGPS.ViewNumber;            /*卫星个数*/;             //星数
			g_CardLocaData.AreaType = g_AreaType;               //区域类型							
			g_LoraSendOkCnt++;
			RYDW_UpdateCardInfoTable(g_CardLocaData, g_LoraSendOkCnt, &g_CardTable);	//更新定位卡信息表/*添加自己的*/
			RYDW_CheckCardOffline(g_LoraSendOkCnt, &g_CardTable);
			RYDW_GenerateCardRecord(g_CardTable, g_SecTxBuf1, &g_UploadRecordLen);
			UartSend(&g_Uart1,g_SecTxBuf1,g_UploadRecordLen);	
			g_LYSendLocalDelay = TIM2->CNT;
			if(g_LYAnsDataNum < 255)
				g_LYAnsDataNum++;
			else
				g_LYAnsDataNum = 0;
		}
	}
	
	if (g_ServiceResponseLY > 0)
	{
		ReturnServiceControlToLY();
		g_ServiceResponseLY = 0;
	}
}

/************************************** 卡数据处理初始化函数 ************************************************/


/******************************************************************
函数名称: void RYDW_InitCardInfo
功能描述: 初始化定位卡数据
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_InitCardInfo(CardData_t *CardInfo)
{
	uint8_t i;
	
	for (i=0; i<7; i++)
	{
		CardInfo->ID.array[i] = 0x00;
	}
	CardInfo->Latitude = 0;
	CardInfo->Longitude = 0;
	CardInfo->PosQuality = 0;
	memset(CardInfo->Altitude, 0x00, 3);
	CardInfo->Alram.all = 0;
	CardInfo->BatVoltage = 0;
	CardInfo->AlramType.all = 0;
	CardInfo->FenceOrder = 0;
	CardInfo->CTimeOrder = 0;
//	CardInfo->CWarnLEDOrder = 0;
	CardInfo->ViewNumber = 0;
	CardInfo->AreaType = 0;
	memset(CardInfo->Reserver, 0x00, 7);
}
/******************************************************************
函数名称: void RYDW_InitCardTable
功能描述: 初始化定位卡信息表
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_InitCardTable(CardInfoTable_t *CardTable)
{
	uint8_t i;
	
	for (i=0; i<RYDW_CARDTABLE_MAXNUM; i++)
	{
		RYDW_InitCardInfo(&CardTable->Info[i]);
		CardTable->CardRecvCnt[i] = 0;
	}
}

/************************************** 卡数据处理函数 ************************************************/

/******************************************************************
函数名称: bool RYDW_IsCardIDSame
功能描述: 检查定位卡ID是否相同
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
bool RYDW_IsCardIDSame(CardData_t SrcCard, CardData_t DestCard)
{
	uint8_t i;
	
	for (i=0; i<7; i++)
	{
		if (SrcCard.ID.array[i] != DestCard.ID.array[i])
			return false;
	}
	
	return true;
}

/******************************************************************
函数名称: void RYDW_CopyCardInfo
功能描述: 复制(更新)定位卡信息
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_CopyCardInfo(CardData_t SrcCard, CardData_t *DestCard)
{
	uint8_t i;
	
	for (i=0; i<7; i++)
	{
		DestCard->ID.array[i] = SrcCard.ID.array[i];
	}
	DestCard->Latitude = SrcCard.Latitude;
	DestCard->Longitude = SrcCard.Longitude;
	DestCard->PosQuality = SrcCard.PosQuality;
	memcpy(DestCard->Altitude, SrcCard.Altitude, 3);
	DestCard->Alram.all = SrcCard.Alram.all;
	DestCard->BatVoltage = SrcCard.BatVoltage;
	DestCard->AlramType.all = SrcCard.AlramType.all;
	DestCard->FenceOrder = SrcCard.FenceOrder;	
	DestCard->CTimeOrder = SrcCard.CTimeOrder;
	DestCard->ViewNumber = SrcCard.ViewNumber;
	DestCard->AreaType = SrcCard.AreaType;

	memcpy(DestCard->Reserver, SrcCard.Reserver, 6);
}
/******************************************************************
函数名称: void RYDW_UpdateCardInfoTable
功能描述: 更新定位卡信息表
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_UpdateCardInfoTable(CardData_t NowCardData, uint8_t RecvCnt, CardInfoTable_t *CardTable)
{
	uint16_t i;
	char str_cardid[32]={0};
	
	/* 检查定位卡信息表 */
	for (i=0; i<CardTable->CardNbr; i++)
	{
		/* 接收定位卡数据和卡列表中数据相同 更新列表中卡信息 */
		if (RYDW_IsCardIDSame(CardTable->Info[i], NowCardData) == true)
		{
			NowCardData.Alram.bit.Online = 1;				//接收到定位卡信息 更新定位卡在线
			CardTable->CardRecvCnt[i] = RecvCnt;			//更新定位卡接收编号
			RYDW_CopyCardInfo(NowCardData, &CardTable->Info[i]);
			Arr2AscStr(CardTable->Info[i].ID.array, 7, 0x00, str_cardid);
			break;
		}
	}
	/* 不在信息表里 新定位卡 添加到列表中 */
	if (i == CardTable->CardNbr)
	{
		NowCardData.Alram.bit.Online = 1;			//定位卡在线
		CardTable->CardRecvCnt[i] = RecvCnt;
		RYDW_CopyCardInfo(NowCardData, &CardTable->Info[CardTable->CardNbr]);		//增加
		CardTable->CardNbr++;
		Arr2AscStr(NowCardData.ID.array, 7, 0x00, str_cardid);
	}
}

/******************************************************************
函数名称: void RYDW_CheckCardOffline
功能描述: 检查离线卡
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_CheckCardOffline(uint8_t SendCnt, CardInfoTable_t *CardTable)
{
	uint16_t i;
	uint8_t diff;
	
	for (i=0; i<CardTable->CardNbr; i++)
	{
		/* 计算差 */
		if (SendCnt >= CardTable->CardRecvCnt[i])
		{
			diff = SendCnt - CardTable->CardRecvCnt[i];
		}
		else
		{
			diff = 0xFF - CardTable->CardRecvCnt[i] + SendCnt;
		}
		
		/* 连续3个都没有接收到 判断设备离线 */
		if (diff > 3)
		{
			CardTable->Info[i].Alram.bit.Online = 0;
			CardTable->Info[i].Alram.bit.Alram = 0;		//设备离线清除报警状态
		}
	}
}

//充电自检
uint8_t ChargeSelfBlueTooth(void)
{
	//串口切到蓝牙
	if ((g_SelfCheck.EnterSelfCheck == 1) && (SWITCHTYPE == 1))
	{
		BT_POW_DISEN();
		SWITCHTYPE=0;
		CheckToolInit();
		Select_uart();
		InitUSART1();
		delay_ms(200);
		BT_POW_EN();
	}
	
	if(JudgeSomeDelayInProOk(g_SelfCheck.BlueTooth.Delay, Delay1Sec))
	{ 
		g_LYInitPro = 1;
		g_SelfCheck.BlueTooth.Num++;
		g_SelfCheck.BlueTooth.Delay = TIM2->CNT;
	}
		
	if(strstr((const char*)g_RxBuf1, btok) != NULL)
	{
		g_SelfCheck.BlueTooth.Result = 0;
		g_SelfCheck.Mode485 = 0;
		BT_POW_DISEN();
		
		return 1;
	}
	
	if (g_SelfCheck.BlueTooth.Num > 5)
	{
		g_SelfCheck.BlueTooth.Result = 1;	
		g_SelfCheck.Mode485 = 0;
		BT_POW_DISEN();
	
		return 1;
	}
	
	return 0;
}