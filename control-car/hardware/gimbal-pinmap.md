# 云台 PinMap：地猛星 MSPM0G3507 + DCC-100v3 步进电机驱动

> 来源：
> - `参考文件-已实现案例/2026_04_地猛星电赛控制题配套资料/【云台】08_按键上下左右遥控开环电机/yuntai_8_yao2kong4kai1huan2/empty.c`
> - `step_motor.h` 注释中的接线说明
> - `【云台】03_step_motor/15_step_motor_2/Debug/ti_msp_dl_config.h`（SysConfig 生成）

---

## 1. DCC-100v3 步进电机驱动模块

DCC-100v3 是一个双路步进电机驱动模块，每个模块可驱动两个 42 步进电机。

### 1.1 步进电机 1（X 轴 / 水平）

| 功能 | MSPM0 引脚 | DCC-100v3 | 说明 |
|------|-----------|-----------|------|
| PWM（脉冲） | 待确认 | PUL1 | 定时器 CCP 输出，脉冲频率=角速度 |
| DIR（方向） | 待确认 | DIR1 | 高/低电平控制旋转方向 |
| DCY（衰减） | 待确认 | DCY1 | 衰减模式选择 |
| SLP（睡眠） | 待确认 | SLP1 | 低功耗睡眠 |
| RST（复位） | 待确认 | RST1 | 驱动器复位 |

### 1.2 步进电机 2（Y 轴 / 俯仰）

| 功能 | MSPM0 引脚 | DCC-100v3 | 说明 |
|------|-----------|-----------|------|
| PWM（脉冲） | **PA12** | PUL2 | DCC_100_PWM2_INST |
| DIR（方向） | **PA13** | DIR2 | 高电平=正向，低电平=反向 |
| DCY（衰减） | **PA14** | DCY2 | 衰减模式 |
| SLP（睡眠） | **PA15** | SLP2 | 高电平=正常工作 |
| RST（复位） | **PA16** | RST2 | 高电平=正常工作 |

> 电机 2 的引脚在 `step_motor.h` 注释中明确标注。
> 电机 1 的引脚需查看 `【云台】08` 的 `ti_msp_dl_config.h` 确认，注释中标注为 PA6-PA10 范围。

---

## 2. 步进电机参数

| 参数 | 值 | 说明 |
|------|---|------|
| 步距角 | 1.8° | 42 步进电机标准值 |
| 微步细分 | 32 | DCC-100v3 默认 32 微步 |
| 单脉冲角度 | **0.05625°** | 1.8° / 32 = 0.05625° |
| 整圈脉冲数 | **6400** | 360° / 0.05625° |
| 工作电压 | 12-24V | DCC-100v3 供电 |

## 3. 控制算法（来自参考案例 step_motor.c）

### 3.1 角速度 → 脉冲频率

```c
// 角速度 angle_speed (°/s) → 脉冲频率 (Hz)
frequency = (uint32_t)(angle_speed / 0.05625);

// 频率 → 定时器装载值（假设 TIMER_CLK = 40MHz）
period = TIMER_CLK / frequency;
// 限制范围 800~65535，对应约 610Hz~50kHz
```

例：30°/s 角速度 → 533.33 Hz → period ≈ 75000（超 65535 则限定为 65535）

### 3.2 角度 → 脉冲数

```c
// 目标角度 → 脉冲数
step_count = (uint32_t)(angle / 0.05625);
```

例：90° → 1600 个脉冲

### 3.3 定时器中断驱动

```c
void PWM_INST_IRQHandler(void) {
    if (DL_TIMER_IIDX_LOAD) {
        if (step_remain == 0) {
            step_motor_stop(id);  // 脉冲发完，停定时器
            return;
        }
        step_remain--;  // 每进一次中断消耗一个脉冲
    }
}
```

---

## 4. DCC-101v1 闭环步进电机（UART 协议）

DCC-101v1 自带驱动板和编码器，通过 UART 协议控制：

### 4.1 帧格式

```
AA 55 [ADDR] [CMD] [LEN] [DATA...] [CHECKSUM]
```

示例：旋转到 90° 位置

| 字节 | 值 | 说明 |
|------|---|------|
| 0-1 | AA 55 | 帧头 |
| 2 | 01 | 设备地址 |
| 3 | 11 | 指令编号 |
| 4 | 05 | 数据长度 |
| 5 | 01 | 方向（0=反转, 1=正转） |
| 6-7 | 00 00 | 数据1-2 |
| 8-9 | — | 角度步数（低八位+高八位） |
| 10 | — | 校验和（字节2-9求和取低8位） |

### 4.2 步数换算

```c
// 16384 = 一整圈的编码器计数 * 微步细分
angle_steps = 16384 / 4;  // 90° = 4096 步
```

---

## 5. 多维按键板引脚（云台遥控）

| 按键 | MSPM0 引脚 | 用途 |
|------|-----------|------|
| 10号 | **PB3** | 激光笔开关（长按 300 周期） |
| 1号 | **PB6** | 步进电机 1 方向 1 |
| 2号 | **PB7** | 步进电机 1 方向 2 |
| 3号 | **PA26** | 步进电机 2 方向 1 |
| 4号 | **PA25** | 步进电机 2 方向 2 |
| 公共端 | 3.3V | 按键板供电 |

---

## 6. 激光笔接线

| 激光笔 | 连接 |
|--------|------|
| + | 5V |
| - | GND |
| S（控制） | **PA17**（GPIO 输出，高电平点亮） |

> 注意激光安全，不要直射人眼或镜面反射物。

---

## 7. C 语言宏定义汇总

```c
// === DCC-100v3 步进电机 2（单路参考） ===
#define STEP_PWM2_PORT      GPIOA
#define STEP_PWM2_PIN       DL_GPIO_PIN_12   // PA12, 脉冲信号
#define STEP_DIR2_PORT      GPIOA
#define STEP_DIR2_PIN       DL_GPIO_PIN_13   // PA13, 方向控制
#define STEP_DCY2_PORT      GPIOA
#define STEP_DCY2_PIN       DL_GPIO_PIN_14   // PA14, 衰减模式
#define STEP_SLP2_PORT      GPIOA
#define STEP_SLP2_PIN       DL_GPIO_PIN_15   // PA15, 睡眠控制
#define STEP_RST2_PORT      GPIOA
#define STEP_RST2_PIN       DL_GPIO_PIN_16   // PA16, 复位

// === 步进电机参数 ===
#define STEP_DEG_PER_PULSE  0.05625f         // 单脉冲角度
#define STEP_PULSE_PER_REV  6400             // 整圈脉冲数 (360/0.05625)
#define STEP_MICROSTEP      32               // 微步细分

// === 激光笔 ===
#define LASER_PORT          GPIOA
#define LASER_PIN           DL_GPIO_PIN_17   // PA17

// === 按键板（多维按键） ===
#define KEY_1_PORT          GPIOB
#define KEY_1_PIN           DL_GPIO_PIN_6    // PB6, 电机1正
#define KEY_2_PORT          GPIOB
#define KEY_2_PIN           DL_GPIO_PIN_7    // PB7, 电机1反
#define KEY_3_PORT          GPIOA
#define KEY_3_PIN           DL_GPIO_PIN_26   // PA26, 电机2正
#define KEY_4_PORT          GPIOA
#define KEY_4_PIN           DL_GPIO_PIN_25   // PA25, 电机2反
#define KEY_10_PORT         GPIOB
#define KEY_10_PIN          DL_GPIO_PIN_3    // PB3, 激光开关
```
