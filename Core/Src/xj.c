/**
 * @file    xj.c
 * @brief   循迹模块 I2C 通信驱动
 *
 * 硬件连接（由 SysConfig 配置）：
 *   I2C0: SCL=PA1, SDA=PA0, 主机模式 100kHz
 *
 * 循迹模块协议：
 *   I2C 地址 0x12（7-bit）
 *   寄存器 0x01（写）：1=进入校准模式, 0=退出校准模式
 *   寄存器 0x30（读）：8 路探头数据, bit0~bit7 各对应一个探头
 *                      1=探头在黑线上, 0=探头在白色地板上
 *                      例：0x8F = 0b10001111
 *                          探头 7,3,2,1,0 在黑线上
 *                          探头 6,5,4 在白色地板上
 */

#include "main.h"
#include "xj.h"

/* ───────────────────────────────────────────────────────────────
 * 超时：轮询 I2C 状态时的最大循环次数
 * 100000 次 ≈ 几毫秒（@ 80MHz CPU, 100kHz I2C）
 * ─────────────────────────────────────────────────────────────── */
#define I2C_TIMEOUT 100000U

/* ───────────────────────────────────────────────────────────────
 * 等待 I2C 控制器回到空闲状态
 * BUSY=0 且 IDLE=1 表示上一次传输已完成
 * ─────────────────────────────────────────────────────────────── */
static bool I2C_WaitIdle (void)
{
    uint32_t t = I2C_TIMEOUT;
    while (--t)
    {
        uint32_t s = DL_I2C_getControllerStatus(I2C_0_INST);
        if (!(s & DL_I2C_CONTROLLER_STATUS_BUSY) && (s & DL_I2C_CONTROLLER_STATUS_IDLE))
        {
            return true; /* 传输完成 */
        }
    }
    return false; /* 超时，可能总线被拉死 */
}

/* ───────────────────────────────────────────────────────────────
 * 强制复位 I2C 控制器传输状态
 * 出错时调用，清除总线挂死
 * ─────────────────────────────────────────────────────────────── */
static void I2C_Abort (void)
{
    DL_I2C_resetControllerTransfer(I2C_0_INST);
}

/* ================================================================
 *                        初始化
 * SYSCFG_DL_init() 已在 main() 里调用，完成了 I2C0 时钟+引脚初始化
 * ================================================================ */
void XJ_I2C_Init (void)
{
}

/* ================================================================
 * 探测设备 —— 只发地址 + 写方向，检查是否有 ACK
 *
 * MSPM0 I2C 控制器流程：
 *   1. startControllerTransfer(addr, TX, length=0) —— 只发地址，不发数据
 *   2. 轮询 getControllerStatus()
 *   3. 检查 ADDR_ACK 位：1=有设备应答, 0=无设备
 *   4. abort() 结束传输
 *
 * @param addr  7-bit I2C 从机地址（0x08 ~ 0x77）
 * @return      true=设备存在, false=无应答/超时
 * ================================================================ */
bool XJ_I2C_Probe (uint8_t addr)
{
    /* 只发地址，长度=0 —— 纯探测，不发送数据 */
    DL_I2C_startControllerTransfer(I2C_0_INST, addr, DL_I2C_CONTROLLER_DIRECTION_TX, 0);

    uint32_t t = 10000;
    while (--t)
    {
        uint32_t s = DL_I2C_getControllerStatus(I2C_0_INST);

        /* ERROR: 总线错误（仲裁丢失、超时等） */
        if (s & DL_I2C_CONTROLLER_STATUS_ERROR)
        {
            I2C_Abort();
            return false;
        }

        /* BUSY=0: 传输结束 */
        if (!(s & DL_I2C_CONTROLLER_STATUS_BUSY))
        {
            /* ADDR_ACK=0: 没有设备应答该地址 */
            if (!(s & DL_I2C_CONTROLLER_STATUS_ADDR_ACK))
            {
                I2C_Abort();
                return false;
            }
            return true; /* 地址被 ACK，设备存在 */
        }
    }
    I2C_Abort();
    return false;
}

/* ================================================================
 * I2C 总线扫描 —— 遍历 0x08~0x77，打印所有在线设备
 * ================================================================ */
void XJ_I2C_Scan (void)
{
    printf("I2C Scan (0x08~0x77):\n");
    uint8_t n = 0;
    for (uint8_t a = 0x08; a <= 0x77; a++)
    {
        if (XJ_I2C_Probe(a))
        {
            printf("  Found: 0x%02X\n", a);
            n++;
        }
    }
    printf("  Total: %d device(s)\n", n);
}

/* ================================================================
 * 写寄存器 —— 向从机写一个或多个字节
 *
 * I2C 协议时序：
 *   START + SLAVE_ADDR(W) + REG + DATA[0..N] + STOP
 *
 * MSPM0 API 流程：
 *   1. fillControllerTXFIFO(buf, len)       —— 填充发送缓冲区
 *   2. startControllerTransfer(addr, TX, len) —— 启动传输
 *   3. 轮询 getControllerStatus()           —— 等待 BUSY=0 & IDLE=1
 *   4. 检查 ADDR_ACK 和 DATA_ACK            —— 确保从机收到了
 *
 * @param addr  从机 7-bit 地址
 * @param reg   寄存器地址
 * @param data  要写入的数据
 * @param len   数据长度
 * @return      true=写入成功, false=失败/超时
 * ================================================================ */
bool XJ_I2C_WriteReg (uint8_t addr, uint8_t reg, const uint8_t* data, uint8_t len)
{
    /* 拼接：寄存器地址 + 数据 */
    uint8_t buf[16];
    if (len + 1 > sizeof(buf))
        return false;
    buf[0] = reg;
    for (uint8_t i = 0; i < len; i++)
        buf[1 + i] = data[i];

    /* 填充 TX FIFO */
    DL_I2C_fillControllerTXFIFO(I2C_0_INST, buf, len + 1);

    /* 启动主机发送：地址 + TX方向 + 字节数 */
    DL_I2C_startControllerTransfer(I2C_0_INST, addr, DL_I2C_CONTROLLER_DIRECTION_TX, len + 1);

    /* 等待传输完成 */
    uint32_t t = I2C_TIMEOUT;
    while (--t)
    {
        uint32_t s = DL_I2C_getControllerStatus(I2C_0_INST);
        if (s & DL_I2C_CONTROLLER_STATUS_ERROR)
        {
            I2C_Abort();
            return false;
        }
        if (!(s & DL_I2C_CONTROLLER_STATUS_BUSY))
        {
            if (s & DL_I2C_CONTROLLER_STATUS_IDLE)
                return true;
        }
    }
    I2C_Abort();
    return false;
}

/* 写单个字节（满足 XJ_I2C_WriteReg 的便捷封装） */
bool XJ_I2C_WriteByte (uint8_t addr, uint8_t reg, uint8_t val)
{
    return XJ_I2C_WriteReg(addr, reg, &val, 1);
}

/* ================================================================
 * 读寄存器 —— 先写寄存器地址，再读数据
 *
 * I2C 协议时序：
 *   START + SLAVE_ADDR(W) + REG + STOP
 *   START + SLAVE_ADDR(R) + DATA[0..N] + STOP
 *
 * MSPM0 API 流程：
 *   第一步：写寄存器地址
 *     1. fillControllerTXFIFO(&reg, 1)       —— 填充寄存器地址
 *     2. startControllerTransfer(addr, TX, 1) —— 发送
 *     3. 等待 BUSY=0 & IDLE=1               —— 传输完成
 *
 *   第二步：读数据
 *     1. startControllerTransfer(addr, RX, len) —— 启动接收
 *     2. 轮询 isControllerRXFIFOEmpty()      —— RX FIFO 有数据
 *     3. receiveControllerData()             —— 读出一个字节
 *     4. 重复 len 次                         —— 读完所有字节
 *
 * @param addr  从机 7-bit 地址
 * @param reg   寄存器地址
 * @param data  接收缓冲区
 * @param len   读取长度
 * @return      true=读取成功, false=失败/超时
 * ================================================================ */
bool XJ_I2C_ReadReg (uint8_t addr, uint8_t reg, uint8_t* data, uint8_t len)
{
    /* ── 第一步：告诉从机要读的寄存器地址 ── */
    DL_I2C_fillControllerTXFIFO(I2C_0_INST, &reg, 1);
    DL_I2C_startControllerTransfer(I2C_0_INST, addr, DL_I2C_CONTROLLER_DIRECTION_TX, 1);

    uint32_t t = I2C_TIMEOUT;
    while (--t)
    {
        uint32_t s = DL_I2C_getControllerStatus(I2C_0_INST);
        if (s & DL_I2C_CONTROLLER_STATUS_ERROR)
        {
            I2C_Abort();
            return false;
        }
        if (!(s & DL_I2C_CONTROLLER_STATUS_BUSY) && (s & DL_I2C_CONTROLLER_STATUS_IDLE))
        {
            break; /* 寄存器地址已发出 */
        }
    }
    if (t == 0)
    {
        I2C_Abort();
        return false;
    }

    /* ── 第二步：读取数据 ── */
    DL_I2C_startControllerTransfer(I2C_0_INST, addr, DL_I2C_CONTROLLER_DIRECTION_RX, len);

    for (uint8_t i = 0; i < len; i++)
    {
        t = I2C_TIMEOUT;
        while (--t)
        {
            /* RX FIFO 不为空 = 有数据可读 */
            if (!DL_I2C_isControllerRXFIFOEmpty(I2C_0_INST))
            {
                data[i] = DL_I2C_receiveControllerData(I2C_0_INST);
                break;
            }
            if (DL_I2C_getControllerStatus(I2C_0_INST) & DL_I2C_CONTROLLER_STATUS_ERROR)
            {
                I2C_Abort();
                return false;
            }
        }
        if (t == 0)
        {
            I2C_Abort();
            return false;
        }
    }

    return I2C_WaitIdle();
}

/* 读单个字节 */
bool XJ_I2C_ReadByte (uint8_t addr, uint8_t reg, uint8_t* val)
{
    return XJ_I2C_ReadReg(addr, reg, val, 1);
}

/* ================================================================
 *                  循迹模块专用接口
 * ================================================================ */

/*
 * 进入校准模式
 * 写 0x01 寄存器 = 1，模块开始采集黑/白色地板阈值
 * 校准时应让小车跨越黑白边界来回移动
 */
void XJ_CalibEnter (void)
{
    XJ_I2C_WriteByte(XJ_I2C_ADDR, XJ_REG_CALIB, 1);
}

/*
 * 退出校准模式
 * 写 0x01 寄存器 = 0，模块结束校准，使用新阈值进行检测
 */
void XJ_CalibExit (void)
{
    XJ_I2C_WriteByte(XJ_I2C_ADDR, XJ_REG_CALIB, 0);
}

/*
 * 读取 8 路探头原始值
 *
 * 返回值每位含义（bit0=探头0, bit7=探头7）：
 *   1 = 该探头检测到黑线
 *   0 = 该探头在白色地板上
 *
 * 典型值：
 *   0x8F = 0b10001111 → 两端和中间偏左探头在线上
 *   0x18 = 0b00011000 → 中间两个探头在线上（居中）
 *   0x00 = 0b00000000 → 完全偏离，所有探头都在白色地板
 */
uint8_t XJ_ReadSensors (void)
{
    uint8_t val = 0;
    XJ_I2C_ReadByte(XJ_I2C_ADDR, XJ_REG_SENSOR, &val);
    return val;
}

/*
 * 检查指定探头是否在黑线上
 * @param probe 探头编号（0~7）
 * @return true=在黑线上, false=在白色地板
 */
bool XJ_SensorOnLine (uint8_t probe)
{
    if (probe > 7)
        return false;
    return (XJ_ReadSensors() >> probe) & 1;
}

/*
 * RTT 打印探头状态
 * 输出格式：
 *   Sensors: 0x8F [00011110]
 *   1 = 黑线, 0 = 白色地板
 *   最左边是探头 7，最右边是探头 0
 */
void XJ_PrintSensors (void)
{
    uint8_t v = XJ_ReadSensors();
    printf("Sensors: 0x%02X [", v);
    for (int i = 7; i >= 0; i--)
    {
        printf("%c", (v >> i) & 1 ? '1' : '0');
    }
    printf("]\n");
}
