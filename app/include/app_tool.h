/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_TOOL_H
#define __APP_TOOL_H

#include <stdint.h>
#include "stdint.h"
#include "stdbool.h"
#include "bsp.h"
#include "main.h"

/* 以下为主板和客服工具通信协议 */
//包头 --- 通用和专用
//通用包头
#define   RECEIVETAG_ALL                     0xC1        //通用协议的接收包头
#define   SENDTAG_ALL                        0xC2        //通用协议的发送包头
//类型 --- 通用和专用
//通用协议
#define   SET_MY_NUMBER_TYPE                 0x03       //设置本机号
#define   GET_MY_NUMBER_TYPE                 0x04       //索要本机号
#define   SET_IP_TYPE                        0x05       //设置IP
#define   GET_IP_TYPE                        0x06       //索要IP
#define   GET_PARAM_INFOR_TYPE               0x07       //索要参数信息

extern uint8_t g_ToolCodeTypeErrorFlag;		//程序程序类型错误标记
extern uint8_t g_ToolOpenDoorFlag;
extern uint8_t g_ToolGetICCIDFlag;
extern uint8_t g_ToolGetICCIDStep;
extern uint8_t g_ShouldJumpToStartFlag;




void ToolProcess(void);
void InitToolVar(void);

#endif /* __APP_TOOL_H */
