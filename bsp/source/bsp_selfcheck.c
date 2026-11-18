#include "bsp.h"
#include "app_ly.h"
#include "app_gps.h"

SelfCheck g_SelfCheck;
uint32_t g_SendChargeSelfTime = 0;
uint32_t g_TimeSelfCheckTime = 0;
uint8_t g_KeySelfCheckStep = 0;
uint8_t g_SelfCheckRtkNum = 0;
uint32_t g_KeySelfCheckStepTime = 0;
uint32_t g_SelfCheckRtkTime = 0;

void ChargeInit(void)
{
	g_SelfCheck.Dtu.Result = 1;
	g_SelfCheck.BlueTooth.Result = 1;
	g_SelfCheck.Rtk.Result = 1;
	g_SelfCheck.Imu.Result = 0;
	g_SelfCheck.Flash.Result = 1;
	
	g_SelfCheck.Dtu.Delay = TIM2->CNT;
	g_SelfCheck.BlueTooth.Delay = TIM2->CNT;
	g_SelfCheck.Rtk.Delay = TIM2->CNT;
	g_SelfCheck.Imu.Delay = TIM2->CNT;
	g_SelfCheck.Flash.Delay = TIM2->CNT;
	
	g_SelfCheck.Dtu.Num = 0;
	g_SelfCheck.BlueTooth.Num = 0;
	g_SelfCheck.Rtk.Num = 0;
	g_SelfCheck.Imu.Num = 0;
	g_SelfCheck.Flash.Num = 0;
	
	g_SelfCheck.Dtu.Finish = 0;
	g_SelfCheck.BlueTooth.Finish = 0;
	g_SelfCheck.Rtk.Finish = 0;
	g_SelfCheck.Imu.Finish = 1;
	g_SelfCheck.Flash.Finish = 0;
	
	//开始自检的时间
	g_SelfCheck.StartSelfCheckTime = TIM2->CNT;
	
	//初始化为未进入充电自检
	g_SelfCheck.EnterSelfCheck = 1;
	
	//第一次自检
	g_SelfCheck.FirstSelfCheck = 0;
	
	g_TimeSelfCheckTime = TIM2->CNT;
}

uint8_t IsStartCharge(void)
{
	uint16_t AdcResult = 0;
	
	AdcResult = Get_Adc_Average(ADC_CHANNEL_16, 10);
	if (AdcResult > 3300)
	{
		return 1;
	}
	else
	{
		return 0;
	}
	
	return 0;
}

//充电自检
void ChargeSelfCheck(void)
{
	//蓝牙自检
	if (g_SelfCheck.BlueTooth.Finish ==0)
	{
		g_SelfCheck.BlueTooth.Finish = ChargeSelfBlueTooth();
	}
	
	//rtk自检
	if (g_SelfCheck.Rtk.Finish ==0)
	{
		g_SelfCheck.Rtk.Finish = ChargeSelfRtk();
	}
	
	//Flash自检
	if (g_SelfCheck.Flash.Finish ==0)
	{
		g_SelfCheck.Flash.Finish = ChargeSelfFlash();
	}
	
	//4g充电自检
	if (g_SelfCheck.Dtu.Finish ==0)
	{
		g_SelfCheck.Dtu.Finish = ChargeSelf4g();
	}
}

//控制rtk/蓝牙/4g断电
void ControlledPowerDisable(void)
{
	PWR_RTK_DISABLE();
	BT_POW_DISEN();
}

//控制rtk/蓝牙/4g上电
void ControlledPowerEnable(void)
{
	PWR_RTK_ENABLE();
	BT_POW_EN();
	
	g_LYInitPro = 1;
	g_LYLink = 0;
}

void SendChargeSelfData()
{
	BcdDateTime_t bcdtime;
	uint8_t chksum_t;
	uint8_t answerbuff[193];
	memset(answerbuff,0,sizeof(answerbuff));

	memcpy((uint8_t*)&g_CardLocaData.ID, (uint8_t*)&(g_IDInFlash.IDnum), sizeof(g_IDInFlash.IDnum));			
	g_CardLocaData.BatVoltage = g_BatVal; 	
	
	/* 生成记录数据头 */
	answerbuff[0] = 0xA1;
	answerbuff[1] = 0x00;
	answerbuff[2] = 0x00;
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
	//预留
	//数据长度
	answerbuff[30] = 160;
	answerbuff[31] = 0;
	//定位数据部分
	//时间
	answerbuff[32] = g_SysTime.year;
	answerbuff[33] = g_SysTime.month;
	answerbuff[34] = g_SysTime.date;
	answerbuff[35] = g_SysTime.hour;
	answerbuff[36] = g_SysTime.minute;
	answerbuff[37] = g_SysTime.second;
	//自检结果(bit0:DTU模块 bit1:蓝牙模块 bit2:RTK模块 bit3:IMU模块 bit4:FLASH模块)
	if (g_SelfCheck.Dtu.Result == 0)
	{
		answerbuff[39] = answerbuff[39] | 0X01;
	}
	
	if (g_SelfCheck.BlueTooth.Result == 0)
	{
		answerbuff[39] = answerbuff[39] | 0X02;
	}
	
	if (g_SelfCheck.Rtk.Result == 0)
	{
		answerbuff[39] = answerbuff[39] | 0X04;
	}
	
	if (g_SelfCheck.Imu.Result == 0)
	{
		answerbuff[39] = answerbuff[39] | 0X08;
	}
	
	if (g_SelfCheck.Flash.Result == 0)
	{
		answerbuff[39] = answerbuff[39] | 0X10;
	}
	
	//answerbuff[39] = 0X00;
	//充电标记(0:未充电 1:充电 2:充满)
	if (g_SelfCheck.StartCharge == 0)
	{
		answerbuff[40] = 0X00;
	}
	else if((g_SelfCheck.StartCharge == 1) && (g_BatVal >= 41))
	{
		answerbuff[40] = 0X02;
	}
	else
	{
		answerbuff[40] = 0X01;
	}
	//工作模式
	answerbuff[41] = 0X00;
	//定位数据间隔
	answerbuff[42] = g_LocationAndSelfcheckDiffFlash.Diff & 0xFF; 
    answerbuff[43] = (g_LocationAndSelfcheckDiffFlash.Diff >> 8) & 0xFF;
	/*
	if (g_FirstSendChargeSelf == 1)
	{
		answerbuff[44] = 0X01;
		g_FirstSendChargeSelf = 0;
	}
	else
	{
		answerbuff[44] = 0X00;
	}
	*/
	
	
	//电池电量
	answerbuff[183] = g_CardLocaData.BatVoltage;
	//设备编号
	memcpy(answerbuff + 103,EquipmentNum,40);
	//硬件版本号
	memcpy(answerbuff + 143,HardwareVer,40);
	//版本号
	memcpy(answerbuff + 184,g_VersionAscii,4);
	//booloader版本号
	memcpy(answerbuff + 188,g_VersioninFlash.bootversion, 4); 
	//预留139个字节
	//校验和
	chksum_t = CheckSum(192, answerbuff);
	answerbuff[192] = chksum_t;
	
	UartSend(&g_Uart1, (uint8_t*)&answerbuff, 193);
}

void ResetSelfCheckFlag()
{
	g_SelfCheck.Dtu.Num = 0;
	g_SelfCheck.BlueTooth.Num = 0;
	g_SelfCheck.Rtk.Num = 0;
	g_SelfCheck.Flash.Num = 0;
			
	g_SelfCheck.BlueTooth.Finish = 0;
	g_SelfCheck.Rtk.Finish = 0;
	g_SelfCheck.Flash.Finish = 0;
	g_SelfCheck.Dtu.Finish = 0;
	g_SelfCheck.EnterSelfCheck = 1;
			
	g_SelfCheck.Dtu.Delay = TIM2->CNT;
	g_SelfCheck.BlueTooth.Delay = TIM2->CNT;
	g_SelfCheck.Rtk.Delay = TIM2->CNT;
	g_SelfCheck.Flash.Delay = TIM2->CNT;
	
	g_SelfCheck.SelfCheckSuccess = 0;
}

void SelfCheckProcess(void)
{	
	//开机即插电与正在远程升级时，不做充电自检
	//if ((JudgeSomeDelayInProOk(g_SelfCheck.PowerOnTime, Delay3Sec) == false) || (g_RemoteUpdateGCFlag > 0))
	//{
	//	return;
	//}
	
	if (g_RemoteUpdateGCFlag > 0)
	{
		return;
	}
	
	g_SelfCheck.StartCharge = IsStartCharge();
	if (g_SelfCheck.StartCharge == 1)
	{
		GPIO_PinState OutputState = HAL_GPIO_ReadPin(LED_R_GPIO_PORT, LED_R_PIN);
        if (OutputState == GPIO_PIN_RESET)
        {
			LED_R_ON();
        }
	}

	//切换到485
	if ((g_SelfCheck.StartCharge == 1) && (g_SelfCheck.Mode485 == 0))
	{
		SWITCHTYPE=1;
		g_SelfCheck.Mode485 = 1;
		InitChargeGpio();
		HAL_GPIO_WritePin(GPIOA,TOOL_GPIO_PIN,GPIO_PIN_RESET);
		Select_485();
		InitChargeUSART1();	
	}
	//切换到串口
	else if ((g_SelfCheck.StartCharge == 0) && (g_SelfCheck.Mode485 == 1))
	{
		SWITCHTYPE=0;
		g_SelfCheck.Mode485 = 0;
		CheckToolInit();
		Select_uart();
		InitUSART1();
		BT_POW_EN();
		BTInit();
	}
	
	if ((SWITCHTYPE == 1) && (g_SelfCheck.StartCharge == 1))
	{
		if(JudgeSomeDelayInProOk(g_Uart1.usTxTimer,Delay20mSec) == true && g_Uart1DirFlag == 2)
		{
			if (g_Uart1.SendOver != 0)
			{
				g_Uart1.SendOver();
			}
		
			g_Uart1DirFlag = 1;
		}
		
		if(g_Uart1.usRxCount > 0 && JudgeSomeDelayInProOk(g_Uart1RxTimer,Delay50mSec))
		{
			if(g_RxBuf1[0] == 0xA1 && g_RxBuf1[1] == 0x00 && g_RxBuf1[15] == CheckSum(15,g_RxBuf1))
			{
				SendChargeSelfData();
			}
			
			g_Uart1.usRxCount = 0;
		}
	}
	
	if (g_SelfCheck.StartCharge == 1)
	{
		if (JudgeSomeDelayInProOk(g_SelfCheck.StartSelfCheckTime, Delay30Min) == true)
		{			
			ResetSelfCheckFlag();
			g_SelfCheck.StartSelfCheckTime = TIM2->CNT;
			ControlledPowerEnable();
			
			if (gprs_link_status == 0)
			{
				//处理4g模块
				PWR_4G_ENABLE();
				current_step = 0xAC;
			}
		}
		
		if (g_SelfCheck.EnterSelfCheck == 1)
		{
			ChargeSelfCheck();
		
			if ((g_SelfCheck.BlueTooth.Finish == 1) && (g_SelfCheck.Rtk.Finish == 1) && 
					(g_SelfCheck.Flash.Finish == 1) && (g_SelfCheck.Dtu.Finish == 1))
			{
				g_AskTimeFlag = 0;
				g_SendLocationFirst = 0;
				g_SelfCheck.EnterSelfCheck = 0;
				ControlledPowerDisable();
				g_SelfCheck.SelfCheckSuccess = 1;
				g_SendChargeSelfTime = TIM2->CNT;
				g_ServiceAnsewerUpgradeTime = TIM2->CNT;
				g_SelfCheck.StartSelfCheckTime = TIM2->CNT;
			}
		}
		
		if (g_SelfCheck.FirstSelfCheck == 0)
		{
			g_SelfCheck.FirstSelfCheck = 1;
		}
	}
	
	if ((g_SelfCheck.StartCharge == 0) && (g_SelfCheck.FirstSelfCheck == 1))
	{	
		ControlledPowerEnable();
		//处理4g模块
		PWR_4G_ENABLE();
		current_step = 0xAC;
		ResetSelfCheckFlag();
		g_SendLocationFirst = 0;
		gprs_link_status = 0;
		diff_gprs_link_status = 0;
		g_SelfCheck.FirstSelfCheck = 0;
		g_SelfCheck.EnterSelfCheck = 1;
	}
	
	//在充电期间，超过2分没有收到服务器的数据给4g模块断电
	if ((JudgeSomeDelayInProOk(g_ServiceAnsewerUpgradeTime, Delay2Min) == true) &&
			(g_SelfCheck.StartCharge == 1) && (g_SelfCheck.SelfCheckSuccess == 1))
	{
		gprs_link_status = 0;
		diff_gprs_link_status = 0;
		current_step = 0x20;
		g_SelfCheck.SelfCheckSuccess = 0;
	}
}

void SelfCheckRtk(void)
{
	if ((JudgeSomeDelayInProOk(g_SelfCheckRtkTime, Delay1Sec) == true))
	{
		if (g_PosQuality < 4)
		{
			if (g_SelfCheckRtkNum >= 30)
			{
				g_SelfCheckRtkNum = 30;
			}
			else 
			{
				g_SelfCheckRtkNum++;
			}
		}
		else 
		{
			g_SelfCheckRtkNum = 0;
		}
			
		g_SelfCheckRtkTime = TIM2->CNT;
	}
}

//定时检测设备是否异常
void TimeSelfCheck(void)
{	
	if (g_LocationAndSelfcheckDiffFlash.SelfcheckDiff == 0)
	{
		return;
	}

	if ((JudgeSomeDelayInProOk(g_TimeSelfCheckTime, g_LocationAndSelfcheckDiffFlash.SelfcheckDiff * Delay1Min) == true))
	{
		g_TimeSelfCheckTime = TIM2->CNT;
		
		if (gprs_link_status == 0)
		{
			return;
		}
		
		if (g_PosQuality < 4)
		{
			return;
		}
		
		g_YYPlay.status |= 0x08;
	}
}

uint8_t KeySelfCheckFlag = 0;
void KeySelfCheck(void)
{	
	if (g_KeySelfCheck == 0)
	{	
		KeySelfCheckFlag = 0;
		return;
	}
	
	if ((gprs_link_status == 0) && (g_KeySelfCheckStep == 0))
	{
		g_YYPlay.status |= 0x01;
		g_KeySelfCheckStep = 1;
		KeySelfCheckFlag = 1;
		g_KeySelfCheckStepTime = TIM2->CNT;
		
		return;
	}
	else if ((gprs_link_status == 1) && (g_KeySelfCheckStep == 0))
	{
		g_KeySelfCheckStep = 2;
		g_KeySelfCheckStepTime = TIM2->CNT;
		
		return;
	}
		
	if ((g_KeySelfCheckStep == 1) && (JudgeSomeDelayInProOk(g_KeySelfCheckStepTime, Delay5Sec) == true))
	{
		g_KeySelfCheckStep = 2;
	}
	
	if ((g_PosQuality < 4) && (g_KeySelfCheckStep == 2))
	{
		g_YYPlay.status |= 0x02;
		g_KeySelfCheckStep = 3;
		KeySelfCheckFlag = 1;
		g_KeySelfCheckStepTime = TIM2->CNT;
		
		return;
	}
	else if ((g_PosQuality >= 4) && (g_KeySelfCheckStep == 2))
	{
		g_KeySelfCheckStep = 4;
		g_KeySelfCheckStepTime = TIM2->CNT;
		
		return;
	}
	
	if ((g_KeySelfCheckStep == 3) && (JudgeSomeDelayInProOk(g_KeySelfCheckStepTime, Delay5Sec) == true))
	{
		g_KeySelfCheckStep = 4;
	}
	
	if ((battery < 50) && (g_KeySelfCheckStep == 4))
	{
		g_YYPlay.status |= 0x04;
		g_KeySelfCheck = 0;
		KeySelfCheckFlag = 1;

		g_KeySelfCheckStep = 0;
		
		return;
	}
	else if ((battery >= 50) && (g_KeySelfCheckStep == 4))
	{
		g_KeySelfCheck = 0;
		g_KeySelfCheckStep = 0;
		
		if (KeySelfCheckFlag == 0)
		{
			g_YYPlay.status |= 0x08;
		}
	}
}

void WorkSelfCheckProcess(void)
{
	SelfCheckRtk();
	TimeSelfCheck();
	KeySelfCheck();
}