#ifndef __APP_WT588E02B_H
#define __APP_WT588E02B_H

#include "stdint.h"
#include "stdbool.h"

extern uint8_t DATA_SONG[512]; //存放音频数据
extern uint8_t ReplaceVoicePackage;

#define _WT588E02B_CS_GPIO       GPIOC
#define _WT588E02B_CS_PIN        GPIO_PIN_11


#define _WT588E02B_SO_GPIO       GPIOA
#define _WT588E02B_SO_PIN        GPIO_PIN_10

#define _WT588E02B_SI_GPIO       GPIOA          
#define _WT588E02B_SI_PIN        GPIO_PIN_11

#define _WT588E02B_SCK_GPIO      GPIOA
#define _WT588E02B_SCK_PIN       GPIO_PIN_9

#define WT588E02B_CS(n)		    (n?HAL_GPIO_WritePin(_WT588E02B_CS_GPIO,_WT588E02B_CS_PIN,GPIO_PIN_SET):HAL_GPIO_WritePin(_WT588E02B_CS_GPIO,_WT588E02B_CS_PIN,GPIO_PIN_RESET))
#define WT588E02B_SI(n)		    (n?HAL_GPIO_WritePin(_WT588E02B_SI_GPIO,_WT588E02B_SI_PIN,GPIO_PIN_SET):HAL_GPIO_WritePin(_WT588E02B_SI_GPIO,_WT588E02B_SI_PIN,GPIO_PIN_RESET))
#define WT588E02B_SCK(n)	    (n?HAL_GPIO_WritePin(_WT588E02B_SCK_GPIO,_WT588E02B_SCK_PIN,GPIO_PIN_SET):HAL_GPIO_WritePin(_WT588E02B_SCK_GPIO,_WT588E02B_SCK_PIN,GPIO_PIN_RESET))

#define WT588E02B_SO()	        HAL_GPIO_ReadPin(_WT588E02B_SO_GPIO,_WT588E02B_SO_PIN)


#define   KJZSBZC         1           /*开机中，设备正常*/
#define   KJZSBYC         2           /*开机中，设备异常*/
#define   KJZSBDLD        3           /*开机中，设备电量低，请及时充电*/
#define   ZJZSBZC         4           /*自检中，设备正常*/
#define   ZJZSBYC         5           /*自检中，设备异常*/
#define   ZJZSBDLD        6           /*自检中，设备电量低，请及时充电*/
#define   ZACD            7           /*正在充电*/
#define   LCXD            8           /*来车下道*/
#define   QXD             9           /*请下道*/
#define   WXQQZY         10           /*进入危险区，请注意*/
#define   CFWSG          11           /*超范围施工*/
#define   DLD            12           /*设备电量低，请及时充电*/
#define   SBZC           13           /*设备正常*/
#define   DDZ            14           /*打点中*/
#define   TQSD           15           /*施工未开始，请下道*/
#define   SGZQSD         16           /*开始施工，请上道*/
#define   CSSG           17           /*施工结束，请下道*/
#define   DWYC           18           /*定位异常*/
#define   DWXHR          19           /*定位信号弱*/
#define   GDJDW          20           /*固定解定位*/
#define   FDJDW          21           /*浮点解定位*/
#define   DDJDW          22           /*单点解定位*/
#define   CFSJYC         23           /*差分数据异常*/
#define   WXTXYC         24           /*无线通信异常*/
#define   WXTXZC         25           /*无线通信正常*/
#define   LYLJ           26           /*蓝牙已连接*/
#define   LYDK           27           /*蓝牙已断开*/
#define   ZYSJCG         28           /*施工时间接收成功*/
#define   DZWLCG         29           /*电子围栏接收成功*/
#define   LCXDCG         30           /*来车下道接收成功*/
#define   JRAQQ          31           /*进入安全区*/
#define   JRSGQ          32           /*进入施工区*/
#define   SXLCQZY        33           /*上行来车，请注意*/
#define   XXLCQZY        34           /*下行来车，请注意*/
#define   DXLCQZY        35           /*单线来车，请注意*/
#define   LXLCQZY        36           /*邻线来车，请注意*/
#define   FMQPFX         37           /*模拟蜂鸣器声音(频繁响)*/
#define   FMQKX          38           /*模拟蜂鸣器声音（快响）*/
#define   FMQMX          39           /*模拟蜂鸣器声音（慢响)*/
#define   ZBXD           40           /*准备下道*/
#define   JKXD           41           /*尽快下道*/
#define   YLZYQ          42           /*远离作业区*/
#define   JJLX           43           /*接近邻线*/
#define   KJLX           44           /*靠近邻线*/
#define   JRLX           45           /*进入邻线*/ /*进入非作业区*/
#define   SXLC           46           /*上行来车*/
#define   XXLC           47           /*下行来车*/
#define   SXYG           48           /*上行预告*/
#define   SXJC           49           /*上行解除*/
#define   XXYG           50           /*下行预告*/
#define   XXJC           51           /*下行解除*/
#define   SGPQX          52           /*施工牌倾斜，请注意*/
#define   SGPQD          53           /*施工牌倾倒，请注意*/
#define   SGPWZCW        54           /*施工牌位置错误，请注意*/
#define   SGPWB          55           /*施工牌未拔，请注意*/
#define   JCWZYYSWZBF    56           /*检测位置与预设位置不符*/
#define   KJZSBDCYC      57           /*开机中，设备电池异常*/
#define   BDDWZDQDWJDD   58           /*北斗定位中，当前定位精度低*/

#define   XCCFW          59           /*小车超范围*/
#define   XCHBBF         60           /*小车与计划行别不符*/
#define   XCTQSD         61           /*小车提前上道*/
#define   XCCSSG         62           /*小车超时施工*/
#define   XCCFWCSJ       63           /*小车超范围超时间*/
#define   XCZC           64           /*小车正常*/
#define   YLFH           65           /*远离防护*/


bool DATA_CHANGE(void);
void W25qxx_ReadYY(void);
void Get512Byte(uint16_t num);
void WT588E_PLAY(uint8_t DDATA);
void WT588EInit(void);
void JudgeUpdateVoice(void);
#endif


