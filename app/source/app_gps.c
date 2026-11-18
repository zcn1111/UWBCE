/*
*********************************************************************************************************
*
*	模块名称 : GPS定位模块驱动程序
*	文件名称 : app_gps.c
*	版    本 : V1.1
*	说    明 : 解码GPS协议 NMEA-0183协议
*
*	修改记录 :
*
*********************************************************************************************************
*/

#include "main.h"
#include "bsp.h"

#define SET_BEITIAN_NMEA	     "$POLCFGMSG,0,"
#define SET_YIYUAN_NMEA          "$PAIR062,"

//默认是北天
uint8_t IsBeiTianOrYiYuan = 0;
//$POLCFGMSG,0,5,1   除了0和5以外别的都关闭
uint8_t CurrentIndex = 0;
uint8_t CurrentCLoseNMEA = 1;
uint32_t SendCLoseNMEATime = 0;
uint8_t SendCLoseNMEANum = 0;
uint8_t CloseBeiTianNMEA[CLOSE_BEITIAN_NMEA_NUM] = {1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t CloseYiYuanNMEA[CLOSE_YIYUAN_NMEA_NUM] = {1, 2, 3, 5};

//充电自检Rtk模块
uint8_t ChargeSelfRtk(void)
{
	//充电自检开始后，10s的时间内如果没有获得Rtk的数据，就判定为Rtk自检失败
	if (JudgeSomeDelayInProOk(g_SelfCheck.Rtk.Delay,Delay10Sec))
	{
		if (g_SelfCheck.Rtk.Num > 0)
		{
			g_SelfCheck.Rtk.Result = 0;
		}
		else
		{
			g_SelfCheck.Rtk.Result = 1;
		}
		
		return 1;
	}
	
	return 0;
}

uint8_t NmeaCheckSum(const char *sentence) 
{
    uint8_t checksum = 0;
    
    //跳过开头的 '$'
    const char *p = (*sentence == '$') ? sentence + 1 : sentence;
    
    //累加直到字符串结束
    while (*p != '\0' && *p != '*') 
	{
        checksum ^= *p++;
    }
    
    return checksum;
}

/****************************************************************************
函数名称: void RTKSendOrder(char* s)
功能描述: 
参数输入: 
参数输出: 
返  回  值: 无
其      他: 无
*****************************************************************************/
void RTKSendOrder(char* SendOrder)
{	
	uint8_t SendOrderLen = 0;
  	SendOrderLen = strlen(SendOrder);  //求出所发AT命令的长度
	UartSend(&g_Uart2,(uint8_t *)SendOrder,SendOrderLen);	
}

void SetRtkNMEA(void)
{
	//默认是北天模块
	if (IsBeiTianOrYiYuan == 0)
	{
		if (CurrentIndex >= CLOSE_BEITIAN_NMEA_NUM)
		{
			return;
		}
		
		//失败次数大于3,可能是移远的模块
		if (SendCLoseNMEANum >= 3)
		{
			CurrentIndex = 0;
			CurrentCLoseNMEA = 1;
			IsBeiTianOrYiYuan = 1;
			SendCLoseNMEANum = 0;
			SendCLoseNMEATime = TIM2->CNT;
			
			return;
		}
		
		switch (CurrentCLoseNMEA)
		{
			case 1:
			{
				if(JudgeSomeDelayInProOk(SendCLoseNMEATime,Delay2Sec))
				{
					char str[50];
					memset(str,0,sizeof(str));
					sprintf(str,"%s%d,%d\r\n", SET_BEITIAN_NMEA, CloseBeiTianNMEA[CurrentIndex], 0);
					RTKSendOrder(str);
					//printf("%s\n", str);
					SendCLoseNMEATime = TIM2->CNT;
					CurrentCLoseNMEA = 2;
					SendCLoseNMEANum++;
				}
				break;
			}
			case 2:
			{
				if(JudgeSomeDelayInProOk(SendCLoseNMEATime,Delay3Sec) == true)
				{
					SendCLoseNMEATime = TIM2->CNT;
					CurrentCLoseNMEA = 1;
				}	
				break;
			}
			default:
			{
				break;
			}
		}
	}
	//移远模块
	else if (IsBeiTianOrYiYuan == 1)
	{
		if (CurrentIndex >= CLOSE_YIYUAN_NMEA_NUM)
		{
			return;
		}
		
		if (SendCLoseNMEANum >= 3)
		{
			CurrentIndex = CLOSE_YIYUAN_NMEA_NUM;
		
			return;
		}
		
		switch (CurrentCLoseNMEA)
		{
			case 1:
			{
				if(JudgeSomeDelayInProOk(SendCLoseNMEATime,Delay2Sec))
				{
					char str[50];
					char Sendstr[50];
					memset(str,0,sizeof(str));
					memset(Sendstr,0,sizeof(Sendstr));
					//$PAIR062,<Type>,<OutputRate>*<Checksum><CR><LF>
					sprintf(str,"%s%d,%d*", SET_YIYUAN_NMEA, CloseYiYuanNMEA[CurrentIndex], 0);
					sprintf(Sendstr,"%s%02X\r\n", str, NmeaCheckSum(str));
					RTKSendOrder(Sendstr);
					//printf("%s\n", Sendstr);
					SendCLoseNMEATime = TIM2->CNT;
					CurrentCLoseNMEA = 2;
					SendCLoseNMEANum++;
				}
				break;
			}
			case 2:
			{
				if(JudgeSomeDelayInProOk(SendCLoseNMEATime,Delay3Sec) == true)
				{
					SendCLoseNMEATime = TIM2->CNT;
					CurrentCLoseNMEA = 1;
				}	
				break;
			}
			default:
			{
				break;
			}
		}
	}
}