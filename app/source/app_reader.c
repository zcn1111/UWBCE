
#include "bsp.h"
#include "main.h"

uint8_t g_WGBuf[26];			//接收韦根数据26位
uint8_t g_ReadCnt;
uint8_t g_EvenBit;			//韦根读卡器偶校验位
uint8_t g_OddBit;				//韦根读卡器奇校验位
uint8_t g_WGstr[3];
uint32_t g_ReadWGDelay;		//韦根读卡间隔计时

uint32_t g_ID;

bool g_HaveWG;				//有韦根记录
bool g_HaveNewWG;				//有韦根记录
bool g_HaveEffectiveTime;

UnitCard g_Card;
AuthorizedUnit g_AuthorizedUnit[28];

void InitReaderVar(void)
{
	g_ReadCnt = 0;
	memset(g_WGstr,0,sizeof(g_WGstr));
	g_HaveNewWG = false;
}

void WGPro(void)
{
//	if(g_WgsigPowerOn && g_ReaderPowerOn &&(JudgeSomeDelayInProOk(g_PowerOnReaderDelay,Delay1Sec)==1))
	{
		g_HaveWG = true;
		if(!g_HaveNewWG)
		{
			switch(g_ReadCnt)
			{
				case 0:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_EvenBit = 0;
					}
					else
					{
						g_EvenBit = 1;
					}
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 1:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[0] &= 0x7F;
					}
					else
					{
						g_WGstr[0] |= 0x80;
					}
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 2:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[0] &= 0xBF;
					}
					else
					{
						g_WGstr[0] |= 0x40;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 3:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[0] &= 0xDF;
					}
					else
					{
						g_WGstr[0] |= 0x20;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 4:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[0] &= 0xEF;
					}
					else
					{
						g_WGstr[0] |= 0x10;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 5:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[0] &= 0xF7;
					}
					else
					{
						g_WGstr[0] |= 0x08;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 6:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[0] &= 0xFB;
					}
					else
					{
						g_WGstr[0] |= 0x04;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 7:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[0] &= 0xFD;
					}
					else
					{
						g_WGstr[0] |= 0x02;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 8:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[0] &= 0xFE;
					}
					else
					{
						g_WGstr[0] |= 0x01;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 9:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[1] &= 0x7F;
					}
					else
					{
						g_WGstr[1] |= 0x80;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 10:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[1] &= 0xBF;
					}
					else
					{
						g_WGstr[1] |= 0x40;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 11:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[1] &= 0xDF;
					}
					else
					{
						g_WGstr[1] |= 0x20;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 12:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[1] &= 0xEF;
					}
					else
					{
						g_WGstr[1] |= 0x10;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 13:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[1] &= 0xF7;
					}
					else
					{
						g_WGstr[1] |= 0x08;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 14:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[1] &= 0xFB;
					}
					else
					{
						g_WGstr[1] |= 0x04;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 15:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[1] &= 0xFD;
					}
					else
					{
						g_WGstr[1] |= 0x02;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 16:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[1] &= 0xFE;
					}
					else
					{
						g_WGstr[1] |= 0x01;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 17:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[2] &= 0x7F;
					}
					else
					{
						g_WGstr[2] |= 0x80;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 18:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[2] &= 0xBF;
					}
					else
					{
						g_WGstr[2] |= 0x40;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 19:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[2] &= 0xDF;
					}
					else
					{
						g_WGstr[2] |= 0x20;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 20:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[2] &= 0xEF;
					}
					else
					{
						g_WGstr[2] |= 0x10;
					}
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 21:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[2] &= 0xF7;
					}
					else
					{
						g_WGstr[2] |= 0x08;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 22:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[2] &= 0xFB;
					}
					else
					{
						g_WGstr[2] |= 0x04;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 23:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[2] &= 0xFD;
					}
					else
					{
						g_WGstr[2] |= 0x02;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;
				case 24:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_WGstr[2] &= 0xFE;
					}
					else
					{
						g_WGstr[2] |= 0x01;
					}					
					g_ReadCnt++;
					g_ReadWGDelay = TIM2->CNT;
					break;				
				case 25:
					if(READ_WG0_STA() == GPIO_PIN_RESET)//WG0
					{
						g_OddBit = 0;
					}
					else
					{
						g_OddBit = 1;
					}
					g_ReadCnt = 0;
					g_HaveNewWG = true;
					g_ReadWGDelay = TIM2->CNT;
					break;
				default:
					break;
			}
		}
	}
}


/******************************************************************
函数名称: bool CheckWiegand(void)
功能描述: 韦根奇偶校验
参数输入: 
参数输出: 
返回值: 
其他: hly
******************************************************************/
bool CheckWiegand(uint8_t* ID,uint8_t evenbit,uint8_t oddbit)
{
    uint8_t i,tmpcheck,onecnt,tmpeven,tmpodd;

	onecnt = 0;
	tmpcheck = ID[0];          //进行偶校验
	for(i=0;i<8;i++)                //前8位
	{
		if(tmpcheck & 0x01)         //判断是否为1
		{
			onecnt++;               //若为1，onecnt加1
		}
		tmpcheck>>=1;               //右移
	}
	tmpcheck = ID[1];          
	for(i=0;i<4;i++)                //9-12位
	{
		if(tmpcheck & 0x80)         //判断是否为1
		{
			onecnt++;               //若为1，onecnt加1
		}
		tmpcheck<<=1;               //左移
	}
	onecnt%2==0?(tmpeven = 0):(tmpeven = 1);
	onecnt = 0;

	if(tmpeven == evenbit)        //偶校验正确，进行奇校验
	{
		tmpcheck = ID[1];      
		for(i=0;i<4;i++)            //13-17位
		{
			if(tmpcheck & 0x01)
			{
				onecnt++;
			}
			tmpcheck>>=1;
		}
		tmpcheck = ID[2];
		for(i=0;i<8;i++)            //13-17位
		{
			if(tmpcheck & 0x01)
			{
				onecnt++;
			}
			tmpcheck>>=1;
		}
		onecnt%2==0?(tmpodd=1):(tmpodd=0);
		onecnt = 0;

		if(tmpodd == oddbit)      //奇校验正确则返回true，反之返回false
		{
			return true;
		}
		else
		{
			memset(ID,0,3);
			return false;
		}
	}
	else
	{
		memset(ID,0,3);
		return false;              //偶校验错误返回false
	}
}




uint8_t FindUnitRank(uint16_t unit)
{
	uint8_t rank=0,i=0,blankunit=0;
	
	for(i=0;i<10;i++)
	{
		W25qxx_ReadBytes((uint8_t*)&g_Card,FLASH_UNITCARD_START+4096*i,sizeof(g_Card));
		if(g_Card.Flag == 0xAABB && g_Card.UnitNo == unit)
		{
			rank = i;
			return rank;
		}
		else if(g_Card.Flag != 0xAABB)
		{
			if(blankunit == 0)
			{
				blankunit = i;
				if(i == 0)
				{
					blankunit = 0xFE;
				}
			}
		}
		
		if(i == 9)
		{
			if(blankunit != 0)
			{
				if(blankunit == 0xFE)
					blankunit = 0;
				rank = blankunit;
				return rank;
			}
			else
			{
				rank = 0xFF;//已满无效
			}
		}
	}
	if(rank == 0xFE)
		rank = 0;
	return rank;
}

/******************************************************************
函数名称: void SaveUnitWhiteName(uint8 rank,uint16 no,uint32 ID)
功能描述: 增加权限卡
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
bool SaveUnitWhiteName(uint8_t rank,uint16_t no,uint32_t ID)
{
	uint16_t i,j,pagecnt,pageoff;
	
	pagecnt = sizeof(g_Card)/256;
	pageoff = sizeof(g_Card)%256;
	
	memset((uint8_t*)&g_Card,0,sizeof(g_Card));
	W25qxx_ReadBytes((uint8_t*)&g_Card,FLASH_UNITCARD_START+4096*rank,sizeof(g_Card));
	if(g_Card.Flag != 0xAABB)
	{
		memset((uint8_t*)&g_Card,0xFF,sizeof(g_Card));
		g_Card.Flag = 0xAABB;
		g_Card.UnitNo = no;
		g_Card.CardID[0] = ID;
		W25qxx_EraseSector(FLASH_UNITCARD_START/4096);
		for(j=0;j<pagecnt;j++)
		{
			W25qxx_WriteBytes((uint8_t*)&g_Card+256*j,FLASH_UNITCARD_START+256*j,256);
		}
		if(pageoff != 0)
			W25qxx_WriteBytes((uint8_t*)&g_Card+256*j,FLASH_UNITCARD_START+256*j,pageoff);
		return true;
	}
	else
	{
		if(g_Card.Flag == 0xAABB)
		{			
			for(i=0;i<UNIT_CARD_PERPAGE;i++)
			{
				if(g_Card.CardID[i] == 0xFFFFFFFF)
				{
					g_Card.CardID[i] = ID;
					W25qxx_EraseSector(FLASH_UNITCARD_START/4096);	
					for(j=0;j<pagecnt;j++)
					{
						W25qxx_WriteBytes((uint8_t*)&g_Card+256*j,FLASH_UNITCARD_START+rank*4096+256*j,256);
					}
					if(pageoff != 0)
						W25qxx_WriteBytes((uint8_t*)&g_Card+256*j,FLASH_UNITCARD_START+rank*4096+256*j,pageoff);
					return true;
				}
				else
				{
					if(g_Card.CardID[i] == ID)
					{
						return true;
					}
				}
			}			
		}
		else
		{
			memset((uint8_t*)&g_Card,0xFF,sizeof(g_Card));
			g_Card.Flag = 0xAABB;
			g_Card.UnitNo = no;
			g_Card.CardID[0] = ID;
			W25qxx_EraseSector(FLASH_UNITCARD_START/4096);	
			for(j=0;j<pagecnt;j++)
			{
				W25qxx_WriteBytes((uint8_t*)&g_Card+256*j,FLASH_UNITCARD_START+rank*4096+256*j,256);
			}
			if(pageoff != 0)
				W25qxx_WriteBytes((uint8_t*)&g_Card+256*j,FLASH_UNITCARD_START+rank*4096+256*j,pageoff);
			return true;
		}
	}
	return false;
}

/******************************************************************
函数名称: void DisauthorizeCard(uint8 no,uint32 ID)
功能描述: 删除权限卡
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
bool DisauthorizeCard(uint8_t rank,uint16_t no,uint32_t ID)
{
	uint16_t i,j,pagecnt,pageoff;
	bool ret = false;
	
	pagecnt = sizeof(g_Card)/256;
	pageoff = sizeof(g_Card)%256;
	
	memset((uint8_t*)&g_Card,0,512);
	
	W25qxx_ReadBytes((uint8_t*)&g_Card,FLASH_UNITCARD_START+4096*rank,sizeof(g_Card));
	if(g_Card.Flag == 0xAABB && g_Card.UnitNo == no)
	{
		for(i=0;i<UNIT_CARD_PERPAGE;i++)
		{
			if(ID == g_Card.CardID[i])
			{
				g_Card.CardID[i] = 0xFFFFFFFF;				
				ret = true;
			}
		}
		W25qxx_EraseSector(FLASH_UNITCARD_START/4096);	
		for(j=0;j<pagecnt;j++)
		{
			W25qxx_WriteBytes((uint8_t*)&g_Card+256*j,FLASH_UNITCARD_START+rank*4096+256*j,256);
		}
		if(pageoff != 0)
			W25qxx_WriteBytes((uint8_t*)&g_Card+256*j,FLASH_UNITCARD_START+rank*4096+256*j,pageoff);
	}	
	return ret;
}

/******************************************************************
函数名称: void SaveAuthorizedUnit(AuthorizedUnit uint)
功能描述: 增加授权单位
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
bool SaveAuthorizedUnit(AuthorizedUnit uint)
{
	uint16_t i,j,pagecnt,pageoff;
	
	pagecnt = sizeof(g_AuthorizedUnit)/256;
	pageoff = sizeof(g_AuthorizedUnit)%256;
	
	for(i=0;i<28;i++)
	{
		if(g_AuthorizedUnit[i].Flag != 0xAB)
		{
			memcpy((uint8_t*)&g_AuthorizedUnit[i],(uint8_t*)&uint,sizeof(uint));
			W25qxx_EraseSector(FLASH_UNITINFO_START/4096);	
			for(j=0;j<pagecnt;j++)
			{
				W25qxx_WriteBytes((uint8_t*)&g_AuthorizedUnit+256*j,FLASH_UNITINFO_START+256*j,256);
			}
			if(pageoff != 0)
				W25qxx_WriteBytes((uint8_t*)&g_AuthorizedUnit+256*j,FLASH_UNITINFO_START+256*j,pageoff);
			return true;
		}
		else
		{
			if(memcmp((uint8_t*)&g_AuthorizedUnit[i],(uint8_t*)&uint,sizeof(uint))==0)
			{
				return true;
			}
		}
	}
	return false;
}
/******************************************************************
函数名称: void DelAuthorizedUnit(AuthorizedUnit uint)
功能描述: 增加授权单位
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
bool DelAuthorizedUnit(AuthorizedUnit unit)
{
	uint16_t i,j,pagecnt,pageoff;
	
	pagecnt = sizeof(g_AuthorizedUnit)/256;
	pageoff = sizeof(g_AuthorizedUnit)%256;
	
	for(i=0;i<28;i++)
	{		
		if(g_AuthorizedUnit[i].UnitNo == unit.UnitNo && memcmp(g_AuthorizedUnit[i].CardID,unit.CardID,4) == 0
			&& memcmp((uint8_t*)&g_AuthorizedUnit[i].StartTime,(uint8_t*)&unit.StartTime,5)==0
				&& memcmp((uint8_t*)&g_AuthorizedUnit[i].StopTime,(uint8_t*)&unit.StopTime,5)==0)
		{
			memset((uint8_t*)&g_AuthorizedUnit[i],0,sizeof(g_AuthorizedUnit[i]));
			W25qxx_EraseSector(FLASH_UNITINFO_START/4096);	
			for(j=0;j<pagecnt;j++)
			{
				W25qxx_WriteBytes((uint8_t*)&g_AuthorizedUnit+256*j,FLASH_UNITINFO_START+256*j,256);
			}
			if(pageoff != 0)
				W25qxx_WriteBytes((uint8_t*)&g_AuthorizedUnit+256*j,FLASH_UNITINFO_START+256*j,pageoff);
			return true;
		}
		 
	}
	return false;
}

/******************************************************************
函数名称: void JudgeEffectiveUnit(void)
功能描述: 判断有效的授权
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void JudgeEffectiveUnit(void)
{
	uint8_t i,flag=0,saveflag = 0;
	uint16_t j,pagecnt,pageoff;
	DATETIME stoptime;
	DATETIME starttime;

	pagecnt = sizeof(g_AuthorizedUnit)/256;
	pageoff = sizeof(g_AuthorizedUnit)%256;
	
	W25qxx_ReadBytes((uint8_t*)&g_AuthorizedUnit,FLASH_UNITINFO_START,sizeof(g_AuthorizedUnit));	
	for(i=0;i<28;i++)
	{
		TimeBcdToHex(&stoptime,(uint8_t*)&g_AuthorizedUnit[i].StopTime);
		TimeBcdToHex(&starttime,(uint8_t*)&g_AuthorizedUnit[i].StartTime);
		if((memcmp(&stoptime,&g_SysTime,10) <= 0) && g_AuthorizedUnit[i].Flag == 0xAB)
		{
			memset((uint8_t*)&g_AuthorizedUnit[i],0,sizeof(g_AuthorizedUnit[i]));
			saveflag = 1; 
		}
		if((memcmp(&starttime,&g_SysTime,10)<=0) && (memcmp(&g_SysTime,&stoptime,10)<0))
		{
			flag = 1;
			g_HaveEffectiveTime = true;		
			if(g_SysPar.locktype == DJ02 && g_ReaderPowerOn == 0)
			{
				PWR_READER_ENABLE();
				g_ReaderPowerOn = 1;
			}
		}
		else if(i == 27 && flag != 1)
		{
			g_HaveEffectiveTime = false;
			if(g_SysPar.locktype == DJ02 && g_ReaderPowerOn == 1)
			{
				PWR_READER_DISABLE();
				g_ReaderPowerOn = 0;
			}
		}
	}
	if(saveflag == 1)
	{
		W25qxx_EraseSector(FLASH_UNITINFO_START/4096);	
		for(j=0;j<pagecnt;j++)
		{
			W25qxx_WriteBytes((uint8_t*)&g_AuthorizedUnit+256*j,FLASH_UNITINFO_START+256*j,256);
		}
		if(pageoff != 0)
			W25qxx_WriteBytes((uint8_t*)&g_AuthorizedUnit+256*j,FLASH_UNITINFO_START+256*j,pageoff);
	}
}
/******************************************************************
函数名称: Bool FindUnitAuthorizedCard(uint32 ID)
功能描述: 判断是否为权限卡
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
bool FindUnitAuthorizedCard(uint32_t ID)
{
//	UnitCard card;
	uint8_t i;
	uint16_t j;
	DATETIME starttime,stoptime;
	uint8_t cmpbuf[4];
	
	memset((uint8_t*)&g_Card,0,sizeof(g_Card));
	memset((uint8_t*)&cmpbuf,0,4);
	
	for(i=0;i<28;i++)
	{
		if(g_AuthorizedUnit[i].Flag == 0xAB)
		{
			TimeBcdToHex(&starttime,(uint8_t*)&g_AuthorizedUnit[i].StartTime);
			TimeBcdToHex(&stoptime,(uint8_t*)&g_AuthorizedUnit[i].StopTime);
			if(g_AuthorizedUnit[i].Flag == 0xAB && (memcmp(g_AuthorizedUnit[i].CardID,cmpbuf,4)== 0))
			{				
				g_AuthorizedUnit[i].Rank=FindUnitRank(g_AuthorizedUnit[i].UnitNo);
				W25qxx_ReadBytes((uint8_t*)&g_Card,FLASH_UNITCARD_START+4096*g_AuthorizedUnit[i].Rank,sizeof(g_Card));
				if(g_Card.Flag == 0xAABB)
				{
					for(j=0;j<UNIT_CARD_PERPAGE;j++)
					{
						if(ID == g_Card.CardID[j])
						{
							if((memcmp(&g_SysTime ,&starttime,10)>=0)&& (memcmp(&g_SysTime,&stoptime,10)<0))
							{
								return true;
							}
							
						}
						else
						{
							if(i==27 && j == UNIT_CARD_PERPAGE-1)
							{								
								return false;								
							}
						}
					}
				}
			}
		}
		else
		{
			if(i==27 )
			{
				return false;
		}
		}
	}
	return false;
}

/******************************************************************
函数名称: bool ReaderPro(void)
功能描述: 
参数输入: 
参数输出: 
返回值: 
其他: hly
******************************************************************/
void ReaderPro(void)
{
	if(!g_TimeNoAdjustFlag && g_CalMinuteFlag == 1 )
	{
		JudgeEffectiveUnit();
		g_CalMinuteFlag = 0;
	}
	
	if(g_HaveWG &&(JudgeSomeDelayInProOk(g_ReadWGDelay,Delay1Sec)==1)) //#define T2Delay1Sec 200000
    {
		g_ReadCnt = 0;
		memset(g_WGstr,0,sizeof(g_WGstr));
		g_HaveWG = false;
    }
	
	if(g_HaveNewWG)
	{
		if(CheckWiegand(g_WGstr,g_EvenBit,g_OddBit))
		{
			g_ID = (g_WGstr[0]<<16)+(g_WGstr[1]<<8)+g_WGstr[2];
			if(g_HaveEffectiveTime)
			{
				if(FindUnitAuthorizedCard(g_ID))
				{
					LOCK_OPENENABLE();
					g_OpenLock = true;
					g_Open1MinDelay = TIM2->CNT;
					g_OpenLockType = 1;
					g_OpenDoorCardID = g_ID;
				}
			}
		}
		memset(g_WGstr,0,3);
		g_HaveNewWG = false;
		g_HaveWG = false;
	}	
}
