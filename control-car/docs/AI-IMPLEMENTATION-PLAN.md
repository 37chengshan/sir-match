# AI 实施计划：MSPM0G3507 循迹小车训练平台

> 面向对象：Claude、Codex、ChatGPT、Cursor、其他代码代理。  
> 核心目标：把 `control-car/` 做成电赛控制类题目的可复用训练平台，而不是一次性 demo。  
> 当前优先级：先打通 MSPM0G3507 + 电机 + 编码器 + 循迹 + PID + 串口/OLED 调参。

---

## 0. AI 先读规则

AI 在处理本文件夹任务时，必须按以下顺序阅读：

1. 仓库根目录 `README.md`：确认仓库结构原则。
2. 根目录 `AGENTS.md` / `CLAUDE.md`：确认 agent 文件放置规范。
3. `control-car/README.md`：确认本题目定位。
4. 本文件：执行具体任务。
5. `docs/purchase-and-build-plan.md`：确认硬件器材和阶段。

不要把本题目文件放入 `problems/`、`agent/`、旧 `docs/` 或根目录零散文件。所有小车专用内容都放在 `control-car/` 内。

---

## 1. 项目结论

本项目要解决的问题不是“再买很多开发板”，而是围绕 TI MSPM0G3507，把电赛控制类高频能力提前做成可复用模块：

```text
电源稳定
→ 电机驱动
→ 编码器测速
→ 循迹传感器读取
→ 速度 PID
→ 转向 PID
→ OLED/串口调参
→ 模块化代码库
→ 可迁移到云台、视觉、避障、通信等题目
```

AI 的实现目标是输出一个结构清晰、可渐进实现、可测试、可复用、适合电赛现场快速调试的 MSPM0G3507 小车工程骨架。

---

## 2. 当前硬件假设

### 2.1 主控

- 主控：TI MSPM0G3507，优先使用立创·天猛星 MSPM0G3507 开发板。
- 可选官方板：LP-MSPM0G3507 LaunchPad。
- 开发环境：CCS Theia + MSPM0 SDK + SysConfig。
- 下载调试：按 TI/立创 MSPM0 官方方式，不使用 ST-LINK。

### 2.2 小车结构

推荐结构：

```text
两轮差速 + 1 个万向轮
```

原因：

- 控制模型简单；
- 适合循迹；
- 适合编码器测速；
- 适合速度 PID 和转向 PID；
- 机械误差相对容易补偿；
- 后续可扩展云台/视觉。

### 2.3 电机

推荐：

- N20 减速电机，带 AB 相编码器；
- 左右轮各 1 个；
- 优先买相同减速比、相同额定电压、相同轮径的成对电机。

AI 代码中不要假设电机完全一致，必须预留左右轮校准系数。

### 2.4 电机驱动

推荐：TB6612FNG。

抽象接口：

```c
motor_set_pwm(left_pwm, right_pwm);
motor_set_dir(left_dir, right_dir);
motor_stop();
motor_brake();
```

底层可根据 PCB 实际引脚映射到：

- AIN1/AIN2/PWMA
- BIN1/BIN2/PWMB
- STBY

### 2.5 循迹传感器

推荐优先级：

1. 8 路灰度传感器阵列；
2. 5 路红外/灰度循迹模块；
3. 单独灰度模块组合。

代码必须支持两种读法：

- 数字量读取：黑/白判断；
- 模拟量读取：ADC 读取灰度值。

优先实现数字量循迹，后续扩展模拟量加权循迹。

### 2.6 显示与调参

推荐组合：

- OLED：I2C，显示模式、速度、偏差、PID 参数、电池电压、错误码；
- UART：串口打印与调参；
- 按键：启动/停止、模式切换、圈数设定、参数切换。

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
│   │   ├── motor_tb6612.c
│   │   ├── motor_tb6612.h
│   │   ├── encoder.c
│   │   ├── encoder.h
│   │   ├── line_sensor.c
│   │   ├── line_sensor.h
│   │   ├── oled_i2c.c
│   │   ├── oled_i2c.h
│   │   ├── button.c
│   │   ├── button.h
│   │   ├── buzzer.c
│   │   ├── buzzer.h
│   │   ├── battery_adc.c
│   │   └── battery_adc.h
│   ├── control/
│   │   ├── pid.c
│   │   ├── pid.h
│   │   ├── speed_control.c
│   │   ├── speed_control.h
│   │   ├── line_control.c
│   │   └── line_control.h
│   ├── protocol/
│   │   ├── uart_protocol.c
│   │   └── uart_protocol.h
│   └── utils/
│       ├── ring_buffer.c
│       ├── ring_buffer.h
│       ├── lowpass_filter.c
│       └── lowpass_filter.h
├── config/
│   ├── pinmap-default.md
│   ├── pid-defaults.md
│   └── serial-protocol.md
└── tests/
    ├── motor-test.md
    ├── encoder-test.md
    ├── line-sensor-test.md
    └── pid-tuning-log.md
```

如果暂时没有真实 CCS 工程，先提交文档和接口骨架，不要伪造不可编译的大段代码。

---

## 4. 固件分层设计

### 4.1 分层原则

```text
main.c
  ↓
app 层：状态机、模式切换、任务调度
  ↓
control 层：PID、速度环、循迹环
  ↓
drivers 层：电机、编码器、循迹、OLED、按键、蜂鸣器、ADC
  ↓
board 层：引脚、时钟、SysConfig 生成内容适配
```

AI 写代码时必须避免：

- 把所有逻辑塞进 `main.c`；
- 把 PID 和电机 GPIO 控制混在一起；
- 把硬件引脚硬编码在多个文件里；
- 无注释魔法数；
- 没有状态机，靠 delay 死等。

### 4.2 任务周期建议

建议使用 1ms 系统 tick，再分频调度：

| 任务 | 周期 | 内容 |
|---|---:|---|
| motor PWM 输出 | 硬件 PWM | 由定时器输出 |
| encoder update | 5ms / 10ms | 计算左右轮速度 |
| line sensor update | 5ms / 10ms | 读取循迹偏差 |
| speed PID | 10ms | 左右轮速度闭环 |
| line PID | 10ms | 根据循迹偏差计算转向量 |
| OLED refresh | 100ms / 200ms | 刷新状态 |
| UART parse | 1ms / loop | 解析串口命令 |
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
| ERROR | 低电压、传感器异常、电机异常等 |

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

必须有：

- 输出限幅；
- 积分限幅；
- PID reset；
- dt 参数；
- 串口动态改参数能力。

### 6.2 速度环

输入：编码器计算出的左右轮速度。  
输出：左右轮 PWM。

```text
left_target_speed  -> left_speed_pid  -> left_pwm
right_target_speed -> right_speed_pid -> right_pwm
```

需要预留：

- 左右电机方向修正；
- 左右电机 PWM 死区补偿；
- 左右轮速度标定系数；
- 最大 PWM 限制；
- 起步 PWM。

### 6.3 转向环 / 循迹环

输入：循迹偏差 `line_error`。  
输出：转向修正量 `turn_correction`。

推荐第一版公式：

```text
base_speed = 固定基础速度
turn = line_pid(line_error)
left_target_speed  = base_speed - turn
right_target_speed = base_speed + turn
```

后续扩展：

- 急弯降速；
- 丢线处理；
- 十字线/弯道识别；
- 圈数统计；
- 起点识别；
- 曲率前馈。

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

若没有检测到黑线：

- 使用上一次偏差方向继续转向；
- 限速；
- 进入 `line_lost_count` 计数；
- 超过阈值进入 ERROR 或 SEARCH 状态。

### 7.2 模拟量版本

若使用 ADC 灰度值：

1. 每路做白底/黑线标定；
2. 转换为归一化黑线强度；
3. 用加权平均得到偏差；
4. 用低通滤波减少抖动。

标定数据必须存入文档或后续 NVM：

```text
sensor_i_white
sensor_i_black
threshold_i
```

---

## 8. 编码器设计

### 8.1 基本目标

必须完成：

1. 判断左右编码器方向；
2. 统计单位时间脉冲数；
3. 转换为轮速；
4. 显示/串口输出左右轮速度；
5. 用速度反馈做 PID。

### 8.2 参数

代码中必须集中定义：

```c
#define ENCODER_PPR          具体值待实测
#define MOTOR_GEAR_RATIO     具体值待实测
#define WHEEL_DIAMETER_MM    具体值待实测
#define CONTROL_PERIOD_MS    10
```

AI 不要随意编造电机参数。未确认时用 `TODO_MEASURE` 注释。

### 8.3 速度换算

基本思路：

```text
pulses_per_period → pulses_per_second → wheel_rps → mm_per_second
```

如果电机编码器是 AB 相正交编码器，需确认是单边沿计数、双边沿计数还是四倍频计数。

---

## 9. UART 调参协议

### 9.1 设计原则

- 人可手打；
- 便于串口助手使用；
- 每条命令一行；
- 参数错误要返回错误提示；
- 所有关键状态可查询。

### 9.2 推荐命令

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
pid show
pid reset

sensor show
encoder show
battery show
log on
log off
```

### 9.3 返回格式

建议返回：

```text
OK: message
ERR: reason
DATA: key=value,key=value
```

示例：

```text
DATA: state=FOLLOW_PID,base=220,line_error=-3.2,left_speed=198,right_speed=242,batt=7.41
```

---

## 10. OLED 页面设计

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

OLED 不要高频刷新，建议 100~200ms 一次。

---

## 11. 电源与抗干扰要求

AI 在写硬件说明或接线图时必须强调：

1. 电机电源和逻辑电源要分清；
2. MSPM0 不要直接给电机供电；
3. 舵机/电机不要直接从开发板 5V 引脚取大电流；
4. 电机驱动附近加大电解电容；
5. 逻辑 GND 与电机 GND 必须共地；
6. 电池电压要监测；
7. 线束要固定，避免跑车时接触不良；
8. 小车控制和瞄准模块后续需要独立开关与电源指示。

---

## 12. 分阶段任务拆解

### 阶段 1：最小硬件闭环

目标：让 MSPM0G3507 能控制电机并读回基础传感器。

任务：

1. 建 `firmware/mspm0/README.md`；
2. 建 pin map 文档；
3. 明确 TB6612 引脚；
4. 实现/规划 PWM 输出；
5. 实现左右电机正反转；
6. 实现电机 stop/brake；
7. 串口输出启动日志；
8. OLED 显示 BOOT/IDLE。

验收：

- 左轮可正转/反转/停止；
- 右轮可正转/反转/停止；
- PWM 从低到高可明显调速；
- 串口能看到状态；
- 小车不会一上电乱跑。

### 阶段 2：编码器测速

任务：

1. 确认编码器供电电压；
2. 确认左右编码器 A/B 相接线；
3. 读取脉冲；
4. 判断方向；
5. 计算速度；
6. OLED/串口显示速度；
7. 记录不同 PWM 下速度表。

验收：

- 手转轮子，计数方向正确；
- 左右轮速度数值稳定；
- PWM 增大时速度增大；
- 能测出电机死区。

### 阶段 3：循迹传感器

任务：

1. 读取 5 路/8 路传感器；
2. 输出二进制传感器状态；
3. 做黑白阈值标定；
4. 计算 line_error；
5. 处理丢线；
6. 实现 open-loop 循迹。

验收：

- 黑线左偏时 error 为负；
- 黑线右偏时 error 为正；
- 中间压线时 error 接近 0；
- 基础循迹能慢速跑完整圈。

### 阶段 4：速度 PID

任务：

1. 实现通用 PID；
2. 左右轮各一个速度 PID；
3. 串口可调 kp/ki/kd；
4. 记录阶跃响应；
5. 加输出限幅和积分限幅。

验收：

- 目标速度改变后能平稳跟踪；
- 左右轮同目标速度时直线偏航明显减少；
- 电池电压变化时速度仍相对稳定。

### 阶段 5：循迹 PID

任务：

1. line_error 输入转向 PID；
2. 输出左右轮目标速度差；
3. 急弯降速；
4. 丢线恢复；
5. 串口调参；
6. OLED 显示当前偏差和 PID 参数。

验收：

- 小车能稳定沿黑线跑；
- 不频繁左右振荡；
- 弯道不冲出线；
- 可通过串口快速调参数。

### 阶段 6：工程化沉淀

任务：

1. 写接线图；
2. 写故障排查表；
3. 写调参记录模板；
4. 写测试数据 CSV 格式；
5. 写模块接口文档；
6. 准备报告素材。

验收：

- 换一个同学也能按文档复现接线；
- 出问题能根据故障表定位；
- PID 参数可追溯；
- 代码模块可迁移到后续题目。

---

## 13. 未来扩展：云台/视觉/瞄准

在小车底盘稳定之前，不要优先实现视觉云台。

扩展顺序：

1. 二自由度舵机云台；
2. PCA9685 舵机驱动；
3. OpenMV 矩形识别/色块识别；
4. UART 传输目标偏差；
5. 云台 PID；
6. 小车行驶状态与云台补偿耦合；
7. 靶面标定与数据拟合。

安全要求：

- 光源/激光相关实验必须经过老师或实验室安全确认；
- 调试阶段优先使用普通 LED 或低风险可见光替代；
- 不要把光源照向人眼、皮肤或反光表面。

---

## 14. 文档与数据要求

### 14.1 必须产生的文档

```text
hardware/pinmap.md
hardware/wiring-guide.md
hardware/power-design.md
firmware/mspm0/config/serial-protocol.md
firmware/mspm0/config/pid-defaults.md
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
10. 是否能被另一个同学复现？

---

## 16. 不要做什么

AI 不要：

- 不确认引脚就写死板级代码；
- 用 STM32 HAL 思路硬套 MSPM0；
- 写不可编译的“看起来很完整”的工程；
- 一开始上树莓派、K230、OpenCV 大系统；
- 推荐 L298N 作为主方案；
- 推荐无编码器电机作为 PID 训练主方案；
- 把光源/激光作为早期必做项目；
- 忽略电源、电机干扰、接线固定；
- 忽略调参入口；
- 忽略测试数据记录。

---

## 17. 最小可交付版本 MVP

第一版最小可交付物：

```text
1. control-car/firmware/mspm0/README.md
2. control-car/hardware/pinmap.md
3. control-car/hardware/wiring-guide.md
4. control-car/firmware/mspm0/config/serial-protocol.md
5. control-car/firmware/mspm0/config/pid-defaults.md
6. 一套 MSPM0G3507 工程骨架或伪代码级接口设计
7. OLED/串口调试说明
8. 电机、编码器、循迹、PID 的分阶段验收表
```

MVP 验收句：

> 小车可以在 MSPM0G3507 控制下完成左右电机调速、编码器测速、循迹传感器读数、基础循迹和串口/OLED 状态显示，并具备继续加入速度 PID、转向 PID 的代码结构。

---

## 18. 给下一位 AI 的直接执行提示

如果你是接手执行的 AI，请按下面步骤开始：

1. 不要改根目录结构。
2. 在 `control-car/firmware/mspm0/` 创建固件说明和配置文档。
3. 在 `control-car/hardware/` 创建引脚表、接线教程、电源设计说明。
4. 先写文档和接口，不要乱猜实际 PCB 引脚。
5. 等用户上传 PCB 原理图/焊接图后，再填写真实 pin map。
6. 若要写代码，先写通用 PID、状态机、串口协议、驱动接口骨架。
7. 每次提交都保证能解释“这个文件解决了哪个电赛训练问题”。
