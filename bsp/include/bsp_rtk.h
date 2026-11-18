#ifndef __BSP_RTK_H
#define __BSP_RTK_H

#include "stm32l4xx.h"
#include "stdbool.h"
#include "bsp.h"

#define JudgStaNum                  1
#define _LAPSETIME                  1
#define _STOPLAPSETIME              30
#define MIN_60                      60

/*判断状态次数数据 */
typedef struct
{
	uint8_t g_Uplink;               /*上道判断次数*/	
	uint8_t g_Train;                /*判断来车后时候下道标记*/	
	uint8_t g_ReleAlarm;            /*解除报警次数*/
	uint8_t g_SetAlarm;             /*设置报警次数*/	
	uint8_t g_Ahead;                /*提前上道报警次数*/
	uint8_t g_DisAhead;             /*解除提前上道报警次数*/
	uint8_t g_TimeOut;              /*超时施工报警次数*/
	uint8_t g_DisTimeOut;           /*解除超时施工报警次数*/
	uint8_t g_OverStep;             /*超范围施工报警次数*/
	uint8_t g_DisOverStep;          /*解除超范围施工报警次数*/
	uint8_t g_AnoLocal;             /*定位异常报警次数*/
	uint8_t g_DisAnoLocal;          /*定位异常解除报警次数*/
} _JUDENUM;

extern _JUDENUM g_StaJudeNum;    /*判断状态次数*/

/* 报警类型 */
typedef union
{
	struct
	{
		uint8_t	Risk:1;	        //b0:非作业区 0=正常;1=报警
		uint8_t KeepAwayWorkArea:1; //b1:远离作业区 0=正常;1=报警
		uint8_t TrainForeshow:1; //b2:列车预告
		uint8_t TrainApproach:1; //b3:列车接近
		uint8_t KeepAwaySafeArea:1;	//b4:远离安全区
		uint8_t Ahead:1;	    //b5:提前上道报警标记
		uint8_t TimeOut:1;      //b6:超时施工报警标记
		uint8_t AnomLocal:1;    //b7:定位异常报警标记
		uint8_t CrossAdjacentLine:1; //b8:跨越邻线
		uint8_t StartUp:1;   //b9:第一次开机
		uint8_t LateStartUp:1;   //b10:晚开机
		uint8_t PreOnLine:1;   //b11:提前上线
		uint8_t PreUpperTrackStart:1;  //b12:提前上道
		uint8_t LateOffLineStart:1;  //b13:晚下线
		uint8_t LateLowerTrackStart:1;  //b14:晚下道
		uint8_t LowerTrackLateStart:1;  //b15:下道晚
		uint8_t ProtectionPosition:1;  //b16:脱离位置防护
	} bit;
	uint32_t all;
} AlramType_t;

extern AlramType_t g_CarWarnFlag;          /*报警类型*/
extern uint8_t g_GPSOkFlag;
extern	uint8_t g_LastPosQuality;
extern uint8_t g_LocalStaFlag;             /*定位状态=0，未定位，=1定位成功*/
extern uint32_t  g_RTKInitDelay;
extern	uint8_t ggadata[256];
extern	uint16_t ggadatalen;
extern uint8_t  g_NewGPSData;         /*收到新测试点数据标记*/	
extern uint8_t g_AreaTypeAlarm;
extern uint8_t g_BigAreaType;
typedef struct
{
	uint8_t UartOk;		/* 1表示串口数据接收正常，即可以收到GPS的命令字符串 */
	
	/* 从GGA 命令 : 时间、位置、定位类型  (位置信息可有 RMC命令获得) */

	/* 定位有效标志, 0:未定位 1:SPS模式，定位有效 2:差分，SPS模式，定位有效 3:PPS模式，定位有效 */
	uint8_t PositionOk;
	uint32_t Altitude;	/* 海拔高度，相对海平面多少米   x.x  */

	/* GLL：经度、纬度、UTC时间 */
	/* 样例数据：$GPGLL,3723.2475,N,12158.3416,W,161229.487,A*2C */


	/* GSA：GPS接收机操作模式，定位使用的卫星，DOP值 */
	/* 样例数据：$GPGSA,A,3,07,02,26,27,09,04,15, , , , , ,1.8,1.0,1.5*33 */
	uint8_t ModeAM;				/* M=手动（强制操作在2D或3D模式），A=自动 */
	uint8_t Mode2D3D;			/* 定位类型 1=未定位，2=2D定位，3=3D定位 */
	uint8_t SateID[12];			/* ID of 1st satellite used for fix */
	uint16_t PDOP;				/* 位置精度, 0.1米 */
	uint16_t HDOP;				/* 水平精度, 0.1米 */
	uint16_t VDOP;				/* 垂直精度, 0.1米 */

	/* GSV：可见GPS卫星信息、仰角、方位角、信噪比（SNR） */
	/*
		$GPGSV,2,1,07,07,79,048,42,02,51,062,43,26,36,256,42,27,27,138,42*71
		$GPGSV,2,2,07,09,23,313,42,04,19,159,41,15,12,041,42*41
	*/
	uint8_t ViewNumber;			/* 可见卫星个数 */
	uint8_t Elevation[12];		/* 仰角 elevation in degrees  度,最大90°*/
	uint16_t Azimuth[12];		/* 方位角 azimuth in degrees to true  度,0-359°*/
	uint8_t SNR[12];			/* 载噪比（C/No）  信噪比？ ， dBHz   范围0到99，没有跟踪时为空 */

	/* RMC：时间、日期、位置	、速度 */
	/* 样例数据：$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120598, ,*10 */
	uint32_t WeiDu_Du;			/* 纬度，度 */
	uint32_t WeiDu_Fen;			/* 纬度，分. 232475；  小数点后4位, 表示 23.2475 分 */
	char     NS;				/* 纬度半球N（北半球）或S（南半球） */

	uint32_t JingDu_Du;			/* 经度，单位度 */
	uint32_t JingDu_Fen;		/* 经度，单位度 */
	char     EW;				/* 经度半球E（东经）或W（西经） */

	uint16_t Year;				/* 日期 120598 ddmmyy */
	uint8_t  Month;
	uint8_t  Day;
	uint8_t  Hour;				/* UTC 时间。 hhmmss.sss */
	uint8_t  Min;				/* 分 */
	uint8_t  Sec;				/* 秒 */
	uint16_t mSec;				/* 毫秒 */
	char   TimeOk;				/* A=UTC时间数据有效；V=数据无效 */
	uint16_t fDiffAge;            //差分龄期

	/* VTG：地面速度信息 */
	/* 样例数据：$GPVTG,309.62,T, ,M,0.13,N,0.2,K*6E */
	uint16_t TrackDegTrue;		/* 以真北为参考基准的地面航向（000~359度，前面的0也将被传输） */
	uint16_t TrackDegMag;		/* 以磁北为参考基准的地面航向（000~359度，前面的0也将被传输） */
	uint16_t SpeedKnots;		/* 地面速率（000.0~999.9节，前面的0也将被传输） */
	uint32_t SpeedKM;			/* 地面速率（000.0~999.9节，前面的0也将被传输） */

}GPS_T;

typedef struct  
{										    
 	uint16_t year;	//年份
	uint8_t month;	//月份
	uint8_t date;	//日期
	uint8_t hour; 	//小时
	uint8_t min; 	//分钟
	uint8_t sec; 	//秒钟
}nmea_time;
// 定义一个结构体来表示GPS坐标
typedef struct {
    double latitude;       /*纬度*/
    double longitude;      /*精度*/
} GPSPoint;
/* 人员卡ID */
typedef union
{
	struct
	{
		uint8_t		AdminID;		//局ID
		uint8_t		SectionID;		//段ID
		uint8_t		VrtWorkshopID;	//虚拟车间ID
		uint8_t		WorkshopID;		//车间ID
		uint8_t		WorkareaID;		//工区ID
		uint16_t    UserID;			//卡ID
	} byte;
	uint8_t array[7];
} UserCardID_t;
/* 报警标记 */
typedef union
{
	struct
	{
		uint8_t	Alram:1;	//b0:报警标记 0=正常;1=报警
		uint8_t SOS:1;		//b1:求救标记 0=正常;1=求救
		uint8_t LowPower:1;	//b2:低电量标记 0=正常;1=低电量
		uint8_t Bribe:1;    //b3:打点标记   0=正常;1=打点标记
		uint8_t Charge:1;	//b4:充电
		uint8_t :2;			//b5-b6:预留
		uint8_t Online:1;	//b7:在线标记 0=离线;1=在线
	} bit;
	uint8_t all;
} AlramFlag_t;
/* 人员卡数据 */
typedef struct
{
	UserCardID_t	ID;			//用户卡ID
	uint32_t		Latitude;	//纬度 单位=0.0000001度
	uint32_t		Longitude;	//经度 单位=0.0000001度
	uint8_t			PosQuality;	//定位质量
	uint8_t			Altitude[3];//海拔高度
	uint8_t			SpeedKnots[2];//速度
	AlramFlag_t		Alram;		//报警标记
	uint8_t			BatVoltage;	//电量
	AlramType_t		AlramType;  //报警类型
	uint8_t			FenceOrder;	//电子围栏编号
	uint8_t			CTimeOrder;	//施工时间编号
	uint8_t         ViewNumber;//星数;
	uint8_t         AreaType;//区域类型
	uint8_t			Reserver[5];//预留7个字节	
} CardData_t;

typedef struct 
{
	double last_lat;          // 上次纬度 (度)
    double last_lon;          // 上次经度 (度)
} RtkData_t;

extern uint8_t     g_AreaType;           /*区域类型*/
extern GPS_T g_tGPS;
extern DataType         g_DataSta;
extern uint8_t g_PosQuality;            /*定位质量*/
extern GPSPoint polygonbuff;          /*测量点经纬度*/
extern CardData_t       g_CardLocaData;

extern uint32_t   g_testDelay;
extern uint8_t g_SendLocationData;


void gps_pro(uint8_t *bPtr,int32_t len);
void SendFenceToRTK(void);
void Localiza_Init(void);
void EFence_pro(uint8_t *bPtr,int32_t len);
void InitRMCVari(void);
void InitGGAVari(void);
void gpsGNGGA(uint8_t *_ucaBuf, uint16_t _usLen);
void gpsGNRMC(uint8_t *_ucaBuf, uint16_t _usLen);
void UTCDate(nmea_time*	utc_time, int8_t timezone);
void CalculateSpeedInit(void);
#endif


