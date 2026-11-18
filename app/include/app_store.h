/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_STORE_H
#define __APP_STORE_H

#include <stdint.h>
#include <stdbool.h>
#include "bsp_store.h"


extern DATAINFLASH g_C3DatainFlash;	            /*C3数据*/
extern DATAINFLASH g_B9DatainFlash;	            /*B9数据*/
extern DATAINFLASH g_DADatainFlash;	            /*D5数据*/
extern DATAINFLASH g_LLDatainFlash;	    /*LL数据*/
extern DATAINFLASH g_WindNorDataFlashVer;
extern DATAINFLASH g_RadarLiquidDataFlashVer;
extern uint32_t g_SaveSensorDataPageAddr;
extern uint32_t g_ReadSensorDataPageAddr;


extern uint32_t g_SaveLoraWarnPageAddr;
extern uint32_t g_SaveLoraWarnPageOff;
extern uint32_t g_ReadLoraWarnPageAddr;
extern uint32_t g_ReadLoraWarnPageOff;
extern bool g_HaveLoraWarnInFlash;

extern uint32_t g_SaveUploadStatusDataPageAddr;
extern uint32_t g_SaveUploadStatusDataPageOff;
extern uint32_t g_ReadUploadStatusDataPageAddr;
extern uint32_t g_ReadUploadStatusDataPageOff;
extern bool g_HaveUploadStatusDataInFlash;




typedef struct{
	uint8_t warnbag[16];
	uint32_t sensordataaddr;
}WARNDATA;
extern WARNDATA g_LoraWarnData;

extern uint8_t g_HaveNewLoraWarn;


uint8_t ReadDTUStatusDataFromFlash(void);
uint8_t ReadLoraWarnFromFlash(void);
void GenerateStatus(uint8_t flg, uint16_t no, uint8_t *time);
void GenerateData(uint16_t no, uint8_t *buf, uint8_t *time);
void GenerateLoraData(uint8_t bagcnt, uint8_t bagno, uint8_t datacnt, uint8_t *buf);
void InitFlashPar(void);


#endif /* __APP_STORE_H */

