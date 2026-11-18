#include "bsp.h"
#include "main.h"
#include "app_air780ep.h"



/* 4G  */


//uint8_t SendBuff[2048];
uint8_t  GsmReceBuff[MAX_GSM_LEN]; 	/* 手机模块接收缓冲 */
//uint8_t  GsmSendBuff[560];  	    /* 像手机模块发送数据的缓冲器 */
uint8_t *pnt2,*pnt1;
bool bGsmAckOk;					          /* =1:收到手机模块应答 */ 
bool bSendOnlinePak;    		          /* =1:给服务器发送上线包 */
_MOBILE_SIM g_Mobile_Sim;

uint8_t TransComm = 0 ;  //开启透明传输模式  = 1 透传 = 0非透传
uint8_t SendGprsNum;				      /* 发送GPRS数据允许次数 */
uint8_t AckErrNum = 0 ;  /* 未收到应答次数 */
uint8_t current_step = 0x10;
uint32_t step_timer = 0;
uint32_t wait_timer = 0;
uint8_t gprs_link_status = 0;
uint8_t  signal = 0;                    /*信号强度*/
uint8_t Flyflag = 0;
uint8_t  diffflag = 0;
uint8_t noanswernum = 0;
uint8_t	firstsend = 0;
uint8_t sendonline = 0;
void GPRS_Connection_Handler(void)
{
	if(TYPE_4G==0)
	{
//		  switch(current_step)
//		{
//			case 0x10:  // 开机流程
//				HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_RESET);
//				HAL_GPIO_WritePin(POWERKEY_PORT, POWERKEY_PIN, GPIO_PIN_SET);
//				step_timer = TIM6->CNT;
//				current_step = 0x11;
//				break;
//	
//			case 0x11:  // AT指令检测
//				if(JudgeSomeDelayInProOk(step_timer,Delay2Sec)) 
//				{
//					HAL_GPIO_WritePin(POWERKEY_PORT, POWERKEY_PIN, GPIO_PIN_RESET);
//					step_timer = TIM6->CNT;
//					wait_timer = TIM6->CNT;
//					current_step = 0x12;
//					at_tx(ATE);
//				}
//				break;
//			case 0x12:  
//				if(JudgeSomeDelayInProOk(wait_timer,Delay2Sec*45)) // 90秒超时
//				{ 
//					current_step = 0x10;
//				} 
//				else if(JudgeSomeDelayInProOk(step_timer,Delay3Sec))
//				{
//					at_tx(AT);
//					current_step = 0x13;
//					step_timer = TIM6->CNT;
//					wait_timer = TIM6->CNT;
//				}
//				break;
//			case 0x13: 
//				if(JudgeSomeDelayInProOk(wait_timer,Delay3Sec))
//				{
//					inclson(_ok);
//					if(bGsmAckOk) 
//					{
//						current_step = 0x14;
//					}
//					else
//					{
//						current_step = 0x12;
//					}
//					
//				}
//				break;
//			case 0x14:  // SIM卡检测
//				if(JudgeSomeDelayInProOk(step_timer,Delay500mSec))
//				{
//					at_tx(cpin);
//					step_timer = TIM6->CNT;
//					current_step = 0x15;
//					wait_timer = TIM6->CNT;
//				}
//				
//				break;
//			case 0x15: 
//				inclson(cpin_ready);
//				if(bGsmAckOk) 
//				{
//					current_step = 0x17;
//					AckErrNum = 0;
//				} 
//				else 
//				{
//					if(JudgeSomeDelayInProOk(wait_timer,Delay3Sec) == true)
//					{
//						AckErrNum = 0;
//						step_timer = TIM6->CNT;
//						current_step = 0x16;
//					}
//					else if(JudgeMilSecondDelayInPro(wait_timer,Delay2Sec) == true)
//					{
//						if(JudgeSomeDelayInProOk(step_timer,Delay500mSec))
//						{
//							AckErrNum++;
//							current_step = 0x14;
//						}
//					}
//					
//				}	
//					
//				
//				break;
//			case 0x16: //进入飞行模式
//				if(Flyflag == 0)
//				{
//					at_tx(cfun0);
//					step_timer = TIM6->CNT;
//					Flyflag = 1;
//					
//				}
//				if(JudgeSomeDelayInProOk(step_timer,Delay5Sec))
//				{
//					at_tx(cfun1);
//					current_step = 0x14;
//					Flyflag = 0;
//				}
//				break;
//			case 0x17:  // 信号强度检测 一般信号强度 18 以上才能稳定通讯；11-17 能连接，会有断网的可能，10 以下基本连接不上，在不能联网的情况下，可以先参考一下信号强度如何。	
//				if(JudgeSomeDelayInProOk(step_timer,Delay500mSec))	
//				{
//					at_tx(csq);
//					step_timer = TIM6->CNT;
//					current_step = 0x18;
//				}
//				break;
//			case 0x18:
//				CSQ_BackInfo();
//				if(signal >= 10) // 有效信号值
//				{ 
//					current_step = 0x19;
//					AckErrNum = 0;
//				} 
//				else 
//				{
//					if(AckErrNum>60)
//					{
//						AckErrNum = 0;
//						current_step = 0x16;
//					}
//					else
//					{
//						if(JudgeSomeDelayInProOk(step_timer,Delay2Sec))  //若没收到正确回复500ms重发
//						{
//							AckErrNum++;
//							current_step = 0x17;
//						}
//					}
//				}
//				break;
//			case 0x19:  // 网络注册
//				if(JudgeSomeDelayInProOk(step_timer,Delay500mSec))	
//				{
//					at_tx(cgatt);
//					step_timer = TIM6->CNT;
//					current_step = 0x1A;
//				}
//				break;
//			case 0x1A:
//				if(CGATT_BackInfo())
//				{
//					current_step = 0x1B;
//					AckErrNum = 0;
//				}
//				else  
//				{
//					if(AckErrNum>60)
//					{
//						AckErrNum = 0;
//						current_step = 0x16;
//					}
//					else
//					{
//						if(JudgeSomeDelayInProOk(step_timer,Delay2Sec))	//若没收到正确回复500ms重发
//						{
//							AckErrNum++;
//							current_step = 0x19;
//						}
//					}
//				}
//				break;
//			case 0x1B:  // 关闭移动网络
//				if(JudgeSomeDelayInProOk(step_timer,Delay500mSec))
//				{
//					at_tx(cipshut);
//					step_timer = TIM6->CNT;
//					current_step = 0x1C;
//					
//				}
//				break;
//			case 0x1C:
//				inclson(_cipshut);
//				if(bGsmAckOk) 
//				{	
//					current_step = 0x1E;
//					step_timer = TIM6->CNT;
//				} 
//				else
//				{
//					if(JudgeSomeDelayInProOk(step_timer,Delay2Sec))
//					{
//						current_step = 0x16;
//					}
//				}
//				break;
//			case 0x1E://强制等待2S确保关闭
//				if(JudgeSomeDelayInProOk(step_timer,Delay2Sec))
//				{
//					current_step = 0x1F;
//				}
//				break;
//			case 0x1F: //单链接
//				if(JudgeSomeDelayInProOk(step_timer,Delay500mSec))
//				{
//					at_tx(cipmux);  
//					step_timer = TIM6->CNT;
//					current_step = 0x20;
//				}
//				break;
//			case 0x20:
//				inclson(_ok);
//				if(bGsmAckOk)
//				{
//					current_step = 0x21;
//				}
//				else
//				{
//					if (JudgeSomeDelayInProOk(step_timer , Delay3Sec) == true)
//					{
//						current_step = 0x1C;
//					}
//				}
//				break;
//			case 0x21:  //开启透传
//				if(JudgeSomeDelayInProOk(step_timer,Delay500mSec))
//				{
//					at_tx(cipmode);  
//					step_timer = TIM6->CNT;
//					current_step = 0x22;
//				}
//				break;
//			case 0x22:
//				inclson(_ok);
//				if(bGsmAckOk)
//				{
//					current_step = 0x23;
//					TransComm = 1 ;
//				}
//				else
//				{
//					if (JudgeSomeDelayInProOk(step_timer , Delay3Sec) == true)
//					{
//						current_step = 0x1B;
//					}
//				}
//				break;
//			case 0x23:  // APN配置
//				if(JudgeSomeDelayInProOk(step_timer,Delay500mSec))
//				{
//					at_tx(cstt);
//					step_timer = TIM6->CNT;
//					current_step = 0x24;
//				}
//				break;
//			case 0x24:
//				inclson(_ok);
//				if(bGsmAckOk) 
//				{
//					current_step = 0x25;
//				} 
//				else 
//				{
//					if(JudgeSomeDelayInProOk(step_timer,Delay3Sec))
//					{
//						current_step = 0x1B;
//					}
//				}
//				break;
//			case 0x25:  // 激活网络
//				if(JudgeSomeDelayInProOk(step_timer,Delay500mSec))
//				{
//					at_tx(ciicr);
//					step_timer = TIM6->CNT;
//					wait_timer = uwTick;
//					current_step = 0x26;
//				}
//				break;
//			case 0x26: //
//				inclson(_ok);
//				if(bGsmAckOk) 
//				{
//					current_step = 0x27;
//					
//				} 
//				else if (JudgeMilSecondDelayInPro(wait_timer , Delay1Min) == true)
//				{
//					current_step = 0x1B;
//				}
//				break;
//			case 0x27:// TCP连接
//				if(JudgeSomeDelayInProOk(step_timer,Delay500mSec))
//				{
//					at_tx(ip);
//					step_timer = TIM6->CNT;
//					wait_timer = uwTick;
//					current_step = 0x28;
//				}
//				break;
//			case 0x28:  // 等待连接
//				inclson(_ip);
//				if(bGsmAckOk) 
//				{
//					
//					bSendOnlinePak  = 1;//发送上线包
//					current_step = 0x29;
//					firstsend = 0;
//				} 
//				else if (JudgeMilSecondDelayInPro(wait_timer , Delay1Min) == true)
//				{
//					current_step = 0x1B;
//					gprs_link_status = 0;
//				}
//				break;
//			case 0x29:  /* 模块已建立连接,开始发送上线包 */ 
//				if (bSendOnlinePak == 1)	/* 模块已建立连接,开始发送上线包 */
//				{
//					if(JudgeSomeDelayInProOk(step_timer,Delay2Sec)||firstsend ==1)
//					{
//						if (++SendGprsNum >= 10)
//						{
//							gprs_link_status = 0;
//							bSendOnlinePak = 0;
//							SendGprsNum = 0x00;
//							current_step = 0xAA;
//						}
//						else
//						{
//							TransSendOnLine();
//							bSendOnlinePak = 0;
//							step_timer = TIM6->CNT;
//							current_step = 0x2A;
//							diffflag = 0;
//						}
//					}
//				}
//				break;
//			case 0x2A:
//				if((GsmReceBuff[0] == 0xD2 )&&(GsmReceBuff[15] == CheckSum(15,GsmReceBuff)))
//				{
//					gprs_link_status = 1;
////					send_timer=uwTick;
//					lastCardTriggerTime = uwTick;
//					memset(GsmReceBuff,0,sizeof(GsmReceBuff));
//					SendYYWarn(WXTXZC);
//					errornum=0;
//					AnsewerErrNum = 0;
//					firstsend =0;
//					SendGprsNum = 0x00;
//				}
//				else if(gprs_link_status == 0 && (JudgeSomeDelayInProOk(step_timer,Delay5Sec)))
//				{
//					current_step = 0x29;
//					bSendOnlinePak = 1;
//					step_timer = TIM6->CNT;
//					firstsend =1;
//				}
//				if((g_CardLocaData.Longitude != 0)&&(g_CardLocaData.Latitude!=0)&&diffflag == 0)
//				{
//					DiffTime = uwTick;
//					diffflag = 1;
//				}
//				if(gprs_link_status == 1)
//				{
//					inclson(error);
//					if(bGsmAckOk) 
//					{
//						current_step = 0x1B;
//						gprs_link_status = 0;
//	//					SendYYWarn(WXTXYC);
//					}
//					inclson(closed);
//					if(bGsmAckOk) 
//					{
//						current_step = 0x1B;
//						gprs_link_status = 0;
//	//					SendYYWarn(WXTXYC);
//					}
//					inclson(bootrom);
//					if(bGsmAckOk) 
//					{
//						gprs_link_status = 0;
//						step_timer = TIM6->CNT;
//						current_step = 0xAC;
//	//					SendYYWarn(WXTXYC);
//					}
//					inclson(rdy);
//					if(bGsmAckOk) 
//					{
//						gprs_link_status = 0;
//						step_timer = TIM6->CNT;
//						current_step = 0xAC;
//	//					SendYYWarn(WXTXYC);
//					}
//				}
//				
//				break;
//			case 0xAA: //关闭透传
//				HAL_Delay(1000);
//				at_tx(CloseTransComm);
//				current_step = 0xAB;
//				HAL_Delay(1000);
//				step_timer = TIM6->CNT;
//				break;
//			case 0xAB:
//				inclson(_ok);
//				if(bGsmAckOk) 
//				{
//					current_step = 0x1B;
//					TransComm = 0;
//				}
//				else
//				{
//					if (JudgeSomeDelayInProOk(step_timer , Delay5Sec) == true)
//					{
//						current_step = 0xAA;
//					}
//				}
//				break;
//			case 0xAC:
//				if(JudgeSomeDelayInProOk(step_timer,Delay500mSec))	
//				{
//					at_tx(AT);
//	//				SendYYWarn(LYDK);
//					step_timer = TIM6->CNT;
//					wait_timer = uwTick;
//					current_step = 0xAD;
//					flaglagao = 0;
//				}
//				break;
//			case 0xAD:
//				inclson(_ok);
//					if(bGsmAckOk) 
//					{
//						current_step = 0x14;
//						step_timer = TIM6->CNT;
//						noanswernum = 0;
//					} 
//					else if (JudgeMilSecondDelayInPro(wait_timer ,3000) == true)
//					{
//						current_step = 0xAC;
//						if(++noanswernum>2)
//						{
//							current_step = 0xAE;
//						}
//					}
//				break;
//			case 0xAE:
//				if(flaglagao==0)
//				{
//					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
//					errornum_timer = uwTick;
//					flaglagao = 1;
//				}
//				else if(flaglagao==1)
//				{
//					if(JudgeMilSecondDelayInPro(errornum_timer,Delay10Sec) == true)
//					{
//						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
//						flaglagao = 0;
//						errornum=0;
//						noanswernum = 0;
//						TransComm = 0;
//						current_step = 0x10;
//					}
//				}
//				break;	
//			case 0xAF:
//				if(JudgeSomeDelayInProOk(step_timer,Delay500mSec))	
//				{
//					at_tx(EXINFO);
//					current_step = 0x14;
//				}
//				
//				break;
//			case 0xB0:
//				break;
//	        default:
//	            current_step = 0x10;
//				gprs_link_status = 0;
//	            break;
//		}
	}
	else
	{
		  switch(current_step)
		{
			case 0x10:  // 连接服务器
				if(JudgeSomeDelayInProOk(step_timer,Delay3Sec)) 
				{
					at_tx(mipopen);
					current_step = 0x11;
					step_timer = TIM6->CNT;
				}
				
				break;
			case 0x11: 
				inclson(_mipopen,GsmReceBuff);
				if(bGsmAckOk) 
				{
					current_step = 0x12;
					bSendOnlinePak  = 1;//发送上线包
				} 
				else
				{
					if(JudgeSomeDelayInProOk(step_timer,Delay3Sec))
					{
						current_step = 0x10;
						step_timer = TIM6->CNT;
						gprs_link_status = 0;
					}
				}
				break;
			case 0x12: 
				if (bSendOnlinePak == 1)	/* 模块已建立连接,开始发送上线包 */
				{
					if(JudgeSomeDelayInProOk(step_timer,Delay2Sec)||firstsend ==1)
					{
						if (++SendGprsNum >= 10)
						{
							gprs_link_status = 0;
							bSendOnlinePak = 0;
							SendGprsNum = 0x00;
							errornum = 3;
						}
						else
						{
							sendonline = 1;
							bSendOnlinePak = 0;
							step_timer = TIM6->CNT;
							current_step = 0x13;
							diffflag = 0;
						}
					}
				}
				break;
			case 0x13:
				if(gprs_link_status == 0)
				{
					for (int i = 0; i <= sizeof(GsmReceBuff) - 16; i++)
					{
						if((GsmReceBuff[i] == 0xD2 )&&(GsmReceBuff[i+15] == CheckSum(15,&GsmReceBuff[i])))
						{
							gprs_link_status = 1;
//							send_timer=uwTick;
							lastCardTriggerTime = uwTick;
//							memset(GsmReceBuff,0,sizeof(GsmReceBuff));
							WT588E_PLAY(WXTXZC);
							errornum=0;
							AnsewerErrNum = 0;
							firstsend =0;
							SendGprsNum = 0x00;
							sendonline = 0;
							break;
						}
					}
				}
				if(gprs_link_status == 0 && (JudgeSomeDelayInProOk(step_timer,Delay5Sec)))
				{
					current_step = 0x12;
					bSendOnlinePak = 1;
					step_timer = TIM6->CNT;
					firstsend =1;
				}
				if((g_CardLocaData.Longitude != 0)&&(g_CardLocaData.Latitude!=0)&&diffflag == 0)
				{
					DiffTime = uwTick;
					diffflag = 1;
				}
				if(gprs_link_status == 1)
				{
					inclson(disconn0,GsmReceBuff);
					if(bGsmAckOk) 
					{
						current_step = 0x10;
						gprs_link_status = 0;
	//					SendYYWarn(WXTXYC);
					}
				}
				break;
			case 0xAA: //关闭透传
					HAL_Delay(1000);
					at_tx(CloseTransComm);
					current_step = 0xAB;
					HAL_Delay(1000);
					step_timer = TIM6->CNT;
				
				break;
			case 0xAB:
				inclson(_ok,GsmReceBuff);
				if(bGsmAckOk) 
				{
					current_step = 0x10;
					TransComm = 0;
				}
				else
				{
					if (JudgeSomeDelayInProOk(step_timer , Delay5Sec) == true)
					{
						current_step = 0xAA;
					}
				}
				break;
			default:
	            current_step = 0x10;
				gprs_link_status = 0;
	            break;
		}
	}

}
/******************************************************************************
 * 函数名称: void	CSQ_BackInfo(void)           
 * 功能描述: CSQ命令返回,查询信号质量
 * 输　  入: 无
 * 输　  出: 无
 * 调用模块: 无
********************************************************************************/
void	CSQ_BackInfo(void)
{
	char *p;
	uint8_t rt;
//	p = inclson(_csq);
	if(bGsmAckOk)
	{
		if(p[2] == ',')
		{
			rt = Asc2Hex(p[0])*10+Asc2Hex(p[1]);
		}	
//		else
//		{
//			rt = Asc2Bcd(p[0],p[1]);
//		}
		signal = rt;                		/* 有手机信号处理流程 */
	}		
}
/******************************************************************************
 * 函数名称: void	CGATT_BackInfo(void)           
 * 功能描述: CGATT命令返回,查询网络注册情况
 * 输　  入: 无
 * 输　  出: 无
 * 调用模块: 无
********************************************************************************/
_Bool	CGATT_BackInfo(void)
{
	char *p;
	uint8_t rt;
//	p = inclson(_cgatt);
	if(bGsmAckOk)
	{
		if(p[1] == 0x31)  //0x31--"1"
		{
			return 1;
		}	
		else
		{
			return 0;
		}
	}		
}
// 辅助函数
void Reset_Module(void)
{
    HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_RESET);
    HAL_Delay(160);
    HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_SET);
}

//字符串清零子函数:字符串中不能出现0x00='\0'
void	ClearBuff(uint8_t *pBuf, uint16_t Len) 
{
	uint16_t i;
  	for (i=0; i<Len; i++)
  	{ 
		*(pBuf + i) = 0x00;
	}
 }
//发送AT命令子函数
void 	at_tx(char *atcom)
{ 
	uint8_t len;
//  	ClearBuff(GsmReceBuff, MAX_GSM_LEN);
  	len = strlen(atcom);  //求出所发AT命令的长度
  	pnt2 = GsmReceBuff;
	UartSend(&g_Uart3,(uint8_t *)atcom,len);
}
char inclson(char *son,uint8_t *buff)
{ 
	uint16_t i,j,sonl,len;
	bGsmAckOk = 0;
	len		= RXlen;//atlen(GsmReceBuff);
	sonl	= strlen(son);
	for(i = 0;i < len;i ++)
	{ 
		if(buff[i] == *son)
		{ 
			for(j = 1;j < sonl;j ++)
			{ 
				if(buff[i + j] == *(son + j))
				{ 
					bGsmAckOk = 1;
				}
				else
				{ 
					bGsmAckOk = 0;
					break;
				}
			}
		}
		if(bGsmAckOk)
			return 1;
	}
	return 0;
}
//char *inclson1(char *son)
//{ 
//	uint16_t i,j,sonl,len;
//	bGsmAckOk = 0;
//	len		= RXlen;//atlen(GsmReceBuff);
//	sonl	= strlen(son);
//	for(i = 0;i < len;i ++)
//	{ 
//		if(CorsDiffDataBuff[i] == *son)
//		{ 
//			for(j = 1;j < sonl;j ++)
//			{ 
//				if(CorsDiffDataBuff[i + j] == *(son + j))
//				{ 
//					bGsmAckOk = 1;
//				}
//				else
//				{ 
//					bGsmAckOk = 0;
//					break;
//				}
//			}
//		}
//		if(bGsmAckOk)
//			return (char*)CorsDiffDataBuff + i + j;
//	}
//	return (char*)CorsDiffDataBuff;
//}
/******************************************************************************
 * 函数名称: void	TransSendOnLine(void)         
 * 功能描述: 发送GPRS上线包
 * 输　  入: 无
 * 输　  出: 无
 * 调用模块: 无
********************************************************************************/
void	TransSendOnLine(void)
{
	uint8_t i=0,k=0;
	uint8_t SendBuff[16];
//	uint8_t end = 0x1A;
	memset(SendBuff,0,sizeof(SendBuff));
	SendBuff[0] = 0xd2;
	k = 0xd2;
	for(i = 0; i< 6; i++)
	{
		SendBuff[1+i] = g_Mobile_Sim.SIM_Phone[0][i];
		k += g_Mobile_Sim.SIM_Phone[0][i];
	}
	for(i = 7; i< 15; i++)
	{
           SendBuff[i] = 0x00;
	}
	k = ~k + 1;
	SendBuff[15] = k;         //发送校验和
    UartSend(&g_Uart3,(uint8_t*)&SendBuff,16);	
//	delay_ms(2);
//	UartSend(&g_Uart3,&end,1);

}	





