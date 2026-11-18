#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "g_fun.h"

const uint8_t montday[12] ={31,28,31,30,31,30,31,31,30,31,30,31};


/******************************************************************************
 * 函数名称: CRC16
 * 功能描述: 计算校验值
 * 输　  入: pchMsg，要校验的数据指针，wDataLen，数据长度
 * 输　  出: 无
 * 返    回：返回校验值
********************************************************************************/
uint16_t CRC16(uint8_t* pchMsg, uint16_t wDataLen)
{
 	uint16_t crc;
	uint16_t i;
	
	crc=0xFFFF;

	while(wDataLen--!=0)
	{

		crc=(crc ^ ((*pchMsg)&0xff))&0xffff;
		for(i=0;i<8;i++)
		{
			if((crc & 0x0001)!=0)
			{
				crc=((crc)>>1);
				crc=((crc) ^ 0xA001);
			}
			else
			{
				crc=((crc)>>1);
			}
		}
		pchMsg++;
	}
	return(crc&0xffff);
}
/******************************************************************************
 * 函数名称: bool CheckCRC16(uint8_t* pchMsg, uint16_t wDataLen)
 * 功能描述: 判断校验和是否正确
 * 输　  入: pchMsg，要校验的数据指针，wDataLen，数据长度
 * 输　  出: 
 * 返    回：TRUE 正确， FALSE 错误
********************************************************************************/

bool CheckCRC16(uint8_t* pchMsg, uint16_t wDataLen)
{
	uint16_t crc,tmp;
	
	crc = CRC16(pchMsg, wDataLen-2);
	tmp = *(pchMsg+wDataLen-1);
	tmp = (tmp<<8)|*(pchMsg+wDataLen-2);
	if(crc  == tmp)
		return true;
	else
		return false;
}


/****************************************************************************
** 函数名称：CheckSum
** 函数功能：校验数据,返回校验和,累加和取反加1
** 入口参数：需要校验的字符串指针
** 出口参数：无
****************************************************************************/
uint8_t CheckSum(uint16_t len_Array,uint8_t *Array)
{
	uint16_t i;
	uint8_t Check = 0;

	//计算校验和
	for(i = 0;i< len_Array;i++)
	{
		Check += Array[i];
	}
	Check = ~Check + 1;	 //取反加1
	
	return Check;
}

/****************************************************************************
** 函数名称：CheckSum
** 函数功能：校验数据,返回累加和
** 入口参数：需要校验的字符串指针
** 出口参数：无
****************************************************************************/
uint8_t CheckSumNoC(uint8_t *Array,uint16_t len_Array)
{
	uint16_t i;
	uint8_t Check = 0;

	//计算校验和
	for(i = 0;i<len_Array;i++)
	{
		Check += Array[i];
	}
	
	return Check;
}
/****************************************************************************
** 函数名称：XorFunction
** 函数功能：异或运算
** 入口参数：需要校验的字符串指针(hex)
** 出口参数：无
** agj
****************************************************************************/
uint8_t XorFunction(uint32_t len, uint8_t *data)
{
    uint8_t result;
    uint32_t i;
    
    result = 0;
    for(i=0;i<len;i++)
    {
        result ^= data[i];
    }
    return result;
}
/****************************************************************************
** 函数名称：CheckNumIsBCD
** 函数功能：校验数据是否为合格的BCD
** 入口参数：需要校验的字符串指针(hex)
** 出口参数：无
****************************************************************************/
bool CheckNumIsBCD(uint8_t *data,uint32_t len)
{
    uint32_t i;
    uint8_t beforeh,afterh;
    
    for(i=0;i<len;i++)
    {
        beforeh = (data[i]&0xf0)>>4;
        afterh = data[i]&0x0f;
        if(beforeh > 9 || afterh > 9)   //if(beforeh > 9 || afterh > 9 || (beforeh == 0 && afterh == 0))
        {
            return false;
        }
    }
    return true;
}
/****************************************************************************
** 函数名称：CheckSumNoFan
** 函数功能：校验数据,返回累加和
** 入口参数：需要校验的字符串指针
** 出口参数：无
****************************************************************************/
uint8_t CheckSumNoFan(uint16_t len_Array,uint8_t *Array,uint8_t BeforeSum)
{
	uint16_t i;
	uint8_t Check = 0;

	//计算校验和
	for(i = 0;i< len_Array;i++)
	{
		Check += Array[i];
	}
	Check += BeforeSum;
	return Check;
}

/****************************************************************************
** 函数名称：Uint32ToStr
** 函数功能：32位的int类型转换为字符串,4个uint8的数组(0x12345678->0x12,0x34,0x56,0x78)
** 入口参数：Value:需要转换的uint32值
** 出口参数：strOut:转换后的字符串
****************************************************************************/
void Uint32ToStr(uint32_t Value,uint8_t *strOut)
{
	strOut[0] = (uint8_t)(Value & 0xFF);
	strOut[1] = (uint8_t)(Value>>8 & 0xFF);
	strOut[2] = (uint8_t)(Value>>16 & 0xFF);
	strOut[3] = (uint8_t)(Value>>24 & 0xFF);
}

/****************************************************************************
** 函数名称：StrToUint32
** 函数功能：4字节的字符串转换为32位uint32型变量
** 入口参数：strIn:需要转换的数组
** 出口参数：
** 返 回 值：转换后的值
****************************************************************************/
uint32_t StrToUint32(uint8_t *strIn)
{
	uint32_t ret = 0;
	
	ret = strIn[0] | (strIn[1]<<8) | (strIn[2]<<16) | (strIn[3]<<24);
	return ret;
}
/******************************************************************************
函数名称: uint8_t Hex2Bcd
功能描述: 十六进制转换为BCD码
参数输入: uint8_t	Hex	十六进制字符
参数输出: 无
返 回 值: uint8_t BCD码
其    他: 无
********************************************************************************/
uint8_t Hex2Bcd(uint8_t Hex)
{
	uint8_t bcdh, bcdl;
	uint8_t ret;
	
	if (Hex > 99)
	{
		ret = 0;
	}
	else
	{
		bcdh = Hex / 10;
		bcdl = Hex % 10;
		ret = (bcdh << 4) + bcdl;
	}
	
	return ret;
}

/******************************************************************************
 * 函数名称: char Hex2Asc(uint8 hex)
 * 功能描述: 十六进制数转换为ascii码 0~F转换为'0'~'F'
 * 输　  入: 十六进制 0x00~0x0F或0~15
 * 输　  出: 对应的ascii码
********************************************************************************/
char Hex2Asc(uint8_t hex)
{
	char ret;
	if (hex < 0x0A) //如果小于10
	{
		ret = hex + 0x30;//'0'的ASCII是0x30
	}
	else
	{
		ret = hex + 0x37;//'A'的ASCII是0x41
	}
	return ret;	
}

/******************************************************************************
 * 函数名称: uint8 Asc2Hex(char asc)
 * 功能描述: ascii码转换为十六进制数 '0'~'F'转换为0~F
 * 输　  入: ascii码
 * 输　  出: 十六进制 0x00~0x0F或0~15
********************************************************************************/
uint8_t Asc2Hex(char asc)
{
	uint8_t ret;

	if (asc <= '9')		//asc:1--9
	{
		ret = asc - '0';
	}
	else if(asc <= 'Z')	//asc:A--F
	{
		ret = asc - 0x37;
	}
	else				//asc:a--f
	{
		ret = asc - 0x57;
	}
	return ret;	
}

/******************************************************************************
 * 函数名称: Bcd2Asc
 * 功能描述: bcd码转换为两个ascii码字符 例:0x2F转换为"2F"
 * 输　  入: bcd:十六进制
 * 输　  出: retstr:2个ascii码字符
********************************************************************************/
void Bcd2Asc(uint8_t bcd,uint8_t *retstr)
{
	uint8_t tmp;

	/*取出字节的高4位*/
	tmp = bcd >> 4;
	retstr[0] = Hex2Asc(tmp);

	/*取出字节的低4位*/
	tmp = bcd & 0x0F;
	retstr[1] = Hex2Asc(tmp);
}

/******************************************************************************
 * 函数名称: uint8 Asc2Bcd(char ascH, char ascL)
 * 功能描述: 两个ascii码字符转换为bcd码 例:"2F"转换为0x2F
 * 输　  入: 2个ascii码字符 ascH:高字符,ascL:低字符
 * 输　  出: 十六进制
********************************************************************************/
uint8_t Asc2Bcd(char ascH, char ascL)
{
	return ((Asc2Hex(ascH) << 4) | Asc2Hex(ascL));
}

/******************************************************************************
 * 函数名称: CharSwap
 * 功能描述: 一个字节的半字节交换
 * 输　  入: 要转换的字节
 * 输　  出: 转换完的字节
********************************************************************************/
uint8_t CharSwap(uint8_t chr)
{
	return  (((chr << 4) & 0xF0) | ((chr >> 4) & 0x0F));
}

/******************************************************************************
 * 函数名称: uint32 GetValFromString(char *ptr,uint8 len)
 * 功能描述: 从字符串ptr 开始取len 个数据，小端模式。变成正常的数据 
 * 输　     入: ptr 字符串地址，LEN 几个8位
 * 输　     出: 正常顺序的值。
 * 返           回:正常顺序的值。是32位，16的数据与 0XFFFF 就可以了。
********************************************************************************/

uint32_t GetValFromString(uint8_t *ptr,uint8_t len)
{
	uint32_t val = 0,tmp;
	uint8_t i;
	
	for(i=0;i<len;i++)
	{
		tmp = *ptr++;		
		val |= (tmp<<(8*i));
	}
	return val;
}
/******************************************************************************
 * 函数名称: int8 StrNcmpString(uint8 *string1,uint8 *string2,uint16 len)
 * 功能描述: 比较两个字符串是否相等
 * 输　  入: 字符串及长度
 * 输　  出: 
 * 返    回: 1 大于 -1 小于 0 等于
********************************************************************************/

uint8_t StrNcmpString(uint8_t *string1,uint8_t *string2,uint16_t len,uint16_t *pos)
{
	uint16_t i;
	uint8_t ret;

	for(i=0;i<len;i++)
	{
		if(*string1 > *string2)
		{
			ret = 1;
			*pos = i;
			break;
		}
		else if(*string1 < *string2)
		{
			ret =  2;
			*pos = i;
			break;
		}
		string1++;
		string2++;
	}
	if( i == len )
		ret = 0;
	return ret;
}

/******************************************************************************
 * 函数名称: uint16 Get16ValueFrom8Buf(uint8 *buf,uint8 no)
 * 功能描述: 从8 位数据中得到16 位数据
 * 输　     入: buf-- 8 位数据字符串 ，no 数据长度
 * 输　     出: 16 位数据的结果
 * 返           回:  得到的16 位的数据
********************************************************************************/
uint16_t Get16ValueFrom8Buf(uint8_t *buf,uint8_t no)
{
	uint16_t k;

	k=buf[no+1];
	k =( k<<8)+buf[no];
	return k;
}

/**
 * @brief	十六进制年转换为字符串
 *
 * @param	
 *
 * @return  
 */
void YearToStr(uint16_t Year, char* pStr)
{
	int16_t  i;

	for(i=3; i>=0; i--)
	{
		pStr[i] = 0x30 + (Year % 10);
		Year /= 10;
	}
}

/**
 * @brief	十六进制时间转换为字符串
 *
 * @param	
 *
 * @return  
 */
void HexTimeToStr(uint16_t val, char* pStr)
{
	pStr[0] = 0x30 + (val/10);
	pStr[1] = 0x30 + (val%10);
}

/**
 * @brief	BCD时间转换为字符串
 *
 * @param	
 *
 * @return  
 */
void BcdTimeToStr(uint16_t val, char* pStr)
{
	pStr[0] = 0x30 + (val>>4);
	pStr[1] = 0x30 + (val&0x0f);
}

/**
 * @brief	字符串转换为年
 *
 * @param	
 *
 * @return  
 */
uint16_t StrToYear(char* pStr)
{
	int16_t i;
	int16_t tt=1;
	uint16_t wTemp=0;

	for(i=3; i>=0; i--)
	{
		wTemp += (pStr[i]-0x30) * tt;
		tt *= 10;
	}
	return wTemp;
}

/**
 * @brief	ASCII转换为16进制
 *
 * @param	huart	串口句柄
 *
 * @return  
 */
uint8_t StrToTime(uint8_t* pStr)
{
	return ( (pStr[1]-0x30) + (pStr[0]-0x30) * 10 );
}

/**
 * @brief	十六进制转换为BCD
 *
 * @param	
 *
 * @return  
 */
uint8_t HexToBCD(uint8_t val)	
{
	uint8_t tmp;

	tmp = ( val /10 );
	tmp = ( tmp << 4 );
	tmp += ( val % 10 );

	return tmp;
}

/**
 * @brief	BCD转换为十六进制
 *
 * @param	
 *
 * @return  
 */
uint8_t BCDToHex(uint8_t val)	
{
	uint8_t tmp;

	tmp =(((val>>4)&0x0f)*10)+(val&0x0f);

	return tmp;
}

/**
 * @brief	十六进制时间转换为BCD格式
 *
 * @param	
 *
 * @return  
 */
void TimeHexToBcd(DATETIME *dest ,uint8_t *src)
{
	dest->year = HexToBCD(*src++);
	dest->month= HexToBCD(*src++);
	dest->date= HexToBCD(*src++);
	dest->hour= HexToBCD(*src++);
	dest->minute= HexToBCD(*src++);
	dest->second = HexToBCD(*src);
}

/**
 * @brief	BCD时间转换为十六进制
 *
 * @param	
 *
 * @return  
 */
void TimeBcdToHex(DATETIME *dest ,uint8_t *src)
{
	dest->year = BCDToHex(*src++);
	dest->month= BCDToHex(*src++);
	dest->date= BCDToHex(*src++);
	dest->hour= BCDToHex(*src++);
	dest->minute= BCDToHex(*src++);	
	dest->second = BCDToHex(*src);
}

/******************************************************************
函数名称: uint8_t CalcSumByInitialVal
功能描述: 带初值计算累加和(数据累加和 不计算补码)
参数输入: uint8_t		*Buf	接收缓存
		  uint32_t		Len		数据长度
		  uint8_t		InitVal	初值
参数输出: 无
返 回 值: uint8_t 累加和
其    他: 无
******************************************************************/
uint8_t CalcSumByInitialVal(uint8_t *Buf, uint32_t Len, uint8_t InitVal)
{
	uint32_t i;
	uint8_t sum=0;
	
	for(i=0; i<Len; i++)
	{
		sum += Buf[i];
	}
	sum = sum + InitVal;
	
	return sum;
}

/******************************************************************
函数名称: uint16_t ConvArrayToUint16
功能描述: 从数组指定位置取2字节转换为uint16
参数输入: uint8_t	*Buf	要转换的字符串
		  uint16_t	Pos		取字节位置
		  uint8_t	Mode	大小端模式
参数输出: 无
返 回 值: uint16_t 转换后的16位值
其    他: 无
******************************************************************/
uint16_t ConvArrayToUint16(uint8_t *Buf, uint16_t Pos, uint8_t Mode)
{
	uint16_t val=0;
	
	if (Mode == M_LITTLE_ENDIAN)
		val = (Buf[Pos+1] << 8) | (Buf[Pos]);
	else if (Mode == M_BIG_ENDIAN)
		val = (Buf[Pos] << 8) | (Buf[Pos+1]);
	
	return val;
}


uint8_t CalThisYearHaveDays(uint16_t Year)
{
	uint8_t leep;
	
	leep = 0;	
	if(Year%4==0)
	{
		if(Year%100==0)
		{
			if(Year%400==0)	//能被400整除为闰年
			{				
				leep = 1;
			}
			else	//能被100整除但不能被400整除不为闰年
			{
				leep = 0;
			}
		}
		else
		{
	
			leep = 1;
		}
	}
	else //不能被4整除不是闰年
	{
		leep = 0;	
	}
	return leep;
}

uint32_t CalHowManyMinuteFromNow( DATETIME sNowTime )
{
	//RainDateTime sNowTime 其中的变量是16进制值.
	/* 计算从2000-1-1 00:00 开始到 sNowTime 有多少分钟*/
	uint32_t tmpMin,tmpDate;
	uint16_t i;
	uint8_t leep;
	
	tmpMin = 0;
	tmpDate = 0;
	for( i = 00 ; i < sNowTime.year; i++ )
	{
		leep = CalThisYearHaveDays( i + 2000 );
		if( leep == 1)
			tmpDate += 366;
		else
			tmpDate += 365;		
	}
	for( i = 1 ; i < sNowTime.month; i++ )
	{
		
		tmpDate += montday[i-1];
	}
	leep = CalThisYearHaveDays( sNowTime.year + 2000 );
	if( leep == 1)
		tmpDate += 1;
	tmpDate += sNowTime.date;
	tmpMin = tmpDate* 1440; //Minute
	tmpMin += ( sNowTime.hour - 1 ) * 60 + sNowTime.minute;
	return tmpMin;
}


uint32_t CalHowManyDateFrom2000_01_01( DATETIME sNowTime )
{
	/* 计算从2000-1-1 00:00 开始到 sNowTime 有多少分钟*/
	uint32_t tmpDate;
	uint16_t i,year,month;
	uint8_t leep;
	
	tmpDate = 0;
	year = sNowTime.year;
	for( i = 00 ; i < year; i++ )
	{
		leep = CalThisYearHaveDays( i + 2000 );
		if( leep == 1)
			tmpDate += 366;
		else
			tmpDate += 365;		
	}
	month = sNowTime.month;		
	for( i = 1 ; i < month; i++ )
	{
		
		tmpDate += montday[i-1];
	}
	leep = CalThisYearHaveDays( year + 2000 );
	if( leep == 1)
		tmpDate += 1;
	tmpDate += sNowTime.date;
	return tmpDate;
}



/******************************************************************
函数名称: int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
功能描述: 7bit编码
参数输入: pSrc-源字符串指针 pDst-目标编码串指针 nSrcLength-源字符串长度
参数输出: 
返 回 值: 目标编码长度
其    他: 无
******************************************************************/
int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int nSrc;        	//源字符串的计数值
    int nDst;        	//目标编码串计数值
    int nChar;       	//当前正在处理的组内字符字节的序号,范围是0-7
    unsigned char nLeft;//上一字节残余的数据
   
    // 计数值初始化
    nSrc = 0;
    nDst = 0;
   
    // 将源串每8个字节分为一组,压缩成7个字节
    // 循环该处理过程，直至源串被处理完
    // 如果分组不到8字节,也能正确处理
    while(nSrc<nSrcLength)
    {
        //取源字符串的计数值的最低3位
        nChar = nSrc & 7;
   
        //处理源串的每个字节
        if(nChar == 0)
        {
            //组内第一个字节,只是保存起来,待处理下一个字节时使用
            nLeft = *pSrc;
        }
        else
        {
            //组内其他字节,将其右边部分与残余数相加,得到一个目标编码字节
            *pDst = (*pSrc << (8-nChar)) | nLeft;
   
            //将该字节剩下的左边部分作为残余数据保存起来
            nLeft = *pSrc >> nChar;
            //修改目标串的指针和计数值
			pDst++;
            nDst++;
        }
       
        //修改源串的指针和计数值
        pSrc++; 
		nSrc++;
    }
   
    //返回目标串长度
    return nDst;
}

/***************************************************************************
	函数名称：DateTime Sys_CalTimeDiff( RainDateTime sStopTimeHex,RainDateTime sStartTimeHex,uint8 CanOnlyToDate)

	函数参数：计算2 个日期的时间差，以天为单位计算。

				       计算2个日期差多少分钟
					CanOnlyToDate -- =1 只计算差几天。=0 计算差多少分钟
				changtime =1 bcdtohex
	函数描述：

	函数返回值：差多少分钟
***************************************************************************/
int32_t Sys_CalTimeDiff( DATETIME sStopTimeHex,DATETIME sStartTimeHex,uint8_t CanOnlyToDate,uint8_t changtime)
{
	int32_t tmpMinute;
	uint32_t tmpStopMin , tmpStartMin;
	DATETIME sStopTime;
	DATETIME sStartTime;
	
	if(changtime == 1)
	{
		TimeBcdToHex( &sStopTime , (uint8_t*)&sStopTimeHex );
		TimeBcdToHex( &sStartTime,  (uint8_t*)&sStartTimeHex );	
	}
	tmpMinute = INVALIDTIME;
	
//	if(sStartTime.Year == 0xff && sStartTime.Month >12 && sStartTime.Date >31)		// 应该为或的关系-WT
//		return tmpMinute;
//	else if(sStopTime.Year == 0xff && sStopTime.Month >12 && sStopTime.Date >31 )
//		return tmpMinute;
    
	if(sStartTime.year == 0xff || sStartTime.month >12 || sStartTime.date >31 || sStopTime.year == 0xff || sStopTime.month >12 || sStopTime.date >31)
        return tmpMinute;
	if(sStopTime.year < sStartTime.year)						
		return tmpMinute;
	else if(sStopTime.year == sStartTime.year)
	{
		if((sStopTime.month < sStartTime.month)||((sStopTime.month == sStartTime.month)&&(sStopTime.date <sStartTime.date)))
			return tmpMinute;			
	}
	if(CanOnlyToDate == 1)
	{
		sStartTime.hour= 1;					// 只比较日期,时间人为赋值为相等
		sStartTime.minute = 0;
		
		sStopTime.hour= 1;	
		sStopTime.minute = 0;
	}
	tmpStopMin = CalHowManyMinuteFromNow(sStopTime);
	
	tmpStartMin = CalHowManyMinuteFromNow(sStartTime);

	tmpMinute = (tmpStopMin - tmpStartMin);
    return   tmpMinute;
}

/******************************************************************
函数名称: int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
功能描述: 7bit解码
参数输入: pSrc-源编码串指针 pDst-目标字符串指针 nSrcLength-源编码串长度
参数输出: 
返 回 值: 目标字符串长度
其    他: 无
******************************************************************/
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    int nSrc;        	//源字符串的计数值
    int nDst;        	//目标解码串的计数值
    int nByte;       	//当前正在处理的组内字节的序号,范围是0-6
    unsigned char nLeft;//上一字节残余的数据
   
    //计数值初始化
    nSrc = 0;
    nDst = 0;
   
    //组内字节序号和残余数据初始化
    nByte = 0;
    nLeft = 0;
   
    //将源数据每7个字节为一组，解压缩成8个字节
    //循环该处理过程，直至源数据被处理完
    //如果分组不到7字节,也能正确处理
    while(nSrc<nSrcLength)
    {
        //将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节
        *pDst = ((*pSrc << nByte) | nLeft) & 0x7f;
        //将该字节剩下的左边部分，作为残余数据保存起来
        nLeft = *pSrc >> (7-nByte);
   
        //修改目标串的指针和计数值
        pDst++;
        nDst++;
   
        //修改字节计数值
        nByte++;
   
        //到了一组的最后一个字节
        if(nByte == 7)
        {
            //额外得到一个目标解码字节
            *pDst = nLeft;
   
            //修改目标串的指针和计数值
            pDst++;
            nDst++;
   
            //组内字节序号和残余数据初始化
            nByte = 0;
            nLeft = 0;
        }
   
        //修改源串的指针和技术值
        pSrc++;
        nSrc++;
    }
   
    *pDst = 0;
   
    //返回目标串长度
    return nDst;
}


/******************************************************************************
函数名称: void Hex2AscInString
功能描述: 16进制转换为ASCII码字符数组
参数输入: uint8_t	*Hex	16进制数组
参数输出: uint8_t	*strAsc	ASC码字符数组
返 回 值: 
其    他: 无
********************************************************************************/
void Hex2AscInString(uint8_t Hex, uint8_t *strAsc)
{
	uint8_t hexh, hexl;
	
	hexh = (Hex >> 4) & 0x0F;
	hexl = Hex & 0x0F;
	
	/* 处理高半字节 */
	*strAsc = Hex2Asc(hexh);
	
	/* 处理低半字节 */
	*(strAsc + 1) = Hex2Asc(hexl);
}
/******************************************************************************
函数名称: uint16_t Arr2AscStr
功能描述: 数组转换为ASCII码字符串
参数输入: uint8_t	*Arr		16进制数组
		  uint16_t	ArrLen		数组长度
		  char		SpiltChr	分隔符 0x00=不分隔输出
参数输出: char		*AscStr	ASC码字符串
返 回 值: uint16_t	输出字符串长度
其    他: 无
********************************************************************************/
uint16_t Arr2AscStr(uint8_t *Arr, uint16_t ArrLen, char SpiltChr, char *AscStr)
{
	uint16_t len=0;
	uint16_t i;
	
	for (i=0; i<ArrLen; i++)
	{
		Hex2AscInString(Arr[i], (uint8_t*)(AscStr + len));
		len += 2;
		if (SpiltChr != 0x00)
		{
			AscStr[len] = SpiltChr;
			len += 1;
		}
	}
	return len;
}



/*
*********************************************************************************************************
*	函 数 名: StrToIntFix
*	功能说明: 将ASCII码字符串转换成十进制, 给定长度
*	形    参: _pStr :待转换的ASCII码串. 可以以逗号或0结束
*			 _ucLen : 固定长度
*	返 回 值: 二进制整数值
*********************************************************************************************************
*/
int32_t StrToIntFix(char *_pStr, uint8_t _ucLen)
{
	uint8_t flag;
	char *p;
	uint32_t ulInt;
	uint8_t i;
	uint8_t ucTemp;

	p = _pStr;
	if (*p == '-')
	{
		flag = 1;	/* 负数 */
		p++;
		_ucLen--;
	}
	else
	{
		flag = 0;
	}

	ulInt = 0;
	for (i = 0; i < _ucLen; i++)
	{
		ucTemp = *p;
		if (ucTemp == '.')	/* 遇到小数点，自动跳过1个字节 */
		{
			p++;
			ucTemp = *p;
		}
		if ((ucTemp >= '0') && (ucTemp <= '9'))
		{
			ulInt = ulInt * 10 + (ucTemp - '0');
			p++;
		}
		else
		{
			break;
		}
	}

	if (flag == 1)
	{
		return -ulInt;
	}
	return ulInt;
}



int32_t StrToIntFix_EFence(char *_pStr, uint8_t _ucLen)
{
	uint8_t flag;
	char *p;
	uint32_t ulInt;
	uint8_t i;
	uint8_t ucTemp;

	p = _pStr;
	if (*p == '-')
	{
		flag = 1;	/* 负数 */
		p++;
		_ucLen--;
	}
	else
	{
		flag = 0;
	}

	ulInt = 0;
	for (i = 0; i < _ucLen; i++)
	{
		ucTemp = *p;
		if (ucTemp == '.')	/* 遇到小数点，自动跳过1个字节 */
		{
			p++;
		}
		else if ((ucTemp >= '0') && (ucTemp <= '9'))
		{
			ulInt = ulInt * 10 + (ucTemp - '0');
			p++;
		}
		else
		{
			break;
		}
	}

	if (flag == 1)
	{
		return -ulInt;
	}
	return ulInt;
}


/*
*********************************************************************************************************
*	函 数 名: StrToInt
*	功能说明: 将ASCII码字符串转换成十进制
*	形    参: _pStr :待转换的ASCII码串. 可以以逗号或0结束
*	返 回 值: 二进制整数值
*********************************************************************************************************
*/
int32_t StrToInt(char *_pStr)
{
	uint8_t flag;
	char *p;
	uint32_t ulInt;
	uint8_t i;
	uint8_t ucTemp;

	p = _pStr;
	if (*p == '-')
	{
		flag = 1;	/* 负数 */
		p++;
	}
	else
	{
		flag = 0;
	}

	ulInt = 0;
	for (i = 0; i < 15; i++)
	{
		ucTemp = *p;
		if (ucTemp == '.')	/* 遇到小数点，自动跳过1个字节 */
		{
			p++;
			ucTemp = *p;
		}
		if ((ucTemp >= '0') && (ucTemp <= '9'))
		{
			ulInt = ulInt * 10 + (ucTemp - '0');
			p++;
		}
		else
		{
			break;
		}
	}

	if (flag == 1)
	{
		return -ulInt;
	}
	return ulInt;
}


/******************************************************************
函数名称: uint32_t ConvArrayToUint32
功能描述: 从数组指定位置取Len字节转换为uint32
参数输入: uint8_t	*Buf	要转换的字符串
		  uint16_t	Pos		取字节位置
		  uint8_t	Mode	大小端模式
参数输出: 无
返 回 值: uint32_t 转换后的32位值
其    他: 无
******************************************************************/
uint32_t ConvArrayToUint32(uint8_t *Buf, uint16_t Pos, uint8_t Mode)
{
	uint32_t val=0;
	
	if (Mode == M_LITTLE_ENDIAN)
		val = (Buf[Pos+3] << 24) | (Buf[Pos+2] << 16) | (Buf[Pos+1] << 8) | (Buf[Pos]);
	else if (Mode == M_BIG_ENDIAN)
		val = (Buf[Pos] << 24) | (Buf[Pos+1] << 16) | (Buf[Pos+2] << 8) | (Buf[Pos+3]);
	
	return val;
}


/******************************************************************************
函数名称: uint8_t areAllElementsZero
功能描述: 判断数组是否有值
参数输入: 
参数输出: 无
返 回 值: uint8_t BCD码
其    他: 无
********************************************************************************/
uint8_t areAllElementsZero(uint8_t *array, uint8_t size) 
{
    uint8_t i;
	for (i = 0; i < size; i++)
	{
        if (array[i] != 0)    // 如果找到任何一个非零元素，则返回0
		{
            return 1;
        }
    }
    // 如果循环结束没有发现非零元素，返回1
    return 0;
}

/******************************************************************
函数名称: void ConvUint32ToArray
功能描述: 从uint32转换为数组
参数输入: uint32_t	Val		要转换的32位值
		  uint8_t	Mode	大小端模式
参数输出: uint8_t	*OutBuf	输出数组
返 回 值: 无
其    他: 无
******************************************************************/
void ConvUint32ToArray(uint32_t Val, uint8_t *OutBuf, uint8_t Mode)
{
	if (Mode == M_LITTLE_ENDIAN)
	{
		OutBuf[0] = (uint8_t)(Val & 0xFF);
		OutBuf[1] = (uint8_t)((Val >> 8) & 0xFF);
		OutBuf[2] = (uint8_t)((Val >> 16) & 0xFF);
		OutBuf[3] = (uint8_t)((Val >> 24) & 0xFF);
	}
	else if (Mode == M_BIG_ENDIAN)
	{
		OutBuf[0] = (uint8_t)((Val >> 24) & 0xFF);
		OutBuf[1] = (uint8_t)((Val >> 16) & 0xFF);
		OutBuf[2] = (uint8_t)((Val >> 8) & 0xFF);
		OutBuf[3] = (uint8_t)(Val & 0xFF);
	}
}

/******************************************************************
函数名称: void ConvUint16ToArray
功能描述: 从uint16转换为数组
参数输入: uint16_t	Val		要转换的32位值
		  uint8_t	Mode	大小端模式
参数输出: uint8_t	*OutBuf	输出数组
返 回 值: 无
其    他: 无
******************************************************************/
void ConvUint16ToArray(uint16_t Val, uint8_t *OutBuf, uint8_t Mode)
{
	if (Mode == M_LITTLE_ENDIAN)
	{
		OutBuf[0] = (uint8_t)(Val & 0xFF);
		OutBuf[1] = (uint8_t)((Val >> 8) & 0xFF);
	}
	else if (Mode == M_BIG_ENDIAN)
	{
		OutBuf[0] = (uint8_t)((Val >> 8) & 0xFF);
		OutBuf[1] = (uint8_t)(Val & 0xFF);
	}
}

/******************************************************************
函数名称: uint16_t CalcCRC16
功能描述: 计算CRC16 (CRC16-ModBus)
参数输入: uint8_t	*Buf	要校验的数据
		  uint16_t	Len		数据长度
参数输出: 无
返 回 值: uint16_t 计算CRC16结果
其    他: 无
******************************************************************/
uint16_t CalcCRC16(uint8_t *Buf, uint16_t Len)
{
 	uint16_t crc;
	uint16_t i;
	
	crc=0xFFFF;		//CRC计算赋初值

	while(Len--!=0)
	{
		crc=(crc ^ ((*Buf) & 0xff)) & 0xffff;		// 数据和CRC寄存器低8位异或 保存在CRC寄存器

		for(i=0;i<8;i++)
		{
			if((crc & 0x0001) != 0)		
			{
				crc=((crc) >> 1);		//判断最低位为1 先右移1位再与多项式异或
				crc=((crc) ^ 0xA001);	//CRC16-ModBus多项式 X16+X15+X2+1=0x8005 倒序多项式=0xA001
			}
			else
			{
				crc=((crc) >> 1);		//判断最低位为0 右移1位
			}
		}
		Buf++;
	}
	return (crc & 0xffff);
}

uint8_t FindStrCopy(const char* src, char* dest, size_t dest_size, char* FindStr)
{
    // 查找 "GJC" 位置
    const char* found = strstr(src, FindStr);
    
    if (found == NULL) 
	{
        dest[0] = '\0';  // 未找到，返回空字符串
        return 1;
    }
    
    // 复制 "GJC" 及其后的所有内容
    size_t len = strlen(found);
    if (len >= dest_size) 
	{
        len = dest_size - 1;  // 防止溢出
    }
    
    strncpy(dest, found, len);
    dest[len] = '\0';
    
    return 0;
}

void* mem_mem(const void* haystack, size_t haystack_len,
              const void* needle, size_t needle_len)
{
    if (haystack == NULL || needle == NULL || haystack_len < needle_len)
        return NULL;

    const unsigned char* h = (const unsigned char*)haystack;
    const unsigned char* n = (const unsigned char*)needle;

    for (size_t i = 0; i <= haystack_len - needle_len; ++i) {
        if (memcmp(h + i, n, needle_len) == 0)
            return (void*)(h + i);
    }
    return NULL;
}