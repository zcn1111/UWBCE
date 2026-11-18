/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __G_FUN_H
#define __G_FUN_H


#include <stdint.h>
#include <stdbool.h>
#include "bsp_timer.h"
#define INVALIDTIME 0xFFFFFFFF
#define M_LITTLE_ENDIAN		0
#define M_BIG_ENDIAN		1

uint8_t CheckSum(uint16_t len_Array,uint8_t *Array);
uint8_t CheckSumNoC(uint8_t *Array,uint16_t len_Array);
uint16_t CRC16(uint8_t* pchMsg, uint16_t wDataLen);
bool CheckCRC16(uint8_t* pchMsg, uint16_t wDataLen);
uint8_t XorFunction(uint32_t len, uint8_t *data);
bool CheckNumIsBCD(uint8_t *data,uint32_t len);
uint8_t CheckSumNoFan(uint16_t len_Array,uint8_t *Array,uint8_t BeforeSum);
void Uint32ToStr(uint32_t Value,uint8_t *strOut);
uint32_t StrToUint32(uint8_t *strIn);
char Hex2Asc(uint8_t hex);
uint8_t Asc2Hex(char asc);
void Bcd2Asc(uint8_t bcd,uint8_t *retstr);
uint8_t Asc2Bcd(char ascH, char ascL);
uint8_t CharSwap(uint8_t chr);
uint32_t GetValFromString(uint8_t *ptr,uint8_t len);
uint8_t StrNcmpString(uint8_t *string1,uint8_t *string2,uint16_t len,uint16_t *pos);
uint16_t Get16ValueFrom8Buf(uint8_t *buf,uint8_t no);
void YearToStr(uint16_t Year, char* pStr);
void HexTimeToStr(uint16_t val, char* pStr);
void BcdTimeToStr(uint16_t val, char* pStr);
uint16_t StrToYear(char* pStr);
uint8_t StrToTime(uint8_t* pStr);
void TimeHexToBcd(DATETIME *dest ,uint8_t *src);
void TimeBcdToHex(DATETIME *dest ,uint8_t *src);
uint8_t HexToBCD(uint8_t val);	
uint8_t BCDToHex(uint8_t val);	
uint32_t CalHowManyMinuteFromNow( DATETIME sNowTime );
uint32_t CalHowManyDateFrom2000_01_01( DATETIME sNowTime );
int32_t Sys_CalTimeDiff( DATETIME sStopTimeHex,DATETIME sStartTimeHex,uint8_t CanOnlyToDate,uint8_t changtime);
uint16_t Arr2AscStr(uint8_t *Arr, uint16_t ArrLen, char SpiltChr, char *AscStr);
int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength);
int32_t StrToIntFix(char *_pStr, uint8_t _ucLen);
int32_t StrToIntFix_EFence(char *_pStr, uint8_t _ucLen);
int32_t StrToInt(char *_pStr);
uint8_t CalcSumByInitialVal(uint8_t *Buf, uint32_t Len, uint8_t InitVal);
uint16_t ConvArrayToUint16(uint8_t *Buf, uint16_t Pos, uint8_t Mode);
/* 数据格式转换 */
uint32_t ConvArrayToUint32(uint8_t *Buf, uint16_t Pos, uint8_t Mode);
uint8_t areAllElementsZero(uint8_t *array, uint8_t size) ;
void ConvUint32ToArray(uint32_t Val, uint8_t *OutBuf, uint8_t Mode);
void ConvUint16ToArray(uint16_t Val, uint8_t *OutBuf, uint8_t Mode);
uint8_t Hex2Bcd(uint8_t Hex);
uint16_t CalcCRC16(uint8_t *Buf, uint16_t Len);
uint8_t FindStrCopy(const char* src, char* dest, size_t dest_size, char* FindStr);
void* mem_mem(const void* haystack, size_t haystack_len,
              const void* needle, size_t needle_len);
#endif /* __G_FUN_H */
