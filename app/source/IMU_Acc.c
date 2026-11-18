/*
 ******************************************************************************
 * @file    read_data_simple.c
 * @author  MEMS Software Solution Team
 * @date    30-November-2017
 * @brief   This file show the simplest way to get data from sensor.
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "IMU_Acc.h"
#include "lsm6dsl_reg.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif
#include "bsp.h"

typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union{
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;


// 位姿状态枚举定义
typedef enum {
    POSTURE_NORMAL,		// 0 = 正常状态
    POSTURE_TILTED,		// 1 = 倾斜状态
    POSTURE_FALLEN,		// 2 = 倾倒状态
	POSTURE_UNKNOWN		// 3 = 未知状态（传感器故障或设备运动）
} PostureState;

/* 阈值定义 (可根据实际施工牌特性和安装方式调整) */
#define NORMAL_ANGLE_THRESHOLD    15.0f    // 正常状态的最大倾角阈值（度）
#define TILTED_ANGLE_THRESHOLD    45.0f    // 倾斜状态的最大倾角阈值（度）
// 超过45度视为倾倒状态

/* 静止检测阈值 */
#define STATIONARY_ACC_THRESHOLD  100.0f  // 加速度静止阈值(mg)：重力加速度约为1000mg，允许±200mg的误差
#define STATIONARY_GYRO_THRESHOLD 100.0f  // 陀螺仪静止阈值(mdps)：小于此值认为处于静止状态

ConstructionSignIMUData imu_data;

/* Private variables ---------------------------------------------------------*/
static axis3bit16_t data_raw_acceleration;
static axis3bit16_t data_raw_angular_rate;
static axis1bit16_t data_raw_temperature;
static uint8_t whoamI, rst;
uint8_t IMU_init_err = 0x00;


float filteredAccX = 0.0;
float filteredAccY = 0.0;
float filteredAccZ = 0.0;
float totalAngle = 0.0;
float angleX = 0.0;
float angleZ = 0.0;
float angleY = 0.0;
	
static KalmanFilter kalmanAccX = {0}, kalmanAccY = {0}, kalmanAccZ = {0};
static KalmanFilter kalmanGyoX = {0}, kalmanGyoY = {0}, kalmanGyoZ = {0};
static MovingAverage avgAccX = {0}, avgAccY = {0}, avgAccZ = {0};
static MovingAverage avgGyoX = {0}, avgGyoY = {0}, avgGyoZ = {0};


/* Extern variables ----------------------------------------------------------*/
stmdev_ctx_t	g_IMUDevCtx;
IMU_Val_t		g_IMUVal;
IMU_Val_t		g_IMUValStand;
IMU_Result_t	g_IMUValResult;

/* Private functions ---------------------------------------------------------*/
void IMU_Filter_Init(void);
/*
 *   Replace the functions "platform_write" and "platform_read" with your
 *   platform specific read and write function.
 *   This example use an STM32 evaluation board and CubeMX tool.
 *   In this case the "*handle" variable is useful in order to select the
 *   correct interface but the usage of "*handle" is not mandatory.
 *   使用自己平台的读写函数替换 platform_write 和 platform_read 函数
 */
static int32_t platform_write(void *handle, uint8_t Reg, uint8_t *Bufp,
                              uint16_t len)
{
	HAL_I2C_Mem_Write(handle, LSM6DSL_I2C_ADD_H, Reg,
                      I2C_MEMADD_SIZE_8BIT, Bufp, len, 1000);
  return 0;
}

static int32_t platform_read(void *handle, uint8_t Reg, uint8_t *Bufp,
                             uint16_t len)
{

	HAL_I2C_Mem_Read(handle, LSM6DSL_I2C_ADD_H | 0x01, Reg,
                       I2C_MEMADD_SIZE_8BIT, Bufp, len, 1000);	  
  return 0;
}


/******************************************************************
函数名称: void IMU_InitDriver
功能描述: IMU驱动初始化
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void IMU_InitDriver(stmdev_ctx_t *pDevCtx)
{
	
	/* 配置驱动读写函数 */
	pDevCtx->handle = &hi2c1;
	pDevCtx->write_reg = platform_write;
	pDevCtx->read_reg = platform_read;
	
	uint8_t retry = 0x00;
	
	/* 检查设备ID */
	whoamI = 0;
	do
	{
		lsm6dsl_device_id_get(pDevCtx, &whoamI);
		retry++;																	
	}while(whoamI != LSM6DSL_ID && retry < 3 );	// 重试3次
	
	if(retry >= 3)
	{
			IMU_init_err = 0x01;							//错误标志置位
			retry = 0x00;
			return ;
	}
	
	/* 等待设备复位 */
	lsm6dsl_reset_set(pDevCtx, PROPERTY_ENABLE);
	do
	{
		lsm6dsl_reset_get(pDevCtx, &rst);
	} while (rst && retry < 3);
	
	if(retry >= 3)
	{
		IMU_init_err = 0x01;							//错误标志置位
		retry = 0x00;
		return ;
	}
	
	/*
	*  Enable Block Data Update
	*/
	lsm6dsl_block_data_update_set(pDevCtx, PROPERTY_ENABLE);
	/*
	* Set Output Data Rate
	*/
	lsm6dsl_xl_data_rate_set(pDevCtx, LSM6DSL_XL_ODR_12Hz5);
	lsm6dsl_gy_data_rate_set(pDevCtx, LSM6DSL_GY_ODR_12Hz5);
	/*
	* Set full scale
	*/ 
	lsm6dsl_xl_full_scale_set(pDevCtx, LSM6DSL_2g);
	lsm6dsl_gy_full_scale_set(pDevCtx, LSM6DSL_2000dps);

	/*
	* Configure filtering chain(No aux interface)
	*/ 
	/* Accelerometer - analog filter */
	lsm6dsl_xl_filter_analog_set(pDevCtx, LSM6DSL_XL_ANA_BW_400Hz);

	/* Accelerometer - LPF1 path ( LPF2 not used )*/
	//lsm6dsl_xl_lp1_bandwidth_set(&dev_ctx, LSM6DSL_XL_LP1_ODR_DIV_4);

	/* Accelerometer - LPF1 + LPF2 path */  
	lsm6dsl_xl_lp2_bandwidth_set(pDevCtx, LSM6DSL_XL_LOW_NOISE_LP_ODR_DIV_100);

	/* Accelerometer - High Pass / Slope path */
	//lsm6dsl_xl_reference_mode_set(&dev_ctx, PROPERTY_DISABLE);
	//lsm6dsl_xl_hp_bandwidth_set(&dev_ctx, LSM6DSL_XL_HP_ODR_DIV_100);

	/* Gyroscope - filtering chain */
	lsm6dsl_gy_band_pass_set(pDevCtx, LSM6DSL_HP_260mHz_LP1_STRONG);
	
	
	IMU_Filter_Init(); // 重新初始化所有滤波器
}


/******************************************************************
函数名称: IMUResult IMU_GetAccelerationInMMg
功能描述: IMU获取加速度值(毫g)
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
IMUResult IMU_GetAccelerationInMMg(stmdev_ctx_t *pDevCtx, float *AccX, float *AccY, float *AccZ)
{
	lsm6dsl_reg_t	reg;
	float			acc_mg[3];
	
	IMUResult		ret;

	lsm6dsl_status_reg_get(pDevCtx, &reg.status_reg);	//获取寄存器状态
	ret = IMURET_NODATA_ERR;
	
	/* 加速度有数据 */
	if (reg.status_reg.xlda)
	{
		/* Read magnetic field data */
		memset(data_raw_acceleration.u8bit, 0x00, 3*sizeof(int16_t));
		lsm6dsl_acceleration_raw_get(pDevCtx, data_raw_acceleration.u8bit);
		acc_mg[0] = lsm6dsl_from_fs2g_to_mg(data_raw_acceleration.i16bit[0]);
		acc_mg[1] = lsm6dsl_from_fs2g_to_mg(data_raw_acceleration.i16bit[1]);
		acc_mg[2] = lsm6dsl_from_fs2g_to_mg(data_raw_acceleration.i16bit[2]);

		*AccX = acc_mg[0];
		*AccY = acc_mg[1];
		*AccZ = acc_mg[2];
		
		ret = IMURET_OK;
	}
	
	return ret;
}

/******************************************************************
函数名称: IMUResult IMU_ConvAccMg2g
功能描述: 加速度mg转换为g
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
IMUResult IMU_ConvAccMg2g(IMU_Val_t *RawVal, IMU_Val_t *StdVal)
{
	StdVal->AccX = RawVal->AccX / 1000;
	StdVal->AccY = RawVal->AccY / 1000;
	StdVal->AccZ = RawVal->AccZ / 1000;
	
	return IMURET_OK;
}

/******************************************************************
函数名称: uint16_t IMU_GetAngularRateInMMdps
功能描述: IMU获取角速率值(毫弧度每秒mdps)
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
IMUResult IMU_GetAngularRateInMMdps(stmdev_ctx_t *pDevCtx, float *GyoX, float *GyoY, float *GyoZ)
{
	lsm6dsl_reg_t	reg;
	float			gyo_mdps[3];
	
	IMUResult		ret;
	
	lsm6dsl_status_reg_get(pDevCtx, &reg.status_reg);	//获取寄存器状态
	ret = IMURET_NODATA_ERR;
	
	/* 陀螺仪有数据 */
	if (reg.status_reg.gda)
	{
		/* Read magnetic field data */
		memset(data_raw_angular_rate.u8bit, 0x00, 3*sizeof(int16_t));
		lsm6dsl_angular_rate_raw_get(pDevCtx, data_raw_angular_rate.u8bit);
		gyo_mdps[0] = lsm6dsl_from_fs2000dps_to_mdps(data_raw_angular_rate.i16bit[0]);
		gyo_mdps[1] = lsm6dsl_from_fs2000dps_to_mdps(data_raw_angular_rate.i16bit[1]);
		gyo_mdps[2] = lsm6dsl_from_fs2000dps_to_mdps(data_raw_angular_rate.i16bit[2]);

		*GyoX = gyo_mdps[0];
		*GyoY = gyo_mdps[1];
		*GyoZ = gyo_mdps[2];
		
		ret = IMURET_OK;
	}
	
	return ret;
}

/******************************************************************
函数名称: IMUResult IMU_ConvGyoMdps2rds
功能描述: 角速度mdps转换为rds(毫弧度/s转换为rad/s)
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
IMUResult IMU_ConvGyoMdps2rds(IMU_Val_t *RawVal, IMU_Val_t *StdVal)
{
	StdVal->GyoX = RawVal->GyoX * M_PI / 180000.0f;
	StdVal->GyoY = RawVal->GyoY * M_PI / 180000.0f;
	StdVal->GyoZ = RawVal->GyoZ * M_PI / 180000.0f;
	
	return IMURET_OK;
}


/******************************************************************
函数名称: uint16_t IMU_GetTemperatureInCelsius
功能描述: IMU获取温度值(摄氏度)
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
IMUResult IMU_GetTempratureInCelsius(stmdev_ctx_t *pDevCtx, float *Temp)
{
	lsm6dsl_reg_t	reg;
	float			tempc;
	
	IMUResult		ret;
	
	lsm6dsl_status_reg_get(pDevCtx, &reg.status_reg);	//获取寄存器状态
	ret = IMURET_NODATA_ERR;
	
	/* 温度计有数据 */
    if (reg.status_reg.tda)
    {  
		/* Read temperature data */
		memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
		lsm6dsl_temperature_raw_get(pDevCtx, data_raw_temperature.u8bit);
		tempc = lsm6dsl_from_lsb_to_celsius(data_raw_temperature.i16bit);

		*Temp = tempc;
		
		ret = IMURET_OK;
    }
	
	return ret;
}

/************************************** IMU数据处理函数 ************************************************/
// 初始化卡尔曼滤波器
static void initKalman(KalmanFilter *kf, float q, float r, float initial_value) {
    kf->q = q;
    kf->r = r;
    kf->x = initial_value;
    kf->p = 1.0f;
    kf->k = 0.0f;
    kf->initialized = true;
}

// 更新卡尔曼滤波器
static float updateKalman(KalmanFilter *kf, float measurement) {
    if (!kf->initialized) {
        initKalman(kf, KALMAN_Q_ACC, KALMAN_R_ACC, measurement);
        return measurement;
    }
    
    // 预测阶段
    kf->p = kf->p + kf->q;
    
    // 更新阶段
    kf->k = kf->p / (kf->p + kf->r);
    kf->x = kf->x + kf->k * (measurement - kf->x);
    kf->p = (1 - kf->k) * kf->p;
    
    return kf->x;
}

// 初始化移动平均滤波器
static void initMovingAverage(MovingAverage *ma) {
    memset(ma->buffer, 0, sizeof(ma->buffer));
    ma->index = 0;
    ma->sum = 0.0f;
    ma->count = 0;
}


// 滤波器初始化函数
void IMU_Filter_Init(void) {
    // 初始化移动平均滤波器
    initMovingAverage(&avgAccX);
    initMovingAverage(&avgAccY);
    initMovingAverage(&avgAccZ);
    initMovingAverage(&avgGyoX);
    initMovingAverage(&avgGyoY);
    initMovingAverage(&avgGyoZ);
    
    // 卡尔曼滤波器会在第一次使用时自动初始化
    // 重置初始化标志
    kalmanAccX.initialized = false;
    kalmanAccY.initialized = false;
    kalmanAccZ.initialized = false;
    kalmanGyoX.initialized = false;
    kalmanGyoY.initialized = false;
    kalmanGyoZ.initialized = false;
}


// 更新移动平均滤波器
static float updateMovingAverage(MovingAverage *ma, float newValue) {
    if (ma->count < MOVING_AVG_SIZE) {
        // 填充缓冲区阶段
        ma->buffer[ma->count] = newValue;
        ma->sum += newValue;
        ma->count++;
        return ma->sum / ma->count;
    }
    
    // 正常滑动窗口阶段
    ma->sum -= ma->buffer[ma->index];
    ma->buffer[ma->index] = newValue;
    ma->sum += newValue;
    ma->index = (ma->index + 1) % MOVING_AVG_SIZE;
    
    return ma->sum / MOVING_AVG_SIZE;
}

// 简化的滤波处理函数
static float applySimpleFilter(float rawValue, KalmanFilter *kalman, MovingAverage *movingAvg) {
    // 先进行卡尔曼滤波
    float kalmanFiltered = updateKalman(kalman, rawValue);
    
    // 再进行移动平均滤波
    float finalValue = updateMovingAverage(movingAvg, kalmanFiltered);
    
    return finalValue;
}


/******************************************************************
函数名称: float calculateTrueInclinationAngle
功能描述: 计算与重力方向的综合倾角（0-180度）
参数输入: accX, accY, accZ - 三轴加速度值(mg)
参数输出: 无
返 回 值: float - 综合倾角(度)
******************************************************************/
float calculateTrueInclinationAngle(float accX, float accY, float accZ)
{
    // 计算与重力方向（0,1,0）的夹角
    float dotProduct = accY; // 因为重力方向是(0,1,0)
    float magnitude = sqrtf(accX*accX + accY*accY + accZ*accZ);
    
    // 防止除零错误和数值误差
    if (magnitude < 1.0f) magnitude = 1.0f;
    
    // 限制点积范围，防止acos参数超出[-1,1]
    float cos_angle = dotProduct / magnitude;
    if (cos_angle > 1.0f) cos_angle = 1.0f;
    if (cos_angle < -1.0f) cos_angle = -1.0f;
    
    // acos返回0-180度
    float angle = acosf(cos_angle) * 180.0f / M_PI;
    
    return angle;
}

void SendFloatWithNewline(float value) {
    const int max_data_length = 16 - 3; // 预留\r\n和终止符空间
    
    // 转换float为字符串
    int length = snprintf((char*)g_SecTxBuf1, max_data_length, "%.2f", value);
    
    if (length < 0 || length >= max_data_length) {
        // 错误处理或截断
        length = (length < 0) ? 0 : (max_data_length - 1);
    }
    
    // 添加\r\n
    g_SecTxBuf1[length++] = '\r';
    g_SecTxBuf1[length++] = '\n';
    
    // 发送数据
    UartSend(&g_Uart1, g_SecTxBuf1, length);
}


/******************************************************************
函数名称: PostureState IMU_DetectPosture
功能描述: 检测施工牌的位姿状态，基于综合倾角判断
参数输入: stmdev_ctx_t *pDevCtx - IMU设备上下文指针
参数输出: 无
返 回 值: PostureState - 检测到的位姿状态枚举值
******************************************************************/
PostureState IMU_DetectPosture(stmdev_ctx_t *pDevCtx)
{
    static float accX, accY, accZ;       // 三轴加速度值（单位：mg）
    static float gyoX, gyoY, gyoZ;       // 三轴角速度值（单位：mdps）
    IMUResult accRet, gyroRet;    // 传感器数据读取返回值
    
    // 读取传感器数据
    accRet = IMU_GetAccelerationInMMg(pDevCtx, &accX, &accY, &accZ);
    gyroRet = IMU_GetAngularRateInMMdps(pDevCtx, &gyoX, &gyoY, &gyoZ);
    
    // 检查数据读取是否成功
    //if (accRet != IMURET_OK) {
    //   return POSTURE_UNKNOWN;
    //}

    // 应用简化滤波（无异常值检测）
    filteredAccX = applySimpleFilter(accX, &kalmanAccX, &avgAccX);
    filteredAccY = applySimpleFilter(accY, &kalmanAccY, &avgAccY);
    filteredAccZ = applySimpleFilter(accZ, &kalmanAccZ, &avgAccZ);
    
    float filteredGyoX = applySimpleFilter(gyoX, &kalmanGyoX, &avgGyoX);
    float filteredGyoY = applySimpleFilter(gyoY, &kalmanGyoY, &avgGyoY);
    float filteredGyoZ = applySimpleFilter(gyoZ, &kalmanGyoZ, &avgGyoZ);
    
    // 使用滤波后的数据进行后续计算
    float totalAcc = sqrtf(filteredAccX * filteredAccX + 
                    filteredAccY * filteredAccY + 
                    filteredAccZ * filteredAccZ);
    
    float gyroMagnitude = sqrtf(filteredGyoX * filteredGyoX + 
                         filteredGyoY * filteredGyoY + 
                         filteredGyoZ * filteredGyoZ);
    
    // 验证数据有效性 - 检查总加速度
    if (fabsf(totalAcc - 1000.0f) > STATIONARY_ACC_THRESHOLD) {
        return POSTURE_UNKNOWN; // 非静止状态
    }
    
    // 计算综合倾角（使用滤波后的加速度数据）
    totalAngle = calculateTrueInclinationAngle(filteredAccX, filteredAccY, filteredAccZ);
		// 计算XYZ轴的角度（重力方向为Y轴正向）
    // X轴倾角：X轴与水平面的夹角
    angleX = atan2f(filteredAccX, filteredAccY) * 180.0f / M_PI;
    // Z轴倾角：Z轴与水平面的夹角
    angleZ = atan2f(filteredAccZ, filteredAccY) * 180.0f / M_PI;
    // Y轴倾角：Y轴与重力方向的夹角（当设备垂直时接近0°，水平时接近90°）
    angleY = atan2f(sqrtf(filteredAccX * filteredAccX + filteredAccZ * filteredAccZ), filteredAccY) * 180.0f / M_PI;
    // 调试测试输出
    //SendFloatWithNewline(totalAngle);
    
    // 根据综合倾角判断状态（三个状态：正常、倾斜、倾倒）
    if (totalAngle <= NORMAL_ANGLE_THRESHOLD) {
        return POSTURE_NORMAL;    // 正常状态
    }
    else if (totalAngle <= TILTED_ANGLE_THRESHOLD) {
        return POSTURE_TILTED;    // 倾斜状态
    }
    else {
        return POSTURE_FALLEN;    // 倾倒状态
    }
}


/******************************************************************
函数名称: void IMU_PostureMonitoringTask
功能描述: 位姿监控任务，定期检测状态变化
参数输入: stmdev_ctx_t *pDevCtx - IMU设备上下文指针
参数输出: 无
返 回 值: 无
******************************************************************/
void IMU_PostureMonitoringTask(stmdev_ctx_t *pDevCtx)
{
    static PostureState lastPosture = POSTURE_UNKNOWN;
    static uint32_t stableCount = 0;
    const uint32_t STABLE_THRESHOLD = 3; // 需要连续3次相同状态才确认
    PostureState currentPosture = POSTURE_UNKNOWN;
		if(IMU_init_err == 0x01)				//初始化异常直接返回
				return ;
    currentPosture = IMU_DetectPosture(pDevCtx);
    
    // 状态稳定性检测
    if (currentPosture == lastPosture) {
        stableCount++;
    } else {
        stableCount = 0;
        lastPosture = currentPosture;
    }
    
    // 只有稳定状态才触发更新上传数据
    if (stableCount == STABLE_THRESHOLD) {
				//__nop();
				if(currentPosture != POSTURE_UNKNOWN)// 设备运动不更新上传状态
				{
					imu_data.status = currentPosture;
					imu_data.angle_x = angleX;
					imu_data.angle_y = angleY;
					imu_data.angle_z = angleZ;
					imu_data.accel_x = filteredAccX;
					imu_data.accel_y = filteredAccY;
					imu_data.accel_z = filteredAccZ;
					imu_data.tilt_angle = totalAngle;
				}
				
				//根据IMU姿态变化动态调整上传间隔时间
    }
}




