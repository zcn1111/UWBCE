#include "bsp.h"
#include "main.h"

uint8_t WriteMobileNumFlag = 0;

/*
*********************************************************************************************************
*	函 数 名: c_uart_pro
*	功能说明: 解析C口数据包。分析结果存放在全局变量
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void c_uart_pro(uint8_t *bPtr,int32_t len)
{
	uint16_t i = 0;
	uint16_t j = 0;
	uint8_t ReturnFlag = 0;
	uint16_t finflag = 0;
	uint8_t DataBuf[256];
	
	if (len == 0)
	{
		return;
	}
	
	memset(DataBuf, 0x00, sizeof(DataBuf));
	for (i = 0; i < len; i++)
	{
		switch (finflag)
		{
			case 0:
				if (*(bPtr + i) == 0x0D)
				{
					finflag = 1;
				}
				else
				{
					DataBuf[j] = *(bPtr + i);
					finflag = 0;
					j++;
				}
				break;
			case 1:
				if (*(bPtr + i) == 0x0A)
				{
					finflag = 2;
				}
				else
				{
					DataBuf[j] = *(bPtr + i);
					finflag = 0;
					j++;
				}
				break;
			default:
				break;
		}
		if (finflag == 2)
		{
			ReturnFlag = 1;
			
			break;
		}
	}
	
	if (ReturnFlag == 0)
	{
		return;
	}
	
	if(memcmp(DataBuf,"#Cnum_Write,",12) == 0)
	{//设置本机号
		for(i = 0; i < 6; i++)
		{					
			g_MobileInFlash.mobileno[4][i] = Asc2Bcd(DataBuf[12 + i * 2], DataBuf[13 + i * 2]); 
		}
		WriteDTUSysPar();	 /*存入flash*/	    				
		debug_printf("Cnum_Write,OK!\r\n");
	}
	else if(memcmp(DataBuf,"#Cnum_Read",10) == 0)
	{//读取本机号
		uint8_t MobNum[13];
		memset(MobNum, 0, sizeof(MobNum));
		ReadDTUSysPar();
		for(i = 0; i < 6; i++)
		{					
			Bcd2Asc(g_MobileInFlash.mobileno[4][i],&MobNum[i * 2]);
		}
		debug_printf("Cnum_Read,%s,OK!\r\n",MobNum);					
	}
	else if(memcmp(DataBuf,"#ID_Write,",10) == 0)
	{//设置ID号
		for(i=0;i<7;i++)
		{					
			g_IDInFlash.IDnum[i] = Asc2Bcd(DataBuf[10 + i * 2],DataBuf[11 + i * 2]); 
		}						
		WriteIDToFlash();
		debug_printf("ID_Write,OK!\r\n");
	}
	else if(memcmp(DataBuf,"#ID_Read",8) == 0)
	{//读取ID号
		uint8_t MobNum[15];
		memset(MobNum, 0, sizeof(MobNum));
		ReadIDFromFlash();
		for(i=0;i<7;i++)
		{					
			Bcd2Asc(g_IDInFlash.IDnum[i],&MobNum[i * 2]);
		}					
		debug_printf("ID_Read,%s,OK!\r\n",MobNum);	
	}
	else if(memcmp(DataBuf,"#Equipnum_Write,",16) == 0)
	{//写设备编号		
		memset(EquipmentNum,0x00,sizeof(EquipmentNum));
		memcpy(EquipmentNum,DataBuf + 16,sizeof(EquipmentNum));      
		Write_EQUIPNUM_EEPROM();
		debug_printf("Equipnum_Write,OK!\r\n");
	}
	else if(memcmp(DataBuf,"#Equipnum_Read",8) == 0)
	{//读取设备编号
		Read_EQUIPNUM_EEPROM();
		debug_printf("Equipnum_Read,%s,OK!\r\n",EquipmentNum);	
	}
	else if(memcmp(DataBuf,"#SimType_Write,",15) == 0)
	{//设置手机卡类型
		g_SIMType.num = Asc2Hex(DataBuf[15]);
		WriteSIMTYPEFromFLASH();
		debug_printf("#SimType_Write,OK!\r\n");
	}
	else if (memcmp(DataBuf,"#SimType_Read",13) == 0)
	{//读取手机卡类型
		uint8_t MobNum[2];
		memset(MobNum, 0, sizeof(MobNum));
		ReadSIMTYPEFromFLASH();
		MobNum[0] = Hex2Asc(g_SIMType.num);
		debug_printf("SimType_Read,%s,OK!\r\n",MobNum);
	}
	else if(memcmp(DataBuf,"#HWVer_Write,",13) == 0)
	{//写入硬件版本号		
				
		memset(HardwareVer,0x00,sizeof(HardwareVer));
		memcpy(HardwareVer,DataBuf+13,sizeof(HardwareVer));      				
		Write_HARDWAREVER_EEPROM();
		debug_printf("HWVer_Write,OK!\r\n");
	}
	else if(memcmp(DataBuf,"#HWVer_Read",11) == 0)
	{//读取硬件版本号
		Read_HARDWAREVER_EEPROM();
		debug_printf("Equipnum_Read,%s,OK!\r\n",HardwareVer);	
	}
	else if(memcmp(DataBuf,"#IP_Write,",10) == 0)
	{//写入IP		
		uint8_t ip_hex[4];
		uint16_t port_hex;
		ReadServerInfoPar();  
		uint8_t cmpipbuf[2][6];												
		memset(cmpipbuf[0],0,sizeof(cmpipbuf[0]));
		memset(cmpipbuf[1],0xff,sizeof(cmpipbuf[1]));								

		int ip0, ip1, ip2, ip3, port0;
		// 解析格式：固定前缀 + 数字.数字.数字.数字:数字
		if (sscanf((char *)DataBuf, "#IP_Write,%d.%d.%d.%d:%d", &ip0, &ip1, &ip2, &ip3, &port0) != 5)
		{
			debug_printf("IP_Write,ERR!\r\n");
		}
		else
		{
			if (ip0 < 0 || ip0 > 255 || ip1< 0 || ip1 > 255 || ip2 < 0 || ip2 > 255 || ip3 < 0 || ip3 > 255 ||
				port0 < 0 || port0 > 65535) 
			{
				debug_printf("IP_Write,ERR!\r\n");
			}
			else
			{
				ip_hex[0] = (uint8_t)ip0;
				ip_hex[1] = (uint8_t)ip1;
				ip_hex[2] = (uint8_t)ip2;
				ip_hex[3] = (uint8_t)ip3;
				port_hex = (uint16_t)port0;	
				if( memcmp(ip_hex,cmpipbuf[0],4)== 0 || memcmp((uint8_t *)&port_hex,cmpipbuf[0],2)== 0 
										|| memcmp(ip_hex,cmpipbuf[1],4) == 0 || memcmp((uint8_t *)&port_hex,cmpipbuf[1],2)== 0 )
				{
					debug_printf("IP_Write,ERR!\r\n");
				}
				else
				{
					memcpy(g_ServerInfo.serveripaddr[2],ip_hex,4);
					g_ServerInfo.serverportaddr[2] = port_hex;
					WriteServerInfoPar();
					debug_printf("IP_Write,OK!\r\n");
					g_NeedReInitDTU = 1;//重连服务器
				}				      				
			}
		}
	}
	else if(memcmp(DataBuf,"#IP_Read",8) == 0)
	{//读IP
		ReadServerInfoPar();
		debug_printf("IP_Read,%d.%d.%d.%d:%d,OK!\r\n",g_ServerInfo.serveripaddr[2][0],g_ServerInfo.serveripaddr[2][1],g_ServerInfo.serveripaddr[2][2],g_ServerInfo.serveripaddr[2][3],g_ServerInfo.serverportaddr[2]);	
	}
	else if(memcmp(DataBuf,"#ICCID_Read",11) == 0)
	{//读ICCID
		debug_printf("ICCID_Read,%s,OK!\r\n",iccidbuff);	
	}
	/* 读UWB诊断信息 26.06.25 */
	else if (memcmp(DataBuf, "#UWB_Diag", 9) == 0)
	{
		DW1000_Diag_DumpAll();
	}
}

void UartProcess(void)
{
	if(WriteMobileNumFlag == 1)
	{
		//解析C口数据，手机号，卡号，报警类型
		c_uart_pro(g_RxBuf1, g_Uart1.usRxCount);
		WriteMobileNumFlag = 0;
	}
}