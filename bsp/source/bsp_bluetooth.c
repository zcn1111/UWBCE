#include "bsp.h"

DataType       g_LYRturnType;                   //应答蓝牙数据类型
//DataType g_RecLYDataType;
EFenceDataType g_RecLYEFenceData;         //蓝牙接收电子围栏数据
char           g_LYID[14];                 /*ID数组*/

uint8_t        g_LYInitPro;                   /*蓝牙初始化流程*/
uint8_t        g_RecLYOkFlag;          //AT命令发送成功标记
uint8_t        g_LYInitErr=0;             //AT命令发送出错次数
uint8_t        g_StrSelfCheck_LY;         //蓝牙自检标记
uint8_t 	   g_LYRecLocalDataFlag = 0;  /*蓝牙是否开始接收定位数据*/
uint8_t 	   g_LYAnsDataNum = 0;        /*蓝牙定位数据未应答次数*/

void BTInit(void)
{
	g_LYInitPro = 1;          /*蓝牙配置流程标记*/
	g_RecLYOkFlag = 0;        /*AT命令发送成功标记*/
	g_LYInitErr = 0;
	g_LYRecLocalDataFlag = 0;  /*蓝牙是否开始接收定位数据*/
	g_LYAnsDataNum = 0;        /*蓝牙定位数据未应答次数*/
	memset(&g_LYRturnType,0,sizeof(g_LYRturnType));/*蓝牙应答标记*/
}
/****************************************************************************
函数名称: void BTSendAT(char* s)
功能描述: 
参数输入: 
参数输出: 
返  回  值: 无
其      他: 无
*****************************************************************************/
void BTSendAT(char* s)
{
	uint16_t len;
	
	memset(g_SecTxBuf1,0,sizeof(g_SecTxBuf1));
	len = strlen(s);
	memcpy(g_SecTxBuf1,s,len);
	UartSend(&g_Uart1,g_SecTxBuf1,len);		
}
