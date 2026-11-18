/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __4G_COMM_H
#define __4G_COMM_H

#include "stdint.h"
#include "bsp.h"
/******* 宏定义 *******/
/* 数据包头 */
#define _4G_HEADER			        0xFA
#define EARTH_RADIUS                6371000.0 // 地球半径（米）
/* 定位卡列表 */
#define RYDW_CARDTABLE_MAXNUM		10			//定位卡最大总数
//#define RYDW_LOCATION_CORS			0			//CORS定位
#define D67bit	0
#define D68bit	1
/******* 返回状态定义 *******/
typedef enum
{
	_4GRET_OK				= 0,	/* 0 = 成功 */
	_4GRET_MEM_FAIL			= 1,	/* 1 = 申请内存失败 */
	_4GRET_SEND_FAIL		= 2,	/* 2 = 发送失败 */
	_4GRET_RECV_FAIL		= 3,	/* 3 = 接收失败 */
	_4GRET_CHK_ERR			= 4,	/* 4 = 校验和错误 */
	_4GRET_LEN_ERR			= 5,	/* 5 = 数据长度错误 */
	_4GRET_NODATA_ERR		= 6,	/* 6 = 无数据 */
	_4GRET_KEYWORD_ERR		= 7,	/* 7 = 关键字错误 */
	_4GRET_TYPE_ERR			= 8,	/* 8 = 类型错误 */
	_4GRET_INCOMPLETE_ERR	= 9,	/* 9 = 未完成 */
	_4GRET_TIMEOUT_ERR		= 10,	/* 10 = 超时 */
	_4GRET_PHONENUM_ERR		= 11,	/* 11 = 手机号错误 */
} _4GResult;


typedef enum {
    SEND_IDLE,
    SEND_AT_WAITING,   // 已发送AT，等待>
    SEND_DATA_PENDING,  // 收到>，待发数据
    SEND_DATA_COMPLETE  // 数据发送完成
} SendState;
typedef enum {
    onlineTask,
    cardTask,   // 已发送AT，等待>
    planTask,  // 收到>，待发数据
    trainTask,  // 数据发送完成
	GGATask,
	Null
} Task;
// 定义任务状态
typedef enum {
    TASK_NONE,
    TASK_CARD_WAIT_PROMPT,                   // 卡片任务等待>
    TASK_GGA_WAIT_PROMPT,                    // GGA任务等待>
	TASK_REMOTE_UPGRADE_TYPE_WAIT_PROMPT,    //获取远程升级类型
	TASK_REMOTE_UPGRADE_BIN_WAIT_PROMPT,     //获取远程升级二进制数据
	TASK_LY_DATA_UPLOAD,                     //蓝牙数据上传
	TASK_ASK_WORKPLAN,                       //索要施工计划
	TASK_SEND_D6,                            //发送D6数据
	TASK_RESPONSE,                           //下发施工计划、来车下道、施工牌应答、区域报警、远程升级应答
	TASK_SEND_DATA_TO_DIFFSERVICE
} TaskState;
/* BCD码时间日期 */
typedef struct
{
	uint8_t	Year;	//年
	uint8_t Month;	//月
	uint8_t Day;	//日
	uint8_t	Hour;	//时
	uint8_t	Minute;	//分
	uint8_t	Second;	//秒
} BcdDateTime_t;
typedef struct
{
	CardData_t	Info[RYDW_CARDTABLE_MAXNUM];		//定位卡信息
	uint8_t		CardRecvCnt[RYDW_CARDTABLE_MAXNUM];	//定位卡通信接收数据编号
	uint8_t		CardNbr;							//定位卡设备数
} CardInfoTable_t;
typedef struct
{
	uint8_t     TrainApproach;         //列车接近
	uint8_t     AppTrainApproach;      //手机APP
}TrainInfo;

typedef struct
{
	uint8_t SIM_Phone[3][7];	    //0-本机号，1-本机号倒序，2-短信中心号
    uint16_t Crc16;
}_MOBILE_SIM;

typedef struct
{
	uint8_t		    type;
	uint16_t 		Crc16;
}WorkMode_t;

typedef struct
{
	uint16_t		Diff;
	uint8_t         SelfcheckDiff;
	uint16_t 		Crc16;
}LocationAndSelfcheckDiff_t;

typedef struct
{
	uint8_t JudgementCondition;
	uint16_t Distance;
	uint8_t	PositionQuality;
	uint8_t Area;
	uint16_t VoiceCode;
	uint8_t	Latitude[4];
	uint8_t	Longitude[4];
}ProtectionPositionEle_t;

typedef struct
{
	uint8_t	Num;
	uint8_t NewData;
	ProtectionPositionEle_t ProtectionPositionEle[2];
}ProtectionPosition_t;

//extern _MOBILE_SIM g_Mobile_Sim;
extern	uint32_t lastCardTriggerTime;
extern CardInfoTable_t	g_CardTable;		//定位卡信息表
extern	bool expectingPrompt;
extern	bool ggaPending ; 
extern	 uint32_t cardSendCompleteTime;
extern	TaskState currentTask;
extern	uint8_t planState;
extern	uint8_t trainState;
extern	uint8_t cors_gprs_link_status;
extern TrainInfo 	g_TrainInfo;
extern	uint8_t sendonline;
extern uint32_t	DiffTime;
extern uint16_t 	RXlen;
extern uint32_t 	ServiceAnsewerTime;
extern uint32_t 	DiffServiceAnsewerTime; 
extern	uint8_t 	Cardinfo[69];
extern	uint8_t		sendATflag;
extern  uint8_t     gprs_link_status;
extern uint8_t     diff_gprs_link_status;
extern uint8_t g_RemoteUpdateGCFlag;
extern uint8_t 	g_UpdateType;
extern uint8_t g_AskParFlag;
extern WorkMode_t  g_ModeInFlash;
extern uint32_t g_CardSNum;
extern uint8_t g_ServiceResponseLY;
extern uint8_t     g_NeedReInitDTU;
extern uint32_t    g_ControlMsgDelay;
extern uint8_t	   g_CtrlMsgFlag;
extern LocationAndSelfcheckDiff_t g_LocationAndSelfcheckDiffFlash;
extern uint32_t   g_TrainForeshowTime;
extern uint8_t g_SendLocationFirst;
extern uint32_t g_ServiceAnsewerUpgradeTime;
extern uint32_t g_ReceiveAreaTime;
extern uint8_t BigAreaTypeValue[3];
extern uint8_t CarFlag;
extern ProtectionPosition_t ProtectionPositionInfo;
extern uint32_t ProtectionPositionTime;
extern uint8_t g_SendLocationDataToDiffService;
extern uint8_t g_SetIpAndPort;
extern uint32_t g_IssueWorkPlan;

void 	DTUProcess(void);
_4GResult AnalysisDiffData(uint8_t *diffBuf, uint16_t buffer_size) ;
BcdDateTime_t HexTime2BcdTime(DATETIME HexTime);
void RYDW_GenerateCardInfo(CardInfoTable_t CardTable, uint8_t *RecordBuf);
void RYDW_AnswerPlanServer(void);
void RYDW_AnswerTrainServer(void);
void RYDW_SendCardInfo(void);
void Gsmsenddata(void);
void CheckTimeouts(void);
_4GResult AnalysisAnswerData(uint8_t *diffBuf, uint16_t buffer_size);
_4GResult AnalysisWorkPlan(uint8_t *receBuf, uint16_t buffer_size) ;
_4GResult AnalysisTrainAndConstructionSignInfo(uint8_t *receBuf, uint16_t buffer_size) ;
_4GResult AnalysisAreaAlarmInfo(uint8_t *receBuf, uint16_t buffer_size) ;
_4GResult RTK_GetRTCMInfo(uint8_t *InBuf, uint16_t BufLen);
_4GResult AnalysisRemoteUpgradeData(uint8_t *UpgradeBuf, uint16_t buffer_size);
_4GResult AnalysisRemoteUpgradeTypeData(uint8_t *UpgradeBuf, uint16_t buffer_size);
_4GResult AnalysisRemoteUpgradeBinData(uint8_t *UpgradeBuf, uint16_t buffer_size);
_4GResult AnalysisParameterData(uint8_t *ParameterDataBuf, uint16_t buffer_size);
_4GResult AnalysisParameterResponseData(uint8_t *ResponseBuf, uint16_t buffer_size);
_4GResult AnalysisLYResponseData(uint8_t *receBuf, uint16_t buffer_size);
_4GResult AnalysisAskWorkPlanData(uint8_t *receBuf, uint16_t buffer_size);
_4GResult AnalysisAreaTypeData(uint8_t *receBuf, uint16_t buffer_size);
_4GResult AnalysisProtectionPositionInfo(uint8_t *receBuf, uint16_t buffer_size);
void	TransSendOnLine(void);
void    DTUMode_ReInit(void);
#endif


