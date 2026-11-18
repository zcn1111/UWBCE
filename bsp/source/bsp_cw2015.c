/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "bsp.h"
#define IIC_PORT  hi2c1


uint8_t LowPowerWarnFlag;          /*低功耗报警 =1电量小于50%,=2电量小于80%  =0 电量大于等于80% */
uint8_t USE_CW2015 = 0;
uint8_t battery = 0;
uint8_t BatteryPowOnStaInit = 0;

void cw2015_write_byte(uint8_t reg, uint8_t data)
{
	HAL_I2C_Mem_Write(&IIC_PORT, CW2015_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
}

void cw2015_read_byte(uint8_t reg, uint8_t* data)
{
  HAL_I2C_Mem_Read(&IIC_PORT, CW2015_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, 1000);
}

void cw2015_wakeup(void)
{
  cw2015_write_byte(REG_MODE, 0x00);
}

void cw2015_sleep(void)
{
  cw2015_write_byte(REG_MODE, 0xC0);
}

uint8_t cw2015_read_battery(void)
{
  uint8_t cw2015_soc;
  cw2015_read_byte(REG_SOC, &cw2015_soc);
  //printf("cw2015_soc = %d\r\n", cw2015_soc);
  return cw2015_soc;
}


uint8_t cw2015_init(void)
{
  uint8_t cw2015_ver;
  cw2015_wakeup();
  //HAL_Delay(50);
  cw2015_read_byte(REG_VERSION, &cw2015_ver);
  if(cw2015_ver != 0x73)
    return 0;
  
  return 1;
}

void Read_Battery(void)
{
	if(USE_CW2015 == 1)
	{
		battery = cw2015_read_battery();
		
		if(battery < 50)
		{
			LowPowerWarnFlag = 1; 
		}
		else
		{
			LowPowerWarnFlag = 0;
		}
	}
}

void CW2015Init(void)
{
    USE_CW2015 = cw2015_init();

    Read_Battery();
	if(USE_CW2015 == 1)
	{
		if(battery < 50)
		{
			WT588E_PLAY(KJZSBDLD);
			//HAL_Delay(1000);
			//WT588E_PLAY(KJZSBDLD);
		}
		else
		{
			WT588E_PLAY(KJZSBZC); 
			//HAL_Delay(200);
			//WT588E_PLAY(KJZSBZC);			
		}
	}
	else
	{
		WT588E_PLAY(KJZSBYC);
		//HAL_Delay(200);
		//WT588E_PLAY(KJZSBYC);
	}
}

