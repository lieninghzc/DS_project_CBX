#ifndef XJ_H
#define XJ_H

#include <stdint.h>
#include <stdbool.h>

/* ── 循迹模块参数 ── */
#define XJ_I2C_ADDR 0x12   /* I2C 从机地址 */
#define XJ_REG_CALIB 0x01  /* 校准寄存器（写）：1=进入校准, 0=退出 */
#define XJ_REG_SENSOR 0x30 /* 探头数据（读）：bit0~7 对应 8 个探头 */

/* ── 底层 I2C（通用） ── */
void XJ_I2C_Init (void);
void XJ_I2C_Scan (void);
bool XJ_I2C_Probe (uint8_t addr);
bool XJ_I2C_WriteReg (uint8_t addr, uint8_t reg, const uint8_t* data, uint8_t len);
bool XJ_I2C_WriteByte (uint8_t addr, uint8_t reg, uint8_t val);
bool XJ_I2C_ReadReg (uint8_t addr, uint8_t reg, uint8_t* data, uint8_t len);
bool XJ_I2C_ReadByte (uint8_t addr, uint8_t reg, uint8_t* val);

/* ── 循迹专用接口 ── */

/** 进入校准模式 */
void XJ_CalibEnter (void);

/** 退出校准模式 */
void XJ_CalibExit (void);

/** 读取 8 路探头原始值（bit0~bit7，1=黑线上，0=白色地板） */
uint8_t XJ_ReadSensors (void);

/** 检查指定探头是否在黑线上（探头号 0~7） */
bool XJ_SensorOnLine (uint8_t probe);

/** 打印探头状态（8 位二进制 + 格式化的探头位） */
void XJ_PrintSensors (void);

#endif /* XJ_H */
