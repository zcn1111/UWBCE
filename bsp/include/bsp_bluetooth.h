/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_BLUETOOTH_H
#define __BSP_BLUETOOTH_H

#include "stdint.h"
#include "stdbool.h"

#define btat				    "AT\r\n"
#define btatct				    "AT+CT\r\n"
#define btok				    "OK\r\n"
#define bterror				    "ERROR"
#define checkname_ble			"AT+TM\r\n"
#define _nameaddr_spp           "TD+ZRDW_"
#define setname_ble				"AT+BM"
#define checkname_spp			"AT+TD\r\n"
#define setname_spp				"AT+BD"
#define enble                   "AT+B401\r\n"
#define disble                  "AT+B400\r\n"
#define enspp                   "AT+B501\r\n"
#define disspp                  "AT+B500\r\n"
#define setuuidser              "AT+U0F000\r\n"            /*指定服务*/
#define setuuidwrite            "AT+U1F001\r\n"            /*写不应答 + 监听*/
#define setuuidread             "AT+U2F002\r\n"            /*读+监听*/
#define close_startinfor        "AT+CR00\r\n"//芯片上电LOG信息
#define open_startinfor         "AT+CR01\r\n"//开启开机LOG
#define power_fir               "AT+BR09\r\n"//开启开机LOG   //：AT+BR01，默认为最大功率09
#define ResLY                   "AT+CZ\r\n"   /*AT+U3F003写不应答*/
#define DisLY                   "AT+DISC\r\n" //断开蓝牙模块

typedef struct
{
	uint8_t AreaNum;                  /*区域编号*/
	uint8_t AreaType;                 /*区域类型*/
	uint8_t reserve[6];				  /*预留*/
	uint8_t Total;            
}EFenceDataType;

typedef struct
{
    uint8_t TrainAppro;               /*列车接近标志*/
    uint8_t Uplink;                   /*上下道标记*/
	uint8_t ElecFence;                /*电子围栏标记*/	
	uint8_t ConstrTime;               /*施工时间*/
	uint8_t WarnLEDSta;               /*指示灯配置*/	
	uint8_t RemoteControl;            /*遥控器标记*/	
}DataType;
extern DataType g_LYRturnType;
extern EFenceDataType g_RecLYEFenceData;

extern char       g_LYID[14];                 /*ID数组*/

extern uint8_t 	g_LYAnsDataNum;        /*蓝牙定位数据未应答次数*/
extern uint8_t    g_LYInitPro;
extern uint32_t	g_StrSelfCheck_LYDelay; 
extern uint8_t 	g_LYRecLocalDataFlag;  /*蓝牙是否开始接收定位数据*/
extern uint8_t    g_LYInitErr;             //AT命令发送出错次数
extern uint8_t    g_RecLYOkFlag;         //AT命令发送成功标记

extern uint8_t  RemoteControl_data[3];  //[0] 按键编号（1234） [1] 按键按下（1 按下，0 无操作）[3] 电池电量百分比

void BTSendAT(char* s);
void BTInit(void);

#endif
