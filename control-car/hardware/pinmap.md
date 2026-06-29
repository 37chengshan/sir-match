# PinMap：地猛星 MSPM0G3507 + TB6612 循迹小车

> 来源：`参考文件-已实现案例/2026_04_地猛星电赛控制题配套资料/【小车】03_PID_car_灰度模块小车巡线/11_PID_car/Debug/ti_msp_dl_config.h`
>
> 本表为 SysConfig 生成的真实网络映射，已在实际工程中验证。写代码前用万用表确认高低电平方向。

---

## 1. TB6612 电机驱动

### 1.1 左电机

| 功能 | MSPM0 引脚 | IOMUX | TB6612 引脚 |
|------|-----------|-------|------------|
| PWM | **PA12** | PINCM34, TIMG0 CCP0 | PWMA |
| 方向1 | **PA9** | PINCM20 | AIN1 |
| 方向2 | **PA8** | PINCM19 | AIN2 |

### 1.2 右电机

| 功能 | MSPM0 引脚 | IOMUX | TB6612 引脚 |
|------|-----------|-------|------------|
| PWM | **PA13** | PINCM35, TIMG0 CCP1 | PWMB |
| 方向1 | **PA7** | PINCM14 | BIN1 |
| 方向2 | **PB18** | PINCM44 | BIN2 |

### 1.3 公共控制

| 功能 | MSPM0 引脚 | IOMUX | TB6612 引脚 |
|------|-----------|-------|------------|
| 待机/使能 | **PB24** | PINCM52 | STBY |

### 1.4 TB6612 方向控制逻辑

| 动作 | AIN1/BIN1 | AIN2/BIN2 | 说明 |
|------|-----------|-----------|------|
| 停止 | 0 | 0 | 滑行 |
| 正转 | 1 | 0 | PWM 在 PWMA/PWMB |
| 反转 | 0 | 1 | PWM 在 PWMA/PWMB |
| 刹车 | 1 | 1 | 不建议默认使用 |

---

## 2. 编码器（AB 相霍尔编码器）

### 2.1 左电机编码器

| 功能 | MSPM0 引脚 | IOMUX | 说明 |
|------|-----------|-------|------|
| A 相 | **PA21** | PINCM46 | GPIO 中断计数 |
| B 相 | **PA22** | PINCM47 | GPIO 中断计数 |

### 2.2 右电机编码器

| 功能 | MSPM0 引脚 | IOMUX | 说明 |
|------|-----------|-------|------|
| A 相 | **PB19** | PINCM45 | GPIO 中断计数 |
| B 相 | **PB20** | PINCM48 | GPIO 中断计数 |

### 2.3 编码器参数

```c
#define ENCODER_PPR         260    // 编码器线数（每圈脉冲数）
#define WHEEL_DIAMETER_MM    67    // 轮胎直径 mm
#define GEAR_RATIO           20    // 减速比 1:20
```

### 2.4 速度换算

```
pulses_per_period → pulses_per_second → wheel_rps → mm_per_second

speed_mm_s = counter / ENCODER_PPR * PI * WHEEL_DIAMETER_MM * (1000 / CONTROL_PERIOD_MS)
```

使用原始参数：
```c
// 1kHz 控制周期下的速度计算
speed_mm_s = counter / 260.0f * 3.14f * 67.0f * 20.0f;
// 计算完后清零计数器
```

---

## 3. 灰度循迹传感器（5 路数字量）

| 传感器位置 | MSPM0 引脚 | IOMUX | 说明 |
|-----------|-----------|-------|------|
| L2（最左） | **PA17** | PINCM39 | GPIO 输入 |
| L1 | **PB8** | PINCM25 | GPIO 输入 |
| M（中间） | **PB9** | PINCM26 | GPIO 输入 |
| R1 | **PA24** | PINCM54 | GPIO 输入 |
| R2（最右） | **PA2** | PINCM7 | GPIO 输入 |

供电：按模块规格接 3.3V 或 5V，GND 共地。

### 3.1 读数方式

```c
// 数字量读法：DL_GPIO_readPins(port, pin) 返回非零为检测到黑线
uint8_t value = (DL_GPIO_readPins(port, pin) & pin) ? 1 : 0;
```

### 3.2 加权偏差算法（8路示例，5路需缩减权重）

```
S0 S1 S2 S3 S4 S5 S6 S7
-7 -5 -3 -1 +1 +3 +5 +7

line_error = sum(active_i * weight_i) / active_count
```

---

## 4. PID 控制定时器

| 定时器 | 时钟源 | 装载值 | 频率 | 用途 |
|--------|--------|--------|------|------|
| **TIMA0** | 40 MHz | 39999 | **1 kHz** | 速度计算 + PID 更新 |

```c
#define CONTROL_FREQ_HZ      1000
#define CONTROL_PERIOD_MS    1
#define TIMER_LOAD_VALUE     39999   // 40MHz / (39999+1) = 1kHz
```

中断服务函数：
```c
void MOTOR_PID_INST_IRQHandler(void)
{
    switch (DL_Timer_getPendingInterrupt(MOTOR_PID_INST))
    {
    case DL_TIMER_IIDX_LOAD:
        calculate_speed(1);
        calculate_speed(2);
        DC_MOTOR_PID(1);
        DC_MOTOR_PID(2);
        break;
    default:
        break;
    }
}
```

---

## 5. OLED 显示

| OLED 引脚 | 连接 |
|-----------|------|
| VCC | 3.3V 或 5V（按模块规格） |
| GND | GND |
| SCL | 地猛星 I2C SCL 或顶部扩展口 |
| SDA | 地猛星 I2C SDA 或顶部扩展口 |

驱动：SSD1306 / SH1106，I2C 地址 0x3C 或 0x3D。

---

## 6. PWM 参数

```c
#define PWM_TIMER           TIMG0
#define PWM_CLK_FREQ        40000000   // 40 MHz
#define PWM_PERIOD          4000       // 40MHz/4000 = 10kHz PWM频率
#define PWM_MAX_DUTY        4000       // 最大占空比
```

---

## 7. C 语言宏定义汇总

```c
// === TB6612 电机驱动 ===
// 左电机
#define MOTOR_L_PWM_PORT    GPIOA
#define MOTOR_L_PWM_PIN     DL_GPIO_PIN_12   // PA12, TIMG0 CCP0
#define MOTOR_L_AIN1_PORT   GPIOA
#define MOTOR_L_AIN1_PIN    DL_GPIO_PIN_9    // PA9
#define MOTOR_L_AIN2_PORT   GPIOA
#define MOTOR_L_AIN2_PIN    DL_GPIO_PIN_8    // PA8

// 右电机
#define MOTOR_R_PWM_PORT    GPIOA
#define MOTOR_R_PWM_PIN     DL_GPIO_PIN_13   // PA13, TIMG0 CCP1
#define MOTOR_R_BIN1_PORT   GPIOA
#define MOTOR_R_BIN1_PIN    DL_GPIO_PIN_7    // PA7
#define MOTOR_R_BIN2_PORT   GPIOB
#define MOTOR_R_BIN2_PIN    DL_GPIO_PIN_18   // PB18

// 公共
#define MOTOR_STBY_PORT     GPIOB
#define MOTOR_STBY_PIN      DL_GPIO_PIN_24   // PB24

// === 编码器 ===
// 左电机
#define ENC_L_A_PORT        GPIOA
#define ENC_L_A_PIN         DL_GPIO_PIN_21   // PA21
#define ENC_L_B_PORT        GPIOA
#define ENC_L_B_PIN         DL_GPIO_PIN_22   // PA22

// 右电机
#define ENC_R_A_PORT        GPIOB
#define ENC_R_A_PIN         DL_GPIO_PIN_19   // PB19
#define ENC_R_B_PORT        GPIOB
#define ENC_R_B_PIN         DL_GPIO_PIN_20   // PB20

// === 灰度传感器 ===
#define LINE_L2_PORT        GPIOA
#define LINE_L2_PIN         DL_GPIO_PIN_17   // PA17
#define LINE_L1_PORT        GPIOB
#define LINE_L1_PIN         DL_GPIO_PIN_8    // PB8
#define LINE_M_PORT         GPIOB
#define LINE_M_PIN          DL_GPIO_PIN_9    // PB9
#define LINE_R1_PORT        GPIOA
#define LINE_R1_PIN         DL_GPIO_PIN_24   // PA24
#define LINE_R2_PORT        GPIOA
#define LINE_R2_PIN         DL_GPIO_PIN_2    // PA2

// === 编码器参数 ===
#define ENCODER_PPR         260
#define WHEEL_DIAMETER_MM   67
#define GEAR_RATIO          20
#define PI                  3.14f

// === 控制参数 ===
#define CONTROL_FREQ_HZ     1000
#define PWM_PERIOD          4000
#define PWM_MAX_DUTY        4000
```
