/**
 * @file    Gyroscope.h
 * @brief   MPU6050 六轴陀螺仪驱动（I2C）
 *
 * 硬件连接：
 *   I2C0: SCL=PA1, SDA=PA0（由 SysConfig 配置）
 *   MPU6050 I2C 地址：0x68（AD0 接地）
 *
 * 坐标系（右手定则）：
 *   加速度：X=前, Y=左, Z=上（静止时 Z≈+1g）
 *   角速度：绕 X 轴=Roll, 绕 Y 轴=Pitch, 绕 Z 轴=Yaw
 */

#ifndef GYROSCOPE_H
#define GYROSCOPE_H

#include <stdint.h>
#include <stdbool.h>

/* ── MPU6050 参数 ── */
#define MPU6050_ADDR 0x68         /* I2C 从机地址（AD0=0） */
#define MPU6050_WHO_AM_I_VAL 0x68 /* MPU6050 */
#define MPU6500_WHO_AM_I_VAL 0x70 /* MPU6500 */

/* ── 量程选择 ── */
typedef enum
{
    GYRO_FS_250 = 0x00,  /* ±250  °/s */
    GYRO_FS_500 = 0x01,  /* ±500  °/s */
    GYRO_FS_1000 = 0x02, /* ±1000 °/s */
    GYRO_FS_2000 = 0x03, /* ±2000 °/s */
} Gyro_Range;

typedef enum
{
    ACCEL_FS_2G = 0x00,  /* ±2g */
    ACCEL_FS_4G = 0x01,  /* ±4g */
    ACCEL_FS_8G = 0x02,  /* ±8g */
    ACCEL_FS_16G = 0x03, /* ±16g */
} Accel_Range;

/* ── 原始传感器数据 ── */
typedef struct
{
        int16_t ax, ay, az; /* 加速度原始值 */
        int16_t gx, gy, gz; /* 角速度原始值 */
        int16_t temp;       /* 温度原始值 */
} MPU6050_RawData;

/* ── 物理单位数据 ── */
typedef struct
{
        float ax, ay, az; /* 加速度 (g) */
        float gx, gy, gz; /* 角速度 (°/s) */
        float temp;       /* 温度 (°C) */
} MPU6050_Data;

/* ================================================================
 * API
 * ================================================================ */

/** 初始化 MPU6050：唤醒、验证 WHO_AM_I、配置量程 */
bool MPU6050_Init (void);

/** 设置陀螺仪量程 */
void MPU6050_SetGyroRange (Gyro_Range range);

/** 设置加速度计量程 */
void MPU6050_SetAccelRange (Accel_Range range);

/** 读取原始 ADC 值（所有 7 个寄存器：加速度×3 + 温度 + 角速度×3） */
bool MPU6050_ReadRaw (MPU6050_RawData* raw);

/** 读取并转换为物理单位 */
bool MPU6050_Read (MPU6050_Data* data);

/** 读取加速度（g） */
bool MPU6050_ReadAccel (float* ax, float* ay, float* az);

/** 读取角速度（°/s） */
bool MPU6050_ReadGyro (float* gx, float* gy, float* gz);

/** 读取温度（°C） */
bool MPU6050_ReadTemp (float* temp);

/** 打印传感器数据（RTT 输出） */
void MPU6050_Print (void);

/** 陀螺仪校准：静置采样求零偏，之后所有读数自动减去偏移 */
void MPU6050_Calibrate (void);

/** 获取陀螺仪零偏值（°/s）*/
void MPU6050_GetBias (float* gx_bias, float* gy_bias, float* gz_bias);

#endif /* GYROSCOPE_H */
