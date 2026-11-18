#ifndef __APP_4G_H
#define __APP_4G_H

#include "stdint.h"
#include "stdbool.h"
#include "bsp.h"
/* -------------------------------lora -------------------------------------- */
#define  RTKDiffTimeOut       500              /*收到数据*/
#define	 MAX_GSM_LEN	      3072
/*中移ML307C*/
#define singlesim_0                 "AT+SINGLESIM=0\x0d"    //指定手机卡0--插拔卡
#define singlesim_1                 "AT+SINGLESIM=1\x0d"    //指定手机卡1--贴片卡
#define singlesim                   "AT+SINGLESIM?\x0d"     //读取手机卡
#define mipstatus                   "AT\x0d"
#define _ok          		        "OK"
#define _err                        "ERROR"
#define mipopen				        "AT+MIPOPEN=0,\"TCP\",\""
#define _mipopen			        "+MIPOPEN: 0,0"
#define sendtimeout0			    "AT+MIPCFG=\"timeout\",0,2\x0d"
#define sendtimeout1			    "AT+MIPCFG=\"timeout\",1,2\x0d"
//#define mipopen_cors				"AT+MIPOPEN=1,\"TCP\",\"39.107.207.235\",8002\x0d"    //非透传
//#define mipopen_diff				"AT+MIPOPEN=1,\"TCP\",\"221.204.213.61\",22042\x0d"
#define mipopen_diff				"AT+MIPOPEN=1,\"TCP\",\""
#define _mipopen1			        "+MIPOPEN: 1,0"
#define	mipsend0				    "AT+MIPSEND=0\x0d"
#define	mipsend1			        "AT+MIPSEND=1\x0d"
#define	_mipsend0				    "AT+MIPSEND=0,"
#define	_mipsend1			        "AT+MIPSEND=1,"
#define _mipsend			        "\x0d\x0a>"
#define IsDisconnect0               "+MIPURC: \"rtcp\",0"
#define IsDisconnect1               "+MIPURC: \"rtcp\",1"
#define	http				        "200"
#define disconn1				    "+MIPURC: \"disconn\",1"
#define disconn0				    "+MIPURC: \"disconn\",0"
#define CloseTransComm   	        "+++"  
#define distcp0				        "AT+MIPCLOSE=0\x0d"
#define distcp1				        "AT+MIPCLOSE=1\x0d"
#define distcp0_ok				    "+MIPCLOSE: 0"
#define distcp1_ok				    "+MIPCLOSE: 1"
#define chkcsca                     "AT+CCLK?\x0d"
#define _chkcclk                    "+CCLK:"
#define iccid                       "AT+MCCID\x0d"
#define _iccid                      "+MCCID:"
#define csq                         "AT+CSQ\x0d"
/******* 返回状态定义 *******/
extern uint8_t          current_step;              //连单位服务器
extern uint8_t          cors_current_step ;        //连服务器步骤标记
extern uint8_t			g_LoraSendOkCnt;	       //Lora发送成功次数
extern uint8_t          GsmReceBuff[MAX_GSM_LEN];  /* 手机模块接收缓冲 */
extern uint8_t          g_IPBuffer[4][4];
extern uint16_t         g_PortNo[4];
extern uint8_t g_AskTimeFlag;
extern char iccidbuff[32];

void create_http_request(
    char *output, 
    size_t out_size,
    const char *add1, 
    const char *add2, 
    const char *add3, 
    const char *add4, 
    const char *add5
);
void combineStrings(char* output, const char* str1, const char* str2,uint8_t flag);
void base64_encode(const char *input, char *output, int size);
char inclson(char *son,uint8_t *buff);
void at_tx(char *atcom);
void connectNTRIPCaster(void);	
uint8_t ChargeSelf4g(void);
void AT_Mipsend(uint16_t MipsendLen);
void AT_DiffMipsend(uint16_t MipsendLen);
void ConnectService(void);
#endif
	