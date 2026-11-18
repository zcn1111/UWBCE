#include "bsp.h"
#include "main.h"
#include "app_workplan.h"
#include "app_WDG.h"
#include "bsp_dw1000.h"
#include "bsp_dw1000_diag.h"
#include "bsp_dw1000_official.h"
#include "uwb_tdma.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "IMU_Acc.h"

#include "instance.h"
extern void dw_main(void);

#define USE_DWMAIN	1
#if (UWB_DEVICE_TYPE == UWB_DEVICE_TAG)
#include "uwb_ranging.h"
#elif (UWB_DEVICE_TYPE == UWB_DEVICE_ANCHOR)
#include "uwb_anchor.h"
#endif

/* ==================== 中断回调状态 ==================== */

/* ==================== 测距辅助 ==================== */
/* DW1000时间戳: 40位, 单位 DWT_TIME_UNITS ≈ 15.65ps */
/* 使用64位变量存储完整40位时间戳，避免溢出 */
/* 基站回复延迟 20ms = 20000us * 63898 ticks/us = 1277960000 DW1000 ticks */
/* 增加延迟时间以确保延迟发送成功，避免处理时间过长导致发送失败 */
#define RANGING_REPLY_DELAY_TICKS   1277960000ULL

/* 天线延迟校准 */
/* 需要通过实际测量校准这个值来提高测距精度 */
#define ANTENNA_DELAY_CALIBRATION   (0.0)  /* 初始值为0，需要根据实际测量调整 */



uint32_t freq;
int main(void)
{
 	SCB->VTOR = FLASH_BASE | 0x10000;
	HAL_Init();
	SystemClock_Config();
	ENABLE_INT();
	delay_init(72);
	freq = HAL_RCC_GetSysClockFreq();
	if(cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
	{
	}
	ADCInit();
	InitLED();

	g_SelfCheck.StartCharge = IsStartCharge();
	if (g_SelfCheck.StartCharge == 1)
	{
		LED_R_ON();
	}

	InitTimer2();
	TIM7Init(500-1, 7200-1);
	TIM6Init(800-1, 7200-1);	//定时器6

	SWITCHTYPE = 2;
	I2C1_Init();
	W25qxx_Init();
	CheckToolInit();
	InitUart();
	BTInit();
	InitKeyGpio();
	WT588EInit();
	PowerContrlInit();
	DTU_IO_Init();
	DTU_4G_Init();
	Localiza_Init();
	
	/* 初始化IMU */
	IMU_InitDriver(&g_IMUDevCtx);		//IMU驱动初始化
	

	/* UWB initialization */
//	debug_printf("\r\n==============================\r\n");
//	debug_printf("[UWB] Starting init...\r\n");
//	if (DW1000_Official_Init() == 0) {
//		debug_printf("[UWB] Init OK!\r\n");
//	} else {
//		debug_printf("[UWB] Init FAILED!\r\n");
//	}

//	/* 注册中断回调函数 */
//	dwt_setcallbacks(&tx_done_cb, &rx_done_cb, &rx_timeout_cb, &rx_error_cb);
//	debug_printf("[UWB] Callbacks registered\r\n");
//	DW1000_Diag_DumpAll();

    /* 1. GPIO初始化 */
    DW1000_GPIO_Init();

    /* 2. 硬件复位 */
    DW1000_HardReset();

    /* 3. SPI初始化 */
    SPI2_Init();
	
    /* 8. 使能STM32 NVIC中断 (EXTI15_10 for PB14) */
    DW1000_NVIC_Init();
    debug_printf("[DW1000] NVIC interrupt enabled\r\n");
	DW1000_PA_HIGH();
	
	/* 根据板卡模式配置输出 */
//	DW1000_SetGpioMode();
//	DW1000_SetNormalMode();

#if (UWB_DEVICE_TYPE == UWB_DEVICE_TAG)
//	Ranging_Init(UWB_PAN_ID, UWB_TAG_ID);
//	Ranging_Start();
	debug_printf("[TAG] Started, PAN=0x%04X, ID=%d\r\n", PAN_ID, tag_id);
#elif (UWB_DEVICE_TYPE == UWB_DEVICE_ANCHOR)
//	Anchor_Init(UWB_ANCHOR_ID, UWB_PAN_ID, UWB_TOTAL_SLOTS);
//	dwt_setrxtimeout(0);
//	dwt_rxenable(DWT_START_RX_IMMEDIATE);		//启动Rx模式
	debug_printf("[ANCHOR] Started, ID=%d, PAN=0x%04X\r\n", UWB_ANCHOR_ID, UWB_PAN_ID);
#endif

	
    e2prom_Init();
	Read_EQUIPNUM_EEPROM();
	Read_HARDWAREVER_EEPROM();
	ChangeVersionFormat();
	InitCalculateEndorsement();
	ChargeInit();
	if (g_SelfCheck.StartCharge == 0)
	{
		CW2015Init();
		CW2015Init();
	}
	g_SelfCheck.PowerOnTime = TIM2->CNT;
	InitWorkPlan();
	Get_Adc_Average(ADC_CHANNEL_6,10);
	CalculateSpeedInit();

#if (USE_DWMAIN)
	dw_main();
	while (1)
	{
		debug_printf("Run failure.\r\n");
	}
#endif
#if (!USE_DWMAIN)
	DW1000_init();
	Ranging_Init(&g_UwbInstance, MODE_TAG, 0, 0xDECA, &config);
	
	while(1)
	{
		/* UWB测距处理 */
		Ranging_Process(&g_UwbInstance);
		
		CheckTool();		
		LEDPowONSta();
		
		if((g_LYInitPro > 0) && (SWITCHTYPE == 0))
		{
			LYParam_Init();
		}
		Read_Flag_Key();
		if (SWITCHTYPE == 0)
		{
			LYProcess();	
		}		
		DTUProcess();
		//连接服务器
		ConnectService();
		if (g_ModeInFlash.type == 1)
		{
			//connectNTRIPCaster();
		}
		Gsmsenddata();
		//未升级程序与未充电时，进行区域判断与报警判断与计算违章信息
		if ((g_RemoteUpdateGCFlag == 0) && (g_SelfCheck.StartCharge == 0))
		{
			JudgEFence();
			LedStatus();
			CalculateEndorsement();
			//CalculateProtectionPosition();
		}
		UartProcess();
		CheckTimeouts();
		if(JudgeSomeDelayInProOk(g_SampADCDelay,Delay1Min) == true)
		{/*AD采样*/				
			Get_Adc_Average(ADC_CHANNEL_6,10);
			Read_Battery();	
			g_SampADCDelay = TIM2->CNT;			
		}
		JudgeUpdateVoice();
		DTUMode_ReInit();
		//充电自检
		if (SWITCHTYPE != 2)
		{
			SelfCheckProcess(); 
		}
		WorkPlanProcess();
		WorkSelfCheckProcess();
		SetRtkNMEA();
	}
#endif	//(!USE_DWMAIN)
}

