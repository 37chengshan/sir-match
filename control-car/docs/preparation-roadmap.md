# 前期准备路线：地猛星 MSPM0G3507 循迹小车

> 参考案例：`D:\work\参考文件-已实现案例\2026_04_地猛星电赛控制题配套资料\`
> 器材清单：`比赛文档/电赛备赛器材清单.md`

## 0. 先读这些

```
control-car/hardware/pinmap.md
control-car/hardware/wiring-guide.md
control-car/hardware/power-design.md
control-car/hardware/bring-up-checklist.md
control-car/firmware/mspm0/config/pid-defaults.md
control-car/docs/AI-IMPLEMENTATION-PLAN.md
```

参考案例代码：
```
D:\work\参考文件-已实现案例\2026_04_地猛星电赛控制题配套资料\
  ├── 【小车】01_DC_MOTOR_PID_1_弄好了基本的旋转控制/   ← TB6612 基础旋转
  ├── 【小车】02_DC_MOTOR_PID_2_可以测速了/             ← 编码器测速
  ├── 【小车】02_DC_MOTOR_PID_3_已完成一路电机PID闭环控制/ ← 速度 PID
  ├── 【小车】03_PID_car_灰度模块小车巡线/               ← 灰度循迹
  ├── 【小车】04_MPU6050_DMP读取角度/                   ← MPU6050
  └── 【小车】06_读取串口陀螺仪数据并显示在OLED屏幕上/    ← 陀螺仪
```

## 1. 阶段 0：环境搭建（1-2 天）

目标：能下载、能串口、能显示。

器材：地猛星开发板、XDS110 仿真器、OLED、USB 转串口

任务：
1. 安装 CCS Theia（已提供 `01_CCS_20.5.0.00028_win.zip`）
2. 安装 MSPM0 SDK
3. 阅读地猛星引脚图和原理图
4. 跑通 LED（参考 `01_LED_completed`）
5. 跑通 UART（参考 `06_uart_已完成`）
6. 跑通 I2C OLED（参考 `05_OLED_completed`）
7. 跑通按键 + ADC（参考 `08_KEY_ADC_已完成`）
8. 跑通 PWM 舵机（参考 `09_PWM_SERVO`）

## 2. 阶段 1：小车最小闭环（核心）

目标：两轮差速闭环循迹。

器材：两轮差速小车、TB6612 模块、5 路灰度、锂电池

对应参考案例：01→02→03

任务：
1. 接 TB6612，实现双电机正反转和 PWM 调速
2. 接编码器，实现双路测速
3. 实现增量式 PI 速度闭环（kp=0.5, ki=0.4 起步）
4. 接灰度传感器，实现 5 路数字量读取
5. 实现循迹差速转向（加权偏差 + 转向 PID）
6. OLED 显示模式/速度/偏差

## 3. 阶段 2：工程化

目标：可调参、可记录、可复现。

任务：
1. 实现 UART 调参协议（改 PID、切换模式、查询状态）
2. OLED 多页状态显示
3. 实现状态机（BOOT→IDLE→MOTOR_TEST→ENCODER_TEST→LINE_FOLLOW_PID→ERROR）
4. 记录 PWM-速度对应表
5. 记录 PID 阶跃响应
6. 编写故障排查文档

## 4. 阶段 3：视觉与扩展模块（后期）

### 4.1 视觉方案选型

| | OpenMV H7 Plus（首选） | K230 | PC + USB 摄像头 |
|---|---|---|---|
| 处理器 | STM32H743 480MHz | K230 双核 + NPU | 电脑 GPU/CPU |
| 编程 | MicroPython，内置算法 | Python，需训模型 | Python/C++ OpenCV |
| 调试 | IDE 帧查看器，实时调阈值 | 靠日志和截图 | 功能最强，调试最方便 |
| 改目标 | 改几行代码换颜色 | 重新标注→训练→转换 | 改参数 |
| 对电赛 | **颜色/形状/二维码/AprilTag** | 复杂分类（多类物体） | 不适合现场 |
| 价格 | 约 400-500 元（已购） | 已有资料 | 已有 |
| 连接 MSPM0 | UART 发坐标 | UART 发坐标 | USB 连电脑 |

**策略**：OpenMV 做比赛主力（色块追踪 + AprilTag 定位），USB 摄像头做 PC 端算法调优，K230 留作深度学习备选。

### 4.2 OpenMV 内置算法（不用训练）

| API | 功能 | 电赛场景 |
|-----|------|---------|
| `find_blobs()` | 色块检测 | 红灯/绿灯/蓝灯识别，色块追踪 |
| `find_apriltags()` | AprilTag 定位 | 高精度位姿，云台对准激光点 |
| `find_lines()` | 巡线 | 替代灰度传感器，高级巡线 |
| `find_qrcodes()` | 二维码 | 读任务编号/参数 |
| `find_circles()` | 圆形检测 | 找圆孔/圆形标记 |
| `find_rects()` | 矩形检测 | 找矩形靶面 |

### 4.3 数据流

```
OpenMV H7 Plus（固定在云台/车头）
  │  find_blobs() / find_apriltags()
  │  返回 (cx, cy, w, h, rotation)
  │
  ├─ UART TX → MSPM0 UART RX
  │              │
  │              ├─ 解析 "(x,y)" 坐标
  │              ├─ 云台步进电机 PID 对准目标
  │              └─ 小车循迹 + 速度控制（并行）
  │
  └─ USB → PC（调试模式，OpenMV IDE 帧查看器）
```

### 4.4 任务

1. OpenMV 开发环境搭建（OpenMV IDE + 连接测试）
2. 色块检测 + 阈值调优（用 IDE 帧查看器可视化调参）
3. UART 输出目标坐标，MSPM0 解析验证
4. AprilTag 定位测试
5. 云台 PID 闭环对准（步进电机 + 视觉反馈）

### 4.5 其他扩展

- MPU6050 姿态检测（参考 `04_MPU6050_DMP读取角度`）
- 串口陀螺仪数据读取（参考 `06_读取串口陀螺仪数据`）
- 步进电机云台控制（参考 `14_step_motor_1` → `15_step_motor_2`）
- K230 深度学习识别（参考 `2026_06_电赛视觉资料`，仅复杂多分类场景启用）

## 5. 当前已有代码

```
control-car/firmware/mspm0/src/control/pid.c/h       # 平台无关的 PID 算法
control-car/firmware/mspm0/src/control/line_follow.c/h # 循迹算法骨架
```

## 6. 关键提醒

- 主控统一使用 **TI MSPM0G3507**，不使用 ST-LINK
- 下载调试使用 **XDS110**
- 电机驱动使用 **TB6612FNG**，不推荐 L298N
- 第一目标是稳定闭环，不是炫技
- 小车底层不稳定时，不要上视觉/云台
- 任何光源/激光相关测试必须经过老师或实验室安全确认
