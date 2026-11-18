#ifndef __IMU_ACC_H
#define __IMU_ACC_H

#include "lsm6dsl_reg.h"
#include <stdbool.h>


/******* 返回状态定义 *******/
typedef enum
{
	IMURET_OK				= 0,	/* 0 = 成功 */
	IMURET_MEM_FAIL			= 1,	/* 1 = 申请内存失败 */
	IMURET_SEND_FAIL		= 2,	/* 2 = 发送失败 */
	IMURET_RECV_FAIL		= 3,	/* 3 = 接收失败 */
	IMURET_CHK_ERR			= 4,	/* 4 = 校验和错误 */
	IMURET_LEN_ERR			= 5,	/* 5 = 数据长度错误 */
	IMURET_NODATA_ERR		= 6,	/* 6 = 无数据 */
	IMURET_KEYWORD_ERR		= 7,	/* 7 = 关键字错误 */
	IMURET_TYPE_ERR			= 8,	/* 8 = 类型错误 */
	IMURET_INCOMPLETE_ERR	= 9,	/* 9 = 未完成 */
	IMURET_TIMEOUT_ERR		= 10,	/* 10 = 超时 */
} IMUResult;

/******* IMU参数定义 *******/
typedef struct
{
	float AccX;		//加速度 - X轴 mg
	float AccY;		//加速度 - Y轴 mg
	float AccZ;		//加速度 - Z轴 mg
	float GyoX;		//角速率 - X轴 mdps
	float GyoY;		//角速率 - Y轴 mdps
	float GyoZ;		//角速率 - Z轴 mdps
	float TmpC;		//温度 摄氏度
} IMU_Val_t;

typedef struct
{
	struct
	{
		float X;	//加速度X轴
		float Y;	//加速度Y轴
		float Z;	//加速度Z轴
	} Acc;
	struct
	{
		float Roll;	//
		float Pitch;
		float Yaw;	//
	} Euler;
} IMU_Result_t;

typedef struct {
    uint8_t status;         // 施工牌状态
    int16_t angle_x;       // X轴角度
    int16_t angle_y;       // Y轴角度
    int16_t angle_z;       // Z轴角度
    int16_t accel_x;       // X轴加速度
    int16_t accel_y;       // Y轴加速度
    int16_t accel_z;       // Z轴加速度
    int16_t tilt_angle;    // 综合倾角
} ConstructionSignIMUData;


// 滤波参数配置
#define KALMAN_Q_ACC    0.001f   // 加速度计过程噪声
#define KALMAN_R_ACC    0.01f    // 加速度计测量噪声
#define KALMAN_Q_GYRO   0.0005f  // 陀螺仪过程噪声  
#define KALMAN_R_GYRO   0.005f   // 陀螺仪测量噪声
#define MOVING_AVG_SIZE 7        // 移动平均窗口大小

// 卡尔曼滤波器结构体
typedef struct {
    float q;        // 过程噪声协方差
    float r;        // 测量噪声协方差
    float x;        // 最优估计值
    float p;        // 估计误差协方差
    float k;        // 卡尔曼增益
    bool initialized;
} KalmanFilter;

// 移动平均滤波器结构体
typedef struct {
    float buffer[MOVING_AVG_SIZE];
    int index;
    float sum;
    int count;
} MovingAverage;



/******* 外部变量定义 *******/

extern stmdev_ctx_t	g_IMUDevCtx;
extern ConstructionSignIMUData imu_data;
extern uint8_t IMU_init_err;
extern IMU_Val_t		g_IMUVal;
extern IMU_Val_t		g_IMUValStand;
extern IMU_Result_t	g_IMUValResult;

/******* 外部函数定义 *******/

/* 驱动初始化 */
void IMU_InitDriver(stmdev_ctx_t *pDevCtx);
/* IMU数据处理结果更新 */
void IMU_PostureMonitoringTask(stmdev_ctx_t *pDevCtx);

IMUResult IMU_GetAccelerationInMMg(stmdev_ctx_t *pDevCtx, float *AccX, float *AccY, float *AccZ);
IMUResult IMU_GetAngularRateInMMdps(stmdev_ctx_t *pDevCtx, float *GyoX, float *GyoY, float *GyoZ);
IMUResult IMU_GetTempratureInCelsius(stmdev_ctx_t *pDevCtx, float *Temp);
IMUResult IMU_ConvAccMg2g(IMU_Val_t *RawVal, IMU_Val_t *StdVal);
IMUResult IMU_ConvGyoMdps2rds(IMU_Val_t *RawVal, IMU_Val_t *StdVal);

#endif

