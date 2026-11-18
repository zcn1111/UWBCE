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

#include "bsp.h"
#include "app_glovar.h"
#include "4G_Comm.h"
#include "app_warn.h"
GPS_T              g_tGPS;
CardData_t         g_CardLocaData;
_JUDENUM           g_StaJudeNum;          /*判断状态次数*/
DataType           g_DataSta;     /*设备状态*/

uint8_t     g_GPSOkFlag;          //定位标记
uint8_t     g_AreaType;       
uint8_t g_AreaTypeAlarm = 0;
uint8_t g_BigAreaType = 0;
uint32_t  g_RTKInitDelay;

uint8_t g_PosQuality;               /*定位质量*/
uint8_t g_LastPosQuality;
uint8_t g_LocalStaFlag;             /*定位状态=0，未定位，=1定位成功*/
AlramType_t g_CarWarnFlag;          /*报警*/
uint8_t ggadata[256];
uint16_t ggadatalen=0;
GPSPoint polygonbuff;          /*测量点经纬度*/	
uint8_t  g_NewGPSData;         /*收到新测试点数据标记*/		
uint32_t   g_testDelay;
RtkData_t g_rtk;
uint8_t g_SendLocationData = 0;

void CalculateSpeed(void);

void Localiza_Init(void)
{
	memset((uint8_t*)&g_StaJudeNum,0,sizeof(g_StaJudeNum));
	g_StaJudeNum.g_Uplink = 0;
	g_StaJudeNum.g_Train = 0;                                                  /*判断来车后是否下道标记*/
	g_StaJudeNum.g_ReleAlarm = 0;
	g_CarWarnFlag.all = 0;          /*报警类型*/
	g_PosQuality = 0;               /*定位质量*/
	g_LocalStaFlag = 0;
	memset((uint8_t *)&g_tGPS,0,sizeof(g_tGPS));
	g_GPSOkFlag = 0;
	ReadDTUSysPar();
	ReadModeFromFlash();
	ReadLocationDiffFromFlash();
	g_NewGPSData = 0;
	ReadIDFromFlash();
	ReadAreaAlarmToFLASH();
	ReadVoiceSwitchFromFLASH();
	g_testDelay = TIM2->CNT;
	memset((uint8_t *)&g_YYPlay,0,sizeof(g_YYPlay));
	g_YYPro=0; 
	g_YYProDelay=TIM2->CNT;
	g_YYProDelayOut=0;	
}


/*
*********************************************************************************************************
*	函 数 名: UTCDate(nmea_time*	utc_time, int8_t timezone)
*	功能说明: 处理时差
*	形    参: utc_time :待转换的ASCII码串. 可以以逗号或0结束
*			 timezone : 时差
*	返 回 值: 二进制整数值
*********************************************************************************************************
*/
void UTCDate(nmea_time*	utc_time, int8_t timezone)
{
	int year,month,day,hour;
    int lastday = 0;					//last day of this month
    int lastlastday = 0;			//last day of last month

    year	= utc_time->year;			 //utc time
    month  	= utc_time->month;
    day 	= utc_time->date;
    hour 	= utc_time->hour + timezone; 
	
    if(month==1 || month==3 || month==5 || month==7 || month==8 || month==10 || month==12)
    {
        lastday = 31;
        if(month == 3)
		{
            if((year%400 == 0)||(year%4 == 0 && year%100 != 0))				//if this is lunar year
                lastlastday = 29;
            else
                lastlastday = 28;
        }
        if(month == 8)
            lastlastday = 31;
    }
    else if(month == 4 || month == 6 || month == 9 || month == 11)
    {
        lastday = 30;
        lastlastday = 31;
    }
    else
    {
        lastlastday = 31;
        if((year%400 == 0)||(year%4 == 0 && year%100 != 0))
            lastday = 29;
        else
            lastday = 28;
    }

    if(hour >= 24)
    {	// if >24, day+1
        hour -= 24;
        day += 1; 

        if(day > lastday)
		{ 		// next month,  day-lastday of this month
            day -= lastday;
            month += 1;

            if(month > 12)
			{		//	next year , month-12
                month -= 12;
                year += 1;
            }
        }
    }
    if(hour < 0)
    {										// if <0, day-1
        hour += 24;
        day -= 1; 
        if(day < 1)
		{					  // month-1, day=last day of last month
            day = lastlastday;
            month -= 1;
            if(month < 1)
			{ 			// last year , month=12
                month = 12;
                year -= 1;
            }
        }
    }
	utc_time->year = year;			 //utc time
    utc_time->month = month;
    utc_time->date = day;
    utc_time->hour = hour; 
}


/*
*********************************************************************************************************
*	函 数 名: gpsGNRMC
*	功能说明: 分析0183数据包中的 GPGSV 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPRMC,024813.640,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*50
字段0：$GPRMC，语句ID，表明该语句为Recommended Minimum Specific GPS/TRANSIT Data（RMC）推荐最小定位信息
字段1：UTC时间，hhmmss.sss格式
字段2：状态，A=定位，V=未定位
字段3：纬度ddmm.mmmm，度分格式（前导位数不足则补0）
字段4：纬度N（北纬）或S（南纬）
字段5：经度dddmm.mmmm，度分格式（前导位数不足则补0）
字段6：经度E（东经）或W（西经）
字段7：速度，节，Knots
字段8：方位角，度
字段9：UTC日期，DDMMYY格式
字段10：磁偏角，（000 - 180）度（前导位数不足则补0）
字段11：磁偏角方向，E=东W=西
字段16：校验值
*/
void gpsGNRMC(uint8_t *_ucaBuf, uint16_t _usLen)
{
	char *p;
	char *ptr;
	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1 UTC时间，hhmmss.sss格式 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{		
		g_tGPS.Hour = StrToIntFix(p, 2);
		p += 2;
		g_tGPS.Min = StrToIntFix(p, 2);
		p += 2;
		g_tGPS.Sec = StrToIntFix(p, 2);
		p += 3;
		g_tGPS.mSec = StrToIntFix(p, 3);
	}
	/* 字段2 状态，A=定位，V=未定位 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{		
		if (*p != 'A')
		{
			/* 未定位则直接返回 */		
			g_tGPS.PositionOk = 0;
			//LED_RTK_ON();
			return;
		}
		g_tGPS.PositionOk = 1;
		p += 1;
	}
	/* 字段3 纬度ddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{		
		g_tGPS.WeiDu_Du = StrToIntFix(p, 2);
		p += 2;
		g_tGPS.WeiDu_Fen = StrToIntFix(p, 2) * 10000;
		p += 3;
		g_tGPS.WeiDu_Fen += StrToIntFix(p, 4);
		p += 4;
		polygonbuff.latitude = (double)g_tGPS.WeiDu_Du + (double)g_tGPS.WeiDu_Fen/600000; /*纬度*/
	}
	/* 字段4 纬度N（北纬）或S（南纬）*/
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{		
		if (*p == 'S')
		{
			g_tGPS.NS = 'S';
		}
		else if (*p == 'N')
		{
			g_tGPS.NS = 'N';
		}
		else
		{
			return;
		}
	}
	/* 字段5 经度dddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{	
		g_tGPS.JingDu_Du = StrToIntFix(p, 3);
		p += 3;
		g_tGPS.JingDu_Fen = StrToIntFix(p, 2) * 10000;
		p += 3;
		g_tGPS.JingDu_Fen += StrToIntFix(p, 4);
		p += 4;
		polygonbuff.longitude = (double)g_tGPS.JingDu_Du + (double)g_tGPS.JingDu_Fen/600000;  /*经度*/
		g_NewGPSData = 1;
	}
	/* 字段6：经度E（东经）或W（西经） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{	
		if (*p == 'E')
		{
			g_tGPS.EW = 'E';
		}
		else if (*p == 'W')
		{
			g_tGPS.EW = 'W';
		}
	}
	/* 字段7：速度，节，Knots  10.05, （000.0~999.9）*/
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{		
		g_tGPS.SpeedKnots = StrToInt(p);
	}
	/* 字段8：方位角，度 ,324.27 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{		
		g_tGPS.TrackDegTrue = StrToInt(p);
	}
	/* 字段9：UTC日期，DDMMYY格式  150706 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{		
		g_tGPS.Day = StrToIntFix(p, 2);
		p += 2;
		g_tGPS.Month = StrToIntFix(p, 2);
		p += 2;
		g_tGPS.Year = StrToIntFix(p, 2);
		p += 2;
	}
	
	CalculateSpeed();
}
void InitRMCVari(void)
{
	g_tGPS.Hour = 0;
	g_tGPS.Min = 0;
	g_tGPS.Sec = 0;
	g_tGPS.mSec = 0;
	g_tGPS.PositionOk = 0;
	g_tGPS.WeiDu_Du = 0;
	g_tGPS.WeiDu_Fen = 0;
	g_tGPS.NS = 0;
	g_tGPS.JingDu_Du = 0;
	g_tGPS.JingDu_Fen = 0;
	g_tGPS.EW = 0;
	g_tGPS.SpeedKnots = 0;
	g_tGPS.TrackDegTrue = 0;
	g_tGPS.Day = 0;
	g_tGPS.Month = 0;
	g_tGPS.Year = 0;
}
///////////////////
void InitGGAVari(void)
{
	g_tGPS.Hour = 0;
	g_tGPS.Min = 0; 
	g_tGPS.Sec = 0;
	g_tGPS.WeiDu_Du = 0;
	g_tGPS.WeiDu_Fen = 0;
	g_tGPS.NS = 0;
	g_tGPS.JingDu_Du = 0;
	g_tGPS.JingDu_Fen = 0;
	g_tGPS.EW = 0;
	g_tGPS.PositionOk = 0;
	g_tGPS.ViewNumber = 0;
	g_tGPS.HDOP = 0;
	g_tGPS.Altitude = 0;
}
void gpsGNGGA(uint8_t *_ucaBuf, uint16_t _usLen)
{
	char *p;
	char *ptr;
	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1 UTC 时间，hhmmss.sss，时分秒格式 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{
		g_tGPS.Hour = StrToIntFix(p, 2);
		p += 2;
		g_tGPS.Min = StrToIntFix(p, 2);
		p += 2;
		g_tGPS.Sec = StrToIntFix(p, 2);
		p += 2;
	}
	/* 字段2 纬度ddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');	
	if ((ptr - p) > 0)
	{	
		g_tGPS.WeiDu_Du = StrToIntFix(p, 2);
		p += 2;
		g_tGPS.WeiDu_Fen = StrToIntFix(p, 2) * 10000;
		p += 3;
		g_tGPS.WeiDu_Fen += StrToIntFix(p, 4);
		polygonbuff.latitude = (double)g_tGPS.WeiDu_Du + (double)g_tGPS.WeiDu_Fen/600000; /*纬度*/
		p += 4;
	}
	/* 字段3 纬度N（北纬）或S（南纬） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{	
		if (*p == 'S')
		{
			g_tGPS.NS = 'S';
		}
		else if (*p == 'N')
		{
			g_tGPS.NS = 'N';
		}
		else
		{
			return;
		}
	}
	/* 字段4  经度dddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{	
		g_tGPS.JingDu_Du = StrToIntFix(p, 3);
		p += 3;
		g_tGPS.JingDu_Fen = StrToIntFix(p, 2) * 10000;
		p += 3;
		g_tGPS.JingDu_Fen += StrToIntFix(p, 4);
		polygonbuff.longitude = (double)g_tGPS.JingDu_Du + (double)g_tGPS.JingDu_Fen/600000;  /*经度*/
		p += 4;
	}
	/* 字段5 经度E（东经）或W（西经） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{	
		if (*p == 'E')
		{
			g_tGPS.EW = 'E';
		}
		else if (*p == 'W')
		{
			g_tGPS.EW = 'W';
		}
	}

	/* 字段6 GPS状态，0=未定位，1=非差分定位，2=差分定位，4=RTK固定解，5=RTK浮点解，6=正在估算 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) != 0)
	{	
		if ((*p == '1') || (*p == '2') || (*p == '4') || (*p == '5') )
		{
			g_tGPS.PositionOk = 1;
	//		if(g_PosQuality !=4 && Asc2Hex(*p) == 4)
	//			g_gpsdwflag = 1;
	//		else if(g_PosQuality ==4 && Asc2Hex(*p) != 4)
	//			g_gpsdwflag = 2;
			g_PosQuality = Asc2Hex(*p);
			if(g_PosQuality == 0x04 || g_PosQuality == 0x05)
			{
				if(g_ManualPunFlag > 0)
					g_ManualPunFlag = 0x02;
			}
		}
		else
		{
			g_tGPS.PositionOk = 0;
			//LED_RTK_ON();
		}
	}
	/* 字段7：正在使用的卫星数量（00 - 12）（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{	
		g_tGPS.ViewNumber = StrToInt(p);
		p += 2;
	}
	/* 字段8：HDOP水平精度因子（0.5 - 99.9） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{		
		g_tGPS.HDOP = StrToInt(p);
		p += 3;
	}
	/* 字段9：海拔高度（-9999.9 - 99999.9） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{		
		g_tGPS.Altitude = StrToInt(p);
	}
	
	g_CardLocaData.Longitude = g_tGPS.JingDu_Du*10000000 + g_tGPS.JingDu_Fen*1000/60;
	g_CardLocaData.Latitude = g_tGPS.WeiDu_Du*10000000 + g_tGPS.WeiDu_Fen*1000/60;		
	if(g_PosQuality>=4)
	{
		if(dealflag == 1)
		{
			position[0].lat = position[1].lat;
			position[0].lng = position[1].lng;
			dealflag = 0;
			p_time[0] = p_time[1];
		}
		position[1].lat	= (double)g_CardLocaData.Latitude/10000000;
		position[1].lng = (double)g_CardLocaData.Longitude/10000000;
		p_time[1] = TIM2->CNT;
	}
	
	//字段10
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	//字段11
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	//字段12
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	//字段13
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{
		char cDiffAge[8];
		for (uint8_t i = 0; p < ptr; i++, p++)
		{
			cDiffAge[i] = *p;

		}
		
		g_tGPS.fDiffAge = (uint16_t)atof(cDiffAge);
	}
}

void CalculateSpeedInit(void)
{
	g_rtk.last_lat = 0.0;
	g_rtk.last_lon = 0.0;
}

void CalculateSpeed(void)
{
	if (g_PosQuality < 4)
	{
		return;
	}
	else
	{
		double Longitude = (double)g_CardLocaData.Longitude /10000000;
		double Latitude = (double)g_CardLocaData.Latitude /10000000;
		double Distance = haversine_distance(Latitude, Longitude, g_rtk.last_lat, g_rtk.last_lon);
		
		if (Distance > 0.6)
		{
			g_rtk.last_lat = Latitude;
			g_rtk.last_lon = Longitude;
			g_SendLocationData = 1;
		}
	}
}
