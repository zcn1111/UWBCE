#include "bsp.h"
#include "main.h"


/* 远程和转储进程 */
/*=1 收到升级命令，等待升级，=2 发送了改IP 升级命令，
=3, 延时1分钟后，请求状态
=4 ,修改IP地址正确后的值  */
uint8_t g_RemoteActionPre;
uint8_t g_RemoteAskNetTimer;      /*max=6*/
uint8_t g_RemoteReviseIPTimer;    /*max=2*/
uint8_t g_RemoteOutTimer;         /*max=3*/
uint8_t g_RemoteDataOkTimer;      /*max=3*///g_RemoteUpdateNoDataTimer
uint8_t g_RemoteErrorReqTimer;    /*max=2*///g_RemoteUpdateErrorReqTimer

/* 手机基本信息 */
uint8_t g_GsmEnergy;              /* 装置的信号强度*/
uint8_t g_GprsOk;                 /* 从DTU 收到的0X81 包中解码得到的GPRS 信号标志 */
uint8_t g_GsmOk;                  /* 从DTU 收到的0X81 包中解码得到的Gsm 信号标志 */
uint8_t g_DtuSendOk;              /* 通过DTU 发送的数据，收到了成功标志位 */
uint8_t g_DtuHaveSendDataOk;      /* DTU有数据发送标志位 */
uint8_t g_GprsOffFlag;            /* 断网标识  */
uint32_t g_GprsOffTime;           /* 断网连续时间  */

uint8_t g_AskSignalFromDtuFlag;       /* 请求信号标志 */
uint8_t g_AskDateFromDtuFlag;         /* 请求时间标志 */
uint32_t g_AskSignalFromDtuDelay;      
uint32_t g_GetSignalFromDtuDelay;
uint32_t g_AskDateFromDtuDelay;
uint8_t g_DeviceStatusFlag;          /*设备状态标识  =0，没信号；=1 没信号，已发信号请求；=2 有信号；=3 已请求时间；=4 已校时 =5，发送本机号请求*/
/*=1 收到但是没有信号强度 = 2 在正常的网络环境中运行  = 3 在升级或者转储请求过程, =4 UP , =5 ZC OK连接状态 ,=6将原来服务器IP地址发送给DTU，=7 等待1分钟，开始查询DTU参数,= 0  没有连网不做发送处理 */
uint8_t g_InNormalNetStatus;
uint8_t g_RemoteZc_UpdateFlag;            /* =0 normal, =1 zc, =2 update */
uint8_t g_ChangeIPFlag;                   //修改IP的标志
uint32_t g_GetChangeIPCmdStartTime;       //收到修改IP命令的起始时间
uint8_t g_DTUVersion;
uint32_t g_RemoteLinkDelay;               /* 远程延时  */
uint32_t g_AskIPAfterUpdate;              /* 远程升级完后，要IP的延时*/
uint32_t g_AskMyMobileDelay;              //请求本机号的延时时间 
uint32_t g_AskMyMobileErrorTimes;         //获取本机号错误的次数  
uint8_t MajorVersion;                     //主版本号   
uint8_t MinorVersion;                     //次版本号  
uint8_t LastMajorVersion;                 //主版本号  
uint8_t LastMinorVersion;                 //次版本号 
uint32_t  LastTotalBagNum;                //上次的包总数 
uint8_t g_LastRemUpdateSuccessFlag;
uint32_t g_DelaytimeUPSuccess;            //

uint8_t g_SendToDtuFailCount;     /* 给DTU发送0x62数据失败次数 */

/***** 远程 数据转储过程****************** 开始***********/
uint16_t g_ZC_StartAddr;                  /* 数据转储开始地址 */
uint16_t g_ZC_PageLength;                 /* 数据转储页长度 */
uint32_t g_FileSize;                      /* 数据转储文件大小 */
uint32_t g_FileOffset;                    /* 数据转储文件偏移地址 */
uint32_t g_PageOffset;                    /* 数据转储页偏移地址 */
uint8_t g_RemoteZCGCFlag;                 //远程数据转储过程标志位
/******远程程序升级****************** 开始************/
uint8_t g_RemoteUpdateFirstFlag;/* 远程升级开始标志 */
/* 远程升级过程标志位= 0 初始， = 1 收到0X85 判断收到的是新的连接成功。发送一次升级数据请求，=2 发送3次升级数据请求，每次超时20秒，发送3次都没有应答就初始为0 ，重发改iP 命令
*/
uint8_t g_RemoteUpdateGCFlag; 
uint16_t g_RemoteUpdatePackNo;
uint16_t g_RemoteUpdatePackTotal;
uint8_t g_ShouldJumpToStartFlag;
uint8_t g_UpdateType;	//0-全量升级 1-增量升级
uint8_t g_GetUpdateTypeFlag;
uint8_t g_UartUPBuf[512];
uint8_t g_checkbuf[512];

//IP和端口号
uint8_t g_IPBuffer[3][4] = {{0xD3,0x8E,0x1F,0x98},{0xD3,0x8E,0x1F,0x98},{0xD3,0x8E,0x1F,0x98}};//0-远程升级IP，1-远程转储IP，2-数据IP
uint16_t g_PortNo[3] = {2021,2029,2020};
uint8_t g_DataIPBack[4];      //数据IP备份
uint16_t g_DataPortBack;      //数据端口号备份
const uint8_t TotalControlMobleNo[6]={0x13,0x75,0x31,0x24,0x37,0x2F};
const uint8_t YFTotalControlMobleNo[6]={0x13,0x75,0x31,0x20,0x58,0x3F};
uint8_t g_D6Number[6];
bool g_TimeNoAdjustFlag;



bool g_ChangeMyMobileNumFlag;
uint8_t g_DtuReturnMyMobileFlag;
uint8_t g_HaveSetMyMobileToDTUFlag;    /* 已经向DTU写入本机号的标识 =1表示写入DTU本机号，=2表示主板没有本机号 agj */
uint8_t g_AskAuthorityFlag;
uint8_t g_DTUPdownFlag;
uint8_t g_AskPowerControlFlag;    //返回电源控制信息
uint8_t g_ResetPowerFlag;         //重启电源标识  1-重启485 2-重启系统 3-重启DTU		-99RTC时间连续错误重启

uint8_t g_B9SendFlag;
uint32_t g_SendB9Delay;

uint8_t g_F0SendFlag;
uint32_t g_SendF0Delay;


/**
 * @brief	DTU断电
 *
 * @param	void
 *
 * @return  void
 */
void AutoPowerOffDTU(void)
{
	PWR_CELLUAR_DISABLE();                                    //给DTU断电
	g_DTUPowerOn = 2;
	g_DTUPowerOffDelay = TIM2->CNT;
	g_GprsOffTime = TIM2->CNT;
	g_AskMyMobileErrorTimes = 0;
	g_DtuReturnMyMobileFlag = 0;
	g_AskMyMobileDelay = TIM2->CNT;
	if(g_ShouldJumpToStartFlag == 0)
	{
		g_RemoteZc_UpdateFlag = 0;
		g_RemoteZCGCFlag = 0;
		g_RemoteUpdateGCFlag = 0;
		//g_RemoteUpdatePackNo = 0;
		g_PageOffset = 0;
		g_ZC_PageLength = 0;
		if(g_ResetPowerFlag == 0)
			g_InNormalNetStatus = 2;
		g_RemoteActionPre = 0;
	}  
	else
	{
		if(g_UpdateType == 1)
		{
			g_UpdateType = 0;
			DiffProgramUserData();
		}
	}
}

/******************************************************************
函数名称: void InitDTUVar(void)
功能描述: DTU 定义的一些全局变量的初始化
参数输入: 无
参数输出: 
返  回  值 : 无
其           他: 无
******************************************************************/
void InitDTUVar(void)
{
    g_TimeNoAdjustFlag = 1;
    g_DTUPdownFlag = 0;
	g_GsmEnergy = 0;
	g_GprsOk = 0;
	g_GsmOk = 0;
	g_DtuSendOk = 1;
    g_DtuHaveSendDataOk = 0;
	g_InNormalNetStatus = 0;
    g_AskSignalFromDtuFlag = 0;
    g_AskDateFromDtuFlag = 0;
    g_DTUVersion = 0;                         
	g_RemoteZc_UpdateFlag = 0;
    g_RemoteUpdatePackNo = 0;   
    memset((uint8_t*)&g_UpdateCodePageTotal,0,sizeof(g_UpdateCodePageTotal)); 
	g_RemoteActionPre = 0;
	g_RemoteAskNetTimer = 20;
	g_RemoteUpdateFirstFlag = 0;
    g_RemoteLinkDelay = 0;        
    g_GprsOffFlag = 0;        
    g_GprsOffTime = TIM2->CNT;            
    g_AskIPAfterUpdate = 0;       
    g_ChangeIPFlag = 0;
    g_GetChangeIPCmdStartTime = 0;
    g_HaveSetMyMobileToDTUFlag = 0;
    g_SendToDtuFailCount = 0;
    g_ShouldJumpToStartFlag = 0;        
	g_ShouldPowerOffDTUDelay = TIM2->CNT;
    memset(g_D6Number,0,sizeof(g_D6Number));    
}
/******************************************************************
函数名称: void Send_Ip_Port(uint8_t *IP, uint8_t Port)
功能描述: 得到接收数据的长度
参数输入: 无
参数输出: 
返  回  值 : 无
其           他: 无
IP类型	1	0-正常数据，1-升级程序，2-数据转储 ,3-修改数据IP、端口号
预留	1	0
******************************************************************/
void Send_Ip_Port(uint8_t *IP, uint16_t Port,uint8_t type)
{
    if(type==0 || type == 1 || type == 2 || type == 3)
    {
    	g_TxBufLP[0] = 0xaa;
    	g_TxBufLP[1] = 0xbb;	
    	g_TxBufLP[2] = 0x66;
    	g_TxBufLP[3] = 0;
    	g_TxBufLP[4] = 0x08;	
    	g_TxBufLP[5] = 0x00;		
    	memcpy( g_TxBufLP+ 6, IP , 4 );
    	g_TxBufLP[10] =(Port/100);
    	g_TxBufLP[11] = (Port%100);	
        g_TxBufLP[12] = type;	
    	g_TxBufLP[13] = 0;	
    	g_TxBufLP[14] = CheckSum( 12 , g_TxBufLP + 2 );	
    	UartSend(&g_LPUart,g_TxBufLP,15);
    }
}
/******************************************************************
函数名称: void Send_ZhuanChuData(void)
功能描述: 发送转储原始数据
参数输入: 无
参数输出: 
返  回  值 : 无
其           他: 无
******************************************************************/
void Send_ZhuanChuData( void )
{
	uint8_t FileName[13];
	uint32_t g_FileOffset,packlen;
	
	packlen = 512;
	g_FileOffset = g_PageOffset*512;
	
	g_TxBufLP[0] = 0xaa;
	g_TxBufLP[1] = 0xbb;
	g_TxBufLP[2] = 0x62;
	g_TxBufLP[3] = 0;
	g_TxBufLP[4] = 0x20;
	g_TxBufLP[5] = 0x02;
	g_TxBufLP[6] = 0xb0;
	memcpy( g_TxBufLP + 7 , g_MobileInFlash.mobileno[4] , 6 ); // 手机号  
	YearToStr(g_SysTime.year,(char*)&FileName);
    HexTimeToStr(g_SysTime.month,(char*)&FileName+4);   
	HexTimeToStr(g_SysTime.date,(char*)&FileName+6);    
	memcpy(FileName+8,".txt",4);
	FileName[12]=0;
	memcpy(g_TxBufLP + 13,FileName,12);

	g_TxBufLP[25] = (g_FileSize&0xff);            //  文件大小
	g_TxBufLP[26] = ((g_FileSize>>8)&0xff);     //  文件大小
	g_TxBufLP[27] = ((g_FileSize>>16)&0xff);    //  文件大小
	g_TxBufLP[28] = ((g_FileSize>>24)&0xff);    //  文件大小
	
	g_TxBufLP[29] =  (g_FileOffset&0xff);        // 偏移地址
	g_TxBufLP[30] =  ((g_FileOffset>>8)&0xff);  // 偏移地址
	g_TxBufLP[31] =  ((g_FileOffset>>16)&0xff); // 偏移地址
	g_TxBufLP[32] =  ((g_FileOffset>>24)&0xff); // 偏移地址

	g_TxBufLP[33] = (packlen&0xff );               //本包数据大小
	g_TxBufLP[34] = ((packlen>>8)&0xff);        //本包数据大小
	g_TxBufLP[35] = ((packlen>>16)&0xff);       //本包数据大小
	g_TxBufLP[36] = ((packlen>>24)&0xff);       //本包数据大小	

	W25qxx_ReadPage(g_tmpBuf,g_ZC_StartAddr+g_PageOffset*2,0,256);
	W25qxx_ReadPage(g_tmpBuf,g_ZC_StartAddr+g_PageOffset*2+1,0,256);
	memcpy( g_TxBufLP + 37 , g_tmpBuf , 512 );

	g_TxBufLP[549] = CheckSum( 543 , g_TxBufLP + 6 );
	g_TxBufLP[550] = CheckSum( 548 , g_TxBufLP + 2 );
	UartSend(&g_LPUart,g_TxBufLP,551);
	
}
/******************************************************************
函数名称: void Update(uint8_t status)
功能描述: 升级程序
参数输入: 无
参数输出: 
返  回  值 : 无
其           他: 无
******************************************************************/
void GetUpdateType(void)
{
	memset(g_TxBufLP,0,sizeof(g_TxBufLP));
	g_TxBufLP[0] = 0xaa;
	g_TxBufLP[1] = 0xbb;	
	g_TxBufLP[2] = 0x62;
	g_TxBufLP[3] = 0;
	g_TxBufLP[4] = 0x10;	
	g_TxBufLP[5] = 0;	
	g_TxBufLP[6] = 0xEA;	
	
	memcpy( g_TxBufLP + 7 , g_MobileInFlash.mobileno[4], 6 );
	memcpy( g_TxBufLP + 13 , (uint8_t*)&g_UpdateCodePageTotal , 3 );	
	g_TxBufLP[16] = g_VersionHex[0];
	g_TxBufLP[17] = g_VersionHex[1];
	g_TxBufLP[21] = CheckSum( 15 , g_TxBufLP + 6 );	
	g_TxBufLP[22] = CheckSum( 20 , g_TxBufLP + 2 );		
	UartSend(&g_LPUart,g_TxBufLP,23);
}

/******************************************************************
函数名称: void Update(uint8_t status)
功能描述: 升级程序
参数输入: 无
参数输出: 
返  回  值 : 无
其           他: 无
******************************************************************/
void NewUpdate(void)
{
	memset(g_TxBufLP,0,sizeof(g_TxBufLP));
	
	g_TxBufLP[0] = 0xaa;
	g_TxBufLP[1] = 0xbb;	
	g_TxBufLP[2] = 0x62;
	g_TxBufLP[3] = 0;
	g_TxBufLP[4] = 0x20;	
	g_TxBufLP[5] = 0;	
	g_TxBufLP[6] = 0xEF;	
	
	memcpy(g_TxBufLP+7,g_MobileInFlash.mobileno[4],6);
	memcpy(g_TxBufLP+13,(uint8_t*)&g_UpdateCodePageTotal,3);
	g_TxBufLP[16] = ( g_UpdateCodePageTotal.programpage & 0xff );		
	g_TxBufLP[17] = (( g_UpdateCodePageTotal.programpage >>8 ) & 0xff );	
	g_TxBufLP[18] = ( g_RemoteUpdatePackNo & 0xff );		
	g_TxBufLP[19] = (( g_RemoteUpdatePackNo >>8 ) & 0xff );	
	if(g_UpdateType == 0xDC)
		g_TxBufLP[20] = 0;	
	else
		g_TxBufLP[20] = g_UpdateType;	
	if(g_UpdateType == 1)
	{
		g_TxBufLP[21] = g_VersionHex[0];
		g_TxBufLP[22] = g_VersionHex[1];
	}
	g_TxBufLP[37] = CheckSum( 31 , g_TxBufLP + 6 );	
	g_TxBufLP[38] = CheckSum( 36 , g_TxBufLP + 2 );		
	UartSend(&g_LPUart,g_TxBufLP,39);
}


/**
 * @brief	转储结束切换IP
 *
 * @param	void
 *
 * @return  void
 */
void ZC_EndReviseIP(void)
{
	g_RemoteZCGCFlag = 0;
	/* 转储数据发送完毕 */
	Send_Ip_Port(g_ServerInfo.serveripaddr[2],g_ServerInfo.serverportaddr[2],0);
	g_RemoteLinkDelay = TIM2->CNT;
	g_InNormalNetStatus = 6;
    g_RemoteActionPre = 5;              
    g_RemoteReviseIPTimer = SETIPTIMERS;   
	g_PageOffset = 0;                  
	g_ZC_PageLength = 0;                
}

/**
 * @brief	升级结束切换IP
 *
 * @param	void
 *
 * @return  void
 */
void Update_EndReviseIP(void)
{
	/* 升级数据发送完毕 */
	Send_Ip_Port(g_ServerInfo.serveripaddr[2],g_ServerInfo.serverportaddr[2],0);
	g_RemoteLinkDelay = TIM2->CNT;
	g_InNormalNetStatus = 6;
    g_RemoteActionPre = 5;                 
    g_RemoteReviseIPTimer = SETIPTIMERS;    
	g_RemoteUpdateFirstFlag = 0;
    g_AskIPAfterUpdate = TIM2->CNT;        
}
/******************************************************************
函数名称: void WriteMyMobileNumToDTU(void)
功能描述: DTU写本机号
参数输入: 无
参数输出: 
返  回  值 : 无
其他: 
******************************************************************/
void WriteMyMobileNumToDTU(void)
{
    memset(g_TxBufLP,0,sizeof(g_TxBufLP));
    g_TxBufLP[0] = 0xaa;
    g_TxBufLP[1] = 0xbb;
    g_TxBufLP[2] = 0x67;
    g_TxBufLP[3] = 0;
    g_TxBufLP[4] = 8;
    g_TxBufLP[5] = 0;
    memcpy(g_TxBufLP+6,g_MobileInFlash.mobileno[4],6);   
    g_TxBufLP[12] = 0;
    g_TxBufLP[13] = 0;
    g_TxBufLP[14] = CheckSum( 12 , g_TxBufLP + 2);
	UartSend(&g_LPUart,g_TxBufLP,15);
}

/******************************************************************
函数名称: void AskDTUStatus(uint8_t type)
功能描述:向DTU索要信号强度、时间日期、本机号及IP地址
参数输入: 01-索要信号强度, 02-索要时间，03-本机号、IP地址
参数输出: 
返  回  值 : 无
其他: 
******************************************************************/
void AskDTUStatus(uint8_t type)
{
    memset(g_TxBufLP,0,sizeof(g_TxBufLP));
    g_TxBufLP[0] = 0xaa;
    g_TxBufLP[1] = 0xbb;
    g_TxBufLP[2] = 0x65;
    g_TxBufLP[3] = 0;
    g_TxBufLP[4] = 8;
    g_TxBufLP[5] = 0;
    g_TxBufLP[6] = type;
    memset(g_TxBufLP+7,0,7);
    g_TxBufLP[14] = CheckSum( 12 , g_TxBufLP + 2);
	UartSend(&g_LPUart,g_TxBufLP,15);
}

/******************************************************************
函数名称: void AnalyzeDTUStatus(void)	
功能描述: 分析DTU状态
参数输入: 无
参数输出: 无
返  回  值 : 无
其他: 
******************************************************************/
void AnalyzeDTUStatus(void)	
{
	if((g_InNormalNetStatus == 0 ) && ( g_GprsOk == 1 ) && ( g_GsmOk == 1 ))
	{
		if(g_GsmEnergy == 0)
			g_GsmEnergy = 3;  
		g_InNormalNetStatus = 1;
		g_RemoteAskNetTimer = 10;
		g_RemoteLinkDelay = TIM2->CNT;
	}
	else if(g_InNormalNetStatus <=2 && g_RemoteActionPre == 0 &&( g_GprsOk == 0 || g_GsmOk == 0))// agj if(( g_GprsOk == 0 ) && ( g_GsmOk == 0))
	{
		g_InNormalNetStatus = 0;
		g_TimeNoAdjustFlag = true;    
	}
    if(g_GprsOk == 1)
    {
        g_GprsOffFlag = 0;
    }
    else
    {
        if(g_GprsOffFlag == 0)
        {
            g_GprsOffTime = TIM2->CNT;
            g_GprsOffFlag = 1;
        }
    }
}	

/******************************************************************
函数名称: void DeviceAskDtuStatus(void)
功能描述: 设备查询DTU状态
参数输入: 无
参数输出: 无
返  回  值 : 无
其他: 
******************************************************************/
void DeviceAskDtuStatus(void)
{
    memset(g_TxBufLP,0,sizeof(g_TxBufLP));
    
    g_TxBufLP[0] = 0xaa;
	g_TxBufLP[1] = 0xbb;
	g_TxBufLP[2] = 0x60;
	g_TxBufLP[3] = 0;
	g_TxBufLP[4] = 0;
	g_TxBufLP[5] = 0;
	g_TxBufLP[6] = CheckSum( 4 , g_TxBufLP + 2);
	UartSend(&g_LPUart,g_TxBufLP,7);
}

/**
 * @brief	索要信号强度
 *
 * @param	void
 *
 * @return  void
 */
void RequireEnergyFunc(void)
{
    AskDTUStatus(ASK_SIGNALSTRENGTH);
    g_AskSignalFromDtuDelay = TIM2->CNT;
    g_AskSignalFromDtuFlag = 2;
}

/**
 * @brief	索要时间
 *
 * @param	void
 *
 * @return  void
 */
void RequireDateFunc(void)
{
    AskDTUStatus(ASK_TIME);
    g_AskDateFromDtuDelay = TIM2->CNT;
    g_AskDateFromDtuFlag = 2;
}
/******************************************************************
函数名称: Bool AskTimeAndSignalWithDtu(void)
功能描述: 处理校时和信号强度函数
参数输入: 无
参数输出: 无
返  回  值 : 无
其他: 
******************************************************************/
bool AskTimeAndSignalWithDtu(void)
{
    if(g_GsmEnergy)
    {
        if(g_AskSignalFromDtuFlag == 1)
        {
            if(g_TimeNoAdjustFlag == true)
            {
                if(g_AskDateFromDtuFlag == 0|| g_AskDateFromDtuFlag == 1)
                {
                    RequireDateFunc();
                    return true;
                }
                else if(g_AskDateFromDtuFlag == 2)
                {
                    if(JudgeSomeDelayInProOk(g_AskDateFromDtuDelay,Delay10Sec) == true)
                    {
                        RequireEnergyFunc();
                        g_AskDateFromDtuFlag = 0;
                        return true;
                    }
                }
            }
            else
            {
                if(JudgeSomeDelayInProOk(g_GetSignalFromDtuDelay,Delay5Sec) == true)
                {
                    RequireEnergyFunc();
                    return true;
                }
            }
        }
        else if(g_AskSignalFromDtuFlag == 2)
        {
            if(JudgeSomeDelayInProOk(g_AskSignalFromDtuDelay,Delay10Sec) == true)
            {
                RequireEnergyFunc();
                return true;
            }
        }
    }
    else
    {
        if(g_AskSignalFromDtuFlag == 0 
          ||(g_AskSignalFromDtuFlag == 1 && JudgeSomeDelayInProOk(g_GetSignalFromDtuDelay,Delay5Sec) == true)
          ||(g_AskSignalFromDtuFlag == 2 && JudgeSomeDelayInProOk(g_AskSignalFromDtuDelay,Delay10Sec) == true))
        {
            RequireEnergyFunc();
            return true;
        }
    }
    return false;
}
/******************************************************************
函数名称: void ReturnDeviceStatusToDTU(void)
功能描述: 
参数输入: 无
参数输出: 无
返  回  值 : 无
其他: 
******************************************************************/
void ReturnDeviceStatusToDTU(void)
{
    memset(g_TxBufLP,0,sizeof(g_TxBufLP));
    
    g_TxBufLP[0] = 0xaa;
	g_TxBufLP[1] = 0xbb;
	g_TxBufLP[2] = 0x61;
	g_TxBufLP[3] = 0;
	g_TxBufLP[4] = 0;
	g_TxBufLP[5] = 0;
	g_TxBufLP[6] = CheckSum( 4 , g_TxBufLP + 2);
	UartSend(&g_LPUart,g_TxBufLP,7);
}

/******************************************************************
函数名称: void AnswerToGsm(uint8_t status)
功能描述: 主板回复，表示接收成功/ 失败
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void AnswerToGsm(uint8_t status)
{
    memset(g_TxBufLP,0,sizeof(g_TxBufLP));
    
	g_TxBufLP[0] = 0xaa;
	g_TxBufLP[1] = 0xbb;	
	g_TxBufLP[2] = 0x63;
	g_TxBufLP[3] =0 ;
	g_TxBufLP[4] = 0x00;	
	g_TxBufLP[5] = 0x00;		
	g_TxBufLP[6] = CheckSum(4,g_TxBufLP+2);	
	UartSend(&g_LPUart,g_TxBufLP,7);
}
/******************************************************************
函数名称: void AnswerDataToGsm(uint16_t len , uint8_t *data)
功能描述: 主板回复数据
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void AnswerDataToGsm( uint16_t len , uint8_t *data )
{
    memset(g_TxBufLP,0,sizeof(g_TxBufLP));
    
	g_TxBufLP[0] = 0xaa;
	g_TxBufLP[1] = 0xbb;	
	g_TxBufLP[2] = 0x62;
	g_TxBufLP[3] = 0;
	g_TxBufLP[4] = ((len+7)&0xff);	
	g_TxBufLP[5] = ((len+7)>>8);	
    g_TxBufLP[6] = 0xD6;	    
	memcpy( g_TxBufLP + 7 ,  g_D6Number , 6);
	g_TxBufLP[12] &= 0xf4;
    memcpy( g_TxBufLP + 13 ,  data , len );

	g_TxBufLP[ len + 13 ] = CheckSum( len + 11 , g_TxBufLP + 2 );	
	UartSend(&g_LPUart,g_TxBufLP,len+14);
}
/******************************************************************
函数名称: void AnswerASCDataToGsm(uint16_t len,uint8_t *data)
功能描述: 主板回复ASCII数据
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void AnswerASCDataToGsm(uint16_t len,uint8_t *data)
{
	uint16_t i;
    
	g_TxBufLP[0] = 0xaa;
	g_TxBufLP[1] = 0xbb;	
	g_TxBufLP[2] = 0x62;
	g_TxBufLP[3] = 0;
	g_TxBufLP[4] = ((len*2+7)&0xff);	
	g_TxBufLP[5] = ((len*2+7)>>8);	
	g_TxBufLP[6] = 0xD6;	
	if(data[0] == 0xcf && data[1] == 0xff && data[2] == 0x0f)
	{
		ReadPowerCtlNoFromFlash(g_TxBufLP+7);
	}
    else
		memcpy(g_TxBufLP+7,g_D6Number,6);  
	g_TxBufLP[12] &= 0xf2;
    for(i=0;i<len;i++)
	{
		Bcd2Asc(*data,g_TxBufLP+13+2*i);
        data++;
	}
	g_TxBufLP[len*2+13] = CheckSum(len*2+11,g_TxBufLP+2);	
	UartSend(&g_LPUart,g_TxBufLP,len*2+14);
}


/**
 * @brief	回复手机号 A8
 *
 * @param	
 *
 * @return  void
 */
void ReturnMobileNotoDTU(void)
{
	uint8_t i;
	
    memset(g_tmpBuf,0,sizeof(g_tmpBuf));
    memset(g_tmpBuf,0xA8,6);
    g_tmpBuf[6] = 0x01;

    for(i=0;i<2;i++)
    {
        memcpy(g_tmpBuf+7+6*i,g_MobileInFlash.mobileno[i],6);    
    }
    memset(g_tmpBuf+19,0xff,12);                        //路局号和工务段号
    memset(g_tmpBuf+31,0,4);            
    memcpy(g_tmpBuf+35,g_MobileInFlash.mobileno[4],6);  //本机号
    memcpy(g_tmpBuf+41,g_MobileInFlash.mobileno[3],6);  //短信接收号
    memset(g_tmpBuf+47,0,33);
    AnswerASCDataToGsm(80,g_tmpBuf);
}

/**
 * @brief	回复手机号 A8
 *
 * @param	
 *
 * @return  void
 */
void ReturnPowerCtlinfotoDTU(void)
{	
    memset(g_tmpBuf,0,sizeof(g_tmpBuf));
    g_tmpBuf[0] = 0xcf;
    g_tmpBuf[1] = 0xff;
    g_tmpBuf[2] = 0x0f;
	if(g_ResetPowerFlag <= 3)
	{
		g_tmpBuf[3] = g_ResetPowerFlag;
	}
    g_tmpBuf[79] = CheckSum(79,g_tmpBuf);
	AnswerASCDataToGsm(80,g_tmpBuf);
	
	g_ResetPowerFlag = 0;
	WritePowerCtlFlagToFlash(g_ResetPowerFlag);
}

/******************************************************************
函数名称: void ProcessDtuNoAnswerFunc(void)
功能描述: 处理DTU无应答
参数输入: 无
参数输出: 
返  回  值 : 无
其       他: 
******************************************************************/
void ProcessDtuNoAnswerFunc(void)
{
    g_SendToDtuFailCount++;
    if(g_SendToDtuFailCount >= 10)
    {
//		delay_ms(200);
		g_SendToDtuFailCount = 0;
		AutoPowerOffDTU();      
    }
}

/******************************************************************
函数名称: bool SendB9DataToGsm(void)
功能描述: 发送B9的函数
参数输入: 无
参数输出: 
返  回  值 : 无
其       他: 无
******************************************************************/
bool SendB9DataToGsm(void)
{
    uint8_t cmpbuf[5];
	
	memset(cmpbuf,0,sizeof(cmpbuf));
	memset(g_tmpBuf,0,sizeof(g_tmpBuf));
	
    g_TxBufLP[0] = 0xaa;
	g_TxBufLP[1] = 0xbb;
	g_TxBufLP[2] = 0x62;
	g_TxBufLP[3] = 0;
    g_TxBufLP[4] = 0x27;
    g_TxBufLP[5] = 0;
    g_TxBufLP[6] = 0xd6; 
    
    if(ReadMobileNoFromFLASH() == true)
    {
        if(CheckNumIsBCD((uint8_t*)g_MobileInFlash.mobileno[3],5) == true && memcmp((uint8_t*)g_MobileInFlash.mobileno[3],cmpbuf,5)!=0)
        {
            memcpy(g_TxBufLP + 7,g_MobileInFlash.mobileno[3],6);
        }
        else
            memset(g_TxBufLP + 7,0xff,6);
    }
    else
        memset(g_TxBufLP + 7,0xff,6);
    
    g_TxBufLP[12] &= 0xf2;
    if(ReadRecordFromFlash(&g_StatusDatainFlash,g_tmpBuf) == 1)
    {
        memcpy(g_TxBufLP + 13,g_tmpBuf,g_StatusDatainFlash.datalen);   
        g_TxBufLP[45] = CheckSum(43,g_TxBufLP + 2);
		UartSend(&g_LPUart,g_TxBufLP,46);
		g_ShouldPowerOffDTUDelay = TIM2->CNT;
        return true;
    }
    else
		return false;
}

/******************************************************************
函数名称: Bool SendB9Func(void)
功能描述: 给DTU发送B9数据
参数输入: 无
参数输出: 
返  回  值 : 无
其           他: 无
******************************************************************/
bool SendB9Func(void)
{
    uint8_t tempdata;
    uint8_t cmpbuf[5];
	uint32_t tmpoffset;

	memset(cmpbuf,0,sizeof(cmpbuf));
    if(g_B9SendFlag == 0 || (g_B9SendFlag == 1 && g_DtuSendOk == 1 && g_DtuHaveSendDataOk == 1))
    {
        if(g_B9SendFlag == 1 && g_DtuSendOk == 1 && g_DtuHaveSendDataOk == 1)
        {
            tempdata = INDEX_SEND_SUC;
			W25qxx_WriteByte(tempdata,g_StatusDatainFlash.readflagaddr);
            g_StatusDatainFlash.needsendcnt--;
			g_B9SendFlag = 0;
			if(g_SaveSendEmptyFlag == 1)
			{
				g_SaveSendEmptyFlag = 0;
				tempdata = BAG_SEND_EMPTY;
				tmpoffset = g_StatusDatainFlash.readflagaddr+g_SaveSendEmptyOffset;
				W25qxx_WriteByte(tempdata,tmpoffset);
			}
        }
        if(g_StatusDatainFlash.needsendcnt > 0 && g_GetMyMobileOKFlag == true)
        {
			if(SendB9DataToGsm() == true)    //发送 B9
			{
				g_B9SendFlag = 1;
				g_DtuSendOk = 2;
				g_SendB9Delay = TIM2->CNT;
			}
			else
				return true;
        }
        else if(g_StatusDatainFlash.needsendcnt == 0)
            return true;
    }
    else if(g_B9SendFlag == 1 && (g_DtuSendOk == 0 || g_DtuSendOk == 2))
    {
        if(JudgeSomeDelayInProOk(g_SendB9Delay , Delay1Min) == true)
        {
			if(SendB9DataToGsm() == true)    //重新发送 B9
			{
				g_DtuSendOk = 2;
				g_SendB9Delay = TIM2->CNT;
				ProcessDtuNoAnswerFunc();
			}
		}
    }
    return false;
}

/******************************************************************
函数名称: Bool SendF0DataToGsm(void)
功能描述: 发送F0的函数
参数输入: 无
参数输出: 
返  回  值 : 无
其       他: 无
******************************************************************/
bool SendF0DataToGsm(void)
{
    uint8_t cmpbuf[5];
	
	memset(cmpbuf,0,sizeof(cmpbuf));
	memset(g_tmpBuf,0,sizeof(g_tmpBuf));
	
    g_TxBufLP[0] = 0xaa;
	g_TxBufLP[1] = 0xbb;
	g_TxBufLP[2] = 0x62;
	g_TxBufLP[3] = 0;
    g_TxBufLP[4] = 0x87;
    g_TxBufLP[5] = 0;
    g_TxBufLP[6] = 0xd6; 
    
    if(ReadMobileNoFromFLASH() == true)
    {
        if(CheckNumIsBCD((uint8_t*)g_MobileInFlash.mobileno[3],5) == true && memcmp((uint8_t*)g_MobileInFlash.mobileno[3],cmpbuf,5)!=0)
        {
            memcpy(g_TxBufLP + 7,g_MobileInFlash.mobileno[3],6);
        }
        else
            memset(g_TxBufLP + 7,0xff,6);
    }
    else
        memset(g_TxBufLP + 7,0xff,6);
    
    g_TxBufLP[12] &= 0xf2;
//    if(ReadRecordFromFlash(&g_SensorDatainFlash,g_tmpBuf) == 1)
//    {
//        memcpy(g_TxBufLP + 13,g_tmpBuf,g_SensorDatainFlash.datalen);   
//        g_TxBufLP[141] = CheckSum(139,g_TxBufLP + 2);
//		UartSend(&g_LPUart,g_TxBufLP,142);
//		g_ShouldPowerOffDTUDelay = TIM2->CNT;
//        return true;
//    }
//    else
		return false;
}
/******************************************************************
函数名称: Bool SendF0Func(void)
功能描述: 给DTU发送F0数据
参数输入: 无
参数输出: 
返  回  值 : 无
其           他: 无
******************************************************************/
bool SendF0Func(void)
{
    uint8_t tempdata;
    uint8_t cmpbuf[5];
	uint32_t tmpoffset;

	memset(cmpbuf,0,sizeof(cmpbuf));
//    if(g_F0SendFlag == 0 || (g_F0SendFlag == 1 && g_DtuSendOk == 1 && g_DtuHaveSendDataOk == 1))
//    {
//        if(g_F0SendFlag == 1 && g_DtuSendOk == 1 && g_DtuHaveSendDataOk == 1)
//        {
//            tempdata = INDEX_SEND_SUC;
//			W25qxx_WriteByte(tempdata,g_SensorDatainFlash.readflagaddr);
//            g_SensorDatainFlash.needsendcnt--;
//			g_F0SendFlag = 0;
//			if(g_SaveSendEmptyFlag == 1)
//			{
//				g_SaveSendEmptyFlag = 0;
//				tempdata = BAG_SEND_EMPTY;
//				tmpoffset = g_SensorDatainFlash.readflagaddr+g_SaveSendEmptyOffset;
//				W25qxx_WriteByte(tempdata,tmpoffset);
//			}
//			return true;
//        }
//        if(g_SensorDatainFlash.needsendcnt > 0 && g_GetMyMobileOKFlag == true)
//        {
//			if(SendF0DataToGsm() == true)    //发送 B9
//			{
//				g_F0SendFlag = 1;
//				g_DtuSendOk = 2;
//				g_SendF0Delay = TIM2->CNT;
//			}
//			else
//				return true;
//        }
//        else if(g_SensorDatainFlash.needsendcnt == 0)
//            return true;
//    }
//    else if(g_F0SendFlag == 1 && (g_DtuSendOk == 0 || g_DtuSendOk == 2))
//    {
//        if(JudgeSomeDelayInProOk(g_SendF0Delay , Delay1Min) == true)
//        {
//			if(SendF0DataToGsm() == true)    //重新发送 B9
//			{
//				g_DtuSendOk = 2;
//				g_SendF0Delay = TIM2->CNT;
//				ProcessDtuNoAnswerFunc();
//			}
//		}
//    }
    return false;
}

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
	uint16_t len,datalen,i,k;
	uint8_t mobile_rxflag = 0,no;  //1-下发控制权限号；2-索要控制权限号
    uint8_t cmpbuf[5],tmpbuf[80];
    uint8_t tempmobile[5][6]; 
    uint8_t numberafterD6[6];
    uint8_t equalflag,exeflag = 0;
	
	if(g_LPUart.usRxCount > 0 && JudgeSomeDelayInProOk(g_LPUartRxTimer,Delay50mSec) == true)
	{
		if(g_LPUart.usRxCount <= LPUART_RX_BUF_SIZE)
		{
			len = g_LPUart.usRxCount;
			ReadRecBuf(&g_LPUart,g_RxBufLP,len);
			g_LPUart.usRxCount = 0;
			g_LPUart.usRxRead = 0;
			g_LPUart.usRxWrite = 0;
			memset(g_DMARxBufLP,0,LPUART_RX_BUF_SIZE);
		}
		else
		{
			ReadRecBuf(&g_LPUart,g_RxBufLP,LPUART_RX_BUF_SIZE);
			g_LPUart.usRxCount = 0;
			g_LPUart.usRxRead = 0;
			g_LPUart.usRxWrite = 0;
			memset(g_DMARxBufLP,0,LPUART_RX_BUF_SIZE);
		}
		g_LPUart1RxSta = 1;
	}
	if(g_LPUart1RxSta == 1)
	{
		g_LPUart1RxSta = 0;
		if(g_RxBufLP[0] == 0xAA && g_RxBufLP[1] == 0xBB)				//找包头 AA BB
		{
			datalen = g_RxBufLP[4]+(g_RxBufLP[5]<<8);
			len = datalen+7;
			if(g_RxBufLP[len-1] == CheckSum(len-3, g_RxBufLP+2))
			{
				switch( g_RxBufLP[2] )
				{		
					case 0x80:                                      /*DTU查询设备状态*/
						g_GsmOk =  (( g_RxBufLP[3] & 0x02)>>1);
						g_GprsOk = (( g_RxBufLP[3] & 0x04)>>2);
						AnalyzeDTUStatus();                         //agj 整合为一个函数
						ReturnDeviceStatusToDTU();
						break;
					case 0x81:                                      /*回复本机状态*/
						g_GsmOk =  (( g_RxBufLP[3] & 0x02)>>1);  //agj 放开注释
						g_GprsOk = (( g_RxBufLP[3] & 0x04)>>2);  //agj 放开注释
						g_DtuSendOk = (( g_RxBufLP[3] & 0x20)>>5);
						g_DtuHaveSendDataOk = (( g_RxBufLP[3] & 0x08)>>3);   //agj
						
						if(g_DtuSendOk == 1 && g_DtuHaveSendDataOk == 1)
							g_SendToDtuFailCount = 0;
						
						AnalyzeDTUStatus();                         //agj 整合为一个函数
						if((g_InNormalNetStatus == 5) &&(( g_RxBufLP[3] &0x20 ) == 0x20))
						{
							g_RemoteZCGCFlag = 1; 
							g_PageOffset++ ;
							if(g_PageOffset == g_ZC_PageLength)
							{   
								AutoPowerOffDTU();
							}
						}
						break;
					case 0x82:                                      /* 模块回复，表示接收成功 */
					{
						g_GsmOk =  (( g_RxBufLP[3] & 0x02)>>1);
						g_GprsOk = (( g_RxBufLP[3] & 0x04)>>2);
						AnalyzeDTUStatus();                         //agj 整合为一个函数
						datalen = Get16ValueFrom8Buf(g_RxBufLP,4);
						g_ShouldPowerOffDTUDelay = TIM2->CNT;
						if(g_RxBufLP[6] == 0xee && (datalen == 16)&&(g_InNormalNetStatus==2)) /* 远程升级程序 */
						{
							uint8_t tmpbuf[8];
							uint16_t pos;
							memset(tmpbuf,0xee,8);
							
							if(StrNcmpString(g_RxBufLP+6, tmpbuf, 8,&pos) == 0)
							{	
								AnswerToGsm(1);	
								memcpy((uint8_t*)&g_UpdateCodePageTotal,g_RxBufLP+14,3);
								if(g_UpdateCodePageTotal.programtype == CODETYPE)
								{
									g_UpdateCodeTypeErrorFlag = 0;
									g_RemoteZc_UpdateFlag = 2;
									g_RemoteLinkDelay = TIM2->CNT;
									g_RemoteActionPre = 1;                                  /*进入10 秒DELAY*/
									g_InNormalNetStatus = 2;
									g_RemoteDataOkTimer = DATAOKTIMERS;
									MajorVersion = g_RxBufLP[15];                        		//主版本号
									MinorVersion = g_RxBufLP[16];                        		//次版本号
									LastMajorVersion = MajorVersion;                        	//备份主版本号
									LastMinorVersion = MinorVersion;                        	//备份次版本号
									LastTotalBagNum = Get16ValueFrom8Buf(g_RxBufLP,17);  		//包总数
									NormalUpdateFileSize.FileSize = 0;
									WriteNormalFileSizeToFlash();
									DiffUpdateFileSize.FileSize = 0;
									WriteDiffFileSizeToFlash();
									g_GetUpdateTypeFlag = 1;
								}
							}	
						}
						if(g_RxBufLP[6] == 0xEB && (datalen == 16)&&(g_InNormalNetStatus==4)) /* 远程升级程序 */
						{								                                  
							memcpy((uint8_t*)&g_UpdateCodePageTotal,g_RxBufLP+13,3);							
							if(g_RxBufLP[16] == g_VersionHex[0] && g_RxBufLP[17] == g_VersionHex[1])
							{
								g_UpdateType = g_RxBufLP[18];
								g_UpdateCodePageTotal.programpage = Get16ValueFrom8Buf(g_RxBufLP,19);
								g_GetUpdateTypeFlag = 3;
							}
						}
						else if((g_RxBufLP[4] == 0x00)&&(g_RxBufLP[5] == 0x02)&&(g_RxBufLP[6] == 0xdc)&&(g_RxBufLP[7] == 0xdc))
						{
							for(i=0;i<4;i++)
							{
								if( g_RxBufLP[ 6 + i ] != 0xDC )
									break;
							}
							if(( i == 4 )&&(g_UpdateCodePageTotal.programpage == Get16ValueFrom8Buf(g_RxBufLP,16))
									&&(g_RemoteUpdatePackNo == Get16ValueFrom8Buf(g_RxBufLP,18)))
							{
								memcpy(g_UartUPBuf,g_RxBufLP+20,496);
								g_UpdateType = g_RxBufLP[10];
								if(g_UpdateType == 1)
								{
									W25q_Write_N(g_UartUPBuf,FLASH_IAP_DIFFUSER_START1+496*g_RemoteUpdatePackNo,496);
									W25qxx_ReadBytes(g_checkbuf,FLASH_IAP_DIFFUSER_START1+496*g_RemoteUpdatePackNo,496);
								}
								else
								{
									W25q_Write_N(g_UartUPBuf,FLASH_IAP_USER_START1+496*g_RemoteUpdatePackNo,496);								
									W25qxx_ReadBytes(g_checkbuf,FLASH_IAP_USER_START1+496*g_RemoteUpdatePackNo,496);	
								}
								
								if(memcmp(g_UartUPBuf,g_checkbuf,496) == 0)
									g_RemoteUpdatePackNo++;
								if( g_RemoteUpdatePackNo == g_UpdateCodePageTotal.programpage )
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
																		
									/* 升级数据发送完毕 */
									g_RemoteUpdateGCFlag = 3;   
									g_ShouldJumpToStartFlag = 1;
									AutoPowerOffDTU();
									
								}
								else
								{
									g_RemoteUpdateGCFlag=1;	// 3													
									g_InNormalNetStatus =4;
									g_RemoteLinkDelay = TIM2->CNT;
									g_RemoteOutTimer=SENDREQTIMERS;													
								}
							}
						
						}
						else
						{
							memcpy(numberafterD6,g_RxBufLP+7,6);     
							AnswerToGsm(1);
							if(CheckNumIsBCD(numberafterD6,5) == false)
							{							
								break;
							}
							else
							{
								if(memcmp(numberafterD6,TotalControlMobleNo,6) == 0)
								{
									exeflag = 1;
								}
								else if(memcmp(numberafterD6,YFTotalControlMobleNo,6) == 0)
								{
									exeflag = 1;
								}
								else if((CheckNumIsBCD(g_MobileInFlash.mobileno[0],5) == true) && (memcmp(numberafterD6,g_MobileInFlash.mobileno[0],6) == 0))
								{
									exeflag = 1;
								}
								else if((CheckNumIsBCD(g_MobileInFlash.mobileno[1],5) == true) && (memcmp(numberafterD6,g_MobileInFlash.mobileno[1],6) == 0))
								{
									exeflag = 1;
								}
								else if((CheckNumIsBCD(g_MobileInFlash.mobileno[0],5) == true) && (memcmp(numberafterD6,g_MobileInFlash.mobileno[1],6) == 0))
								{
									exeflag = 1;
								}
								else if((CheckNumIsBCD(g_MobileInFlash.mobileno[1],5) == true) && (memcmp(numberafterD6,g_MobileInFlash.mobileno[0],6) == 0))
								{
									exeflag = 1;
								}
								if(exeflag == 0)
								{
									break;
								}
							}
							memcpy(g_D6Number, numberafterD6,sizeof(g_D6Number));
							
							if(g_RxBufLP[13] == 'A' && g_RxBufLP[14] == '8' && datalen == 167)
							{
								for(i=0;i<6;i++)
								{
									if(g_RxBufLP[13+2*i] != 'A' || g_RxBufLP[14+2*i] != '8')
									{
										mobile_rxflag = 0;
										break;
									}
								}
								if(i == 6)
								{
									i = 0;
									mobile_rxflag = 1;
								}
							}
							else if(g_RxBufLP[13] == 'A' && g_RxBufLP[14] == '9' && datalen == 167)
							{
								for(i=0;i<6;i++)
								{
									if(g_RxBufLP[13+2*i] != 'A' || g_RxBufLP[14+2*i] != '9')
									{
										mobile_rxflag = 0;
										break;
									}
								}
								if(i == 6)
								{
									if(g_RxBufLP[32] == '0')       //索要内存信息
									{
										mobile_rxflag = 2;
									}
									else if(g_RxBufLP[32] == '8')  //索要控制权限号
									{
										mobile_rxflag = 3;
									}
								}        
							}
							if(mobile_rxflag == 1 && g_RemoteActionPre == 0)
							{
								/* 服务器下发控制权限号和本机号 */
								mobile_rxflag = 0;
								for(k=0;k<5;k++)
								{
									for(i=0;i<6;i++ )
									{
										tempmobile[k][i] = (Asc2Hex(g_RxBufLP[33+2*i+k*16])<<4)+Asc2Hex(g_RxBufLP[34+2*i+k*16]);
									}
								}
								ReadMobileNoFromFLASH();
								for(i=0;i<5;i++)
								{
									if(CheckNumIsBCD((uint8_t*)tempmobile[i],5) == true && memcmp((uint8_t*)tempmobile[i],cmpbuf,5)!=0)
									{
										if(i == 4)
										{
											if(0 != memcmp(g_MobileInFlash.mobileno[4],tempmobile[i],sizeof(tempmobile[i])))
											{
												equalflag = 1;
												memcpy(g_MobileInFlash.mobileno[4],tempmobile[i],sizeof(tempmobile[i]));
											}
										}
										else
											memcpy(g_MobileInFlash.mobileno[i],tempmobile[i],sizeof(tempmobile[i]));
									}
								}   
								WriteMobileNoToFLASH();
								if(ReadMobileNoFromFLASH() == true)
								{
									if(equalflag == 1)
									{
										equalflag = 0;
										g_ChangeMyMobileNumFlag = true;
									}
									g_GetMyMobileOKFlag = true;
									g_AskAuthorityFlag = 1;
									g_RemoteActionPre = 2;					 
									break;
								}
								g_AskAuthorityFlag = 0;
							}
							else if(mobile_rxflag == 3 && g_RemoteActionPre == 0)
							{
							/* 服务器请求返回控制权限和本机号 */
								mobile_rxflag = 0;
								if(ReadMobileNoFromFLASH() == true)
								{
									g_AskAuthorityFlag = 1;             
									g_RemoteActionPre = 2;
								}
							}
							else
							{
								for(i=0;i<80;i++)
								{
									tmpbuf[i] = (Asc2Hex(g_RxBufLP[13+i*2])<<4) + Asc2Hex(g_RxBufLP[14+i*2]);
								}
								if(tmpbuf[0] == 0xCF && tmpbuf[1] == 0x65 && tmpbuf[2] == 0x01)
								{
									
									
								}
							}
						}
						break;
					}
					case 0x83: /* 模块回复，表示接收失败 */										
						break;
					case 0x84: /* 模块向主板发送数据 */
						AnswerToGsm(0);
					case 0x85: /* DTU给设备返时间日期 修改*/
					{
						DATETIME tmptime;
						
						g_GsmOk =  (( g_RxBufLP[3] & 0x02)>>1);
						g_GprsOk = (( g_RxBufLP[3] & 0x04)>>2);
						AnalyzeDTUStatus();         
						g_AskDateFromDtuFlag = 1;
						TimeBcdToHex(&tmptime,g_RxBufLP+6);
						if(g_GprsOk == 1 && tmptime.year <= 99 && tmptime.month >= 1 && tmptime.month <= 12 && tmptime.date >= 1 
							&& tmptime.date <= 31 && tmptime.hour <= 23 && tmptime.minute <= 59 && tmptime.second <= 59)
						{
							g_TimeNoAdjustFlag = false;
							g_DeviceStatusFlag = 4;
							g_SysTime = tmptime;
							RTC_Set_Time(tmptime.hour,tmptime.minute,tmptime.second,RTC_HOURFORMAT_24);
							RTC_Set_Date(tmptime.year,tmptime.month,tmptime.date,1);
							if(g_InNormalNetStatus == 1)
								g_InNormalNetStatus = 2;
						}
					}
					break;	
					case 0x86:  //DTU给设备返信号强度
						g_GsmOk =  (( g_RxBufLP[3] & 0x02)>>1);
						g_GprsOk = (( g_RxBufLP[3] & 0x04)>>2);
						g_GsmEnergy = BCDToHex(g_RxBufLP[6]);            
						
						g_GetSignalFromDtuDelay = TIM2->CNT;
						g_AskSignalFromDtuFlag = 1;
				   
						AnalyzeDTUStatus();                                 
						if(g_RxBufLP[7] > 0)
						{
							if(g_DTUVersion != g_RxBufLP[7])
							{
								g_DTUVersion = g_RxBufLP[7];               //DTU的版本号
							}
						}
						break;
					case 0x87:  //DTU给设备返本机号及IP地址
					{
						uint8_t ipaddr[4];
						uint16_t port;
						uint16_t pos;

						memcpy(ipaddr,g_RxBufLP+12 ,4);
						port = Get16ValueFrom8Buf(g_RxBufLP,16);
						g_GsmOk =  (( g_RxBufLP[3] & 0x02)>>1);
						g_GprsOk = (( g_RxBufLP[3] & 0x04)>>2);
						AnalyzeDTUStatus();              
						
						if(g_DtuReturnMyMobileFlag == 0 || g_GetMyMobileOKFlag == false)        //本机号
						{
							if(CheckNumIsBCD(g_RxBufLP+6,5) == true && memcmp(g_RxBufLP+6,cmpbuf,5)!=0)
							{
								g_DtuReturnMyMobileFlag = 1;
								ReadMobileNoFromFLASH();
								if(memcmp(g_RxBufLP+6,cmpbuf,5)!=0 || (g_RxBufLP[11]&0xF0) != (g_MobileInFlash.mobileno[4][5]&0xF0))
								{
									memcpy(g_MobileInFlash.mobileno[4],g_RxBufLP+6,6);
									WriteMobileNoToFLASH();
								}
								g_GetMyMobileOKFlag = true;							
							}
							else
							{
								g_DeviceStatusFlag = 6;         //本机号不对
							}
						}
						if(g_InNormalNetStatus <= 2)
						{
							if(ipaddr[0] == 0 || ipaddr[1] == 0 || ipaddr[2] == 0 || ipaddr[3] == 0
								|| (ipaddr[0] == 0xff && ipaddr[1] == 0xff && ipaddr[2] == 0xff && ipaddr[3] == 0xff)
								|| (port == 0) || (port == 0xff))
								;
							else
							{
								if((StrNcmpString(ipaddr, g_ServerInfo.serveripaddr[2], 4 ,&pos) == 0)
								&& ((port>>8) == (g_ServerInfo.serverportaddr[2]%100)) 
								&& ((port&0xff) == g_ServerInfo.serverportaddr[2]/100) )        
								{//若是数据IP
									;
								}
								else    
								{//若返回的和主板的IP和端口不一样，更新
									if(g_RemoteZc_UpdateFlag == 0 && ReadServerInfoPar() == true)
									{
										memcpy(g_ServerInfo.serveripaddr[2],ipaddr,sizeof(g_ServerInfo.serveripaddr[2]));
										g_ServerInfo.serverportaddr[2] = (port&0xff)*100 + (port>>8);
										WriteServerInfoPar();               //写入Flash
										if(ReadServerInfoPar() == true)     //从Flash读出
										{
											CmpIPAndPort();                 //若有全零或全F的替换为默认值
										}
									}
								}
							}
						}
						else if(g_InNormalNetStatus == 3)
						{
							if((StrNcmpString(ipaddr, g_ServerInfo.serveripaddr[0], 4 ,&pos) == 0) 
								&& ((port>>8) == (g_ServerInfo.serverportaddr[0]%100)) 
								&& ((port&0xff) == g_ServerInfo.serverportaddr[0]/100) 
								&& ((g_RxBufLP[3]&0x04)==4))
							{//升级IP
								if(g_RemoteUpdateFirstFlag == 0)
								{
									g_RemoteUpdateFirstFlag = 1;
									//g_RemoteUpdatePackNo = 0;
								}
								g_InNormalNetStatus = 4;
								g_RemoteActionPre = 4;
								g_RemoteUpdateGCFlag = 1;//3                            
							}
							else if((StrNcmpString(ipaddr, g_ServerInfo.serveripaddr[1], 4,&pos ) == 0)
									&& ((port>>8) == (g_ServerInfo.serverportaddr[1]%100)) 
									&& ((port&0xff) == g_ServerInfo.serverportaddr[1]/100) 
									&& ((g_RxBufLP[3]&0x04)==4))
							{//转储IP
								g_InNormalNetStatus = 5;
								g_RemoteActionPre = 4;
								g_RemoteZCGCFlag = 1;// = 1;//3                            
							}
						}
						else if(g_InNormalNetStatus == 7)
						{
							if((StrNcmpString(ipaddr, g_ServerInfo.serveripaddr[2], 4 ,&pos) == 0)
								&& ((port>>8) == (g_ServerInfo.serverportaddr[2]%100)) 
								&& ((port&0xff) == g_ServerInfo.serverportaddr[2]/100) )
							{//若是数据IP   
							/* 恢复到原始的状态，清掉远程标志 */
								g_InNormalNetStatus = 2;
								g_RemoteActionPre =0;
								g_RemoteZCGCFlag = 0;
								g_RemoteZc_UpdateFlag = 0; 
							}
						}
					}
					break;
					default:
						break;				
				}
			}
		}
	}
	
	if(JudgeSomeDelayInProOk(g_LPUart.usTxTimer,Delay50mSec) == true && g_DTUPowerOn == 1)//上次发送已完成
	{
		if(AskTimeAndSignalWithDtu() == true)
			return;
		
		if(g_InNormalNetStatus <= 2 && (g_RemoteActionPre == 2 || g_RemoteActionPre == 0))
		{
			if(g_ChangeIPFlag == 1)                               //给DTU发送改IP
			{
				if(JudgeSomeDelayInProOk(g_GetChangeIPCmdStartTime,Delay10Sec) == true)
				{
					Send_Ip_Port(g_ServerInfo.serveripaddr[2], g_ServerInfo.serverportaddr[2],3);
					g_ChangeIPFlag = 0;
					g_RemoteActionPre = 0;
					return;
				}
			}
		}
		
		if(g_GsmOk == 1)
		{
			if(g_ChangeMyMobileNumFlag == true)                           //给DTU写本机号
			{
				if(ReadMobileNoFromFLASH() == true)
				{
					WriteMyMobileNumToDTU();
					g_ChangeMyMobileNumFlag = false;
					g_GetMyMobileOKFlag = true;  
					return;
				}
			}
			if(g_HaveSetMyMobileToDTUFlag == 0 || g_HaveSetMyMobileToDTUFlag == 2) 
				g_HaveSetMyMobileToDTUFlag = 1;
		} 
		if(g_GprsOk == 0) 
		{
			if(JudgeSomeDelayInProOk(g_GprsOffTime,Delay5Min) == true)
			{
				if(g_HaveSetMyMobileToDTUFlag == 0)
				{
					if(ReadMobileNoFromFLASH() == true)
					{
						WriteMyMobileNumToDTU();

						g_GetMyMobileOKFlag = true;
						g_HaveSetMyMobileToDTUFlag = 1;
						g_GprsOffFlag = 0;
					}
					else 
						g_HaveSetMyMobileToDTUFlag = 2;
				}
				if(g_HaveSetMyMobileToDTUFlag == 1 || g_HaveSetMyMobileToDTUFlag == 2)
				{
					if(JudgeSomeDelayInProOk(g_GprsOffTime,Delay20Min) == true)             //20分钟DTU不联网，重启DTU  
					{
						AutoPowerOffDTU();
					}
				}
			}
		}
		if(g_DtuReturnMyMobileFlag == 0 && g_GsmOk == 1)
		{
			if(g_DeviceStatusFlag < 5)                                  
			{
				AskDTUStatus(ASK_MYNUM_IP);                             //跟DTU请求本机号 
				g_AskMyMobileDelay = TIM2->CNT;
				g_AskMyMobileErrorTimes = 0;                            //错误次数=0
				g_DeviceStatusFlag = 5;                                 //置标志
				return;
			}
			else 
			{
				if(g_AskMyMobileErrorTimes < 10)
				{
					if(JudgeSomeDelayInProOk(g_AskMyMobileDelay,Delay30Sec) == true)
					{
						AskDTUStatus(ASK_MYNUM_IP);                     //跟DTU请求本机号 
						g_AskMyMobileDelay = TIM2->CNT;
						g_AskMyMobileErrorTimes++;                      //错误次数加
						return;
					}
				}
				else
				{
					if(ReadMobileNoFromFLASH() == true)
					{
						WriteMyMobileNumToDTU();                         //给DTU写本机号

						g_GetMyMobileOKFlag = true;                      //置标志
						g_AskMyMobileErrorTimes = 0;
						return;
					} 
					g_AskMyMobileErrorTimes = 0;
					return;
				}
			}
		}
		else if(g_GetMyMobileOKFlag == false && g_GsmOk == 1)            
		{
			if(JudgeSomeDelayInProOk(g_AskMyMobileDelay,Delay30Sec) == true)
			{
				AskDTUStatus(ASK_MYNUM_IP);                     //跟DTU请求本机号 
				g_AskMyMobileDelay = TIM2->CNT;
				return;
			}
		}
		if( g_InNormalNetStatus == 2 )                         /* 在通常 情况下处理  */
        {
            if(g_RemoteActionPre == 2)
            {
                if(g_RemoteZc_UpdateFlag == 0)
				{
					if(g_UpdateCodeTypeErrorFlag == 1)
					{
						NewUpdate();
						g_UpdateCodeTypeErrorFlag = 0;
						g_RemoteActionPre = 0;
					}
					else if(g_AskAuthorityFlag == 1)
					{
						ReturnMobileNotoDTU();
						g_AskAuthorityFlag = 0;
						g_RemoteActionPre = 0;
					}
					else if(g_AskPowerControlFlag == 1)
					{
						g_AskPowerControlFlag = 0;
						ReturnPowerCtlinfotoDTU();
						g_RemoteActionPre = 0;
					}
				}
					
            }
            else if(g_RemoteActionPre == 0)
            {
				if(g_F0SendFlag == 0 && g_StatusDatainFlash.needsendcnt > 0)
				{
					SendB9Func();
				}
//				else if(g_SensorDatainFlash.needsendcnt > 0)
//				{
//					SendF0Func();
//				}
            }
            else if(g_RemoteActionPre == 1)                           /* 收到 升级命令且已发送应答 */
            {
                if(JudgeSomeDelayInProOk(g_RemoteLinkDelay , Delay10Sec) == true )	
                {
                    /*  3 . */
                    if( g_RemoteDataOkTimer == 0 )
                    {	
                        if ( g_RemoteZc_UpdateFlag == 2)
                        {
                            Update_EndReviseIP();
                            g_RemoteUpdateGCFlag = 0;
                        }
                        else if( g_RemoteZc_UpdateFlag == 1)
                        {
                            ZC_EndReviseIP();
                            g_RemoteZCGCFlag = 0;
                        }
                    }
                    else
                    {
                        if ( g_RemoteZc_UpdateFlag == 2)
                            Send_Ip_Port( g_ServerInfo.serveripaddr[0] ,  g_ServerInfo.serverportaddr[0],1 );
                        else if( g_RemoteZc_UpdateFlag == 1)
                            Send_Ip_Port( g_ServerInfo.serveripaddr[1] , g_ServerInfo.serverportaddr[1] ,2 );
                        g_RemoteLinkDelay = TIM2->CNT;
                        g_RemoteActionPre = 2;                          /* 发送IP 地址后,DELAY 1 minute */
                        g_RemoteReviseIPTimer =SETIPTIMERS;             /* 发送开始升级的次数最多为0 */
                        g_InNormalNetStatus = 3;                        /* 远程后, 未知 */
                        g_DtuSendOk = 2;
                        g_RemoteDataOkTimer--;                          
                    }
                }
            }		
        }
        else if(g_InNormalNetStatus == 3)
        {
        /* 4 . */
            if((JudgeSomeDelayInProOk(g_RemoteLinkDelay , Delay1Min) == true)
                &&(g_RemoteActionPre == 2))                             /*  1 分DELAY OK  */
            {
                g_RemoteAskNetTimer = ASKSTATUSTIMES;
                g_RemoteLinkDelay = TIM2->CNT;
                g_RemoteActionPre = 3;                                  /* 发送访问IP 地址等信息DELAY 5 Seconds */
                AskDTUStatus(ASK_MYNUM_IP);
            }
            else if((JudgeSomeDelayInProOk(g_RemoteLinkDelay , Delay5Sec) == true)	
                &&(g_RemoteActionPre == 3))                             /*  5s DELAY OK  */
            {
                g_RemoteLinkDelay = TIM2->CNT;
                if(g_RemoteAskNetTimer == 0)
                {
                    if(g_RemoteReviseIPTimer == 0)
                    {
                        /* 返回步骤 2 */
                        g_InNormalNetStatus = 2;
                        g_RemoteActionPre = 1;
                        g_RemoteLinkDelay = TIM2->CNT;
                    }
                    else
                    {
                        if ( g_RemoteZc_UpdateFlag == 2)
                            Send_Ip_Port( g_ServerInfo.serveripaddr[0] ,  g_ServerInfo.serverportaddr[0],1 );
                        else if( g_RemoteZc_UpdateFlag == 1)
                            Send_Ip_Port( g_ServerInfo.serveripaddr[1],  g_ServerInfo.serverportaddr[1],2 );
                        g_RemoteReviseIPTimer --;                       
                        g_RemoteLinkDelay = TIM2->CNT;
                        g_RemoteActionPre = 2;                          /* 发送IP 地址后,DELAY 1  */
                        g_RemoteAskNetTimer = ASKSTATUSTIMES;
                    }					
                }
                else
                {
                    g_RemoteAskNetTimer--;
                    AskDTUStatus(ASK_MYNUM_IP);
                }
            }
        }		
        /* 5 . */
        else if(g_InNormalNetStatus == 4)
        {/* 网络连接成功 升级 */
            if( g_RemoteUpdateGCFlag == 1 )
            {
				if(g_GetUpdateTypeFlag == 1)
				{
					GetUpdateType();
					g_GetUpdateTypeFlag = 2;
				}
				else if(g_GetUpdateTypeFlag == 3)
					NewUpdate();
                g_RemoteLinkDelay = TIM2->CNT;
                g_RemoteOutTimer = SENDREQTIMERS ;
                g_RemoteUpdateGCFlag = 2;
            }
            else if(g_RemoteUpdateGCFlag == 2)
            {
                if(JudgeSomeDelayInProOk(g_RemoteLinkDelay , Delay20Sec ) == true)
                {
                    g_RemoteLinkDelay = TIM2->CNT;
                    if( g_RemoteOutTimer == 0 )
                    {
                        /* 返回步骤 2 */
                        g_InNormalNetStatus = 2;
                        g_RemoteActionPre = 1;
                        g_RemoteUpdateGCFlag = 0;
                    }
                    else  
                    {
                        g_RemoteOutTimer--;
						if(g_GetUpdateTypeFlag == 2)
							GetUpdateType();
						else if(g_GetUpdateTypeFlag == 3)
							NewUpdate();
                    }
                }
            }
        }
        else if(g_InNormalNetStatus == 5)
        {/* 网络连接成功  转储*/
            if( g_RemoteZCGCFlag == 1 )
            {
                Send_ZhuanChuData();
                g_RemoteLinkDelay = TIM2->CNT;
                g_RemoteOutTimer = SENDREQTIMERS ;
                g_RemoteZCGCFlag = 2;
            }
            else if(g_RemoteZCGCFlag == 2)
            {
                if(JudgeSomeDelayInProOk(g_RemoteLinkDelay , Delay20Sec ) == true)
                {
                    g_RemoteLinkDelay = TIM2->CNT;
                    if( g_RemoteOutTimer == 0 )
                    {
                        /* 返回步骤 2 */
                        g_InNormalNetStatus = 2;
                        g_RemoteActionPre = 1;
                        g_RemoteZCGCFlag = 0;
                    }
                    else  
                    {
                        g_RemoteOutTimer--;
                        Send_ZhuanChuData();
                    }
                }
            }
        }
        /* 6 */
        else if((g_InNormalNetStatus == 6)&&(JudgeSomeDelayInProOk(g_RemoteLinkDelay , Delay1Min) == true))
        {//发送0x65
            AskDTUStatus(ASK_MYNUM_IP);
            g_RemoteAskNetTimer = ASKSTATUSTIMES;
            g_InNormalNetStatus = 7;
            g_RemoteActionPre = 6;
        }
        /* 7 */
        else if((g_InNormalNetStatus == 7)&&(JudgeSomeDelayInProOk(g_RemoteLinkDelay , Delay5Sec) == true))
        {	
            g_RemoteLinkDelay = TIM2->CNT;
            if(g_RemoteAskNetTimer == 0)
            {
                if(g_RemoteReviseIPTimer)
                {
                    Send_Ip_Port(g_ServerInfo.serveripaddr[2],g_ServerInfo.serverportaddr[2],0);
                    g_RemoteReviseIPTimer--;
                    g_InNormalNetStatus = 6; 
                    g_RemoteActionPre = 5;              
                    g_RemoteLinkDelay = TIM2->CNT; 
                }
                else
                {//重启DTU电源
                    AutoPowerOffDTU();
                }
                    
            }
            else
            {
                g_RemoteAskNetTimer--;
                AskDTUStatus(ASK_MYNUM_IP);
            }
        }

	}	
	
}






