#ifndef __BSP_GPS_H
#define __BSP_GPS_H

#include "stdint.h"
#include "stdbool.h"
#include "main.h"

#define CLOSE_BEITIAN_NMEA_NUM              15
#define CLOSE_YIYUAN_NMEA_NUM       4
#define _LAPSETIME                  1

#define _STOPLAPSETIME              30

extern uint8_t CurrentIndex;
extern uint8_t CurrentCLoseNMEA;
extern uint8_t SendCLoseNMEANum;

void gps_pro(uint8_t *bPtr,int32_t len);
uint8_t ChargeSelfRtk(void);
void SendFenceToRTK(void);
void EFence_pro(uint8_t *bPtr,int32_t len);
void SetRtkNMEA(void);
#endif
