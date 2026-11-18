#include "bsp.h"
#include "main.h"
#include "Update.h"

/* 收到命令的标志位，是程序升级还是数据转储 =0 什么也不是， = 1 程序升级 =2  数据转储 */
uint8_t g_ToolCommandFlag;
uint8_t g_ToolBaowenFlag;        /* =0 结束或者没有开始，=1 收到头，=2 收到中间*/
uint32_t g_ToolRecTimer;         /* 本地升级数据定时75MS/本地转储定时1Min收不到数据的定时器，重发*/
uint8_t g_ToolResendTimers;
uint8_t g_ToolResendFlag;
/*-----本地数据转储-----------------------------------------------------*/
/*=0 初始值，收到0xe0  =1 ，收到0xe1 =2 ,收到页号= 3 ，收到0xe3 = 4 删除雨量数据 */
uint8_t g_LocalZCFlag;
uint8_t g_LocalZCStartFlashPage;  /* 每64K 数据中页的序号 */
uint8_t g_LocalZCTotal64kPage;    /* 共有多少个64K 页 */
uint8_t g_LocalZCRec64KPageNo;    /* 数据转储包的页号 */
uint8_t g_LocalZC64kSum;          /* 每次发送数据进行计算的校验和 */
uint32_t g_LocalZCRecTimer;		 // /*本地转储定时1Min收不到数据时，重新发送*/

/*-----本地程序升级-----------------------------------------------------*/
/* =0 初始状态，= 1 收到客服工具下发的命令，=2 收到客服下发的升级数据包, =3 发送了数据等待下一个命令*/
uint8_t g_LocalUpdateFlag;
uint16_t g_LocalUpdateQuestPageNo;	/* 主板请求数据包的序号，从0开始 */
uint8_t g_ToolCodeTypeErrorFlag;		//程序程序类型错误标记
uint8_t 	g_ShouldJumpToStartFlag;
/**
 * @brief	初始化客服工具相关变量
 *
 * @param	
 *
 * @return  void
 */
void InitToolVar(void)
{
	g_ToolCommandFlag = 0;
	g_ToolBaowenFlag = 0;
	g_LocalZCFlag = 0;

	g_LocalZCStartFlashPage = 0;/* 每64K 数据中页的序号 */
	g_LocalZCTotal64kPage = 64; /* 共有多少个64K 页 */
	g_LocalZCRec64KPageNo =0;  /* 收到64K 页的序号 */

	g_LocalUpdateFlag = 0;
	g_ToolRecTimer = 0;
	
}

/******************************************************************
函数名称: void Local_Update_Quest(void)
功能描述: 通过客服工具升级程序的应答包
参数输入: 无
参数输出: 
返  回  值 : 无
其           他: 无
******************************************************************/
void Local_Update_Quest(void )
{
	memset(g_TxBuf1,0,sizeof(g_TxBuf1));
	
	g_TxBuf1[0]=0xEE;
	g_TxBuf1[1] = g_MobileInFlash.mobileno[4][0];
	g_TxBuf1[2] = g_MobileInFlash.mobileno[4][1];
	g_TxBuf1[3] = g_MobileInFlash.mobileno[4][2];
	g_TxBuf1[4] = g_MobileInFlash.mobileno[4][3];
	g_TxBuf1[5] = g_MobileInFlash.mobileno[4][4];
	g_TxBuf1[6] = g_MobileInFlash.mobileno[4][5];	
	g_TxBuf1[7] = g_UpdateCodePageTotal.programtype;
	g_TxBuf1[8] = g_UpdateCodePageTotal.mainverno;
	g_TxBuf1[9] = g_UpdateCodePageTotal.secverno;
    g_TxBuf1[10] = ( g_UpdateCodePageTotal.programpage & 0xFF );
	g_TxBuf1[11] = ( g_UpdateCodePageTotal.programpage>>8);
	
	if( g_LocalUpdateQuestPageNo == g_UpdateCodePageTotal.programpage || g_ToolCodeTypeErrorFlag == 1)
	{
		g_TxBuf1[12] = 0xEE;
		g_TxBuf1[13] = 0xEE;
		g_LocalUpdateFlag = 0;
        if(g_ToolCodeTypeErrorFlag != 1)
		{
			if( g_UpdateVoiceTypeFlag == 1)
			{
				VoiceFileSize.FileSize = g_UpdateCodePageTotal.programpage;								
				WriteVoiceFileSizeToFlash();
				g_UpdateVoiceTypeFlag = 0;				
			}
			else 
			{						
				NormalUpdateFileSize.FileSize = g_UpdateCodePageTotal.programpage;
				WriteNormalFileSizeToFlash();
				g_ShouldJumpToStartFlag = 2;
			}
		}
		g_UpFlag = 0;
		g_ToolCodeTypeErrorFlag = 0;	
	}
	else
	{
		g_TxBuf1[12] = ( g_LocalUpdateQuestPageNo & 0xff ) ;
		g_TxBuf1[13] = (( g_LocalUpdateQuestPageNo>>8) & 0xff ) ;
		g_LocalUpdateFlag = 3;
	}
	g_TxBuf1[14] = 0;	
	g_TxBuf1[15] = CheckSum(15*2, (uint8_t*)&g_TxBuf1);
	UartSend(&g_Uart1,(uint8_t*)&g_TxBuf1,16);
}

/******************************************************************
函数名称: void ReturnParamInforToTool(void)
功能描述: 给客服工具返回参数信息
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReturnParamInforToTool(void)
{
	uint8_t i;
    
    memset(g_TxBuf1,0,sizeof(g_TxBuf1));
    
    g_TxBuf1[0] = SENDTAG_ALL;
    g_TxBuf1[1] = GET_PARAM_INFOR_TYPE;
    g_TxBuf1[2] = 64;
//	g_TxBuf1[3] = g_Signal;    					//DTU信号强度
	g_TxBuf1[4] = gprs_link_status;							//GPRS连接状态
    TimeHexToBcd(&g_BCDTime,(uint8_t*)&g_SysTime);
	g_TxBuf1[5] = g_BCDTime.year;
	g_TxBuf1[6] = g_BCDTime.month;
	g_TxBuf1[7] = g_BCDTime.date;
	g_TxBuf1[8] = g_BCDTime.hour;
	g_TxBuf1[9] = g_BCDTime.minute;
	g_TxBuf1[10] = g_BCDTime.second;	
	g_TxBuf1[11] = g_ServerInfo.serveripaddr[2][0];							//数据IP
	g_TxBuf1[12] = g_ServerInfo.serveripaddr[2][1];	
	g_TxBuf1[13] = g_ServerInfo.serveripaddr[2][2];	
	g_TxBuf1[14] = g_ServerInfo.serveripaddr[2][3];	
	g_TxBuf1[15] = (uint8_t)(g_ServerInfo.serverportaddr[2]/100);              //数据端口
	g_TxBuf1[16] = (uint8_t)(g_ServerInfo.serverportaddr[2]%100);
	g_TxBuf1[17] = g_ServerInfo.serveripaddr[0][0];                           //升级IP
	g_TxBuf1[18] = g_ServerInfo.serveripaddr[0][1];	
	g_TxBuf1[19] = g_ServerInfo.serveripaddr[0][2];	
	g_TxBuf1[20] = g_ServerInfo.serveripaddr[0][3];	
	g_TxBuf1[21] = (uint8_t)(g_ServerInfo.serverportaddr[0]/100);               //升级端口
	g_TxBuf1[22] = (uint8_t)(g_ServerInfo.serverportaddr[0]%100);
	g_TxBuf1[23] = g_ServerInfo.serveripaddr[1][0];	                           //转储IP
	g_TxBuf1[24] = g_ServerInfo.serveripaddr[1][1];	
	g_TxBuf1[25] = g_ServerInfo.serveripaddr[1][2];	
	g_TxBuf1[26] = g_ServerInfo.serveripaddr[1][3];	
	g_TxBuf1[27] = (uint8_t)(g_ServerInfo.serverportaddr[1]/100);                //转储端口
	g_TxBuf1[28] = (uint8_t)(g_ServerInfo.serverportaddr[1]%100);
	
	for(i=0;i<5;i++)
	{
		g_TxBuf1[29+6*i] = g_MobileInFlash.mobileno[i][0];
		g_TxBuf1[30+6*i] = g_MobileInFlash.mobileno[i][1];
		g_TxBuf1[31+6*i] = g_MobileInFlash.mobileno[i][2];
		g_TxBuf1[32+6*i] = g_MobileInFlash.mobileno[i][3];
		g_TxBuf1[33+6*i] = g_MobileInFlash.mobileno[i][4];
		g_TxBuf1[34+6*i] = g_MobileInFlash.mobileno[i][5];
	}	
	
	g_TxBuf1[59] = g_VersionAscii[0];
	g_TxBuf1[60] = g_VersionAscii[1];
	g_TxBuf1[61] = g_VersionAscii[2];
	g_TxBuf1[62] = g_VersionAscii[3];
	g_TxBuf1[63] = 0;
	
    g_TxBuf1[67] = CheckSum(67*2,(uint8_t*)&g_TxBuf1);
	UartSend(&g_Uart1,(uint8_t*)&g_TxBuf1,68);
}


/******************************************************************
函数名称: void WriteMyNumberFromTool(uint8_t *buf)
功能描述: 用客服工具给主板写本机号
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void WriteMyNumberFromTool(uint8_t *buf)
{
    uint8_t tempmobile[6];
    uint8_t cmpbuf[5];

	memset(cmpbuf,0,sizeof(cmpbuf));
    memcpy(tempmobile, buf+3, 6);
    if(CheckNumIsBCD((uint8_t*)tempmobile,5) == true && memcmp((uint8_t*)tempmobile,cmpbuf,5)!=0)
    {
        if(0 != memcmp(g_MobileInFlash.mobileno[4],tempmobile,sizeof(tempmobile)))
        {
            memcpy(g_MobileInFlash.mobileno[4],tempmobile,sizeof(tempmobile));
        }
    }
	WriteMobileNoToFLASH();
	if(ReadMobileNoFromFLASH() == true)
	{
		g_GetMyMobileOKFlag = true;
		g_NeedReInitDTU = 1;
	} 	
}

/******************************************************************
函数名称: void ReturnMyMobileToTool(void)
功能描述: 给客服工具返回本机号
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReturnMyMobileToTool(void)
{    
    memset(g_TxBuf1,0,sizeof(g_TxBuf1));
    g_TxBuf1[0] = SENDTAG_ALL;
    g_TxBuf1[1] = GET_MY_NUMBER_TYPE;
    g_TxBuf1[2] = 12;
    if(ReadMobileNoFromFLASH() == true)
    {
		g_TxBuf1[3] = g_MobileInFlash.mobileno[4][0];
		g_TxBuf1[4] = g_MobileInFlash.mobileno[4][1];
		g_TxBuf1[5] = g_MobileInFlash.mobileno[4][2];
		g_TxBuf1[6] = g_MobileInFlash.mobileno[4][3];
		g_TxBuf1[7] = g_MobileInFlash.mobileno[4][4];
		g_TxBuf1[8] = g_MobileInFlash.mobileno[4][5];
    }
    else
        memset(g_TxBuf1+3,0xff,12);
    g_TxBuf1[15] = CheckSum(15*2,(uint8_t*)&g_TxBuf1);
	UartSend(&g_Uart1,(uint8_t*)&g_TxBuf1,16);
}

/******************************************************************
函数名称: void WriteIPFromTool(uint8_t *buf)
功能描述: 用客服工具给主板设置IP
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void WriteIPFromTool(uint8_t *buf)
{
    uint8_t tempmobile[3][6];
    uint8_t j,k;
    
    memcpy(tempmobile,buf+3,sizeof(tempmobile));
    j = 0;
    ReadServerInfoPar();                                                //读Flash

    for( k = 0 ; k < 3 ; k++ )
    {
        if((tempmobile[k][0] == 0 || tempmobile[k][1] == 0 || tempmobile[k][2] == 0 || tempmobile[k][3] == 0) || (tempmobile[k][4] == 0 && tempmobile[k][5] == 0)||
            (tempmobile[k][0] == 0xff || tempmobile[k][1] == 0xff || tempmobile[k][2] == 0xff || tempmobile[k][3] == 0xff) || tempmobile[k][4] == 0xff || tempmobile[k][5] == 0xff)
            ;
        else
        {
            j = 1;
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
        }
    }
    if(j == 1)
    {
		WriteServerInfoPar();
		ReadServerInfoPar();                             //从Flash读出

		CmpIPAndPort();                                         //若有全零或全F的替换为默认值
		if((memcmp(g_DataIPBack,g_ServerInfo.serveripaddr[2],4) != 0) || (g_DataPortBack != g_ServerInfo.serverportaddr[2]))  //若数据IP修改了
		{
			g_NeedReInitDTU = 1;//重连服务器				
		}
    }
}    
/******************************************************************
函数名称: void ReturnIPToTool(void)
功能描述: 给客服工具返回IP
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReturnIPToTool(void)
{
	uint8_t i;
	
    memset(g_TxBuf1,0,sizeof(g_TxBuf1));
    g_TxBuf1[0] = SENDTAG_ALL;
    g_TxBuf1[1] = GET_IP_TYPE;
    g_TxBuf1[2] = 28;
	for(i=0;i<4;i++)
	{
		g_TxBuf1[3+i] = g_ServerInfo.serveripaddr[2][i];				//数据IP
	}
    g_TxBuf1[7] = (uint8_t)(g_ServerInfo.serverportaddr[2]/100);         //数据端口
    g_TxBuf1[8] = (uint8_t)(g_ServerInfo.serverportaddr[2]%100);
    for(i=0;i<4;i++)
	{
		g_TxBuf1[9+i] = g_ServerInfo.serveripaddr[0][i];				//升级IP
	}
    g_TxBuf1[13] = (uint8_t)(g_ServerInfo.serverportaddr[0]/100);        //升级端口
    g_TxBuf1[14] = (uint8_t)(g_ServerInfo.serverportaddr[0]%100);
    for(i=0;i<4;i++)
	{
		g_TxBuf1[15+i] = g_ServerInfo.serveripaddr[1][i];				//转储IP
	}
    g_TxBuf1[19] = (uint8_t)(g_ServerInfo.serverportaddr[1]/100);        //转储端口
    g_TxBuf1[20] = (uint8_t)(g_ServerInfo.serverportaddr[1]%100);
    g_TxBuf1[31] = CheckSum(31*2,(uint8_t*)&g_TxBuf1);
    
	UartSend(&g_Uart1,(uint8_t*)&g_TxBuf1,32);
}


/******************************************************************
函数名称: void ProcessDataWithTool(void)
功能描述: 处理和客服工具的通信
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ProcessDataWithTool(uint8_t *indata, uint8_t inlen)
{	
    switch(indata[0])                           //判断包头
    {
        case RECEIVETAG_ALL:                    //若是通用包头
        {
            switch(indata[1])
            {               
				case SET_MY_NUMBER_TYPE:                //设置本机号
                    WriteMyNumberFromTool(indata);
                    break;
                
                case GET_MY_NUMBER_TYPE:                //索要本机号
                    ReturnMyMobileToTool();
                    break;
                
                case SET_IP_TYPE:                       //设置IP
                    WriteIPFromTool(indata);
                    break;
                
                case GET_IP_TYPE:                       //索要IP
                    ReturnIPToTool();
                    break;
				
                case GET_PARAM_INFOR_TYPE:              //索要参数信息
                    ReturnParamInforToTool();
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

/**
 * @brief	客服工具通信
 *
 * @param	
 *
 * @return  void
 */
void ToolProcess(void)
{
	uint16_t i,j,pos,len;
	uint8_t UartUPBuf[512];
	uint8_t checkbuf[512];
	
	if(g_Uart1.usRxCount > 0 && JudgeSomeDelayInProOk(g_Uart1RxTimer,Delay50mSec))
	{
		len = g_Uart1.usRxCount;
		
		if(g_RxBuf1[0] == 0xEE && g_RxBuf1[1] == 0xEE && g_RxBuf1[15] == CheckSum(15,g_RxBuf1))
		{
			memcpy((uint8_t*)&g_UpdateCodePageTotal,g_RxBuf1+8,3);
			
			g_UpdateCodePageTotal.programpage = Get16ValueFrom8Buf(g_RxBuf1,11);

			if(g_UpdateCodePageTotal.programtype == CODETYPE)
			{
				g_ToolCodeTypeErrorFlag = 0;
				MajorVersion = g_RxBuf1[9];  
				MinorVersion = g_RxBuf1[10];
				j = (g_UpdateCodePageTotal.programpage *496)/w25qxx.SectorSize+1;
				for(i=0;i<j;i++)
				{
					W25qxx_EraseSector(FLASH_IAP_USER_START1/w25qxx.SectorSize+i);
				}
				g_LocalUpdateFlag = 1;					
				g_LocalUpdateQuestPageNo = 0;
				g_ToolCommandFlag = 1;
				g_ToolBaowenFlag = 1;
				NormalUpdateFileSize.FileSize = 0;
				WriteNormalFileSizeToFlash();
				g_UpFlag = 1;
			}
			else if(g_UpdateCodePageTotal.programtype == VOICECODETYPE && g_ShouldUpdateVoiceFlag == 0)
			{
				g_ToolCodeTypeErrorFlag = 0;				
				g_LocalUpdateFlag = 1;					
				g_LocalUpdateQuestPageNo = 0;
				g_ToolCommandFlag = 1;
				g_ToolBaowenFlag =1;															
				MajorVersion = g_RxBuf1[9];  
				MinorVersion = g_RxBuf1[10];
				j = (g_UpdateCodePageTotal.programpage *496)/w25qxx.SectorSize+1;
				g_UpdateVoiceTypeFlag = 1;
				for(i=0;i<j;i++)
				{      
					W25qxx_EraseSector(FLASH_VOICE_START/w25qxx.SectorSize+i);
				}
				VoiceFileSize.FileSize = 0;
				WriteVoiceFileSizeToFlash();
				g_UpFlag = 1;				
			}									
			else
			{
				g_ToolCodeTypeErrorFlag = 1;
				g_LocalUpdateFlag = 1;
				g_ToolCommandFlag = 1;
				g_ToolBaowenFlag = 1;
			}			
		}
		else if(g_RxBuf1[0] == 0xDC && g_RxBuf1[1] == 0xDC && g_RxBuf1[511] == CheckSum(511,g_RxBuf1))
		{
			if((StrNcmpString(g_RxBuf1+7,(uint8_t*)&g_UpdateCodePageTotal,3,&pos) == 0)
				&& (g_RxBuf1[10] == (g_UpdateCodePageTotal.programpage&0xFF)) 
				&& g_RxBuf1[11] == (g_UpdateCodePageTotal.programpage>>8)
				&&( Get16ValueFrom8Buf( g_RxBuf1 , 12 ) == g_LocalUpdateQuestPageNo))					
			{
				memcpy(UartUPBuf,g_RxBuf1+14,496);

				if(g_UpdateVoiceTypeFlag == 1)
				{
					W25q_Write_N(UartUPBuf,FLASH_VOICE_START+496*g_LocalUpdateQuestPageNo,496);								
					W25qxx_ReadBytes(checkbuf,FLASH_VOICE_START+496*g_LocalUpdateQuestPageNo,496);									
				}					
				else
				{
					W25q_Write_N(UartUPBuf,FLASH_IAP_USER_START1+496*g_LocalUpdateQuestPageNo,496);								
					W25qxx_ReadBytes(checkbuf,FLASH_IAP_USER_START1+496*g_LocalUpdateQuestPageNo,496);	
				}
				if(memcmp(UartUPBuf,checkbuf,496) == 0)
					g_LocalUpdateQuestPageNo++;	
				g_LocalUpdateFlag = 2;
				g_ToolBaowenFlag = 1;	
				g_ToolCommandFlag = 1;
				g_ToolResendFlag = 0;
			}
		}
		else if(g_RxBuf1[0] == RECEIVETAG_ALL)
		{
			ProcessDataWithTool(g_RxBuf1,len);
		}
		g_Uart1.usRxCount = 0;		
	}
	
	//发送处理
	if(JudgeSomeDelayInProOk(g_Uart1.usTxTimer,Delay300mSec) == true && g_Uart1DirFlag == 1)
	{
		if(g_ToolBaowenFlag == 1)
		{
			if(g_ToolCommandFlag == 1)		//升级
			{
				if(g_LocalUpdateFlag == 1 || g_LocalUpdateFlag == 2)
				{
					g_LocalUpdateFlag = 0;
					Local_Update_Quest();
					if(g_ShouldJumpToStartFlag == 0)
					{
						g_ToolResendTimers = 20;
						g_ToolResendFlag = 1;
						g_ToolRecTimer = TIM2->CNT;
					}
				}
				else if(g_ToolCodeTypeErrorFlag == 1)
				{
					Local_Update_Quest();
					g_ToolCommandFlag = 0;
				}
			}
			
			g_ToolBaowenFlag = 0;
		}
		else if(g_ShouldJumpToStartFlag == 2)
		{
			g_ShouldJumpToStartFlag = 0;
			SoftReset();
		}
		
		//未收到升级包重新请求
		if(g_ToolCommandFlag == 1 && g_ToolResendFlag == 1 && g_ToolResendTimers!=0
			 &&g_LocalUpdateFlag != 0 && JudgeSomeDelayInProOk(g_ToolRecTimer,Delay100mSec) == true)
		{
			g_LocalUpdateFlag = 0;
			Local_Update_Quest();
			g_ToolResendTimers--;
			g_ToolRecTimer = TIM2->CNT;
		}
	}
}
