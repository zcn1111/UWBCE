#include "bsp.h"
#include "main.h"
#include <string.h>
#include "4G_Comm.h"
#include "Update.h"

const char* host = "39.107.207.235";                    // 服务器地址rtk.ntrip.qxwz.com
const char* port = "8002";                              // 服务端口
const char* mountpoint = "AUTO";                        // NTRIP挂载点
const char* username = "qxtjtg005";                     // 鉴权用户名
const char* password = "81a8556";                       // 鉴权密码
const char* UserAgent = "NTRIP M307Client/1.0";         // 鉴权密码

uint8_t g_IPBuffer[4][4] = {{221,204,213,61},{221,204,213,61},{221,204,213,61},{221,204,213,61}};//0-远程升级IP，1-远程转储IP，2-数据IP， 3-差分数据ip

char auth[128] = {0};
char authBase64[200] = {0};
char request[512] = {0};

bool   bGsmAckOk;					          /* =1:收到手机模块应答 */ 

uint8_t GsmReceBuff[MAX_GSM_LEN]; 	  /* 手机模块接收缓冲 */
uint8_t g_LoraSendOkCnt;	          //Lora发送成功次数
uint8_t *pnt2,*pnt1;
uint8_t cors_current_step = 0x10;     //连服务器步骤标记
uint8_t current_step = 0x10;
uint8_t diff_current_step = 0x10;
uint8_t diffflag = 0;
uint16_t g_PortNo[4]={22042,22042,22042,22058};
uint32_t step_timer1 = 0;
uint32_t g_DisConnect0Delay = 0;
uint8_t g_DisConnect0Num = 0;
uint8_t g_RemoteUpdateLinkErrorNum = 0;
uint32_t g_DTUDisConnDelay = 0;
uint8_t g_AskTimeFlag=0;

char iccidbuff[32]={0};

//发送AT命令子函数
void at_tx(char *atcom)
{ 
	uint8_t len;
  	len = strlen(atcom);  //求出所发AT命令的长度
  	pnt2 = GsmReceBuff;
	UartSend(&g_Uart3,(uint8_t *)atcom,len);
}

void AT_Mipsend(uint16_t MipsendLen)
{
	char str[50];
	
	sprintf(str,"%s%d\r\n",_mipsend0, MipsendLen);
	at_tx(str);
}

void AT_DiffMipsend(uint16_t MipsendLen)
{
	char str[50];
	
	sprintf(str,"%s%d\r\n",_mipsend1, MipsendLen);
	at_tx(str);
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

// Base64编码函数
void base64_encode(const char *input, char *output, int size) 
{
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
	int i = 0;
    unsigned char char_array_3[3], char_array_4[4];
    
    while (size--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i <4 ; i++) 
                *output++ = base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(int k = i; k < 3; k++) 
            char_array_3[k] = '\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (int k = 0; k < i + 1; k++)
            *output++ = base64_chars[char_array_4[k]];
        
        while(i++ < 3) 
            *output++ = '=';
    }
    *output = '\0';
}


void combineStrings(char* output, const char* str1, const char* str2,uint8_t flag) 
{
 
    if(!output || !str1 || !str2) return;  // 防止NULL指针
	int index = 0;
    // 复制第一个字符串
    while (*str1) {
        output[index++] = *str1++;
    }
    if(flag==1)
	{
		// 添加冒号
		output[index++] = ':';
	}
    // 复制第二个字符串
    while (*str2) {
        output[index++] = *str2++;
    }
    
    // 添加字符串结束符
    output[index] = '\0';
}
/*GET /添加1 HTTP/1.1\r\nHost: 添加2:添加3\r\nAuthorization: Basic 添加4\r\nUser-Agent: 添加5\r\nConnection: close\r\n\r\n*/
void create_http_request(
    char *output, 
    size_t out_size,
    const char *add1, 
    const char *add2, 
    const char *add3, 
    const char *add4, 
    const char *add5
) {
    const char *parts[] = {
        "GET /",
        add1,      // 挂载点
        " HTTP/1.1\r\nHost: ",
        add2,      // 主机名
        ":",
        add3,      // 端口号
        "\r\nAuthorization: Basic ",
        add4,      // 认证信息
        "\r\nUser-Agent: ",
        add5,      // 用户代理
        "\r\nConnection: close\r\n\r\n"
    };
    
    int part_count = sizeof(parts) / sizeof(parts[0]);
    char *pos = output;
    size_t remain = out_size;
    *pos = '\0';  // 确保以空字符结尾
    
    // 为NULL的字段设为空字符串
    const char *safe_values[5] = {
        add1 ? add1 : "",
        add2 ? add2 : "",
        add3 ? add3 : "",
        add4 ? add4 : "",
        add5 ? add5 : ""
    };
    
    // 合并所有部分
    for(int i = 0, j = 0; i < part_count && remain > 1; i++) {
        const char *current = (i % 2 == 0) ? parts[i] : safe_values[j++];
        if (!current) current = "";
        
        // 逐个字符复制
        while (*current != '\0' && remain > 1) {
            *pos++ = *current++;
            remain--;
        }
    }
    
    // 确保字符串正确终止
    *pos = '\0';
}

// 连接NTRIP Caster
void connectNTRIPCaster(void)
{
	switch(cors_current_step)
	{
		case 0x10:
			if(JudgeSomeDelayInProOk(step_timer1,Delay3Sec)) 
			{
				//at_tx(mipopen_cors);
				cors_current_step = 0x11;
				step_timer1 = TIM2->CNT;
			}
			break;
		case 0x11: 
			inclson(_mipopen1,GsmReceBuff);
			if(bGsmAckOk) 
			{
				cors_current_step = 0x12;
				combineStrings(auth,username,password,1);
				base64_encode(auth,authBase64,strlen(auth));
				create_http_request(request,sizeof(request),mountpoint,host,port,authBase64,UserAgent);
			} 
			else
			{
				if(JudgeSomeDelayInProOk(step_timer1,Delay3Sec))
				{
					cors_current_step = 0x10;
					step_timer1 = TIM2->CNT;
					cors_gprs_link_status = 0;
				}
			}
			break;
		case 0x12:
			if(sendATflag==1)
			{
				at_tx(mipsend1);
				sendATflag = 0;
				cors_current_step = 0x13;
				step_timer1 = TIM2->CNT;
			}
			break;
		case 0x13:
			inclson(_mipsend,GsmReceBuff);
			if(bGsmAckOk) 
			{
				cors_current_step = 0x14;
				UartSend(&g_Uart3,(uint8_t *)request,strlen(request));
				delay_ms(2);
				uint8_t crtl = 0x1a;
				UartSend(&g_Uart3,(uint8_t *)&crtl,1);
				step_timer1 = TIM2->CNT;
				sendATflag = 1;
			} 
			else
			{
				if(JudgeSomeDelayInProOk(step_timer1,Delay3Sec))
				{
					cors_current_step = 0x12;
					step_timer1 = TIM2->CNT;
					cors_gprs_link_status = 0;
				}
			}
			break;
		case 0x14:
			inclson(http,GsmReceBuff);
			if(bGsmAckOk) 
			{
				cors_current_step = 0x15;
				cors_gprs_link_status = 1;
			} 
			else
			{
				inclson(disconn1,GsmReceBuff);
				if(bGsmAckOk) 
				{
					cors_current_step = 0x10;
					cors_gprs_link_status = 0;
				}
				
			}
			break;
		case 0x15:
			if(cors_gprs_link_status == 1)
				{
					inclson(disconn1,GsmReceBuff);
					if(bGsmAckOk) 
					{
						cors_current_step = 0x10;
						cors_gprs_link_status = 0;
					}
				}
			break;
			
		default:
			/*异常处理*/
			break;	
	}
}

void ConnectService(void)
{
	if (g_DtuOutage == 1)
	{
		return;
	}
	
	switch(current_step)
	{
		case 0x10:  //设置使用哪个手机卡（0为插拔卡，1为贴片卡）
		{
			ReadSIMTYPEFromFLASH();
			if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay2Sec))
			{
				if(g_SIMType.num == 0) 
				{
					at_tx(singlesim_0);
				}
				else if(g_SIMType.num == 1)
				{
					at_tx(singlesim_1);
				}
				g_DTUConnDelay = TIM2->CNT;
				current_step = 0x11;
			}
			
			break;
		}
		case 0x11:
		{
			inclson(_ok,GsmReceBuff);
			if(bGsmAckOk) 
			{
				if (g_SelfCheck.Dtu.Num++ >= 255)
				{
					g_SelfCheck.Dtu.Num = 1;
				}
				current_step = 0x12;
			} 
			else 
			{
				if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec) == true)
				{
					g_DTUConnDelay = TIM2->CNT;
					current_step = 0x10;
					gprs_link_status = 0;
					errornum++;
				}	
			}	
			
			break;
		}
		case 0x12:
		{
			if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec)) 
			{
				at_tx(iccid);
				current_step = 0x13;
				g_DTUConnDelay = TIM2->CNT;
			}
			
			break;
		}
		case 0x13:
		{
			inclson(_iccid,GsmReceBuff);
			if(bGsmAckOk) 
			{
				if (g_SelfCheck.Dtu.Num++ >= 255)
				{
					g_SelfCheck.Dtu.Num = 1;
				}
				char *p = strstr((char*)GsmReceBuff, "+MCCID: ");
				if (p) 
				{
					char id[32];
					memset(iccidbuff, 0, sizeof(iccidbuff));
					sscanf(p + strlen("+MCCID: "), "%s", iccidbuff);				
					current_step = 0x14;
				}
			} 
			else 
			{
				if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec) == true)
				{
					g_DTUConnDelay = TIM2->CNT;
					current_step = 0x10;
					gprs_link_status = 0;
					errornum++;
				}	
			}	
			
			break;
		}
		//先连接业务服务器
		case 0x14:
		{
			//如果业务服务器已连接，则去连接差分服务器
			if (gprs_link_status == 1)
			{
				current_step = 0x18;
			}
			else
			{
				if (JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec))
				{
					at_tx(sendtimeout0);
					current_step = 0x15;
					g_DTUConnDelay = TIM2->CNT;
				}
			}
			
			break;
		}
		case 0x15:
		{
			inclson(_ok,GsmReceBuff);
			if(bGsmAckOk) 
			{
				if (g_SelfCheck.Dtu.Num++ >= 255)
				{
					g_SelfCheck.Dtu.Num = 1;
				}
				current_step = 0x16;
			} 
			else 
			{
				if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec) == true)
				{
					g_DTUConnDelay = TIM2->CNT;
					current_step = 0x10;
					gprs_link_status = 0;
					errornum++;
				}	
			}
			
			break;
		}
		case 0x16:  //连接业务服务器
		{
			if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec)) 
			{
				char str[50];
				
				memset(str,0,sizeof(str));
				sprintf(str,"%s%d.%d.%d.%d\",%d\r",mipopen,
					g_ServerInfo.serveripaddr[2][0],g_ServerInfo.serveripaddr[2][1],g_ServerInfo.serveripaddr[2][2],g_ServerInfo.serveripaddr[2][3],g_ServerInfo.serverportaddr[2]);
				at_tx(str);
//				at_tx(mipopen);
				current_step = 0x17;
				g_DTUConnDelay = TIM2->CNT;
			}
			
			break;
		}
		case 0x17:
		{
			inclson(_mipopen,GsmReceBuff);
			if(bGsmAckOk) 
			{
				if (g_SelfCheck.Dtu.Num++ >= 255)
				{
					g_SelfCheck.Dtu.Num = 1;
				}
				
				if (g_SelfCheck.StartCharge == 0)
				{
					WT588E_PLAY(WXTXZC);
				}
				
				//如果差分服务器已连接，则不去连接差分服务器
				if (diff_gprs_link_status == 0)
				{
					current_step = 0x18;
				}
				else
				{
					current_step = 0x1C;
				}
				
				if (gprs_link_status == 0)
				{
					gprs_link_status = 1;
					ServiceAnsewerTime = TIM2->CNT;
				}
				
				errornum = 0;
			} 
			else
			{
				if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec))
				{
					current_step = 0xAC;
					g_DTUConnDelay = TIM2->CNT;
					gprs_link_status = 0;
					errornum++;
				}
			}
			
			break;
		}
		case 0x18: //连接差分服务器
		{
			//如果差分服务器已连接，则不去连接差分服务器
			if (diff_gprs_link_status == 1)
			{
				current_step = 0x1C;
			}
			else
			{
				if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec)) 
				{
					at_tx(sendtimeout1);
					current_step = 0x19;
					g_DTUConnDelay = TIM2->CNT;
				}
			}
			
			break;
		}
		case 0x19:
		{
			inclson(_ok,GsmReceBuff);
			if(bGsmAckOk) 
			{
				if (g_SelfCheck.Dtu.Num++ >= 255)
				{
					g_SelfCheck.Dtu.Num = 1;
				}
				current_step = 0x1A;
			} 
			else 
			{
				if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec) == true)
				{
					g_DTUConnDelay = TIM2->CNT;
					current_step = 0x18;
					diff_gprs_link_status = 0;
				}	
			}	
			
			break;
		}
		case 0x1A:
		{
			if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec)) 
			{
				char str[50];
				
				memset(str,0,sizeof(str));
				sprintf(str,"%s%d.%d.%d.%d\",%d\r",mipopen_diff,
					g_ServerInfo.serveripaddr[3][0],g_ServerInfo.serveripaddr[3][1],g_ServerInfo.serveripaddr[3][2],g_ServerInfo.serveripaddr[3][3],g_ServerInfo.serverportaddr[3]);
				at_tx(str);

				current_step = 0x1B;
				g_DTUConnDelay = TIM2->CNT;
			}
			
			break;
		}
		case 0x1B:
		{
			inclson(_mipopen1,GsmReceBuff);
			if(bGsmAckOk) 
			{
				if (g_SelfCheck.Dtu.Num++ >= 255)
				{
					g_SelfCheck.Dtu.Num = 1;
				}
				
				current_step = 0x1C;
				g_SendLocationDataToDiffService = 1;
			} 
			else
			{
				if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec))
				{
					current_step = 0x18;
					g_DTUConnDelay = TIM2->CNT;
					diff_gprs_link_status = 0;
				}
			}
			
			break;
		}
		case 0x1C:
		{
			if(diff_gprs_link_status == 0)
			{
				diff_gprs_link_status = 1;
				//if (g_SelfCheck.StartCharge == 0)
				//{
				//	WT588E_PLAY(WXTXZC);
				//}
				DiffServiceAnsewerTime = TIM2->CNT;
			}
			
			if(gprs_link_status == 1)
			{
				inclson(disconn0,GsmReceBuff);
				if(bGsmAckOk) 
				{
					if (g_SelfCheck.Dtu.Num++ >= 255)
					{
						g_SelfCheck.Dtu.Num = 1;
					}
					current_step = 0x10;
					gprs_link_status = 0;
				}
			}
			
			if(diff_gprs_link_status == 1)
			{
				inclson(disconn1,GsmReceBuff);
				if(bGsmAckOk) 
				{
					if (g_SelfCheck.Dtu.Num++ >= 255)
					{
						g_SelfCheck.Dtu.Num = 1;
					}
					current_step = 0x18;
					diff_gprs_link_status = 0;
				}
			}
			
			errornum = 0;
			
			break;
		}
		//断开连接，重新连接
		//先断开业务服务器
		case 0xAC:
		{
			if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec))
			{
				at_tx(distcp0);
				current_step = 0xAD;
				g_DTUConnDelay = TIM2->CNT;
			}
			
			break;
		}
		case 0xAD:
		{
			inclson(_ok,GsmReceBuff);
			if(bGsmAckOk) 
			{
				if (g_SelfCheck.Dtu.Num++ >= 255)
				{
					g_SelfCheck.Dtu.Num = 1;
				}
				
				if (g_SetIpAndPort == 1)
				{
					current_step = 0xAE;
					//g_SetIpAndPort = 0;
				}
				else
				{
					current_step = 0x10;
				}
				gprs_link_status = 0;
			}
			else
			{
				inclson(_err,GsmReceBuff);
				if(bGsmAckOk)
				{
					if (g_SelfCheck.Dtu.Num++ >= 255)
					{
						g_SelfCheck.Dtu.Num = 1;
					}
					
					current_step = 0x10;
					gprs_link_status = 0;
				}
				else if (JudgeSomeDelayInProOk(g_DTUConnDelay , Delay3Sec) == true)
				{
					current_step = 0x10;
					gprs_link_status = 0;
				}
			}
			
			break;
		}
		//断开差分服务器
		case 0xAE:
		{
			if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec))
			{
				at_tx(distcp1);
				current_step = 0xAF;
				g_DTUConnDelay = TIM2->CNT;
			}
			
			break;
		}
		case 0xAF:
		{
			inclson(_ok,GsmReceBuff);
			if(bGsmAckOk) 
			{
				if (g_SelfCheck.Dtu.Num++ >= 255)
				{
					g_SelfCheck.Dtu.Num = 1;
				}
				if (g_SetIpAndPort == 1)
				{
					current_step = 0x10;
					g_SetIpAndPort = 0;
				}
				else
				{
					current_step = 0x18;
				}
			}
			else
			{
				inclson(_err,GsmReceBuff);
				if(bGsmAckOk)
				{
					if (g_SelfCheck.Dtu.Num++ >= 255)
					{
						g_SelfCheck.Dtu.Num = 1;
					}
					if (g_SetIpAndPort == 1)
					{
						current_step = 0x10;
						g_SetIpAndPort = 0;
					}
					else
					{
						current_step = 0x18;
					}
				}
				else if (JudgeSomeDelayInProOk(g_DTUConnDelay , Delay3Sec) == true)
				{
					current_step = 0x10;
				}
			}
			break;
		}
		//断开连接，然后给4g 模块断电
		//先断开业务服务器
		case 0x20:
		{
			if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec))
			{
				at_tx(distcp0);
				current_step = 0x21;
				g_DTUConnDelay = TIM2->CNT;
				errornum++;
			}
			
			break;
		}
		case 0x21:
		{
			inclson(_ok,GsmReceBuff);
			if(bGsmAckOk) 
			{
				if (g_SelfCheck.Dtu.Num++ >= 255)
				{
					g_SelfCheck.Dtu.Num = 1;
				}
				current_step = 0x22;
				errornum = 0;
			}
			else
			{
				inclson(_err,GsmReceBuff);
				if(bGsmAckOk)
				{
					if (g_SelfCheck.Dtu.Num++ >= 255)
					{
						g_SelfCheck.Dtu.Num = 1;
					}
					current_step = 0x22;
				}
				else if (JudgeSomeDelayInProOk(g_DTUConnDelay , Delay5Sec) == true)
				{
					current_step = 0x20;
				}
			}
			
			break;
		}
		case 0x22:
		{
			if(JudgeSomeDelayInProOk(g_DTUConnDelay,Delay3Sec))
			{
				at_tx(distcp1);
				current_step = 0x23;
				g_DTUConnDelay = TIM2->CNT;
				errornum++;
			}
			
			break;
		}
		case 0x23:
		{
			inclson(_ok,GsmReceBuff);
			if(bGsmAckOk) 
			{
				if (g_SelfCheck.Dtu.Num++ >= 255)
				{
					g_SelfCheck.Dtu.Num = 1;
				}
				PWR_4G_DISABLE();
				current_step = 0x24;
				errornum = 0;
			}
			else
			{
				inclson(_err,GsmReceBuff);
				if(bGsmAckOk)
				{
					if (g_SelfCheck.Dtu.Num++ >= 255)
					{
						g_SelfCheck.Dtu.Num = 1;
					}
					PWR_4G_DISABLE();
					current_step = 0x24;
				}
				else if (JudgeSomeDelayInProOk(g_DTUConnDelay , Delay5Sec) == true)
				{
					current_step = 0x22;
				}
			}
			
			break;
		}
		case 0x24:
		{
			break;
		}
		default:
		{
			current_step = 0x10;
			gprs_link_status = 0;
			diff_gprs_link_status = 0;
			
			break;
		}		
	}
}

uint8_t ChargeSelf4g(void)
{
	if(JudgeSomeDelayInProOk(g_SelfCheck.Dtu.Delay,Delay10Sec))
	{
		if (g_SelfCheck.Dtu.Num > 0 || gprs_link_status == 1)
		{
			g_SelfCheck.Dtu.Result = 0;
		}
		else
		{
			g_SelfCheck.Dtu.Result = 1;
		}
		
		return 1;
	}
	
	return 0;
}