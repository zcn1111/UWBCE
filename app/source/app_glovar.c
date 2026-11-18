#include "bsp.h"
#include "main.h"
#include "Update.h"
#include "g_fun.h"
#include "app_workplan.h"

_WORKTIME       g_WorkTime;
_WARNLEDSTA     g_WarnLedSta;
_SYS_PAR        g_SysPar;
_VERSION_FLASH  g_VersioninFlash;
_MOBILE_FLASH   g_MobileInFlash;
_SERVER_FLASH   g_ServerInfo;
uint8_t WorkPlanNum =0 ;
ID_t	g_IDInFlash;
bool   g_GetMyMobileOKFlag;
Workcenter_point position[2];
uint32_t p_time[2];
uint8_t dealflag = 1;
int last_selected_idx = -1;
uint8_t	g_VersionAscii[4];				//ASCII版本号
uint8_t	g_VersionHex[2];				//16进制版本号
UPDATAPROGRAM g_UpdateCodePageTotal;
SIMType       g_SIMType;
VoiceSwitchType g_VoiceSwitch;
uint8_t 	g_UpdateCodeTypeErrorFlag;		//程序程序类型错误标记
AreaAlarm AreaAlarmInfo[11];

uint8_t g_CommanBuf[1024];  
uint8_t g_SendCardInfoInterval = 2;
uint8_t	effectiveplan = 0;
SelectEFence SelectEFenceInfo;
uint32_t PreWorkPlanID = 0;
/******************************************************************
函数名称: void WriteNormalFileSizeToFlash
功能描述: 更新程序参数写入到Flash
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void WriteNormalFileSizeToFlash(void)
{
	FlashReadData(g_CommanBuf , ADDR_EEPROM_UPDATA , 512 , 0);
	NormalUpdateFileSize.Crc16 = CRC16((uint8_t*)&NormalUpdateFileSize, sizeof(NormalUpdateFileSize) - 2);
	memcpy(g_CommanBuf,( uint8_t* )&NormalUpdateFileSize,sizeof(NormalUpdateFileSize));
	W25qxx_EraseSector(ADDR_EEPROM_UPDATA / w25qxx.SectorSize);
	W25qxx_WritePages(g_CommanBuf, ADDR_EEPROM_UPDATA , 512);
}

/******************************************************************
函数名称: Bool ReadNormalFileSizeFromFlash(void)
功能描述: 从Flash读更新程序参数
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
bool ReadNormalFileSizeFromFlash(void)
{
	uint16_t crc;
	
	W25qxx_ReadBytes((uint8_t*)&NormalUpdateFileSize, ADDR_EEPROM_UPDATA, sizeof(NormalUpdateFileSize));
	crc = CRC16((uint8_t*)&NormalUpdateFileSize, sizeof(NormalUpdateFileSize) - 2);
	if (crc != NormalUpdateFileSize.Crc16)
	{
		NormalUpdateFileSize.FileSize = 0;
		WriteNormalFileSizeToFlash();
		return false;
	}
	return true;
}

/******************************************************************
函数名称: Bool WriteDiffFileSizeToFlash(void)
功能描述:  写入增量升级文件大小
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
void WriteDiffFileSizeToFlash(void)
{
    FlashReadData(g_CommanBuf , ADDR_EEPROM_UPDATA , 512 , 0);
	DiffUpdateFileSize.Crc16 = CRC16( ( uint8_t* )&DiffUpdateFileSize , sizeof( DiffUpdateFileSize ) - 2 );		
	memcpy(g_CommanBuf + ADDR_UPDATE_DIFF_FLAG_OFFSET,( uint8_t* )&DiffUpdateFileSize,sizeof(DiffUpdateFileSize));
	W25qxx_EraseSector(ADDR_EEPROM_UPDATA / w25qxx.SectorSize);
	FlashWriteData((uint8_t*)&g_CommanBuf, ADDR_EEPROM_UPDATA , 512);
}

/******************************************************************
函数名称: Bool ReadDiffFileSizeFromFlash(void)
功能描述: 从Flash读增量升级文件大小的函数
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
bool ReadDiffFileSizeFromFlash(void)
{
	W25qxx_ReadBytes((uint8_t*)&DiffUpdateFileSize,ADDR_EEPROM_UPDATA+ADDR_UPDATE_DIFF_FLAG_OFFSET,sizeof(DiffUpdateFileSize));

	if(DiffUpdateFileSize.Crc16 != CRC16((uint8_t*)&DiffUpdateFileSize,sizeof(DiffUpdateFileSize)-2))
	{
		DiffUpdateFileSize.FileSize = 0;
        WriteDiffFileSizeToFlash();
        return false;
	}
    return true;
}

/******************************************************************
函数名称: void WriteModeToFlash(void)
功能描述: 工作模式到Flash
参数输入: 
参数输出: 
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
void WriteModeToFlash(void)
{
	uint16_t crc16;	
	crc16 = CRC16((uint8_t*)&g_ModeInFlash, sizeof(g_ModeInFlash) - 2);			//手机号定义Flash位置为扇区头，需要先执行
	g_ModeInFlash.Crc16 = crc16;
	W25qxx_EraseSector(FLASH_MODE_START/ w25qxx.SectorSize);
	FlashWriteData((uint8_t*)&g_ModeInFlash, FLASH_MODE_START, sizeof(g_ModeInFlash));	
}

/******************************************************************
函数名称: void ReadModeFromFlash
功能描述: 从Flash读工作模式
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReadModeFromFlash(void)
{
	W25qxx_ReadBytes((uint8_t*)&g_ModeInFlash, FLASH_MODE_START, sizeof(g_ModeInFlash));
	
	/* 检查工作模式校验和 */
	if (g_ModeInFlash.Crc16 != CRC16((uint8_t*)&g_ModeInFlash, sizeof(g_ModeInFlash) - 2))
	{
		memset((uint8_t*)&g_ModeInFlash, 0x00, sizeof(g_ModeInFlash));
		g_ModeInFlash.type = 0;
        g_ModeInFlash.Crc16 = CRC16((uint8_t*)&g_ModeInFlash, sizeof(g_ModeInFlash) - 2);
		WriteModeToFlash();
	}		
}

/******************************************************************
函数名称: void WriteLocationDiffToFlash(void)
功能描述: 定位间隔到Flash
参数输入: 
参数输出: 
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
void WriteLocationDiffToFlash(void)
{
	uint16_t crc16;	
	crc16 = CRC16((uint8_t*)&g_LocationAndSelfcheckDiffFlash, sizeof(g_LocationAndSelfcheckDiffFlash) - 2);			//手机号定义Flash位置为扇区头，需要先执行
	g_LocationAndSelfcheckDiffFlash.Crc16 = crc16;
	W25qxx_EraseSector(FLASH_LOCATION_DIFF_START/ w25qxx.SectorSize);
	FlashWriteData((uint8_t*)&g_LocationAndSelfcheckDiffFlash, FLASH_LOCATION_DIFF_START, sizeof(g_LocationAndSelfcheckDiffFlash));	
}

/******************************************************************
函数名称: void ReadLocationDiffFromFlash
功能描述: 从Flash读定位间隔
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReadLocationDiffFromFlash(void)
{
	/* 读Flash中的发送间隔 */
	W25qxx_ReadBytes((uint8_t*)&g_LocationAndSelfcheckDiffFlash, FLASH_LOCATION_DIFF_START, sizeof(g_LocationAndSelfcheckDiffFlash));
	
	/* 检查发送间隔校验和 */
	if (g_LocationAndSelfcheckDiffFlash.Crc16 != CRC16((uint8_t*)&g_LocationAndSelfcheckDiffFlash, sizeof(g_LocationAndSelfcheckDiffFlash) - 2))
	{
		memset((uint8_t*)&g_LocationAndSelfcheckDiffFlash, 0x00, sizeof(g_LocationAndSelfcheckDiffFlash));
		g_LocationAndSelfcheckDiffFlash.Diff = 1;
		g_LocationAndSelfcheckDiffFlash.SelfcheckDiff = 5;
        g_LocationAndSelfcheckDiffFlash.Crc16 = CRC16((uint8_t*)&g_LocationAndSelfcheckDiffFlash, sizeof(g_LocationAndSelfcheckDiffFlash) - 2);
		WriteLocationDiffToFlash();
	}		
}

/******************************************************************
函数名称: void ChangeVersionFormat(void)
功能描述: 转换版本号格式
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
void ChangeVersionFormat(void)
{
	uint8_t i,flag;
	int8_t temp[7];
	
	memset(temp,0,7);
	memset(g_VersionAscii,0,sizeof(g_VersionAscii));
	memset(g_VersionHex,0,sizeof(g_VersionHex));
	
	memcpy(temp,g_Version,sizeof(g_Version));
	
	W25qxx_ReadBytes((uint8_t*)&g_VersioninFlash,FLASH_VERSION_START,sizeof(g_VersioninFlash));	//读boot版本号
	if(g_VersioninFlash.bootversion[0] == 0xFF && g_VersioninFlash.bootversion[1] == 0xFF && g_VersioninFlash.bootversion[2] == 0xFF && g_VersioninFlash.bootversion[3] == 0xFF)
	{
		memset(g_VersioninFlash.bootversion,0x30,4);
		flag = 1;
	}
	
	for(i=0;i<7;i++)    
	{
		if(temp[i] == '.')
		{
			if(i == 3)
			{
				g_VersionAscii[0] = temp[1];
				g_VersionAscii[1] = temp[2];
			}
			else if(i == 2)
			{
				g_VersionAscii[0] = 0x30;
				g_VersionAscii[1] = temp[1];
			}
			g_VersionAscii[2] = temp[i+1];
			if(temp[i+2] == 0x00)
				g_VersionAscii[3] = 0x30;
			else
				g_VersionAscii[3] = temp[i+2];
		}
	}
	g_VersionHex[0] = Asc2Hex(g_VersionAscii[0])*10+Asc2Hex(g_VersionAscii[1]);
	g_VersionHex[1] = Asc2Hex(g_VersionAscii[2])*10+Asc2Hex(g_VersionAscii[3]);
	if(memcmp(g_VersionAscii,g_VersioninFlash.userversion,4) != 0)
	{
		memcpy(g_VersioninFlash.userversion,g_VersionAscii,4);
		flag = 1;
	}
	if(flag == 1)
	{
		g_VersioninFlash.Crc16 = CRC16((uint8_t*)&g_VersioninFlash,sizeof(g_VersioninFlash)-2);
		W25qxx_EraseSector(FLASH_VERSION_START/w25qxx.SectorSize);
		W25qxx_WriteBytes((uint8_t*)&g_VersioninFlash,FLASH_VERSION_START,sizeof(g_VersioninFlash));
	}
}

//写区域报警到flash
void WriteAreaAlarmToFLASH(uint8_t *Data, uint16_t DataLen)
{	
	W25qxx_EraseSector(FLASH_AREA_ALARM_FLASH_START/ w25qxx.SectorSize ); 	
	W25qxx_WritePages(Data, FLASH_AREA_ALARM_FLASH_START, DataLen); 		
}

//将区域报警信息从flash中读出来
void ReadAreaAlarmToFLASH(void)
{
	uint8_t Data[2048];
	
	memset(Data, 0, sizeof(Data));
	FlashReadData(Data, FLASH_AREA_ALARM_FLASH_START, FLASH_AREA_ALARM_FLASH_LEN, 0);
	memset(AreaAlarmInfo, 0x00, sizeof(AreaAlarmInfo));
	/* 帧头校验 (0x5A 0xA5) */
	if (Data[0] != 0x5A || Data[1] != 0xA5) 
	{
		uint8_t DataBuff[98] = {0x5A, 0xA5, 0x00, 0x5C, 0x0A, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x07, 0x02, 0x00, 0x02, 0x02, 0x05, 0x00, 0x03, 0x00, 
		0x02, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x03, 0x00,
		0x05, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
		0x04, 0x02, 0x00, 0x02, 0x02, 0x05, 0x00, 0x03, 0x00,
		0x03, 0x02, 0x00, 0x02, 0x02, 0x07, 0x00, 0x03, 0x00, 
		0x00, 0x00, 0xAA};
	
		for (uint8_t j = 0; j < DataBuff[4]; j++)
		{
			//取出区域类型
			uint8_t AreaType = DataBuff[5 + 9 * j];
			//将区域报警放入数组中
			//马甲灯无列车
			AreaAlarmInfo[AreaType].armourled.notrain = DataBuff[5 + 9 * j + 1];
			//马甲灯列车预告
			AreaAlarmInfo[AreaType].armourled.trainannouncement = DataBuff[5 + 9 * j + 2];
			//马甲灯列车接近
			AreaAlarmInfo[AreaType].armourled.trainnear = DataBuff[5 + 9 * j + 3];
			//马甲灯非作业时间
			AreaAlarmInfo[AreaType].armourled.nojobtime = DataBuff[5 + 9 * j + 4];
			
			//语音无列车
			AreaAlarmInfo[AreaType].voice.notrain = DataBuff[5 + 9 * j + 5];
			//语音列车预告
			AreaAlarmInfo[AreaType].voice.trainannouncement = DataBuff[5 + 9 * j + 6];
			//语音列车接近
			AreaAlarmInfo[AreaType].voice.trainnear = DataBuff[5 + 9 * j + 7];
			//语音非作业时间
			AreaAlarmInfo[AreaType].voice.nojobtime = DataBuff[5 + 9 * j + 8];
		}
		
		return;
	}
	
	//报警区域的总长度
	uint16_t AlarmLen = (Data[3] << 8) + Data[2];
	//区域个数
	uint8_t AreaNum = Data[4];
	
	for (uint8_t j = 0; j < AreaNum; j++)
	{
		//取出区域类型
		uint8_t AreaType = Data[5 + 9 * j];
		//将区域报警放入数组中
		//马甲灯无列车
		AreaAlarmInfo[AreaType].armourled.notrain = Data[5 + 9 * j + 1];
		//马甲灯列车预告
		AreaAlarmInfo[AreaType].armourled.trainannouncement = Data[5 + 9 * j + 2];
		//马甲灯列车接近
		AreaAlarmInfo[AreaType].armourled.trainnear = Data[5 + 9 * j + 3];
		//马甲灯非作业时间
		AreaAlarmInfo[AreaType].armourled.nojobtime = Data[5 + 9 * j + 4];
			
		//语音无列车
		AreaAlarmInfo[AreaType].voice.notrain = Data[5 + 9 * j + 5];
		//语音列车预告
		AreaAlarmInfo[AreaType].voice.trainannouncement = Data[5 + 9 * j + 6];
		//语音列车接近
		AreaAlarmInfo[AreaType].voice.trainnear = Data[5 + 9 * j + 7];
		//语音非作业时间
		AreaAlarmInfo[AreaType].voice.nojobtime = Data[5 + 9 * j + 8];
	}
	
	memset(Data, 0, sizeof(Data));
}

//将flash中电子围栏信息清除掉
void CleanEFenceToFLASH(uint8_t EFenceNumSerial)
{
	uint16_t EraseNum = FLASH_EFENCE_FLASH_START / w25qxx.SectorSize;
	
	EraseNum = EraseNum + EFenceNumSerial - 1;
	W25qxx_EraseSector(EraseNum);
}

//写电子围栏信息到flash   每一包的最大长度是1056个字节
void WriteEFenceToFLASH(uint8_t EFenceNum, uint8_t *Data, uint16_t DataLen, uint8_t PackageOrder)
{
	uint32_t StartAddr = 0;
	
	StartAddr = FLASH_EFENCE_FLASH_START + FLASH_EFENCE_FLASH_LEN * (EFenceNum - 1) + 1089 * (PackageOrder - 1);
		
	//将电子围栏写入flash中
	W25qxx_WritePages(Data, StartAddr, DataLen);
	
	#if 0
	uint8_t Datad[4096];
	memset(Datad, 0, sizeof(Datad));
	FlashReadData(Datad, FLASH_EFENCE_FLASH_START + FLASH_EFENCE_FLASH_LEN * (EFenceNum - 1) , FLASH_EFENCE_FLASH_LEN, 0);
	//if ((Datad[0] != 0X5A) && (Datad[1] != 0XA5))
	//{
	//	return;
	//}
	#endif
}

/******************************************************************
函数名称: void ReadWorkPlanFomBuff(void)
功能描述: 从Buff内读取施工计划的函数
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：agj
******************************************************************/
void ReadWorkPlanFomBuff(uint8_t index)
{    
	uint32_t Len = 0;
	uint8_t AreaCount = 0;
	uint8_t Data[4096];
	uint8_t Offset = 32;
	_EFENCE g_EFence;
	
	memset(&SelectEFenceInfo, 0, sizeof(SelectEFenceInfo));
	memset(&SelectEFenceInfo.order,0xFF,sizeof(SelectEFenceInfo.order));
	memset(&g_EFence, 0, sizeof(g_EFence));
	memset(Data, 0, sizeof(Data));
	uint32_t SatartAddr = FLASH_EFENCE_FLASH_START + FLASH_EFENCE_FLASH_LEN * index;
	FlashReadData(Data, SatartAddr, FLASH_EFENCE_FLASH_LEN, 0);
	
	for (uint16_t i = 0; i < FLASH_EFENCE_FLASH_LEN; i++)
	{
		if (((i + 59) > FLASH_EFENCE_FLASH_LEN))
		{
			continue;
		}
		
		if ((Data[i] != 0XFA) || (Data[i + 1] != 0X00) || (Data[i + 2] != 0X04))
		{
			continue;
		}
		
		//数据总长度
		uint16_t Totaldatalen = (Data[i + 31]<<8) + Data[i + 30];
		
		if ((Data[i + 32] != 0X5A) || (Data[i + 33] != 0XA5))
		{
			continue;
		}
		
		//取到长度
		uint16_t DataLen = (Data[i + 37] << 8) + Data[i + 36];
		//长度判断(这里没有判断加上区域的长度)
		if ((DataLen < 26) || (DataLen > (Totaldatalen - 4)))
		{	
			continue;
		}
		
		//校验和验证
		uint8_t calc_sum = CheckSum(DataLen + 6, Data + 32 + i );
		if(calc_sum != Data[i + DataLen + 38]) 
		{	
			continue ;
		}
		
		//取出这是第几包
		uint8_t PackageOrder = Data[i + 35];
	
		//从第一包中取出施工时间
		//if (PackageOrder == 1)
		//{
			//取出施工时间
		//	memcpy((uint8_t*)&g_WorkTime.start, Data + i + 46, 6);
		//	memcpy((uint8_t*)&g_WorkTime.stop, Data + i + 52, 6);
		//}
		
		//取出区域总个数
		AreaCount = Data[i + 58];
		SelectEFenceInfo.total = SelectEFenceInfo.total + AreaCount;
		
		uint8_t *Ptr = &g_EFence.data[0][0];
		memcpy(Ptr + Len, &Data[i + 59], DataLen - 25);
		
		Len = Len + DataLen - 25;
	}

	//取出电子围栏ID
	memcpy(SelectEFenceInfo.order, Data + 3, 4);
	//取出施工区域类型
	SelectEFenceInfo.IsBigArea = Data[1085];
	
	uint8_t j = 0;
	for(uint8_t i=0;i<SelectEFenceInfo.total;i++)    /*几个区域*/
	{
		g_EFenceValid[i].variable.AreaType = g_EFence.data[i][0];    /*区域类型*/
		g_EFenceValid[i].variable.AreaNum = g_EFence.data[i][1];     /*区域编号*/
		memcpy(g_EFenceValid[i].variable.reserve,&g_EFence.data[i][2],6);
		for(j = 0; j < 15; j++)             /*一个区域有几个有效坐标点*/
		{	
			if(areAllElementsZero((uint8_t *)&g_EFence.data[i][8+j*8],8) == 1)
			{
				//存放每个区域里15个坐标点的经纬度信息
				g_EFenceValid[i].fence[j].latitude = (double)(g_EFence.data[i][8+j*8] | (g_EFence.data[i][9+j*8]<<8) | 
					(g_EFence.data[i][10+j*8]<<16) | (g_EFence.data[i][11+j*8]<<24))/10000000;     /*纬度*/
				g_EFenceValid[i].fence[j].longitude = (double)(g_EFence.data[i][12+j*8] | (g_EFence.data[i][13+j*8]<<8) | 
					(g_EFence.data[i][14+j*8]<<16) | (g_EFence.data[i][15+j*8]<<24))/10000000;	/*经度*/			
			}
			else
				break;
		}
		//区域里面有效坐标点的个数
		g_EFenceValid[i].num = j;
	}
}
int selected_idx = -1;
//double distance[MAX_WORK_PLAN_NUM];
/******************************************************************
函数名称: void SelectWorkPlan(void)
功能描述: 比较施工中心点位置，选取距离最近施工计划
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：agj
******************************************************************/
void SelectWorkPlan(void)
{
	uint8_t j;
	//int selected_idx = -1;
	double distance[MAX_WORK_PLAN_NUM];
	
	/*收到施工计划更新buff*/
	/*根据经纬度计算距离，选取计划*/
	g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
	g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;
	double Longitude = (double)g_CardLocaData.Longitude /10000000;
	double Latitude = (double)g_CardLocaData.Latitude /10000000;
	
	if((g_CardLocaData.Longitude != 0)&&(g_CardLocaData.Latitude!=0)&&(g_SysTime.year!=0)&&(g_SysTime.month!=0))
	{
		memset(distance,0,sizeof(distance));
		for(j=0;j<MAX_WORK_PLAN_NUM;j++)
		{
			distance[j]=haversine_distance(g_WorkPlanSummary[j].lat,g_WorkPlanSummary[j].lng,Latitude,Longitude);
		}
		//找到符合条件的下标
		selected_idx = select_plan(distance);
		/*选取计划*/
		if(selected_idx != -1)
		{
			if(selected_idx != last_selected_idx)
			{
				effectiveplan = 1;
			}

			last_selected_idx = selected_idx;
		}
		else 
		{
			memset(&SelectEFenceInfo,0,sizeof(SelectEFence));
			memset(&SelectEFenceInfo.order,0xFF,sizeof(SelectEFenceInfo.order));
			memset((uint8_t *)&g_EFenceValid,0,sizeof(g_EFenceValid));
			selected_idx = -1;
			last_selected_idx = selected_idx;
		}
	}
	if(effectiveplan == 1)
	{
		if (selected_idx != -1)
		{
			memset(&SelectEFenceInfo,0,sizeof(SelectEFence));
			memset(&SelectEFenceInfo.order,0xFF,sizeof(SelectEFenceInfo.order));
			memset((uint8_t *)&g_EFenceValid,0,sizeof(g_EFenceValid));
			ReadWorkPlanFomBuff(selected_idx);
		}
		
		effectiveplan = 0;
	}
}

double haversine_distance(double lat1, double lon1, double lat2, double lon2)
{
	// 使用更精确的弧度转换系数
    const double DEG_TO_RAD = 0.01745329;
    
    // 转换为弧度（
    double phi1 = lat1 * DEG_TO_RAD;
    double phi2 = lat2 * DEG_TO_RAD;
    double delta_phi = (lat2 - lat1) * DEG_TO_RAD;
    double delta_lambda = (lon2 - lon1) * DEG_TO_RAD;

    // Haversine公式计算（使用双精度数学函数）
    double a = sin(delta_phi / 2.0) * sin(delta_phi / 2.0) +
              cos(phi1) * cos(phi2) *
              sin(delta_lambda / 2.0) * sin(delta_lambda / 2.0);

    // 防止计算误差导致a超出[0,1]范围
    a = fmax(0.0, fmin(a, 1.0));

    // 计算大圆距离
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    
    // 最终计算结果
    double distance = EARTH_RADIUS * c;

    // 添加溢出保护（虽然正常情况下不需要）
    return isfinite(distance) ? distance : -1.0; // 返回-1表示计算错误
}
///******************************************************************
//函数名称: void WriteMobileNumToFLASH(void)
//功能描述: Flash内写入本机号的函数
//参数输入: 无
//参数输出: 
//返  回  值: 无
//其      他: 无
//编写者：agj
//******************************************************************/
//void WriteMobileNumToFLASH(void)
//{
//	g_Mobile_Sim.Crc16 = CRC16((uint8_t*)&g_Mobile_Sim, sizeof(g_Mobile_Sim)-2);
//	W25qxx_EraseSector(FLASH_MOBILE_START/4096);
//	W25qxx_WritePages((uint8_t*)&g_Mobile_Sim,FLASH_MOBILE_START,sizeof(g_Mobile_Sim));	
//}
///******************************************************************
//函数名称: void ReadMobileNumFromFLASH(void)
//功能描述: 从Flash内读取本机号的函数
//参数输入: 无
//参数输出: 
//返  回  值: 无
//其      他: 无
//编写者：agj
//******************************************************************/
//void ReadMobileNumFromFLASH(void)
//{    
//	W25qxx_ReadBytes((uint8_t*)&g_Mobile_Sim , FLASH_MOBILE_START , sizeof( g_Mobile_Sim ));	
//	if(g_Mobile_Sim.Crc16 != CRC16((uint8_t*)&g_Mobile_Sim,sizeof(g_Mobile_Sim)-2))
//	{
//		memset((uint8_t*)&g_Mobile_Sim,0,sizeof(g_Mobile_Sim));
//		WriteMobileNumToFLASH();
//	}
//}
/******************************************************************
函数名称: void compare_datetime(void)
功能描述: 时间比较函数（返回：a早于b返回-1，等于返回0，晚于返回1）
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：agj
******************************************************************/
int compare_datetime(DATETIME a, DATETIME b) {
    if (a.year != b.year)   return a.year < b.year ? -1 : 1;
    if (a.month != b.month) return a.month < b.month ? -1 : 1;
    if (a.date != b.date)   return a.date < b.date ? -1 : 1;
    if (a.hour != b.hour)   return a.hour < b.hour ? -1 : 1;
    if (a.minute != b.minute) return a.minute < b.minute ? -1 : 1;
    return a.second < b.second ? -1 : (a.second > b.second ? 1 : 0);
}

int compare_datetime30(DATETIME a, DATETIME b,uint8_t flag) 
{
	if (flag == 0)
	{
		if (a.year != b.year)   return a.year < b.year ? -1 : 1;
		if (a.month != b.month) return a.month < b.month ? -1 : 1;
		//if (a.date != b.date)   return a.date < b.date ? -1 : 1;
		// 计算当日时间的总秒数差
		uint32_t a_sec = a.date * 24 * 3600 + a.hour * 3600 + a.minute * 60 + a.second;
		uint32_t b_sec = b.date * 24 * 3600 + b.hour * 3600 + b.minute * 60 + b.second;
		int32_t diff = b_sec - a_sec;
		
		if (diff < 0) return 1;    // a的时间晚于b
		if (diff == 0) return 0;    // 时间相同
		return diff > 1800 ? -1 : 1; // 时间差是否≥半小时
	}
	else
	{
		if (a.year != b.year)   return a.year > b.year ? -1 : 1;
		if (a.month != b.month) return a.month > b.month ? -1 : 1;
		//if (a.date != b.date)   return a.date > b.date ? -1 : 1;
		// 计算当日时间的总秒数差
		uint32_t a_sec = a.date * 24 * 3600 + a.hour * 3600 + a.minute * 60 + a.second;
		uint32_t b_sec = b.date * 24 * 3600 + b.hour * 3600 + b.minute * 60 + b.second;
		int32_t diff = b_sec - a_sec;
		
		if (diff > 0) return 1;    // b的时间晚于a
		if (diff == 0) return 0;    // 时间相同
		return diff < -1800 ? -1 : 1; // 时间差是否≥半小时
	}
}

int compare_datetime60(DATETIME a, DATETIME b,uint8_t flag) 
{
	if (flag == 0)
	{
		if (a.year != b.year)   return a.year < b.year ? -1 : 1;
		if (a.month != b.month) return a.month < b.month ? -1 : 1;
		if (a.date != b.date)   return a.date < b.date ? -1 : 1;
		// 计算当日时间的总秒数差
		uint32_t a_sec = a.hour * 3600 + a.minute * 60 + a.second;
		uint32_t b_sec = b.hour * 3600 + b.minute * 60 + b.second;
		int32_t diff = b_sec - a_sec;
		
		if (diff < 0) return 1;    // a的时间晚于b
		if (diff == 0) return 0;    // 时间相同
		return diff > 3600 ? -1 : 1; // 时间差是否≥半小时
	}
	else
	{
		if (a.year != b.year)   return a.year > b.year ? -1 : 1;
		if (a.month != b.month) return a.month > b.month ? -1 : 1;
		if (a.date != b.date)   return a.date > b.date ? -1 : 1;
		// 计算当日时间的总秒数差
		uint32_t a_sec = a.hour * 3600 + a.minute * 60 + a.second;
		uint32_t b_sec = b.hour * 3600 + b.minute * 60 + b.second;
		int32_t diff = b_sec - a_sec;
		
		if (diff > 0) return 1;    // b的时间晚于a
		if (diff == 0) return 0;    // 时间相同
		return diff < -3600 ? -1 : 1; // 时间差是否≥半小时
	}
}

int compare_time_second(DATETIME time1, DATETIME time2, int second)
{
	uint32_t time_t1,time_t2 ;
	time_t1 = CalHowManyMinuteFromNow(time1) * 60;
	time_t2 = CalHowManyMinuteFromNow(time2) * 60;
	
	if ((time_t1 - time_t2) > second)
	{
		return 1;
	}
	else if ((time_t1 - time_t2) == second)
	{
		return 0;
	}
	else if ((time_t1 - time_t2) < second)
	{
		return -1;
	}
}

/******************************************************************
函数名称: void select_plan(void)
功能描述: 选取计划
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：agj
******************************************************************/
#if 0
int select_plan(double *distance) 
{
    /*------------------------------------*
     * 第一阶段：过滤所有过期计划          *
     *------------------------------------*/
    int active_indexes[MAX_WORK_PLAN_NUM] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int active_count = 0;
    
    // 筛选未过期且有效距离的计划且从flash中读取出的有错误的计划
    for (int i = 0; i < MAX_WORK_PLAN_NUM; i++) 
	{
		//g_WorkPlanSummary[i].isanalysis表示解析是否成功
        if ((distance[i] <= 0) || (g_WorkPlanSummary[i].isanalysis == 0)) continue;
		//选择未开始或者结束不到30分钟的施工计划
        if (compare_datetime30(g_WorkPlanSummary[i].stop, g_SysTime,0) >= 0) 
		{
            active_indexes[active_count++] = i;
        }
    }
    if (active_count == 0) 
	{
		return -1;
	}
	if (active_count == 1) 
	{
		return active_indexes[0];
	}
    /*------------------------------------*
     * 第二阶段：找出最近距离候选          *
     *------------------------------------*/
    double min_val = INFINITY;
    int min_candidates[MAX_WORK_PLAN_NUM] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int min_count = 0;
    
    for (int i = 0; i < active_count; i++) 
	{
        int idx = active_indexes[i];
        if (distance[idx] < min_val) 
		{
            min_val = distance[idx];
            min_count = 0;
            min_candidates[min_count++] = idx;
        } 
        else if (fabs(distance[idx] - min_val) < 1e-6) 
		{
            min_candidates[min_count++] = idx;
        }
    }
    if (min_count == 0) 
	{
		return -1;
	}
//	if (min_count == 1) return min_candidates[0];
    /*------------------------------------*
     * 第三阶段：处理相同距离的候选        *
     *------------------------------------*/
	#if 1
    // Case 1: 优选正在执行的计划
    for (int i = 0; i < min_count; i++) 
	{
        int idx = min_candidates[i];
		//选出已经开始的计划
        if (compare_datetime(g_WorkPlanSummary[idx].start, g_SysTime) <= 0) 
		{
            return idx;
        }
    }
	#else
	//优先选出正在执行的计划
	for (int i = 0; i < min_count; i++) 
	{
        int idx = min_candidates[i];
		DATETIME StopTime;
		StopTime.year = g_WorkPlanSummary[idx].stop.year;
		StopTime.month = g_WorkPlanSummary[idx].stop.month;
		StopTime.date = g_WorkPlanSummary[idx].stop.date;
		StopTime.hour = g_WorkPlanSummary[idx].stop.hour;
		StopTime.minute = g_WorkPlanSummary[idx].stop.minute + 1;
		StopTime.second = g_WorkPlanSummary[idx].stop.second;
        if ((compare_datetime(g_WorkPlanSummary[idx].start, g_SysTime) <= 0) &&
			(compare_datetime(StopTime, g_SysTime) >= 0)) 
		{
            return idx;
        }
    }
	
	//再选择结束不到30分钟的计划
	for (int i = 0; i < min_count; i++) 
	{
        int idx = min_candidates[i];
		aaa = compare_datetime(g_WorkPlanSummary[idx].stop, g_SysTime);
		bbb = compare_datetime30(g_SysTime, g_WorkPlanSummary[idx].stop,1);
        if ((compare_datetime(g_WorkPlanSummary[idx].stop, g_SysTime) <= 0) &&
			compare_datetime30(g_SysTime, g_WorkPlanSummary[idx].stop,1) >= 0) 
		{
            return idx;
        }
    }
	#endif
    // Case 2: 选择最近的未来计划
    int nearest_idx = -1;
    DATETIME nearest_start = {0};
    for (int i = 0; i < min_count; i++) 
	{
        int idx = min_candidates[i];
		//选出还未开始的计划   a晚于b
        if (compare_datetime(g_WorkPlanSummary[idx].start, g_SysTime) >= 0) 
		{
			//选出未来最早的时间
            if (nearest_idx == -1 || 
                compare_datetime(g_WorkPlanSummary[idx].start, nearest_start) < 0) 
			{
                nearest_idx = idx;
                nearest_start = g_WorkPlanSummary[idx].start;
            }
        }
    }
	
    // 严格无保底：仅返回有效结果
    return nearest_idx; // 找不到时自动返回-1
}
#else
int select_plan(double *distance) 
{
    /*------------------------------------*
     * 第一阶段：过滤所有过期计划          *
     *------------------------------------*/
    int active_indexes[MAX_WORK_PLAN_NUM] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int active_count = 0;
    
    // 筛选未过期且有效距离的计划且从flash中读取出的有错误的计划
    for (int i = 0; i < MAX_WORK_PLAN_NUM; i++) 
	{
		//限制距离小于10公里
		if ((distance[i] <= 0) || (distance[i] > 10000) || (g_WorkPlanSummary[i].isanalysis == 0))
		{
			continue;
		}
		//选择开始前30分钟或者结束不到30分钟的施工计划
		if (((JudgTimeOrder(g_WorkPlanSummary[i].start, g_SysTime, 30) == SAME) || 
				(JudgTimeOrder(g_WorkPlanSummary[i].start, g_SysTime, 30) == BEFORE)) && 
				((JudgTimeOrder(g_SysTime, g_WorkPlanSummary[i].stop, 30) == SAME) ||
				(JudgTimeOrder(g_SysTime, g_WorkPlanSummary[i].stop, 30) == BEFORE)))
		{
			active_indexes[active_count++] = i;
		}
    }
    if (active_count == 0) 
	{
		return -1;
	}
	if (active_count == 1) 
	{
		return active_indexes[0];
	}
    /*------------------------------------*
     * 第二阶段：找出最近距离候选          *
     *------------------------------------*/
    double min_val = INFINITY;
    int min_candidates[MAX_WORK_PLAN_NUM] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int min_count = 0;
    
    for (int i = 0; i < active_count; i++) 
	{
        int idx = active_indexes[i];
        if (distance[idx] < min_val) 
		{
            min_val = distance[idx];
            min_count = 0;
            min_candidates[min_count++] = idx;
        } 
        else if (fabs(distance[idx] - min_val) < 1e-6) 
		{
            min_candidates[min_count++] = idx;
        }
    }
    if (min_count == 0) 
	{
		return -1;
	}
//	if (min_count == 1) return min_candidates[0];
    /*------------------------------------*
     * 第三阶段：处理相同距离的候选        *
     *------------------------------------*/
	//优先选出正在执行的计划
	for (int i = 0; i < min_count; i++) 
	{
        int idx = min_candidates[i];
		
		if (((JudgTimeOrder(g_SysTime, g_WorkPlanSummary[idx].start, 0) == SAME) || 
				(JudgTimeOrder(g_SysTime, g_WorkPlanSummary[idx].start, 0) == AFTER)) && 
				((JudgTimeOrder(g_SysTime, g_WorkPlanSummary[idx].stop, 0) == SAME) ||
				(JudgTimeOrder(g_SysTime, g_WorkPlanSummary[idx].stop, 0) == BEFORE)))
		{
			return idx;
		}
    }
	
	//再选择结束不到30分钟的计划
	for (int i = 0; i < min_count; i++) 
	{
        int idx = min_candidates[i];
		
		if ((JudgTimeOrder(g_SysTime, g_WorkPlanSummary[idx].stop, 0) == AFTER) && 
				((JudgTimeOrder(g_SysTime, g_WorkPlanSummary[idx].stop, 30) == SAME) ||
				(JudgTimeOrder(g_SysTime, g_WorkPlanSummary[idx].stop, 30) == BEFORE)))
		{
			return idx;
		}
    }

    // Case 2: 选择最近的未来计划
    int nearest_idx = -1;
    DATETIME nearest_start = {0};
    for (int i = 0; i < min_count; i++) 
	{
        int idx = min_candidates[i];
		//选出还未开始的计划
        if (JudgTimeOrder(g_SysTime, g_WorkPlanSummary[idx].start, 0) == BEFORE)
		{
			//选出未来最早的时间
            if (nearest_idx == -1 || 
                JudgTimeOrder(g_WorkPlanSummary[idx].start, nearest_start, 0) == BEFORE) 
			{
                nearest_idx = idx;
                nearest_start = g_WorkPlanSummary[idx].start;
            }
        }
    }
	
    // 严格无保底：仅返回有效结果
    return nearest_idx; // 找不到时自动返回-1
}
#endif

int is_leap_year(int year) {
    if (year % 4 != 0) return 0;
    else if (year % 100 != 0) return 1;
    else if (year % 400 == 0) return 1;
    else return 0;
}

int days_in_month(int year, int month) {
    switch (month) {
        case 1: return 31;
        case 2: return is_leap_year(year) ? 29 : 28;
        case 3: return 31;
        case 4: return 30;
        case 5: return 31;
        case 6: return 30;
        case 7: return 31;
        case 8: return 31;
        case 9: return 30;
        case 10: return 31;
        case 11: return 30;
        case 12: return 31;
        default: return 0;
    }
}

long long datetime_to_seconds(DATETIME a,DATETIME dt) {
    long long days = 0;
    for (int y = a.year; y < dt.year; y++) {
        days += 365 + is_leap_year(y);
    }
    for (int m = 1; m < dt.month; m++) {
        days += days_in_month(dt.year, m);
    }
    days += dt.date - 1;
    return days * 86400LL + dt.hour * 3600LL + dt.minute * 60LL + dt.second;
}

int judge_datetime(DATETIME a, DATETIME b,uint32_t sec) {
    long long a_sec = datetime_to_seconds(a,a);
    long long b_sec = datetime_to_seconds(a,b);
    long long diff = b_sec - a_sec;
    return (diff >= sec) ? 1 : (diff < 0 ? -1 : 0);
}

/*计算瞬时速度*/
float calculateV(Workcenter_point *Positon, uint32_t *time)
{
	if(g_PosQuality >=4)
	{
		float Velocity =(float)haversine_distance(Positon[0].lat,Positon[0].lng,Positon[1].lat,Positon[1].lng)/(float)((time[1]-time[0])/1000.0f);
		dealflag = 1;
		return Velocity;
	}
	else
	{
		return 0;
	}
}

/*                                                           
 * @brief	WriteSysPar
 *
 * @param	存系统参数
 *
 * @return  void
 */
void WriteSysPar(void)
{
	W25qxx_EraseSector(0);
	g_SysPar.crc16 = CRC16((uint8_t*)&g_SysPar,sizeof(g_SysPar)-2);
	W25qxx_WriteBytes((uint8_t*)&g_SysPar,FLASH_SYSPARAMETER_START,sizeof(g_SysPar));
}

/**
 * @brief	读系统参数
 *
 * @param	void
 *
 * @return  void
 */
void ReadSysPar(void)
{		
	W25qxx_ReadBytes((uint8_t*)&g_SysPar,FLASH_SYSPARAMETER_START,sizeof(g_SysPar));
	g_ServerInfo.Crc16 = CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2);
	if(CheckCRC16((uint8_t*)&g_SysPar,sizeof(g_SysPar)) == false)
	{
		g_SysPar.cors = 0;
		g_SysPar.equiptype = 0;
		memset(g_SysPar.res,0,sizeof(g_SysPar.res));
		WriteSysPar();
	}		
}

/**
 * @brief	存DTU参数
 *
 * @param	void
 *
 * @return  void
 */
void WriteDTUSysPar(void)
{
	g_MobileInFlash.Crc16 = CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2);
	g_ServerInfo.Crc16 = CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2);
	W25qxx_EraseSector(FLASH_MOBILE_START/4096);
	W25qxx_WriteBytes((uint8_t*)&g_ServerInfo,FLASH_IPPORT_START,sizeof(g_ServerInfo));
	W25qxx_WriteBytes((uint8_t*)&g_MobileInFlash,FLASH_MOBILE_START,sizeof(g_MobileInFlash));
}

/**
 * @brief	读DTU参数
 *
 * @param	void
 *
 * @return  void
 */
void ReadDTUSysPar(void)
{
	uint8_t err = 0,i;	
	uint8_t cmpbuffer[2][4];
    
    memset(cmpbuffer[0],0,sizeof(cmpbuffer[0]));
    memset(cmpbuffer[1],0xff,sizeof(cmpbuffer[1]));
	
	W25qxx_ReadBytes((uint8_t*)&g_MobileInFlash,FLASH_MOBILE_START,sizeof(g_MobileInFlash));
	W25qxx_ReadBytes((uint8_t*)&g_ServerInfo,FLASH_IPPORT_START,sizeof(g_ServerInfo));
	if(g_MobileInFlash.Crc16 != CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2))
	{
		memset(g_MobileInFlash.mobileno,0,sizeof(g_MobileInFlash.mobileno));    //agj add
        g_MobileInFlash.Crc16 = CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2);
		g_GetMyMobileOKFlag = false;        //获取本机号标识
		err = 1;
	}
	
	if( g_ServerInfo.Crc16 != CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2))
	{
        memset(g_ServerInfo.serveripaddr,0,sizeof(g_ServerInfo.serveripaddr));
		memset(g_ServerInfo.serverportaddr,0,sizeof(g_ServerInfo.serverportaddr));
		g_ServerInfo.Crc16 = CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2);
		err = 1;
	}
	for(i=0;i<4;i++)
    {
        if( memcmp(g_ServerInfo.serveripaddr[i],cmpbuffer[0],sizeof(g_ServerInfo.serveripaddr[i])) == 0 || 
            memcmp(g_ServerInfo.serveripaddr[i],cmpbuffer[1],sizeof(g_ServerInfo.serveripaddr[i])) == 0 ||
            g_ServerInfo.serverportaddr[i] == 0 || g_ServerInfo.serverportaddr[i] == 0xff)
        {
            memcpy(g_ServerInfo.serveripaddr[i],g_IPBuffer[i],sizeof(g_ServerInfo.serveripaddr[i]));
            g_ServerInfo.serverportaddr[i] = g_PortNo[i];
			err = 1;
        }
    }
	if(err == 1)
	{
		g_ServerInfo.Crc16 = CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2);
		WriteDTUSysPar();
	}
}
/**
 * @brief	存本机号
 *
 * @param	void
 *
 * @return  void
 */
void WriteMobileNoToFLASH(void)
{
	g_MobileInFlash.Crc16 = CRC16((uint8_t*)&g_MobileInFlash, sizeof(g_MobileInFlash)-2);	
	WriteDTUSysPar();
}

/**
 * @brief	读取本机号
 *
 * @param	void
 *
 * @return  void
 */
bool ReadMobileNoFromFLASH(void)
{
    uint8_t cmpbuf[5];

	memset(cmpbuf,0,sizeof(cmpbuf));
   
    W25qxx_ReadBytes((uint8_t*)&g_MobileInFlash,FLASH_MOBILE_START,sizeof(g_MobileInFlash));
	if(g_MobileInFlash.Crc16 != CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2))
	{
		memset(g_MobileInFlash.mobileno,0,sizeof(g_MobileInFlash.mobileno));   
        g_MobileInFlash.Crc16 = CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2);
		WriteDTUSysPar();
        g_GetMyMobileOKFlag = false;        //获取本机号标识
        return false;
	}
    else
    {
        if(CheckNumIsBCD(g_MobileInFlash.mobileno[4],5) == true && memcmp(g_MobileInFlash.mobileno[4],cmpbuf,5)!=0)
            g_GetMyMobileOKFlag = true;
    }
    return true;
}

/******************************************************************
函数名称: void CmpIPAndPort(void)
功能描述: 判断IP和端口号若为0或者FF，替换为默认的值
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：
******************************************************************/
void CmpIPAndPort(void)
{
    uint8_t cmpbuffer[2][4];
    uint8_t i;
    
    memset(cmpbuffer[0],0,sizeof(cmpbuffer[0]));
    memset(cmpbuffer[1],0xff,sizeof(cmpbuffer[1]));
    for(i=0;i<3;i++)
    {	
        if( memcmp(g_ServerInfo.serveripaddr[i],cmpbuffer[0],sizeof(g_ServerInfo.serveripaddr[i])) == 0 || 
            memcmp(g_ServerInfo.serveripaddr[i],cmpbuffer[1],sizeof(g_ServerInfo.serveripaddr[i])) == 0 ||
            g_ServerInfo.serverportaddr[i] == 0 || g_ServerInfo.serverportaddr[i] == 0xff)
        {
            memcpy(g_ServerInfo.serveripaddr[i],g_IPBuffer[i],sizeof(g_ServerInfo.serveripaddr[i]));
            g_ServerInfo.serverportaddr[i] = g_PortNo[i];
        }
    }
}

/**
 * @brief	存服务器信息
 *
 * @param	void
 *
 * @return  void
 */
void WriteServerInfoPar(void)
{
	g_ServerInfo.Crc16 = CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2);
	WriteDTUSysPar();
}

/**
 * @brief	读取服务器信息
 *
 * @param	void
 *
 * @return  void
 */
bool ReadServerInfoPar(void)    
{    
	W25qxx_ReadBytes((uint8_t*)&g_ServerInfo,FLASH_IPPORT_START,sizeof(g_ServerInfo));
	if( g_ServerInfo.Crc16 != CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2))
	{
        memset(g_ServerInfo.serveripaddr,0,sizeof(g_ServerInfo.serveripaddr));
		memset(g_ServerInfo.serverportaddr,0,sizeof(g_ServerInfo.serverportaddr));
		g_ServerInfo.Crc16 = CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2);
		WriteDTUSysPar();
        return false;
	}
    return true;
}

/******************************************************************
函数名称: void WriteSIMTYPEFromFLASH(void)
功能描述: 在Flash内写入手机卡类型的函数
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：agj
******************************************************************/
void WriteSIMTYPEFromFLASH(void)
{    		
	g_SIMType.Crc16 = CRC16((uint8_t*)&g_SIMType,sizeof(g_SIMType)-2);
	W25qxx_EraseSector(SET_SIMTYPE_FLASH_ADDR/4096);
	W25qxx_WritePages((uint8_t*)&g_SIMType,SET_SIMTYPE_FLASH_ADDR,sizeof(g_SIMType));		
}
/******************************************************************
函数名称: void ReadSIMTYPEFromFLASH(void)
功能描述: 从Flash内读取手机卡类型的函数
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：agj
******************************************************************/
void ReadSIMTYPEFromFLASH(void)
{    
	W25qxx_ReadBytes((uint8_t*)&g_SIMType , SET_SIMTYPE_FLASH_ADDR , sizeof(g_SIMType));	
	if(g_SIMType.Crc16 != CRC16((uint8_t*)&g_SIMType,sizeof(g_SIMType)-2))
	{
		memset((uint8_t*)&g_SIMType,0,sizeof(g_SIMType));
		g_SIMType.num = 0X01;
		WriteSIMTYPEFromFLASH();
	}
}

/******************************************************************
函数名称: void WriteVoiceSwitchToFLASH(void)
功能描述: 在Flash内写入单点解定位语音开关的函数
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：agj
******************************************************************/
void WriteVoiceSwitchToFLASH(void)
{    		
	g_VoiceSwitch.Crc16 = CRC16((uint8_t*)&g_VoiceSwitch,sizeof(g_VoiceSwitch)-2);
	W25qxx_EraseSector(FLASH_ON_OFF_VOICE_START/4096);
	W25qxx_WritePages((uint8_t*)&g_VoiceSwitch,FLASH_ON_OFF_VOICE_START,sizeof(g_VoiceSwitch));		
}
/******************************************************************
函数名称: void ReadVoiceSwitchFromFLASH(void)
功能描述: 从Flash内读取单点解定位语音开关的函数
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：agj
******************************************************************/
void ReadVoiceSwitchFromFLASH(void)
{    
	W25qxx_ReadBytes((uint8_t*)&g_VoiceSwitch , FLASH_ON_OFF_VOICE_START , sizeof(g_VoiceSwitch));	
	if(g_VoiceSwitch.Crc16 != CRC16((uint8_t*)&g_VoiceSwitch,sizeof(g_VoiceSwitch)-2))
	{
		memset((uint8_t*)&g_VoiceSwitch,0,sizeof(g_VoiceSwitch));
		g_VoiceSwitch.num = 0X00;
		WriteVoiceSwitchToFLASH();
	}
}

/******************************************************************
函数名称: Bool WritePowerFlagToFlash(void)
功能描述: 写Flash的电源标识
参数输入: 无
参数输出: 无
返 回 值: 无
其    他: agj
******************************************************************/
void WritePowerCtlFlagToFlash(uint8_t flag)
{
    uint32_t addr;
	uint8_t buffer[6];
	
	W25qxx_ReadBytes(buffer,FLASH_POWERCTL_START+1,6);
	
	addr = FLASH_POWERCTL_START/4096;
	W25qxx_EraseSector(addr);
	
	W25qxx_WriteBytes(&flag,FLASH_POWERCTL_START,1);
	W25qxx_WriteBytes(buffer,FLASH_POWERCTL_START+1,6);
}

void WritePowerCtlNoTolash(uint8_t *number)
{
    uint32_t addr;
	
	W25qxx_ReadBytes(&g_ResetPowerFlag,FLASH_POWERCTL_START,1);
	
	addr = FLASH_POWERCTL_START/4096;
	W25qxx_EraseSector(addr);
	
	if(CheckNumIsBCD(number,5) == false)
        memset(number,0xff,6);
	
	W25qxx_WriteBytes(&g_ResetPowerFlag,FLASH_POWERCTL_START,1);
	W25qxx_WriteBytes(number,FLASH_POWERCTL_START+1,6);
}

/******************************************************************
函数名称: Bool ReadPowerFlagFromFlash(void)
功能描述: 读Flash的电源标识
参数输入: 无
参数输出: 无
返 回 值: 无
其    他: agj
******************************************************************/
void ReadPowerCtlFlagFromFlash(void)
{
    g_ResetPowerFlag = 0;
	W25qxx_ReadBytes(&g_ResetPowerFlag,FLASH_POWERCTL_START,1);
	
    if(g_ResetPowerFlag == 2)
    {
        g_AskPowerControlFlag = 1; 
    }
}

bool ReadPowerCtlNoFromFlash(uint8_t *number)
{
    g_ResetPowerFlag = 0;
	W25qxx_ReadBytes(number,FLASH_POWERCTL_START+1,6);
	
	if(CheckNumIsBCD(number,5) == true)
        return true;
    else
    {
        memset(number,0xff,6);
        return false;
    }
}

/******************************************************************
函数名称: void WriteVoiceFileSizeToFlash
功能描述: 更新程序参数写入到Flash
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void WriteVoiceFileSizeToFlash(void)
{
	FlashReadData(g_CommanBuf , ADDR_EEPROM_UPDATA , 512 , 0);
	VoiceFileSize.Crc16 = CalcCRC16((uint8_t*)&VoiceFileSize, sizeof(VoiceFileSize) - 2);
	memcpy(g_CommanBuf + ADDR_UPDATE_VOICE_FLAG_OFFSET,( uint8_t* )&VoiceFileSize,sizeof(VoiceFileSize));	
	W25qxx_EraseSector(ADDR_EEPROM_UPDATA / w25qxx.SectorSize);
	W25qxx_WritePages(g_CommanBuf, ADDR_EEPROM_UPDATA , 512);
}

/******************************************************************
函数名称: Bool ReadVoiceFileSizeFromFlash(void)
功能描述: 从Flash读更新程序参数
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
bool ReadVoiceFileSizeFromFlash(void)
{
	uint16_t crc;
	
	W25qxx_ReadBytes((uint8_t*)&VoiceFileSize,ADDR_EEPROM_UPDATA + ADDR_UPDATE_VOICE_FLAG_OFFSET, sizeof(VoiceFileSize));
	crc = CalcCRC16((uint8_t*)&VoiceFileSize, sizeof(VoiceFileSize) - 2);
	if (crc != VoiceFileSize.Crc16)
	{
		VoiceFileSize.FileSize = 0;
		WriteVoiceFileSizeToFlash();
		return false;
	}
	return true;
}

/******************************************************************
函数名称: int16_t WriteIDToFlash(void)
功能描述: 写ID到Flash
参数输入: 
参数输出: 
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
void WriteIDToFlash(void)
{
	uint16_t crc16;	
	crc16 = CalcCRC16((uint8_t*)&g_IDInFlash, sizeof(g_IDInFlash) - 2);			//手机号定义Flash位置为扇区头，需要先执行
	g_IDInFlash.Crc16 = crc16;
	FlashWriteData((uint8_t*)&g_IDInFlash, FLASH_ID_START, sizeof(g_IDInFlash));	
}

/******************************************************************
函数名称: int16_t ReadIPAndMobileFromFlash
功能描述: 从Flash读ID
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReadIDFromFlash(void)
{	
	uint8_t cmpipbuf[2][7];												
	memset(cmpipbuf[0],0,sizeof(cmpipbuf[0]));
	memset(cmpipbuf[1],0xff,sizeof(cmpipbuf[1]));	
	uint8_t cmpidbuf[7];
	
	memset(cmpidbuf,0,sizeof(cmpipbuf[0]));
	cmpidbuf[0] = 0XFF;
	cmpidbuf[1] = 0XF0;
	
	W25qxx_ReadBytes((uint8_t*)&g_IDInFlash, FLASH_ID_START, sizeof(g_IDInFlash));
	
	/* 检查手机号校验和 */		
	if( memcmp(g_IDInFlash.IDnum,cmpipbuf[0],7) == 0 || 
		memcmp(g_IDInFlash.IDnum,cmpipbuf[1],7) == 0 )
	{
		g_IDInFlash.IDnum[0]= 0xFF;
		g_IDInFlash.IDnum[1]= 0xF0 | (g_MobileInFlash.mobileno[4][0] >> 4);
		g_IDInFlash.IDnum[2] = ((g_MobileInFlash.mobileno[4][0] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][1] >> 4);		
		g_IDInFlash.IDnum[3] = ((g_MobileInFlash.mobileno[4][1] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][2] >> 4);	
		g_IDInFlash.IDnum[4] = ((g_MobileInFlash.mobileno[4][2] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][3] >> 4);	
		g_IDInFlash.IDnum[5] = ((g_MobileInFlash.mobileno[4][3] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][4] >> 4);	
		g_IDInFlash.IDnum[6] = ((g_MobileInFlash.mobileno[4][4] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][5] >> 4);	
		g_IDInFlash.Crc16 = CalcCRC16((uint8_t*)&g_IDInFlash, sizeof(g_IDInFlash) - 2);
		WriteIDToFlash();
	}
	else if (memcmp(g_IDInFlash.IDnum,cmpidbuf,7) == 0)
	{
		g_IDInFlash.IDnum[0]= 0xFF;
		g_IDInFlash.IDnum[1]= 0xF0 | (g_MobileInFlash.mobileno[4][0] >> 4);
		g_IDInFlash.IDnum[2] = ((g_MobileInFlash.mobileno[4][0] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][1] >> 4);		
		g_IDInFlash.IDnum[3] = ((g_MobileInFlash.mobileno[4][1] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][2] >> 4);	
		g_IDInFlash.IDnum[4] = ((g_MobileInFlash.mobileno[4][2] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][3] >> 4);	
		g_IDInFlash.IDnum[5] = ((g_MobileInFlash.mobileno[4][3] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][4] >> 4);	
		g_IDInFlash.IDnum[6] = ((g_MobileInFlash.mobileno[4][4] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][5] >> 4);	
		g_IDInFlash.Crc16 = CalcCRC16((uint8_t*)&g_IDInFlash, sizeof(g_IDInFlash) - 2);
		WriteIDToFlash();
	}
	else if (g_IDInFlash.Crc16 != CalcCRC16((uint8_t*)&g_IDInFlash, sizeof(g_IDInFlash) - 2))
	{
		g_IDInFlash.IDnum[0]= 0xFF;
		g_IDInFlash.IDnum[1]= 0xF0 | (g_MobileInFlash.mobileno[4][0] >> 4);
		g_IDInFlash.IDnum[2] = ((g_MobileInFlash.mobileno[4][0] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][1] >> 4);		
		g_IDInFlash.IDnum[3] = ((g_MobileInFlash.mobileno[4][1] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][2] >> 4);	
		g_IDInFlash.IDnum[4] = ((g_MobileInFlash.mobileno[4][2] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][3] >> 4);	
		g_IDInFlash.IDnum[5] = ((g_MobileInFlash.mobileno[4][3] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][4] >> 4);	
		g_IDInFlash.IDnum[6] = ((g_MobileInFlash.mobileno[4][4] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][5] >> 4);	
        g_IDInFlash.Crc16 = CalcCRC16((uint8_t*)&g_IDInFlash, sizeof(g_IDInFlash) - 2);
		WriteIDToFlash();
	}	

	Arr2AscStr(g_IDInFlash.IDnum,7,0,g_LYID);	
}

//充电自检
uint8_t ChargeSelfFlash(void)
{
	ID_t IDInFlash;
	
    W25qxx_ReadBytes((uint8_t*)&IDInFlash, FLASH_ID_START, sizeof(IDInFlash));
	
	if (IDInFlash.Crc16 != CalcCRC16((uint8_t*)&IDInFlash, sizeof(IDInFlash) - 2))
	{
		g_SelfCheck.Flash.Result = 1;
		
        return 1;
	}
    else
    {
		g_SelfCheck.Flash.Result = 0;
		
		return 1;
    }
	
    return 0;
}

/******************************************************************
函数名称: void WriteUwbCoordinatesToFlash(void)
功能描述: 向Flash写UWB坐标
参数输入: uint8_t	*wDat	写入数据
		  uint16_t	wLen	写入数据长度
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
uint16_t WriteUwbCoordinatesToFlash(uint8_t *wDat, uint16_t wLen)
{
	uint16_t crc16=0;
	uint32_t addr=0;
	bool wret=false, wret2=false;
	
	crc16 = CalcCRC16(wDat, wLen);	//计算CRC16
	
	wret = FlashWriteData(wDat, FLASH_UWB_COORDINATES_START, wLen);							//写UWB坐标
	wret2 = FlashWriteData((uint8_t*)&crc16, FLASH_UWB_COORDINATES_START + wLen + 2, 2);	//写校验
	if ((wret == true) && (wret2 == true))
	{
		return wLen;
	}
	
	return 0;
}

/******************************************************************
函数名称: uint16_t ReadUwbCoordinatesFromFlash(void)
功能描述: 从Flash读UWB坐标
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
uint16_t ReadUwbCoordinatesFromFlash(uint8_t *rDat, uint16_t rLen)
{
	uint16_t crc16=0;
	uint16_t readcrc=0;
	
	/* 读坐标和crc校验 */
	W25qxx_ReadBytes(rDat, FLASH_UWB_COORDINATES_START, rLen);
	W25qxx_ReadBytes((uint8_t*)&readcrc, FLASH_UWB_COORDINATES_START + rLen + 2, 2);
	
	crc16 = CalcCRC16(rDat, rLen);	//计算CRC16
	if (crc16 == readcrc)			//读取校验成功
	{
		return rLen;
	}
	
	return 0;
}

/******************************************************************
函数名称: void WriteDataToFlashWithCRC16
功能描述: 向Flash写数据
参数输入: uint8_t	*wDat		写入数据
		  uint16_t	wLen		写入数据长度
		  uint32_t	FlashAddr	Flash地址
		  uint32_t	FlashLen	Flash长度
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
uint16_t WriteDataToFlashWithCRC16(uint8_t *wDat, uint16_t wLen, uint32_t FlashAddr, uint32_t FlashLen)
{
	uint16_t crc16=0;
	uint32_t crc_addr=0;
	bool wdret=false;
	bool wcret=false;
	
	/* 数据有效性确认 */
	if (wLen >= FlashLen)
	{
		return 0;		//数据长度错误
	}
	
	crc16 = CalcCRC16(wDat, wLen);		//计算CRC16
	wdret = FlashWriteData(wDat, FlashAddr, wLen);		//写数据
	if (wdret == true)
	{
		crc_addr = FlashAddr + wLen + 2;	//计算CRC地址
		wcret = FlashWriteData((uint8_t*)&crc16, crc_addr, 2);	//写CRC16
		if (wcret == true)
		{
			return wLen;	//写入成功返回写入字节 不包含crc字节数
		}
	}

	return 0;
}

/******************************************************************
函数名称: void ReadDataFromFlashWithCRC16
功能描述: 从Flash读数据
参数输入: uint8_t	*rDat		读出数据
		  uint16_t	rLen		读出数据长度
		  uint32_t	FlashAddr	Flash地址
		  uint32_t	FlashLen	Flash长度
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
uint16_t ReadDataFromFlashWithCRC16(uint8_t *rDat, uint16_t rLen, uint32_t FlashAddr, uint32_t FlashLen)
{
	uint16_t crc16=0;
	uint16_t readcrc=0;
	uint32_t crc_addr=0;
	bool rdret=false;
	bool rcret=false;
	
	/* 数据有效性确认 */
	if (rLen >= FlashLen)
	{
		return 0;		//数据长度错误
	}
	
	/* 读坐标和crc校验 */
	W25qxx_ReadBytes(rDat, FlashAddr, rLen);
	crc_addr = FlashAddr + rLen + 2;
	W25qxx_ReadBytes((uint8_t*)&readcrc, crc_addr, 2);
	
	crc16 = CalcCRC16(rDat, rLen);		//计算CRC16
	if (crc16 == readcrc)
	{
		return rLen;	//成功读取数据 返回读出字节长度
	}

	return 0;
}

