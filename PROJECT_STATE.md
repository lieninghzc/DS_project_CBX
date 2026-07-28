# DS 项目当前状态 (2026-07-28)

## 一、硬件平台
- MCU: MSPM0G3507 @ 80MHz (40MHz 外部晶振 + PLL)
- 调试: J-Link + Ozone + RTT
- IDE: VSCode + CMake + Ninja + arm-none-eabi-gcc 15.2.1

## 二、项目结构
```
DS/
├── main.c                  → 硬件初始化 + SM_Run() 循环
├── StateMachine.c/h        → 定时发送传感器数据 + LED + 通信轮询
├── Engine.c/h              → 双电机控制（前馈+PID 闭环 + 陀螺旋转）
├── Motor.c/h               → PWM + 方向控制（A 电机反相在 Motor_SetDir）
├── Encoder.c/h             → 编码器脉冲计数 → RPM → MPS（100ms保底归零）
├── PID.c/h                 → 位置式 PID
├── Position.c/h            → 陀螺积分 + 编码器里程计（0.5°/s死区）
├── Gyroscope.c/h           → MPU6050/MPU6500 I2C 驱动 + 零偏校准
├── xj.c/h                  → I2C 通用驱动 + 循迹模块专用接口
├── MaiXPro.c/h             → MaixCAM UART 通信（PING/PONG 握手）
├── BlueTooth.c/h           → HC-05 蓝牙 UART 通信
├── OLED.c/h + OLED_Font.h  → SSD1306 8x16 字体驱动
├── Delay.c/h               → SysTick 精确延迟
├── syscalls.c              → newlib → RTT 重定向
├── sysconfig/MSPM0G3507.syscfg  → TI SysConfig 外设配置
└── cmake/ + CMakeLists.txt + CMakePresets.json
```

## 三、关键发现 & 坑

### 3.1 SWD 断开问题
**根因**: `DL_GPIO_reset(GPIOA)` 杀死 PA19/PA20 的 SWD 功能
**解决**: 自定义 `SYSCFG_DL_init` (weak override), 跳过 GPIOA/B reset

### 3.2 PWM duty=0 满速
**根因**: CCR=0 时 PWM 匹配逻辑异常
**解决**: Motor_SetDuty 钳至最小 1, Motor_Init 初始值改为 1

### 3.3 编码器不分正反
**根因**: 编码器 A 相只能计脉冲数，不分方向
**影响**: PID 速度控制时轮子反向→encoder 读数仍为正→PID 失控
**解决**: Engine_Rotate 方向硬设 + 速度 PID 锁正速, 不用负 duty 表方向

### 3.4 A 电机反相
**位置**: Motor.c - Motor_SetDir 中对 MOTOR_A 做 FWD↔REV 交换
**原因**: 物理接线反了，软件层统一

### 3.5 PID 被 encoder 超时重置
**旧逻辑**: Encoder_IsRunning=false → PID_Reset → 每2秒清积分→扭矩归零
**新逻辑**: Engine_Update 始终调 PID_Compute, 不清积分

### 3.6 printf %f 需要 heap
要求: `_sbrk` 提供堆 (syscalls.c 2KB) + `-Wl,-u,_printf_float`

### 3.7 K_FF 实测
| duty | speed (m/s) | K_FF=duty/speed |
|------|------------|-----------------|
| 100  | 0.14       | 714 (低速低效)   |
| 200  | 0.38       | 526             |
| 300  | 0.63       | 476             |
| 400  | 0.81       | 494             |
| 500  | 0.95       | 526             |
**最终**: K_FF = 500

## 四、PID 参数 (Engine.h)
| 参数 | 值 | 说明 |
|------|-----|------|
| K_FF | 500 | 前馈系数 |
| KP   | 50  | 比例增益 |
| KI   | 400 | 积分增益（扭矩补偿） |
| KD   | 0   | 微分关闭 |
| dt   | 20ms| 控制周期 |
| out  | 0~999 | 输出范围 |

## 五、Engine_Rotate 旋转逻辑
1. 速度 PID 锁轮速 (默认 0.3 m/s)
2. 方向硬设: CCW=A:REV/B:FWD, CW=A:FWD/B:REV
3. 陀螺积分 (Position_Update) 跟踪 yaw
4. ±10° 刹车, 等 500ms 惯性
5. 过冲自动反向纠正 (±2° 死区)

## 六、传感器引脚速查
| 功能 | 引脚 |
|------|------|
| LED1/2 | PA2, PA7 |
| I2C0 SDA/SCL | PA10, PA11 |
| MPU6050 | I2C 0x68 |
| 循迹模块 | I2C 0x12 |
| OLED | I2C 0x3C |
| UART0 (蓝牙) | PA0(TX), PA1(RX) @ 38400 |
| UART1 (相机) | PA8(TX), PA9(RX) @ 9600 |
| PWM A/B | PB26, PB27 (TIMG6) |
| A方向 AIN1/2 | PA4, PA3 |
| B方向 BIN1/2 | PB21, PB22 |
| 编码器A | TIMG0 CCP0 |
| 编码器B | TIMG12 CCP0 |
| SWD | PA19(SWDIO), PA20(SWCLK) |
| 晶振 | PA5(HFXIN), PA6(HFXOUT) 40MHz |

## 七、常用命令
```
# 生成 SysConfig
./gen_sysconfig.bat

# 编译
cmake --build build/MSPM0G3507-Debug

# 清理重编
rm -rf build/MSPM0G3507-Debug && cmake --preset MSPM0G3507-Debug -B build/MSPM0G3507-Debug && cmake --build build/MSPM0G3507-Debug
```
