#ifndef __APP_GLOVAR_H
#define __APP_GLOVAR_H

#include "stdint.h"
#include "stdbool.h"
#include "bsp.h"

#define MAX_WORK_PLAN_NUM   60

#define ADDR_EEPROM_FIRMWARE_UPDATA_OFFSET  sizeof( NormalUpdateFileSize )          //存放固化BIN文件的偏移
#define ADDR_EEPROM_FIRMWARE_UPDATA_LEN		sizeof(FixUpdateFileSize)				//固化BIN文件的大小占2个字节

#define ADDR_BOOT_VERSION_OFFSET            (ADDR_EEPROM_FIRMWARE_UPDATA_OFFSET + sizeof(FixUpdateFileSize) )
#define ADDR_BOOT_VERSION_LEN               sizeof(BootFileSize)

#define ADDR_UPDATE_FLAG_OFFSET             (ADDR_BOOT_VERSION_OFFSET + sizeof(BootFileSize)) 
#define ADDR_UPDATE_FLAG_LEN                1

#define ADDR_VERSION_FLAG_OFFSET            (ADDR_UPDATE_FLAG_OFFSET+1)
#define ADDR_VERSION_FLAG_LEN               sizeof(ChangeVersion)

#define ADDR_UPDATE_VOICE_FLAG_OFFSET     	(ADDR_VERSION_FLAG_OFFSET+sizeof(ChangeVersion))
#define ADDR_UPDATE_VOICE_FLAG_LEN        	sizeof(VoiceFileSize)

#define ADDR_USER_VERSION_OFFSET           (ADDR_UPDATE_VOICE_FLAG_OFFSET + sizeof(VoiceFileSize)) 
#define ADDR_USER_VERSION_LEN              4

#define ADDR_UPDATE_DIFF_FLAG_OFFSET       (ADDR_USER_VERSION_OFFSET+4)
#define ADDR_UPDATE_DIFF_FLAG_LEN          sizeof(DiffUpdateFileSize)

/*系统参数*/
#define FLASH_SYSPARAMETER_START 		1*256
#define FLASH_SYSPARAMETER_LEN 			1*256

/*预留*/
#define FLASH_RESERVED_START 			(FLASH_SYSPARAMETER_START + FLASH_SYSPARAMETER_LEN)
#define FLASH_RESERVED_LEN 				14*256

/*电源控制		   */
#define FLASH_POWERCTL_START			0x1000
#define FLASH_POWERCTL_LEN				1*256

/*预留             */
#define FLASH_RESERVED_START0 			(FLASH_POWERCTL_START + FLASH_POWERCTL_LEN)
#define FLASH_RESERVED_LEN0			    15*256

/*升级		      */
#define ADDR_EEPROM_UPDATA				0x3000
#define ADDR_EEPROM_UPDATA_LEN		    2*256

#define FLASH_WT288E_IAP_FLAG_START     0x4000                                                        /*升级语音包标记*/
#define FLASH_WT288E_IAP_FLAG_LEN       1 

#define FLASH_WT288E_IAP_UPDATA_START   (FLASH_WT288E_IAP_FLAG_START+FLASH_WT288E_IAP_FLAG_LEN)        /*升级语音包大小*/
#define FLASH_WT288E_IAP_UPDATA_LEN     4

#define FLASH_AREA_ALARM_FLASH_START    0x5000         									/*区域报警存储*/
#define FLASH_AREA_ALARM_FLASH_LEN		2*1024

#define FLASH_AlarmLevel_START          0x6000         									/*报警级别参数存储*/
#define FLASH_AlarmLevel_LEN	        22

#define SET_SIMTYPE_FLASH_ADDR          0x7000         									/*存储手机卡类型  28672*/
#define SET_SIMTYPE_FLASH_LEN		    128

//存储工作模式(自收敛还是cros) 0:自收敛  1:cros
#define FLASH_WORK_MODE_START           0x8000
#define FLASH_WORK_MODE_LEN		        140

#define FLASH_VERSION_START				0x9000
#define FLASH_VERSION_LEN				1*256

/*手机号*/
#define FLASH_MOBILE_START				0x10000
#define FLASH_MOBILE_LEN				1*256

/*IP端口号*/
#define FLASH_IPPORT_START				(FLASH_MOBILE_START + FLASH_MOBILE_LEN)
#define FLASH_IPPORT_LEN				1*256

/* id */
#define FLASH_ID_START	                 0x11000
#define FLASH_ID_LEN		             16*256

/*设置工作模式 */
#define FLASH_MODE_START	             0x12000
#define FLASH_MODE_LEN		             16*256

//设置发送定位信息间隔
#define FLASH_LOCATION_DIFF_START	     0x13000
#define FLASH_LOCATION_DIFF_LEN		     1*256

//关闭报"单点解定位"的语音
#define FLASH_ON_OFF_VOICE_START	     0x14000
#define FLASH_ON_OFF_VOICE_LEN		     1*256

#define FLASH_WORK_PLAN_RECORD_START   0x15000
#define FLASH_WORK_PLAN_RECORD_LEN	   4*1024

//这里最大能存放60个电子围栏

//暂时存放电子围栏(用于修改)
#define FLASH_MOD_EFENCE_FLASH_START   0x16000
#define FLASH_MOD_EFENCE_FLASH_LEN	   4096

//存放电子围栏
#define FLASH_EFENCE_FLASH_START       0x30000
#define FLASH_EFENCE_FLASH_LEN		    4096

//以上为参数占500K

//UWB参数
//UWB基站坐标和参数
#define FLASH_UWB_COORDINATES_START		0x00040000
#define FLASH_UWB_COORDINATES_LEN		512
#define FLASH_UWB_PARAMETER_START		0x00040200
#define FLASH_UWB_PARAMETER_LEN			512

//user差分升级差分包地址
#define FLASH_IAP_USER_START1			0x80000   
#define FLASH_IAP_USER_LEN1				512*1024    //500k

//user差分升级解压包地址
#define FLASH_IAP_DIFFUSER_START2		0x100000 		
#define FLASH_IAP_DIFFUSER_LEN2			512*1024    //500k

//boot程序地址
#define FLASH_IAP_BOOT_START1			0x180000
#define FLASH_IAP_BOOT_LEN1				512*1024     //500k

//user程序地址
#define FLASH_IAP_DIFFUSER_START1		0x200000		
#define FLASH_IAP_DIFFUSER_LEN1			512*1024     //500k

#define FLASH_VOICE_START               0x280000       
#define FLASH_VOICE_LEN                 1024*1024  

/**************************************************通用 START*********************************************************************/

#define SIM1ONLY	0
#define SIM2ONLY	1
#define SIMAUTO		2

typedef struct
{
	uint8_t programtype;
	uint8_t mainverno;
	uint8_t secverno;	
	uint16_t programpage;
} UPDATAPROGRAM;
extern UPDATAPROGRAM g_UpdateCodePageTotal;
extern uint8_t g_UpdateCodeTypeErrorFlag;

extern uint8_t	g_VersionHex[2];				//16进制版本号
extern uint8_t	g_VersionAscii[4];				//ASCII版本号

typedef struct
{
    uint8_t bootversion[4];
	uint8_t userversion[4]; 
    uint16_t Crc16;
}_VERSION_FLASH;
extern _VERSION_FLASH g_VersioninFlash;
typedef struct
{
	uint8_t mobileno[5][6];	    
    uint16_t Crc16;
}_MOBILE_FLASH;
extern _MOBILE_FLASH g_MobileInFlash;
typedef struct
{
	uint8_t serveripaddr[4][4];	 //0-远程升级IP，1-远程转储IP，2-数据IP
	uint16_t serverportaddr[4]; 
	uint16_t Crc16;
}_SERVER_FLASH;
extern _SERVER_FLASH g_ServerInfo;
typedef struct
{
	DATETIME start;
	DATETIME stop;
	uint8_t  num;
    uint16_t Crc16;
}_WORKTIME;
extern _WORKTIME g_WorkTime;

typedef struct
{
	uint8_t  data[24][128];	
	uint8_t  num;
}_EFENCE;

typedef struct
{
	uint8_t IsBigArea;   //1是大区域  0是小区域
	uint8_t  total;
	uint8_t  order[4];
}SelectEFence;

extern SelectEFence SelectEFenceInfo;

// 施工计划结构
typedef struct {
    uint16_t header;
	uint16_t length;
    struct {
        uint32_t lat;       // 纬度 (0.0000001度)
        uint32_t lng;       // 经度 (0.0000001度)
    } center_point;
    
    uint8_t  start_time[6]; // BCD格式: YYMMDDHHmmSS
    uint8_t  end_time[6];
    uint8_t  area_count;
    
    struct {
        uint8_t area_type;  // 区域类型
        uint8_t area_id;    // 区域编号
        uint8_t reserve[6];
        struct {
            uint32_t lat;
            uint32_t lng;
        } points[15];       // 每个区域15个坐标点
    } areas[4];
    
    uint8_t  reserve[6];
    uint8_t  checksum;
	uint16_t addressflash;
	uint16_t lenflash;
} WorkPlan;
//extern WorkPlan   SetWorkPlanInfo;

typedef struct {
        double lat;       // 纬度 (0.0000001度)
        double lng;       // 经度 (0.0000001度)
    } Workcenter_point;

typedef struct
{
	//无列车
	uint8_t notrain;
	//列车预告
	uint8_t trainannouncement;
	//列车接近
	uint8_t trainnear;
	//非作业时间
	uint8_t nojobtime;
}AreaAlarmPoint;

typedef struct
{
	//马甲指示灯
	AreaAlarmPoint armourled;     //=0熄灭；=1常亮；=2闪烁
	//语音
	AreaAlarmPoint voice;         //=0无声；=1准备下道；=2尽快下道；=3上行来车；=4下行来车；=5远离作业区；=6靠近邻线；=7进入邻线
}AreaAlarm;

extern AreaAlarm AreaAlarmInfo[11];
extern uint8_t g_CommanBuf[1024]; 

typedef struct
{
	uint8_t   risk[2];    /*危险区域bit8-bit15  =1常亮；=2闪烁；=3关闭         bit0-bit7   =1红灯；=2黄灯；=3绿灯*/	
	uint8_t  overstep[2];  /*超范围*/
	uint8_t  ahead[2];   /*提前上道*/
	uint8_t  timeOut[2];             /*超时施工*/
	uint8_t  tradownroad[2];   /*来车未下道*/
	uint8_t  anomlocal[2];     /*定位异常*/
	uint8_t  num;	
    uint16_t Crc16;
}_WARNLEDSTA;

typedef struct
{
	uint8_t num;
    uint16_t Crc16;
}SIMType;

typedef struct
{
	uint8_t num;
    uint16_t Crc16;
}VoiceSwitchType;

extern VoiceSwitchType g_VoiceSwitch;

typedef struct
{
	uint8_t		    IDnum[7];	//地面号	
	uint16_t 		Crc16;	    //CRC16校验和
} ID_t;

extern SIMType g_SIMType;
extern bool     g_GetMyMobileOKFlag;
extern uint32_t g_SaveUploadStatusDataPageAddr;
extern uint32_t g_SaveUploadStatusDataPageOff;
extern uint32_t g_ReadUploadStatusDataPageAddr;
extern uint32_t g_ReadUploadStatusDataPageOff;
extern bool g_HaveUploadStatusDataInFlash;
extern uint32_t   g_WorkPlanStartAddr;
extern uint8_t g_SendCardInfoInterval;
extern ID_t	g_IDInFlash;
extern _WARNLEDSTA   g_WarnLedSta;
extern Workcenter_point position[2];
extern uint32_t p_time[2];
extern	uint8_t dealflag;
extern int last_selected_idx;
extern uint32_t PreWorkPlanID;

void ReadWorkTimeFomFlash(void);
void CleanEFenceToFLASH(uint8_t EFenceNumSerial);
void WriteEFenceToFLASH(uint8_t EFenceNum, uint8_t *Data, uint16_t DataLen, uint8_t PackageOrder);
void ReadEFenceFomFlash(void);
void WriteIndicatorStaToFLASH(void);
void ReadIndicatorStaToFLASH(void);
void ReadWorkPlanFomBuff(uint8_t index);
void SelectWorkPlan(void);
void ReadAreaAlarmToFLASH(void);
double haversine_distance(double lat1, double lon1, double lat2, double lon2);
//void WriteMobileNumToFLASH(void);
//void ReadMobileNumFromFLASH(void);
int compare_datetime(DATETIME a, DATETIME b) ;
int select_plan(double *distance);
int judge_datetime(DATETIME a, DATETIME b,uint32_t sec);
long long datetime_to_seconds(DATETIME a,DATETIME dt);
int days_in_month(int year, int month) ;
float calculateV(Workcenter_point *Positon, uint32_t *time);

void ReadSysPar(void);
void WriteSysPar(void);
void WriteDTUSysPar(void);
void ReadDTUSysPar(void);
void WriteAreaAlarmToFLASH(uint8_t *Data, uint16_t DataLen);
bool ReadNormalFileSizeFromFlash(void);
void WriteDiffFileSizeToFlash(void);
bool ReadDiffFileSizeFromFlash(void);
bool ReadMobileNoFromFLASH(void);
void WriteMobileNoToFLASH(void);
void WriteNormalFileSizeToFlash(void);
void CmpIPAndPort(void);
void WriteServerInfoPar(void);
bool ReadServerInfoPar(void);
void WritePowerCtlFlagToFlash(uint8_t flag);
void WritePowerCtlNoTolash(uint8_t *number);
void ReadPowerCtlFlagFromFlash(void);
bool ReadPowerCtlNoFromFlash(uint8_t *number);
void ChangeVersionFormat(void);
void WriteSIMTYPEFromFLASH(void);
void ReadSIMTYPEFromFLASH(void);
void WriteModeToFlash(void);
void ReadModeFromFlash(void);
void WriteLocationDiffToFlash(void);
void ReadLocationDiffFromFlash(void);
void ReadVoiceSwitchFromFLASH(void);
void WriteVoiceFileSizeToFlash(void);
bool ReadVoiceFileSizeFromFlash(void);
void WriteIDToFlash(void);
void ReadIDFromFlash(void);
void WriteVoiceSwitchToFLASH(void);
int compare_datetime60(DATETIME a, DATETIME b,uint8_t flag);
uint8_t ChargeSelfFlash(void);
int compare_datetime30(DATETIME a, DATETIME b,uint8_t flag);
int compare_time_second(DATETIME time1, DATETIME time2, int second);

uint16_t WriteUwbCoordinatesToFlash(uint8_t *wDat, uint16_t wLen);
uint16_t ReadUwbCoordinatesFromFlash(uint8_t *rDat, uint16_t rLen);
uint16_t WriteDataToFlashWithCRC16(uint8_t *wDat, uint16_t wLen, uint32_t FlashAddr, uint32_t FlashLen);
uint16_t ReadDataFromFlashWithCRC16(uint8_t *rDat, uint16_t rLen, uint32_t FlashAddr, uint32_t FlashLen);

/***********************************************************************************************************************/
typedef struct
{
	uint8_t cors;  //=0连接服务器 =1 cors
	uint8_t equiptype;  //=0人员定位卡，=1道尺         
	uint8_t res[21];
	uint16_t crc16;
}_SYS_PAR;
extern _SYS_PAR g_SysPar;

#endif   
