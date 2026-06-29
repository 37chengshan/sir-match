# 前期准备路线：地猛星 MSPM0G3507 循迹小车 + K230 视觉

> 参考案例：`D:\work\参考文件-已实现案例\2026_04_地猛星电赛控制题配套资料\`  
> 器材清单：`比赛文档/电赛备赛器材清单.md`  
> 视觉主线：`control-car/docs/k230-vision-plan.md`

## 0. 先读这些

```text
control-car/hardware/pinmap.md
control-car/hardware/wiring-guide.md
control-car/hardware/power-design.md
control-car/hardware/bring-up-checklist.md
control-car/firmware/mspm0/config/pid-defaults.md
control-car/docs/AI-IMPLEMENTATION-PLAN.md
control-car/docs/k230-vision-plan.md
```

参考案例代码：

```text
D:\work\参考文件-已实现案例\2026_04_地猛星电赛控制题配套资料\
  ├── 【小车】01_DC_MOTOR_PID_1_弄好了基本的旋转控制/   ← TB6612 基础旋转
  ├── 【小车】02_DC_MOTOR_PID_2_可以测速了/             ← 编码器测速
  ├── 【小车】02_DC_MOTOR_PID_3_已完成一路电机PID闭环控制/ ← 速度 PID
  ├── 【小车】03_PID_car_灰度模块小车巡线/               ← 灰度循迹
  ├── 【小车】04_MPU6050_DMP读取角度/                   ← MPU6050
  └── 【小车】06_读取串口陀螺仪数据并显示在OLED屏幕上/    ← 陀螺仪

D:\work\参考文件-已实现案例\2025_10_k230运行yolov12\
  └── K230 端侧 YOLO/kmodel 推理参考
```

## 1. 阶段 0：环境搭建（1-2 天）

目标：MSPM0 能下载、能串口、能显示。

器材：地猛星开发板、XDS110 仿真器、OLED、USB 转串口。

任务：

1. 安装 CCS Theia（已提供 `01_CCS_20.5.0.00028_win.zip`）
2. 安装 MSPM0 SDK
3. 阅读地猛星引脚图和原理图
4. 跑通 LED（参考 `01_LED_completed`）
5. 跑通 UART（参考 `06_uart_已完成`）
6. 跑通 I2C OLED（参考 `05_OLED_completed`）
7. 跑通按键 + ADC（参考 `08_KEY_ADC_已完成`）
8. 跑通 PWM 舵机（参考 `09_PWM_SERVO`）

验收：

- XDS110 能下载程序；
- 串口能输出日志；
- OLED 能显示模式；
- 按键能触发状态切换。

## 2. 阶段 1：小车最小闭环（核心）

目标：两轮差速闭环循迹。

器材：两轮差速小车、TB6612 模块、5 路/8 路灰度、锂电池。

对应参考案例：01→02→03。

任务：

1. 接 TB6612，实现双电机正反转和 PWM 调速
2. 接编码器，实现双路测速
3. 实现增量式 PI 速度闭环（kp=0.5, ki=0.4 起步）
4. 接灰度传感器，实现数字量读取
5. 实现循迹差速转向（加权偏差 + 转向 PID）
6. OLED 显示模式/速度/偏差

验收：

- 左右轮方向正确；
- 编码器计数方向正确；
- PWM 增大速度增大；
- 小车能低速沿黑线跑完整圈；
- 丢线时能降速或停车，不乱冲。

## 3. 阶段 2：工程化

目标：可调参、可记录、可复现。

任务：

1. 实现 UART 调参协议（改 PID、切换模式、查询状态）
2. OLED 多页状态显示
3. 实现状态机（BOOT→IDLE→MOTOR_TEST→ENCODER_TEST→LINE_FOLLOW_PID→ERROR）
4. 记录 PWM-速度对应表
5. 记录 PID 阶跃响应
6. 编写故障排查文档
7. 预留 K230 视觉输入接口，但不让视觉影响阶段 1 的基础闭环

验收：

- 换参数不需要重新烧录；
- 出问题能通过串口/OLED 定位；
- 小车底层在不接 K230 时也能稳定运行。

## 4. 阶段 3：K230 视觉接入

### 4.1 视觉方案选型结论

当前主线统一为 **庐山派 K230 / CanMV K230**。

| 方案 | 当前定位 | 用途 |
|---|---|---|
| 庐山派 K230 / CanMV K230 | **主线视觉协处理器** | 视觉循迹、色块、二维码、AprilTag、靶面识别、YOLO/kmodel。 |
| OpenMV H7 Plus | 旧备选/对照参考 | 不再作为本仓库小车视觉主线。 |
| PC + USB 摄像头 | 算法预研 | 可用于调阈值、录数据，但不作为比赛现场主线。 |
| OV2640 裸摄像头 | 底层学习/备用零件 | 不适合直接接 STM32F103C8T6 或作为当前主线。 |

策略：

```text
灰度循迹先跑稳
→ K230 单板启动和摄像头预览
→ K230 输出视觉结果到串口
→ MSPM0 解析视觉帧
→ 视觉辅助循迹/靶面/云台
```

### 4.2 K230 必备准备

| 项目 | 要求 |
|---|---|
| TF 卡 | 8GB/16GB，Class 10/U1 及以上；最低持续写入约 10MB/s。 |
| 固件 | 庐山派 K230 对应 CanMV MicroPython 镜像。 |
| 供电 | 独立 5V 稳压，建议 2A 以上。 |
| 通信 | UART 115200 8N1，K230 TX→MSPM0 RX，K230 RX→MSPM0 TX，GND 共地。 |
| 结构 | 摄像头支架固定，视角可重复。 |

### 4.3 K230 基础视觉任务

优先练不用训练的任务：

1. 摄像头预览；
2. 黑线/灰度阈值分割；
3. 色块识别；
4. 二维码识别；
5. AprilTag 识别；
6. 坐标与偏差计算；
7. UART 输出结构化结果。

通信帧统一使用：

```text
VISION,MODE=LINE,OK=1,CX=154,CY=102,ERR=-6,CONF=83
VISION,MODE=BLOB,OK=1,CX=121,CY=88,W=34,H=29,ID=RED,CONF=90
VISION,MODE=TAG,OK=1,ID=3,CX=118,CY=92,ROT=-12,CONF=95
VISION,MODE=NONE,OK=0,ERR=LOST,CONF=0
```

### 4.4 K230 与 MSPM0 数据流

```text
K230（固定在车头/云台）
  │  识别目标中心、线偏差、二维码/AprilTag ID
  │
  ├─ UART TX → MSPM0 UART RX
  │              │
  │              ├─ 解析 VISION 帧
  │              ├─ OLED 显示 MODE/OK/ERR/CX/CY
  │              ├─ 小车循迹 + 速度控制
  │              └─ 云台 PID / 步进电机对准
  │
  └─ USB → PC（仅调试模式，CanMV IDE 观察画面）
```

### 4.5 视觉接入验收

- K230 单独能预览摄像头；
- K230 能稳定输出 `VISION,...`；
- MSPM0 能解析并显示视觉状态；
- 拔掉 K230 或视觉丢失时，MSPM0 能超时降速/停车；
- 视觉不影响基础灰度循迹闭环。

## 5. 阶段 4：云台控制（详细方案）

> 引脚文档：`control-car/hardware/gimbal-pinmap.md`  
> 参考案例：`【云台】02_step_motor` → `【云台】03_step_motor` → `【云台】08_按键遥控`

### 5.1 DCC-100v3 开环步进控制

| 参数 | 值 |
|------|---|
| 单脉冲角度 | 0.05625°（1.8°/32微步） |
| 整圈脉冲 | 6400 |
| 速度控制 | 定时器脉冲频率（40MHz / period = Hz） |
| 角度控制 | 脉冲数计数，中断自减到零停定时器 |

**步进电机 2 引脚**（已验证）：

```text
PA12 = PWM（脉冲）, PA13 = DIR（方向）
PA14 = DCY（衰减）, PA15 = SLP（睡眠）, PA16 = RST（复位）
```

**核心接口**（来自 step_motor.c）：

```c
step_motor_init();
step_motor_dir_set(direction, stepper_id);
step_set_speed(angle_speed, stepper_id);
step_motor_set_angle(angle, stepper_id);
```

### 5.2 K230 视觉坐标控制云台

数据流：

```text
K230 输出目标中心 (cx, cy)
→ MSPM0 计算 err_x = cx - image_center_x, err_y = cy - image_center_y
→ 云台 PID / 步进电机角度修正
→ OLED 显示目标偏差与云台状态
```

任务：

1. DCC-100v3 单路步进电机基本旋转；
2. 角速度控制 + 角度定位；
3. 双轴云台按键遥控；
4. K230 坐标 → MSPM0 串口解析；
5. 云台根据视觉偏差微调角度；
6. 视觉丢失时云台停止或回中。

## 6. 其他扩展

- MPU6050 姿态检测（参考 `04_MPU6050_DMP读取角度`）
- 串口陀螺仪数据读取（参考 `06_读取串口陀螺仪数据`）
- K230 YOLO/kmodel 深度学习识别（复杂多分类场景启用）

## 7. 当前已有代码

```text
control-car/firmware/mspm0/src/control/pid.c/h        # 平台无关的 PID 算法
control-car/firmware/mspm0/src/control/line_follow.c/h # 循迹算法骨架
```

## 8. 关键提醒

- 主控统一使用 **TI MSPM0G3507**，不使用 ST-LINK。
- 下载调试使用 **XDS110**。
- 电机驱动使用 **TB6612FNG**，不推荐 L298N。
- 第一目标是稳定闭环，不是炫技。
- 小车底层不稳定时，不要上视觉/云台。
- K230 需要 TF 卡才能完整练视觉，推荐 8GB/16GB Class 10/U1。
- K230 要独立稳定 5V 供电，所有模块必须共地。
- K230 不做运动控制，只发视觉结果；MSPM0 做实时控制。
- 任何光源/激光相关测试必须经过老师或实验室安全确认。
