/**
 * @file    Gyroscope.c
 * @brief   MPU6050 六轴陀螺仪驱动实现
 *
 * MPU6050 寄存器地址（从 0x3B 开始连续排列 14 字节）：
 *   0x3B  ACCEL_XOUT_H  Ax 高字节
 *   0x3D  ACCEL_YOUT_H  Ay 高字节
 *   0x3F  ACCEL_ZOUT_H  Az 高字节
 *   0x41  TEMP_OUT_H    温度高字节（共 2 字节）
 *   0x43  GYRO_XOUT_H   Gx 高字节
 *   0x45  GYRO_YOUT_H   Gy 高字节
 *   0x47  GYRO_ZOUT_H   Gz 高字节
 *
 * 数据格式：16 位有符号整数，大端序（高字节在前）
 *   物理值 = ADC 值 / LSB 灵敏度
 *
 * LSB 灵敏度（量程相关）：
 *   陀螺 ±250°/s   → 131.0  LSB/(°/s)
 *   陀螺 ±500°/s   →  65.5  LSB/(°/s)
 *   陀螺 ±1000°/s  →  32.8  LSB/(°/s)
 *   陀螺 ±2000°/s  →  16.4  LSB/(°/s)
 *
 *   加速度 ±2g     → 16384  LSB/g
 *   加速度 ±4g     →  8192  LSB/g
 *   加速度 ±8g     →  4096  LSB/g
 *   加速度 ±16g    →  2048  LSB/g
 *
 *   温度           → 340    LSB/°C，偏移 +36.53°C
 *
 * I2C 底层：复用 xj.c 的 XJ_I2C_ReadReg / XJ_I2C_WriteByte（已封装 MSPM0 DriverLib）
 */

#include "main.h"
#include "Gyroscope.h"
#include "xj.h"

/* ── MPU6050 寄存器 ── */
#define REG_PWR_MGMT_1 0x6B   /* 电源管理：写 0x00 唤醒 */
#define REG_WHO_AM_I 0x75     /* 芯片 ID：应返回 0x68 */
#define REG_CONFIG 0x1A       /* DLPF 数字低通滤波 */
#define REG_GYRO_CONFIG 0x1B  /* 陀螺仪量程 */
#define REG_ACCEL_CONFIG 0x1C /* 加速度计量程 */
#define REG_ACCEL_XOUT_H 0x3B /* 加速度 X 高字节（数据起始地址） */
#define REG_TEMP_OUT_H 0x41   /* 温度高字节 */
#define REG_GYRO_XOUT_H 0x43  /* 角速度 X 高字节 */

/* ── 当前量程（影响物理单位转换系数） ── */
static Gyro_Range g_gyro_range = GYRO_FS_250;
static Accel_Range g_accel_range = ACCEL_FS_2G;

/* ── 各量程对应的灵敏度（LSB/单位） ── */
static const float GYRO_LSB[] = {131.0f, 65.5f, 32.8f, 16.4f};
static const float ACCEL_LSB[] = {16384.0f, 8192.0f, 4096.0f, 2048.0f};

/* ── 陀螺仪零偏校准值（MPU6050_Calibrate 计算，MPU6050_Read 自动扣除） ── */
static float g_gx_bias = 0, g_gy_bias = 0, g_gz_bias = 0;
static float g_ax_bias = 0, g_ay_bias = 0, g_az_bias = 0;

/* ───────────────────────────────────────────────────────────────
 * 将两个大端字节组合为有符号 16 位整数
 *   例：(0x0F, 0xA0) → 0x0FA0 = 4000
 *       (0xFF, 0xCE) → 0xFFCE = -50
 * ─────────────────────────────────────────────────────────────── */
static int16_t Combine16 (uint8_t msb, uint8_t lsb)
{
    return (int16_t)(((uint16_t)msb << 8) | lsb);
}

/* ================================================================
 * 初始化 MPU6050
 *
 * 步骤：
 *   1. 写 PWR_MGMT_1 = 0x00    唤醒芯片（退出休眠模式）
 *   2. 延时 10ms                等陀螺起振稳定
 *   3. 读 WHO_AM_I              验证芯片 ID = 0x68
 *   4. 写 CONFIG = 0x03         配置 DLPF 带宽 42Hz
 *   5. 写 GYRO_CONFIG           设置陀螺量程（默认 ±250°/s）
 *   6. 写 ACCEL_CONFIG          设置加速度量程（默认 ±2g）
 *
 * @return true=初始化成功, false=通信失败或芯片不匹配
 * ================================================================ */
bool MPU6050_Init (void)
{
    uint8_t id;

    /* 唤醒 —— PWR_MGMT_1 bit6=0（退出休眠） */
    if (!XJ_I2C_WriteByte(MPU6050_ADDR, REG_PWR_MGMT_1, 0x00))
    {
        printf("MPU6050: PWR_MGMT_1 write failed\n");
        return false;
    }
    delay_ms(10); /* 晶振起振 + PLL 锁定时间 */

    /* 验证芯片 —— 3 次重试 */
    bool ok = false;
    for (int retry = 0; retry < 3; retry++) {
        if (XJ_I2C_ReadByte(MPU6050_ADDR, REG_WHO_AM_I, &id)) {
            if (id == MPU6050_WHO_AM_I_VAL || id == MPU6500_WHO_AM_I_VAL) {
                ok = true; break;
            }
        }
        delay_ms(5);
    }
    if (!ok) {
        printf("MPU6050: WHO_AM_I failed (got 0x%02X)\n", id);
        return false;
    }

    /* DLPF = 3（带宽 42Hz, 兼顾响应速度与噪声抑制） */
    XJ_I2C_WriteByte(MPU6050_ADDR, REG_CONFIG, 0x03);

    /* 默认量程 */
    MPU6050_SetGyroRange(GYRO_FS_250);
    MPU6050_SetAccelRange(ACCEL_FS_2G);

    /* init OK */
    return true;
}

/* ================================================================
 * 设置陀螺仪量程
 * GYRO_CONFIG bit[4:3] = FS_SEL
 *   0 = ±250°/s, 1 = ±500°/s, 2 = ±1000°/s, 3 = ±2000°/s
 * ================================================================ */
void MPU6050_SetGyroRange (Gyro_Range range)
{
    g_gyro_range = range;
    XJ_I2C_WriteByte(MPU6050_ADDR, REG_GYRO_CONFIG, (uint8_t)(range << 3));
}

/* ================================================================
 * 设置加速度计量程
 * ACCEL_CONFIG bit[4:3] = AFS_SEL
 *   0 = ±2g, 1 = ±4g, 2 = ±8g, 3 = ±16g
 * ================================================================ */
void MPU6050_SetAccelRange (Accel_Range range)
{
    g_accel_range = range;
    XJ_I2C_WriteByte(MPU6050_ADDR, REG_ACCEL_CONFIG, (uint8_t)(range << 3));
}

/* ================================================================
 * 读取原始 ADC 值（所有 7 个传感器寄存器）
 *
 * MPU6050 数据寄存器从 0x3B 开始是连续的 14 字节：
 *
 *   偏移  内容          说明
 *   ────  ────────────  ────────────────
 *   0~1   ACCEL_XOUT    加速度 X（高→低）
 *   2~3   ACCEL_YOUT    加速度 Y
 *   4~5   ACCEL_ZOUT    加速度 Z
 *   6~7   TEMP_OUT      温度
 *   8~9   GYRO_XOUT     角速度 X
 *   10~11 GYRO_YOUT     角速度 Y
 *   12~13 GYRO_ZOUT     角速度 Z
 *
 * 一次 I2C 批量读取 14 字节，效率远高于单个字节读 7 次。
 * ================================================================ */
bool MPU6050_ReadRaw (MPU6050_RawData* raw)
{
    uint8_t buf[14];

    /* 从 ACCEL_XOUT_H (0x3B) 开始连续读 14 字节 */
    if (!XJ_I2C_ReadReg(MPU6050_ADDR, REG_ACCEL_XOUT_H, buf, 14))
    {
        return false;
    }

    /* 解析：每两个字节拼成 16 位有符号整数（MSB 在前） */
    raw->ax = Combine16(buf[0], buf[1]);   /* 加速度 X */
    raw->ay = Combine16(buf[2], buf[3]);   /* 加速度 Y */
    raw->az = Combine16(buf[4], buf[5]);   /* 加速度 Z */
    raw->temp = Combine16(buf[6], buf[7]); /* 温度 */
    raw->gx = Combine16(buf[8], buf[9]);   /* 角速度 X */
    raw->gy = Combine16(buf[10], buf[11]); /* 角速度 Y */
    raw->gz = Combine16(buf[12], buf[13]); /* 角速度 Z */

    return true;
}

/* ================================================================
 * 读取并转换为物理单位
 *
 * 转换公式：
 *   加速度 (g)     = ADC / ACCEL_LSB[量程]
 *   角速度 (°/s)   = ADC / GYRO_LSB[量程]
 *   温度 (°C)      = ADC / 340.0 + 36.53
 *
 * 静止时理想输出：
 *   Ax≈0, Ay≈0, Az≈+1.0g（重力方向）
 *   Gx≈0, Gy≈0, Gz≈0（无旋转）
 *   Temp≈25°C（室温）
 * ================================================================ */
bool MPU6050_Read (MPU6050_Data* data)
{
    MPU6050_RawData raw;
    if (!MPU6050_ReadRaw(&raw))
        return false;

    float gyro_lsb = GYRO_LSB[g_gyro_range];
    float accel_lsb = ACCEL_LSB[g_accel_range];

    data->ax = raw.ax / accel_lsb - g_ax_bias;
    data->ay = raw.ay / accel_lsb - g_ay_bias;
    data->az = raw.az / accel_lsb - g_az_bias;
    data->gx = raw.gx / gyro_lsb - g_gx_bias;
    data->gy = raw.gy / gyro_lsb - g_gy_bias;
    data->gz = raw.gz / gyro_lsb - g_gz_bias;
    data->temp = raw.temp / 340.0f + 36.53f;

    return true;
}

/* ── 便捷读取加速度 ── */
bool MPU6050_ReadAccel (float* ax, float* ay, float* az)
{
    MPU6050_Data d;
    if (!MPU6050_Read(&d))
        return false;
    *ax = d.ax;
    *ay = d.ay;
    *az = d.az;
    return true;
}

/* ── 便捷读取角速度 ── */
bool MPU6050_ReadGyro (float* gx, float* gy, float* gz)
{
    MPU6050_Data d;
    if (!MPU6050_Read(&d))
        return false;
    *gx = d.gx;
    *gy = d.gy;
    *gz = d.gz;
    return true;
}

/* ── 便捷读取温度 ── */
bool MPU6050_ReadTemp (float* temp)
{
    MPU6050_Data d;
    if (!MPU6050_Read(&d))
        return false;
    *temp = d.temp;
    return true;
}

/*
 * 打印带符号浮点数（整数拆分法，免 %f 依赖 malloc）
 * width=整数最小宽度, frac=小数位数
 */
static void PrintFloat (float val, int width, int frac)
{
    /* 符号 */
    int neg = (val < 0.0f);
    if (neg)
        val = -val;
    printf("%c", neg ? '-' : '+');

    /* 整数部分 */
    int32_t ip = (int32_t)val;
    printf("%*d.", width, (int)ip);

    /* 小数部分 */
    uint32_t mul = 1;
    for (int i = 0; i < frac; i++)
        mul *= 10;
    int32_t fp = (int32_t)((val - (float)ip) * mul + 0.5f);
    if (fp >= (int32_t)mul)
        fp = (int32_t)(mul - 1);
    printf("%0*d", frac, (int)fp);
}

/*
 * ── 陀螺仪零偏校准 ──
 * 上电后静置采样 200 次求均值，自动从后续读数中扣除
 */
void MPU6050_Calibrate (void)
{
    float sax = 0, say = 0, saz = 0;
    float sgx = 0, sgy = 0, sgz = 0;
    const int N = 200;

    printf("MPU6050: calibrating (%d samples, keep still)...\n", N);
    for (int i = 0; i < N; i++)
    {
        MPU6050_RawData r; /* 读原始值，避免递归扣减旧偏置 */
        MPU6050_ReadRaw(&r);
        float la = ACCEL_LSB[g_accel_range];
        float lg = GYRO_LSB[g_gyro_range];
        sax += r.ax / la; /* Ax（静止应≈0） */
        say += r.ay / la; /* Ay（静止应≈0） */
        saz += r.az / la; /* Az（静止应≈0） */
        sgx += r.gx / lg; /* Gx（静止应≈0） */
        sgy += r.gy / lg; /* Gy（静止应≈0） */
        sgz += r.gz / lg; /* Gz（静止应≈0） */
        delay_ms(5);
    }
    g_ax_bias = sax / N;
    g_ay_bias = say / N;
    g_az_bias = saz / N;
    g_gx_bias = sgx / N;
    g_gy_bias = sgy / N;
    g_gz_bias = sgz / N;
    printf("MPU6050: gyro bias=[%+.1f,%+.1f,%+.1f]d/s  accel bias=[%+.2f,%+.2f,%+.2f]g\n", g_gx_bias, g_gy_bias, g_gz_bias, g_ax_bias, g_ay_bias, g_az_bias);
}

void MPU6050_GetBias (float* gx_b, float* gy_b, float* gz_b,
                      float* ax_b, float* ay_b, float* az_b)
{
    *gx_b = g_gx_bias; *gy_b = g_gy_bias; *gz_b = g_gz_bias;
    *ax_b = g_ax_bias; *ay_b = g_ay_bias; *az_b = g_az_bias;
}

/* MPU6050_Read 里自动减去零偏 */
void MPU6050_Print (void)
{
    MPU6050_Data d;
    if (!MPU6050_Read(&d))
    {
        printf("MPU6050: read error\n");
        return;
    }
    printf("ACC[");
    PrintFloat(d.ax, 0, 2);
    PrintFloat(d.ay, 0, 2);
    PrintFloat(d.az, 0, 2);
    printf("] GYRO[");
    PrintFloat(d.gx, 0, 1);
    PrintFloat(d.gy, 0, 1);
    PrintFloat(d.gz, 0, 1);
    printf("] T[");
    PrintFloat(d.temp, 0, 1);
    printf("]C\n");
}
