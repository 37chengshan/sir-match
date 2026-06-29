# AI 实施计划：MSPM0G3507 循迹小车训练平台

> 面向对象：Claude、Codex、ChatGPT、Cursor、其他代码代理。  
> 核心目标：把 `control-car/` 做成电赛控制类题目的可复用训练平台，而不是一次性 demo。  
> 当前优先级：先打通 MSPM0G3507 + 电机 + 编码器 + 灰度循迹 + PID + 串口/OLED 调参；视觉主线统一为 **庐山派 K230 / CanMV K230**，但必须在底层闭环稳定后接入。

---

## 0. AI 先读规则

AI 在处理本文件夹任务时，必须按以下顺序阅读：

1. 仓库根目录 `README.md`：确认仓库结构原则。
2. 根目录 `AGENTS.md` / `CLAUDE.md`：确认 agent 文件放置规范。
3. `control-car/README.md`：确认本题目定位。
4. 本文件：执行具体任务。
5. `docs/purchase-and-build-plan.md`：确认硬件器材和阶段。
6. `docs/k230-vision-plan.md`：确认 K230 视觉协处理器方案。

不要把本题目文件放入 `problems/`、`agent/`、旧 `docs/` 或根目录零散文件。所有小车专用内容都放在 `control-car/` 内。

---

## 1. 项目结论

本项目要解决的问题不是“再买很多开发板”，而是围绕 TI MSPM0G3507，把电赛控制类高频能力提前做成可复用模块：

```text
电源稳定
→ 电机驱动
→ 编码器测速
→ 灰度循迹传感器读取
→ 速度 PID
→ 转向 PID
→ OLED/串口调参
→ 模块化代码库
→ K230 视觉协处理
→ 云台/靶面/识别扩展
```

AI 的实现目标是输出一个结构清晰、可渐进实现、可测试、可复用、适合电赛现场快速调试的 MSPM0G3507 小车工程骨架。

关键分工：

```text
MSPM0G3507：实时控制、电机、编码器、PID、状态机、安全保护
庐山派 K230：视觉识别、目标坐标、视觉偏差、类别/二维码/AprilTag 结果
UART：K230 → MSPM0 的低耦合通信边界
```

---

## 2. 当前硬件（已验证，来自参考案例）

> 小车全部 MSPM0 引脚来自 `D:\work\参考文件-已实现案例\2026_04_地猛星电赛控制题配套资料\【小车】03_PID_car_灰度模块小车巡线\11_PID_car\Debug\ti_msp_dl_config.h`（SysConfig 生成）。  
> 不要使用其他来源的 MSPM0 引脚号。

### 2.1 主控

- 主控：地猛星 MSPM0G3507 开发板（小板，立创商城）
- 开发环境：CCS Theia + MSPM0 SDK + SysConfig
- 下载调试：XDS110 仿真器，不使用 ST-LINK
- 引脚图/原理图已在参考案例目录提供

### 2.2 小车结构

两轮差速 + 万向轮。编码器参数：260 线，减速比 1:20，轮径 67mm。

### 2.3 TB6612 电机驱动（真实引脚）

| 功能 | MSPM0 引脚 | TB6612 引脚 |
|------|-----------|------------|
| STBY（使能） | **PB24** | STBY |
| 左 PWM | **PA12** (TIMG0 CCP0) | PWMA |
| 左 AIN1 | **PA9** | AIN1 |
| 左 AIN2 | **PA8** | AIN2 |
| 右 PWM | **PA13** (TIMG0 CCP1) | PWMB |
| 右 BIN1 | **PA7** | BIN1 |
| 右 BIN2 | **PB18** | BIN2 |

PWM 频率 10kHz（40MHz/4000），最大占空比 4000。

方向控制：正转=(AIN1=1, AIN2=0)，反转=(AIN1=0, AIN2=1)，停止=(0,0)。

### 2.4 编码器（真实引脚）

| 功能 | MSPM0 引脚 |
|------|-----------|
| 左 A 相 | **PA21** |
| 左 B 相 | **PA22** |
| 右 A 相 | **PB19** |
| 右 B 相 | **PB20** |

速度计算公式：

```c
speed_mm_s = counter / 260.0f * 3.14f * 67.0f * 20.0f;
```

counter 在每个控制周期清零。

### 2.5 循迹传感器（真实引脚）

5 路灰度，数字量读取：

| 传感器 | MSPM0 引脚 |
|--------|-----------|
| L2（最左） | **PA17** |
| L1 | **PB8** |
| M（中间） | **PB9** |
| R1 | **PA24** |
| R2（最右） | **PA2** |

### 2.6 控制定时器

TIMA0，40MHz / 40000 = **1kHz** PID 控制频率（LOAD_VALUE=39999）。

### 2.7 显示与调参

- OLED：I2C（SSD1306/SH1106）
- UART：115200 8N1
- 按键：START + MODE

### 2.8 K230 视觉协处理器

| 项目 | 要求 |
|---|---|
| 模块 | 立创·庐山派 K230 / CanMV K230 |
| 角色 | 视觉协处理器，不做小车主控 |
| 存储 | TF 卡 8GB/16GB，Class 10/U1 及以上 |
| 供电 | 独立 5V 稳压，建议 2A 以上 |
| 通信 | UART 115200 8N1，TX/RX/GND，必须共地 |
| 输出 | `VISION,...` 文本帧 |

---

## 3. 代码目录规划

未来固件建议放在：

```text
control-car/firmware/mspm0/
├── README.md
├── project/                         # CCS Theia/SysConfig 工程目录，后续真实工程放这里
├── src/
│   ├── main.c
│   ├── app/
│   │   ├── app_main.c
│   │   ├── app_main.h
│   │   ├── app_config.h
│   │   ├── app_state.c
│   │   └── app_state.h
│   ├── board/
│   │   ├── board_pins.h
│   │   ├── board_clock.c
│   │   └── board_clock.h
│   ├── drivers/
│   │   ├── motor_tb6612.c/h
│   │   ├── encoder.c/h
│   │   ├── line_sensor.c/h
│   │   ├── oled_i2c.c/h
│   │   ├── button.c/h
│   │   ├── buzzer.c/h
│   │   └── battery_adc.c/h
│   ├── control/
│   │   ├── pid.c/h
│   │   ├── speed_control.c/h
│   │   └── line_control.c/h
│   ├── protocol/
│   │   ├── uart_protocol.c/h
│   │   └── vision_protocol.c/h       # K230 VISION 帧解析
│   └── utils/
│       ├── ring_buffer.c/h
│       └── lowpass_filter.c/h
├── config/
│   ├── pinmap-default.md
│   ├── pid-defaults.md
│   ├── serial-protocol.md
│   └── vision-uart-protocol.md
└── tests/
    ├── motor-test.md
    ├── encoder-test.md
    ├── line-sensor-test.md
    ├── pid-tuning-log.md
    └── k230-uart-test.md
```

如果暂时没有真实 CCS 工程，先提交文档和接口骨架，不要伪造不可编译的大段代码。

---

## 4. 固件分层设计

```text
main.c
  ↓
app 层：状态机、模式切换、任务调度
  ↓
control 层：PID、速度环、循迹环、视觉辅助控制
  ↓
protocol 层：UART 命令、K230 VISION 帧解析
  ↓
drivers 层：电机、编码器、循迹、OLED、按键、蜂鸣器、ADC
  ↓
board 层：引脚、时钟、SysConfig 生成内容适配
```

AI 写代码时必须避免：

- 把所有逻辑塞进 `main.c`；
- 把 PID 和电机 GPIO 控制混在一起；
- 把硬件引脚硬编码在多个文件里；
- K230 数据解析和运动控制强耦合；
- 无注释魔法数；
- 没有状态机，靠 delay 死等。

### 4.1 任务周期建议

| 任务 | 周期 | 内容 |
|---|---:|---|
| motor PWM 输出 | 硬件 PWM | 由定时器输出 |
| encoder update | 5ms / 10ms | 计算左右轮速度 |
| line sensor update | 5ms / 10ms | 读取循迹偏差 |
| speed PID | 10ms | 左右轮速度闭环 |
| line PID | 10ms | 根据循迹偏差计算转向量 |
| vision frame parse | loop / 1ms | 解析 K230 UART 接收缓冲 |
| vision timeout check | 10ms / 20ms | 超时降级、报警或停车 |
| OLED refresh | 100ms / 200ms | 刷新状态 |
| UART command parse | 1ms / loop | 解析调参命令 |
| key scan | 10ms | 按键消抖 |
| battery adc | 100ms / 500ms | 电池电压检测 |

---

## 5. 状态机设计

必须实现状态机，建议枚举：

```c
typedef enum {
    CAR_STATE_BOOT = 0,
    CAR_STATE_IDLE,
    CAR_STATE_MOTOR_TEST,
    CAR_STATE_ENCODER_TEST,
    CAR_STATE_LINE_SENSOR_TEST,
    CAR_STATE_LINE_FOLLOW_OPEN_LOOP,
    CAR_STATE_SPEED_PID_TEST,
    CAR_STATE_LINE_FOLLOW_PID,
    CAR_STATE_K230_UART_TEST,
    CAR_STATE_VISION_ASSIST,
    CAR_STATE_GIMBAL_TEST,
    CAR_STATE_ERROR,
} car_state_t;
```

状态说明：

| 状态 | 用途 |
|---|---|
| BOOT | 初始化硬件、显示版本信息 |
| IDLE | 等待按键/串口命令 |
| MOTOR_TEST | 左右电机正反转、PWM 死区测试 |
| ENCODER_TEST | 查看编码器方向、脉冲数、速度 |
| LINE_SENSOR_TEST | 查看各路循迹传感器黑白阈值 |
| LINE_FOLLOW_OPEN_LOOP | 不用 PID 的基础循迹 |
| SPEED_PID_TEST | 速度闭环单独测试 |
| LINE_FOLLOW_PID | 速度环 + 转向环联动循迹 |
| K230_UART_TEST | 只验证 K230 串口帧收发，不控制电机 |
| VISION_ASSIST | K230 视觉辅助循迹或目标定位 |
| GIMBAL_TEST | 云台电机/舵机单独测试 |
| ERROR | 低电压、传感器异常、电机异常、视觉超时等 |

AI 实现时所有模式切换必须可由 UART 命令或按键触发。

---

## 6. PID 设计要求

### 6.1 通用 PID 结构体

```c
typedef struct {
    float kp;
    float ki;
    float kd;
    float setpoint;
    float integral;
    float prev_error;
    float output_min;
    float output_max;
    float integral_min;
    float integral_max;
} pid_t;
```

接口：

```c
void pid_init(pid_t *pid, float kp, float ki, float kd,
              float output_min, float output_max);
void pid_reset(pid_t *pid);
float pid_update(pid_t *pid, float measured, float dt);
void pid_set_params(pid_t *pid, float kp, float ki, float kd);
void pid_set_limit(pid_t *pid, float out_min, float out_max,
                   float i_min, float i_max);
```

必须有：输出限幅、积分限幅、PID reset、dt 参数、串口动态改参数能力。

### 6.2 速度环

输入：编码器计算出的左右轮速度。输出：左右轮 PWM。

```text
left_target_speed  -> left_speed_pid  -> left_pwm
right_target_speed -> right_speed_pid -> right_pwm
```

需要预留：左右电机方向修正、PWM 死区补偿、速度标定系数、最大 PWM 限制、起步 PWM。

### 6.3 转向环 / 循迹环

输入：灰度循迹偏差 `line_error` 或视觉偏差 `vision_error`。输出：转向修正量 `turn_correction`。

第一版只用灰度：

```text
base_speed = 固定基础速度
turn = line_pid(line_error)
left_target_speed  = base_speed - turn
right_target_speed = base_speed + turn
```

K230 接入后，视觉只能作为增强输入，不得破坏灰度基础闭环：

```text
if vision_ok && vision_conf >= threshold:
    fused_error = line_error * line_weight + vision_error * vision_weight
else:
    fused_error = line_error
```

---

## 7. 循迹算法设计

### 7.1 数字量版本

若 8 路传感器从左到右为 S0~S7，权重建议：

```text
S0 S1 S2 S3 S4 S5 S6 S7
-7 -5 -3 -1 +1 +3 +5 +7
```

计算：

```text
line_error = sum(active_i * weight_i) / active_count
```

若没有检测到黑线：使用上一次偏差方向继续低速搜索，超过阈值进入 ERROR 或 SEARCH 状态。

### 7.2 模拟量版本

若使用 ADC 灰度值：每路做白底/黑线标定，转换为归一化黑线强度，用加权平均得到偏差，再低通滤波。

---

## 8. 编码器设计

必须完成：

1. 判断左右编码器方向；
2. 统计单位时间脉冲数；
3. 转换为轮速；
4. 显示/串口输出左右轮速度；
5. 用速度反馈做 PID。

代码中必须集中定义：

```c
#define ENCODER_PPR          260
#define MOTOR_GEAR_RATIO     20.0f
#define WHEEL_DIAMETER_MM    67.0f
#define CONTROL_PERIOD_MS    10
```

如果实物参数不同，必须实测后更新，不要凭空编造。

---

## 9. UART 调参协议

### 9.1 人机调参命令

```text
help
status
mode idle
mode motor_test
mode encoder_test
mode line_test
mode follow_open
mode speed_pid
mode follow_pid
mode k230_uart_test
mode vision_assist

motor left 300
motor right 300
motor both 300
motor stop

speed base 200
speed left 200
speed right 200

pid speed_l 1.0 0.0 0.0
pid speed_r 1.0 0.0 0.0
pid line 1.0 0.0 0.0
pid vision 1.0 0.0 0.0
pid show
pid reset

sensor show
encoder show
vision show
battery show
log on
log off
```

### 9.2 返回格式

```text
OK: message
ERR: reason
DATA: key=value,key=value
```

示例：

```text
DATA: state=FOLLOW_PID,base=220,line_error=-3.2,left_speed=198,right_speed=242,batt=7.41
DATA: vision_mode=LINE,vision_ok=1,cx=154,cy=102,err=-6,conf=83,age_ms=24
```

---

## 10. K230 视觉协议

K230 到 MSPM0 使用“一行一帧”的 ASCII 协议，便于串口助手调试。

```text
VISION,MODE=LINE,OK=1,CX=154,CY=102,ERR=-6,CONF=83
VISION,MODE=BLOB,OK=1,CX=121,CY=88,W=34,H=29,ID=RED,CONF=90
VISION,MODE=TAG,OK=1,ID=3,CX=118,CY=92,ROT=-12,CONF=95
VISION,MODE=QR,OK=1,DATA=START_A,CONF=99
VISION,MODE=NONE,OK=0,ERR=LOST,CONF=0
```

MSPM0 侧最小结构体建议：

```c
typedef struct {
    uint8_t ok;
    char mode[12];
    int16_t cx;
    int16_t cy;
    int16_t err;
    int16_t w;
    int16_t h;
    int16_t id;
    int16_t conf;
    uint32_t last_update_ms;
} vision_result_t;
```

安全要求：

- `last_update_ms` 超时 100~300ms 必须降级；
- `OK=0` 不允许继续用旧坐标高速控制；
- `CONF` 过低不参与闭环；
- K230 丢帧不能导致 MSPM0 卡死。

---

## 11. OLED 页面设计

第一页：运行状态

```text
MODE: FOLLOW_PID
BATT: 7.41V
ERR : -2.5
SPD : L198 R242
```

第二页：PID 参数

```text
LINE KP 1.20
LINE KI 0.00
LINE KD 0.15
BASE 220
```

第三页：传感器

```text
S: 00111100
E: -1.0
L: 198
R: 242
```

第四页：K230 视觉

```text
VIS: LINE OK
CX:154 CY:102
ERR:-6 CF:83
AGE:24ms
```

OLED 不要高频刷新，建议 100~200ms 一次。

---

## 12. 电源与抗干扰要求

AI 在写硬件说明或接线图时必须强调：

1. 电机电源和逻辑电源要分清；
2. MSPM0 不要直接给电机供电；
3. K230 建议独立 5V/2A 以上供电；
4. 舵机/电机不要直接从开发板 5V 引脚取大电流；
5. 电机驱动附近加大电解电容；
6. 逻辑 GND、K230 GND 与电机 GND 必须共地；
7. 电池电压要监测；
8. 线束要固定，避免跑车时接触不良；
9. TF 卡运行中不要插拔。

---

## 13. 分阶段任务拆解

### 阶段 1：最小硬件闭环

目标：MSPM0G3507 能控制电机并读回基础传感器。

验收：左右轮可正反转/停止，PWM 可调速，串口/OLED 有状态，小车不会一上电乱跑。

### 阶段 2：编码器测速

验收：手转轮子计数方向正确，PWM 增大速度增大，能测出电机死区。

### 阶段 3：灰度循迹传感器

验收：黑线左偏 error 为负，右偏为正，中间接近 0，基础循迹能慢速跑完整圈。

### 阶段 4：速度 PID

验收：目标速度改变后能平稳跟踪，左右轮同目标速度时直线偏航减少。

### 阶段 5：循迹 PID

验收：小车稳定沿黑线跑，不频繁左右振荡，弯道不冲出线，可串口快速调参。

### 阶段 6：工程化沉淀

验收：换一个同学也能按文档复现接线，PID 参数可追溯，代码模块可迁移。

### 阶段 7：K230 单板练习

任务：TF 卡烧录、CanMV 启动、摄像头预览、基础视觉例程。

验收：K230 不接 MSPM0 时也能稳定输出视觉结果。

### 阶段 8：K230 UART 接入 MSPM0

任务：K230 输出 `VISION,...`，MSPM0 解析并 OLED 显示。

验收：视觉丢失或断开 K230 时，MSPM0 能超时报警/降级/停车。

### 阶段 9：视觉辅助循迹/云台

任务：K230 输出偏差或坐标，MSPM0 低速验证视觉辅助控制。

验收：视觉只增强系统，不破坏灰度基础闭环。

---

## 14. 文档与数据要求

### 14.1 必须产生的文档

```text
hardware/pinmap.md
hardware/wiring-guide.md
hardware/power-design.md
firmware/mspm0/config/serial-protocol.md
firmware/mspm0/config/pid-defaults.md
firmware/mspm0/config/vision-uart-protocol.md
docs/k230-vision-plan.md
test-data/pid-tuning-log.md
report/technical-notes.md
```

### 14.2 测试数据 CSV 建议

`test-data/motor_pwm_speed.csv`

```csv
time_ms,left_pwm,right_pwm,left_speed_mm_s,right_speed_mm_s,battery_v,note
0,100,100,0,0,7.80,start
```

`test-data/line_sensor_calibration.csv`

```csv
sensor_id,white_value,black_value,threshold,note
S0,920,180,550,leftmost
```

`test-data/pid_step_response.csv`

```csv
time_ms,target_speed,left_speed,right_speed,left_pwm,right_pwm,kp,ki,kd
0,200,0,0,0,0,1.0,0.0,0.0
```

`test-data/k230_vision_log.csv`

```csv
time_ms,mode,ok,cx,cy,err,conf,age_ms,note
0,LINE,1,154,102,-6,83,24,start
```

---

## 15. AI 输出质量要求

AI 每次修改后必须自查：

1. 是否符合仓库结构？
2. 是否把题目专用内容放进 `control-car/`？
3. 是否避免把所有代码塞进 `main.c`？
4. 是否有状态机？
5. 是否有清晰接口？
6. 是否预留实际硬件参数待测？
7. 是否没有胡编真实引脚？
8. 是否没有建议使用 ST-LINK 烧 MSPM0？
9. 是否强调电源安全与共地？
10. 是否明确 K230 只做视觉协处理器？
11. 是否能被另一个同学复现？

---

## 16. 不要做什么

AI 不要：

- 不确认引脚就写死板级代码；
- 用 STM32 HAL 思路硬套 MSPM0；
- 写不可编译的“看起来很完整”的工程；
- 一开始就依赖 K230 让小车动起来；
- 让 K230 替代 MSPM0 控制电机/PID；
- 推荐 L298N 作为主方案；
- 推荐无编码器电机作为 PID 训练主方案；
- 把光源/激光作为早期必做项目；
- 忽略电源、电机干扰、接线固定；
- 忽略调参入口；
- 忽略测试数据记录；
- 把 OpenMV 写成当前主线。

---

## 17. 最小可交付版本 MVP

第一版最小可交付物：

```text
1. control-car/firmware/mspm0/README.md
2. control-car/hardware/pinmap.md
3. control-car/hardware/wiring-guide.md
4. control-car/firmware/mspm0/config/serial-protocol.md
5. control-car/firmware/mspm0/config/pid-defaults.md
6. control-car/firmware/mspm0/config/vision-uart-protocol.md
7. control-car/docs/k230-vision-plan.md
8. 一套 MSPM0G3507 工程骨架或伪代码级接口设计
9. OLED/串口调试说明
10. 电机、编码器、循迹、PID、K230 UART 的分阶段验收表
```

MVP 验收句：

> 小车可以在 MSPM0G3507 控制下完成左右电机调速、编码器测速、循迹传感器读数、基础循迹和串口/OLED 状态显示；K230 作为视觉协处理器可以通过 UART 输出结构化视觉结果，MSPM0 能解析、显示并在安全超时机制下使用该结果。

---

## 18. 给下一位 AI 的直接执行提示

如果你是接手执行的 AI，请按下面步骤开始：

1. 不要改根目录结构。
2. 在 `control-car/firmware/mspm0/` 创建固件说明和配置文档。
3. 在 `control-car/hardware/` 创建引脚表、接线教程、电源设计说明。
4. 先写文档和接口，不要乱猜实际 PCB 引脚。
5. 等用户上传 PCB 原理图/焊接图后，再填写真实 pin map。
6. 若要写代码，先写通用 PID、状态机、串口协议、K230 VISION 帧解析、驱动接口骨架。
7. 每次提交都保证能解释“这个文件解决了哪个电赛训练问题”。
