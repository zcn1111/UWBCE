
#include "bsp.h"
#include "main.h"

uint8_t g_BTOpenLockCmd[8] = {0x30,0x37,0x33,0x33,0x35,0x32,0x35,0x32};
uint8_t g_LockMGAStatus;
uint8_t g_LockHeadStatus;
uint8_t g_LockStatus;
uint8_t g_UnLockStatus;
uint8_t g_OCFlag;
uint8_t g_StatusStartChange;
uint8_t g_LockStatusChangeFlag;
uint8_t g_OpenLockType;
uint8_t g_DoorStatusChangeFlag;
uint8_t g_StatusCnt;
uint8_t g_NetOpenDoor;

bool g_DoorIsOpen;
bool g_OpenLock;
bool g_JudgeLockCloseFailFlag;

uint32_t g_JudgeLockStatusDelay;
uint32_t g_StatusChangeDelay;
uint32_t g_DoorOpenDelay;
uint32_t g_Open1MinDelay;

uint32_t g_OpenDoorCardID;
uint32_t g_NetOpenDoorCmd;
uint32_t g_NetOpenDoorCmdLog[3];

DATETIME g_NetOpenDoorAvlTime;
/******************************************************************
函数名称: void Init_Door_Var(void)
功能描述: 门禁参数初始化
参数输入: 无
参数输出: 无
返 回 值: 无
其    他: hly

******************************************************************/
void InitLockVar(void)
{
    g_LockStatus = 0;
    g_LockHeadStatus = 0;//默认门锁关
	g_UnLockStatus = 0;
	g_LockStatusChangeFlag = 0;
	g_StatusStartChange = 0;
	g_LockMGAStatus = 0;//默认门磁关
	g_DoorStatusChangeFlag= 0;
	g_JudgeLockCloseFailFlag = false;

}


/******************************************************************
函数名称: void GnerateStatusRecord(void)
功能描述: 产生一条门禁状态数据 
参数输入: 
参数输出: 
返 回 值: 无
其    他: 
******************************************************************/
void GnerateFinalStatusRecord(void)
{
    uint8_t tmprecord[32];
    uint8_t cmpbuf[5];

    memset(cmpbuf,0,sizeof(cmpbuf));
	memset(tmprecord,0,sizeof(tmprecord));
    tmprecord[0] = 0xB9;
    tmprecord[1] = 0x21;
   
	if(g_MobileInFlash.Crc16 == CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2))
    {
        if((CheckNumIsBCD((uint8_t*)g_MobileInFlash.mobileno[4],5) == true)&&
            (memcmp((uint8_t*)g_MobileInFlash.mobileno[4],cmpbuf,5)!=0))
        {
            memcpy(tmprecord + 2,g_MobileInFlash.mobileno[4],6);
			if(g_SelSIMFlag == SIM1ONLY)
				tmprecord[7] = (tmprecord[7]&0xF0);
			else
				tmprecord[7] = (tmprecord[7]&0xF1);
			ExRtcReadTime(&g_SysTime);
            memcpy( tmprecord + 8 , (uint8_t*)&g_BCDTime, 6 );
            tmprecord[14] = 0xB0;
            tmprecord[15] = g_StatusCnt;						
            tmprecord[16] = g_LockMGAStatus;			
            tmprecord[17] = g_LockHeadStatus;
			if(g_NetOpenDoor == 2)
				tmprecord[18] = 6;
			else
				tmprecord[18] = g_OpenLockType;
			
			if(g_NetOpenDoor == 2)
			{
				tmprecord[19] = (g_NetOpenDoorCmd>>24)&0xFF;
				tmprecord[20] = (g_NetOpenDoorCmd>>16)&0xFF;
				tmprecord[21] = (g_NetOpenDoorCmd>>8)&0xFF;
				tmprecord[22] = g_NetOpenDoorCmd&0xFF;
			}
			else if(g_OpenLockType == 1)
			{
				tmprecord[19] = (g_OpenDoorCardID>>24)&0xFF;
				tmprecord[20] = (g_OpenDoorCardID>>16)&0xFF;
				tmprecord[21] = (g_OpenDoorCardID>>8)&0xFF;
				tmprecord[22] = g_OpenDoorCardID&0xFF;
			}
			else if(g_OpenLockType == 2)
			{
				tmprecord[19] = (g_NetOpenDoorCmd>>24)&0xFF;
				tmprecord[20] = (g_NetOpenDoorCmd>>16)&0xFF;
				tmprecord[21] = (g_NetOpenDoorCmd>>8)&0xFF;
				tmprecord[22] = g_NetOpenDoorCmd&0xFF;
			}
			tmprecord[23] = g_LockMGAStatus;
			tmprecord[24] = 0;
			tmprecord[25] = g_UnLockStatus;
			tmprecord[26] = g_LockStatus;
			tmprecord[27] = g_OCFlag;
			Get_Adc_Average(ADC_CHANNEL_6,10);
			tmprecord[28] = g_BatVal;
			tmprecord[29] = 0xFC;
			tmprecord[30] = 0x3F;
            tmprecord[31] = CheckSum(31,tmprecord );
            SaveRecordToFlash(&g_StatusDatainFlash,tmprecord);
			if((g_StatusCnt++>0xFF))
            {
                g_StatusCnt = 0;
            }
			
			if(g_CelluarPowerOn == 0)
			{
				PWR_CELLUAR_ENABLE();
				delay_ms(50);
				InitCelluarGpio();
				g_CelluarPowerOn = 1;
				g_CelluarPowerOnDelay = TIM2->CNT;
				CelluarStep = STA_PWR_ON;
				InitNet();
			}
		}
    }
}

/******************************************************************
函数名称: void GnerateWarningRecord(uint8 flag)
功能描述: 产生一条门禁报警数据 
参数输入: 0x01-人员逗留
          0x02-通道门开启时间超过10分钟
          0x03-连续5分钟无读卡记录，通道门未关闭
          0x04-电池电量低于警戒值
          0x05-交流电断开
参数输出: 
返 回 值: 无
其    他: 
******************************************************************/
void GnerateWarningRecord(uint8_t flag)
{
    uint8_t tmprecord[32];
    uint8_t cmpbuf[5];

    memset(cmpbuf,0,sizeof(cmpbuf));
    tmprecord[0] = 0xB9;
    tmprecord[1] = 0x21;
    
	if(g_MobileInFlash.Crc16 == CRC16( ( uint8_t* )& g_MobileInFlash , sizeof( g_MobileInFlash ) - 2 ))
    {
        if((CheckNumIsBCD((uint8_t*)g_MobileInFlash.mobileno[4],5) == true)&&
            (memcmp((uint8_t*)g_MobileInFlash.mobileno[4],cmpbuf,5)!=0))
        {
            memcpy(tmprecord + 2,g_MobileInFlash.mobileno[4],6);
            
            ExRtcReadTime(&g_SysTime);
            memcpy( tmprecord + 8 , (uint8_t*)&g_BCDTime, 6 );
            tmprecord[14] = 0xB3;
            tmprecord[15] = flag;
			Get_Adc_Average(ADC_CHANNEL_6,10);
			tmprecord[16] = g_BatVal;
            memset(tmprecord + 17,0,14);
            tmprecord[31] = CheckSum(31,tmprecord );
            SaveRecordToFlash(&g_WarnDatainFlash,tmprecord);
        }
    }
}

uint8_t testopen;

/******************************************************************
函数名称: void LockControl_DJ02(void)
功能描述: 
参数输入: 
参数输出: 
返 回 值: 无
其    他: 
******************************************************************/
void LockControl_DJ02(void)
{
	if((JudgeSomeDelayInProOk(g_LockPowerOnDelay,Delay500mSec)==1) && (JudgeSomeDelayInProOk(g_JudgeLockStatusDelay,Delay300mSec)==1))
	{
		if(READ_DOOR_MAG_STA() == GPIO_PIN_RESET)//读门磁信号为低电平，门磁关
		{
			if(g_LockMGAStatus == 0xFE)///如果门锁门磁状态为门开
			{
				g_LockMGAStatus = 0;
				g_OCFlag = 0;
				if(g_StatusStartChange == 0)
				{
					g_StatusStartChange = 1;
					g_StatusChangeDelay = TIM2->CNT;
				}
                g_DoorStatusChangeFlag = 1;
			}
		}
		else///读到门磁开 ReadMAGSTA == 1
		{
			if(g_LockMGAStatus == 0)//如果门磁状态为门关
			{
				g_LockMGAStatus = 0xFE;
				if(g_StatusStartChange == 0)
				{
					g_StatusStartChange = 1;
					g_StatusChangeDelay = TIM2->CNT;
				}
                g_DoorStatusChangeFlag = 1;
				g_DoorIsOpen = true;
                g_DoorOpenDelay = TIM2->CNT;                //门开时间开始计时
			}
            else//如果门磁状态也为门开
			{			
				if(g_OpenLock)
				{
					g_OpenLock = false;
					LOCK_OPENDISABLE();
					g_JudgeLockCloseFailFlag = false;
				}
				g_ShouldEnterLowpowerDelay = TIM2->CNT;
			}
		}
		
		if(READ_DOOR_LOCK_STA() == GPIO_PIN_RESET)//读上锁状态信号为低电平，锁芯开
		{
			if(g_LockStatus == 0)
			{
				g_LockStatus = 0xFE;
				if(g_OpenLockType!=1 && g_OpenLockType!=2 && g_OpenLockType!=4 && g_OpenLockType!=7 && g_OpenLockType!=8)
				{
					g_OpenLockType = 3;
				}								
			}
		}
		else
		{
			if(g_LockStatus == 0xFE)
			{
				g_LockStatus = 0;
			}
		}
		
		if(READ_DOOR_UNLOCK_STA() == GPIO_PIN_RESET)//读开锁状态信号为低电平，锁芯关
		{
			if(g_UnLockStatus == 0xFE)
			{
				g_UnLockStatus = 0;
			}
		}
		else
		{
			if(g_UnLockStatus == 0)
			{
				g_UnLockStatus = 0xFE;
				if(g_OpenLockType!=1 && g_OpenLockType!=2 && g_OpenLockType!=4 && g_OpenLockType!=7 && g_OpenLockType!=8)
				{
					g_OpenLockType = 3;
				}
			}			
		}
		
		if(g_UnLockStatus == 0 && g_LockStatus == 0)
		{
			if(g_LockMGAStatus == 0xFE)
			{
				g_OpenLockType = 4;
			}
			if(g_LockHeadStatus == 0xFE)
			{
				g_LockHeadStatus = 0;
				g_LockStatusChangeFlag = 1;
				if(g_StatusStartChange == 0)
				{
					g_StatusStartChange = 1;
					g_StatusChangeDelay = TIM2->CNT;
				}
				if(g_OpenLock)
				{
					g_OpenLock = false;
					LOCK_OPENDISABLE();
					g_JudgeLockCloseFailFlag = false;
				}
				
			}
		}
		else 
		{
			if(g_LockHeadStatus == 0)
			{
				g_LockHeadStatus = 0xFE;
				g_LockStatusChangeFlag = 1;
				if(g_LockMGAStatus == 0)
				{
					g_OCFlag = 1;
				}
				if(g_StatusStartChange == 0)
				{
					g_StatusStartChange = 1;
					g_StatusChangeDelay = TIM2->CNT;
				}
			}
			g_ShouldEnterLowpowerDelay = TIM2->CNT;
		}
	
		if(g_DoorStatusChangeFlag == 1 || g_LockStatusChangeFlag == 1)
		{
			if(g_StatusStartChange == 1 && (JudgeSomeDelayInProOk(g_StatusChangeDelay,Delay300mSec)==1))
			{
				if(g_LockMGAStatus == 0xFE && g_LockHeadStatus == 0xFE)
				{
					ExRtcReadTime(&g_SysTime);
					if(!g_TimeNoAdjustFlag )
						GnerateFinalStatusRecord();
					g_DoorStatusChangeFlag = 0;
					g_LockStatusChangeFlag = 0;
					g_StatusStartChange = 0;
				}
				else if(g_LockMGAStatus == 0 && g_LockHeadStatus == 0)
				{
					if(!g_TimeNoAdjustFlag )
						GnerateFinalStatusRecord();
					g_DoorStatusChangeFlag = 0;
					g_LockStatusChangeFlag = 0;
					g_StatusStartChange = 0;
					if(g_OpenLockType != 0)
					{
						g_OpenLockType = 0;
						g_OpenDoorCardID = 0;
					}
					if(g_BTLinkStatus == 1)
					{
						g_SendStatus = 2;
					}
				}
				else if(JudgeSomeDelayInProOk(g_StatusChangeDelay,Delay5Sec)==1)
				{
					if(!g_TimeNoAdjustFlag )
						GnerateFinalStatusRecord();
					g_DoorStatusChangeFlag = 0;
					g_LockStatusChangeFlag = 0;
					g_StatusStartChange = 0;					
				}				
			}		
		}
	}
	if(g_OpenLock)
	{			
		if((JudgeSomeDelayInProOk(g_Open1MinDelay,g_SysPar.closelockdelay*Delay1Min)==1)&& (!g_JudgeLockCloseFailFlag))///远程开门超过1分钟后 把锁关上
		{
			LOCK_OPENDISABLE();
			g_JudgeLockCloseFailFlag = true;
		}
		if((JudgeSomeDelayInProOk(g_Open1MinDelay,g_SysPar.closelockdelay*Delay1Min)==1) && g_JudgeLockCloseFailFlag)
		{
			g_OCFlag = 0;
			if(!g_TimeNoAdjustFlag)
				GnerateFinalStatusRecord();
			g_OpenLock = false;
			g_JudgeLockCloseFailFlag = false;
		}
	}
	
	if(g_CalHourFlag == 1 && (JudgeSomeDelayInProOk(g_LockPowerOnDelay,Delay1Sec)==1))//整点产生一条门禁状态信息
	{
		GnerateFinalStatusRecord();
		g_CalHourFlag = 0;
	}	
}

/******************************************************************
函数名称: void LockControl_DJ09(void)
功能描述: 
参数输入: 
参数输出: 
返 回 值: 无
其    他: 
******************************************************************/
void LockControl_DJ09(void)
{
	if((JudgeSomeDelayInProOk(g_LockPowerOnDelay,Delay500mSec)==1) && (JudgeSomeDelayInProOk(g_JudgeLockStatusDelay,Delay300mSec)==1))
	{
		if(READ_DOOR_MAG_STA() == GPIO_PIN_RESET)//读门磁信号为低电平，门磁关
		{
			if(g_LockMGAStatus == 0xFE)///如果门锁门磁状态为门开
			{
				g_LockMGAStatus = 0;
				g_OCFlag = 0;
				if(g_StatusStartChange == 0)
				{
					g_StatusStartChange = 1;
					g_StatusChangeDelay = TIM2->CNT;
				}
                g_DoorStatusChangeFlag = 1;
			}
		}
		else///读到门磁开 ReadMAGSTA == 1
		{
			if(g_LockMGAStatus == 0)//如果门磁状态为门关
			{
				g_LockMGAStatus = 0xFE;
				if(g_StatusStartChange == 0)
				{
					g_StatusStartChange = 1;
					g_StatusChangeDelay = TIM2->CNT;
				}
                g_DoorStatusChangeFlag = 1;
				g_DoorIsOpen = true;
                g_DoorOpenDelay = TIM2->CNT;                //门开时间开始计时
			}
            else//如果门磁状态也为门开
			{				
				if(g_OpenLock)
				{
					g_OpenLock = false;
					LOCK_OPENDISABLE();
					g_JudgeLockCloseFailFlag = false;
				}
				g_ShouldEnterLowpowerDelay = TIM2->CNT;
			}
		}
		
		if(READ_DOOR_LOCK_STA() == GPIO_PIN_RESET)//读门锁状态信号为低电平，锁芯关
		{
			if(g_LockMGAStatus == 0xFE)
			{
				g_OpenLockType = 4;
			}
			if(g_LockHeadStatus == 0xFE)
			{
				g_LockHeadStatus = 0;
				g_LockStatusChangeFlag = 1;
				if(g_StatusStartChange == 0)
				{
					g_StatusStartChange = 1;
					g_StatusChangeDelay = TIM2->CNT;
				}
				if(g_OpenLock)
				{
					g_OpenLock = false;
					LOCK_OPENDISABLE();
					g_JudgeLockCloseFailFlag = false;
				}
			}
		}
		else 
		{
			if(g_LockHeadStatus == 0)
			{
				g_LockHeadStatus = 0xFE;
				if(g_OpenLockType!=1 && g_OpenLockType!=2 && g_OpenLockType!=4 && g_OpenLockType!=7 && g_OpenLockType!=8)
				{
					g_OpenLockType = 3;
				}	
				g_LockStatusChangeFlag = 1;
				if(g_LockMGAStatus == 0)
				{
					g_OCFlag = 1;
				}
				if(g_StatusStartChange == 0)
				{
					g_StatusStartChange = 1;
					g_StatusChangeDelay = TIM2->CNT;
				}
			}
			g_ShouldEnterLowpowerDelay = TIM2->CNT;
		}
	
		if(g_DoorStatusChangeFlag == 1 || g_LockStatusChangeFlag == 1)
		{
			if(g_StatusStartChange == 1 && (JudgeSomeDelayInProOk(g_StatusChangeDelay,Delay300mSec)==1))
			{
				if(g_LockMGAStatus == 0xFE && g_LockHeadStatus == 0xFE)
				{
					ExRtcReadTime(&g_SysTime);
					GnerateFinalStatusRecord();
					g_DoorStatusChangeFlag = 0;
					g_LockStatusChangeFlag = 0;
					g_StatusStartChange = 0;
				}
				else if(g_LockMGAStatus == 0 && g_LockHeadStatus == 0)
				{
					GnerateFinalStatusRecord();
					g_DoorStatusChangeFlag = 0;
					g_LockStatusChangeFlag = 0;
					g_StatusStartChange = 0;
					if(g_OpenLockType != 0)
					{
						g_OpenLockType = 0;
						g_OpenDoorCardID = 0;
					}					
				}
				else if(JudgeSomeDelayInProOk(g_StatusChangeDelay,Delay5Sec)==1)
				{
					GnerateFinalStatusRecord();
					g_DoorStatusChangeFlag = 0;
					g_LockStatusChangeFlag = 0;
					g_StatusStartChange = 0;					
				}				
			}		
		}
	}
	if(g_OpenLock)
	{			
		if((JudgeSomeDelayInProOk(g_Open1MinDelay,g_SysPar.closelockdelay*Delay1Min)==1)&& (!g_JudgeLockCloseFailFlag))///远程开门超过1分钟后 把锁关上
		{
			LOCK_OPENDISABLE();
			g_JudgeLockCloseFailFlag = true;
		}
		if((JudgeSomeDelayInProOk(g_Open1MinDelay,g_SysPar.closelockdelay*Delay1Min)==1) && g_JudgeLockCloseFailFlag)
		{
			g_OCFlag = 0;
			GnerateFinalStatusRecord();
			g_OpenLock = false;
			g_JudgeLockCloseFailFlag = false;
		}
	}
	
	if(g_CalHourFlag == 1 && (JudgeSomeDelayInProOk(g_LockPowerOnDelay,Delay1Sec)==1))//整点产生一条门禁状态信息
	{
		GnerateFinalStatusRecord();
		g_CalHourFlag = 0;
	}
}
