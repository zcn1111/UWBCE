/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_H
#define __BSP_H

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "g_fun.h"
#include "bsp_uart.h"
#include "bsp_spi.h"
#include "bsp_timer.h"
#include "bsp_adc.h"
#include "bsp_lptim.h"
#include "bsp_exti.h"
#include "bsp_gpio.h"
#include "w25qxx.h"
#include "g_fun.h"
#include "bsp_store.h"
#include "bsp_bluetooth.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "bsp_iic.h"
#include "bsp_rtk.h"
#include "bsp_eeprom.h"
#include "bsp_cw2015.h"
#include "bsp_wt588e02b.h"
#include "bsp_selfcheck.h"
#include "bsp_4g.h"
#include "bsp_dw1000.h"
#include "bsp_dw1000_diag.h"
/* ????????§Ø??? */
#define ENABLE_INT()	__set_PRIMASK(0)	/* ???????§Ø? */
#define DISABLE_INT()	__set_PRIMASK(1)	/* ???????§Ø? */





void SystemClock_Config(void);
__asm void MSR_MSP(uint32_t addr);

void Error_Handler(void);

#endif /* __BSP_H */


