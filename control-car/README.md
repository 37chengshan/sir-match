# control-car：地猛星 MSPM0G3507 循迹小车训练平台

> 定位：基于地猛星 MSPM0G3507 + TB6612 的小车控制积木平台。  
> 目标：沉淀可复用的电源、电机、编码器、循迹、PID、通信、显示、K230 视觉协处理模块。  
> 视觉主线：立创·庐山派 K230 / CanMV K230。

## 1. 硬件方案

| 模块 | 型号 | 说明 |
|------|------|------|
| 主控 | 地猛星 MSPM0G3507 | 小板，适合小车集成；负责电机、编码器、PID、状态机。 |
| 电机驱动 | TB6612FNG | 双路直流电机驱动。 |
| 电机 | N20 减速电机 ×2 | 带 AB 相霍尔编码器，260线，减速比 1:20。 |
| 轮胎 | 直径 67mm | 两轮差速 + 万向轮。 |
| 循迹 | 5 路灰度传感器，后续可换 8 路灰度阵列 | 基础循迹，优先跑稳。 |
| 视觉 | 庐山派 K230 / CanMV K230 | 视觉协处理器；识别黑线、色块、二维码、AprilTag、目标。 |
| K230 存储 | TF 卡 8GB/16GB Class 10/U1 | K230 启动、脚本、模型文件。 |
| 显示 | I2C OLED | SSD1306 / SH1106。 |
| 供电 | 2S 锂电池 7.4V | TB6612 VM 直连电池，K230 单独 5V/2A 以上稳压，逻辑电经电源模块。 |
| 调试 | XDS110 + UART | MSPM0 使用 XDS110；MSPM0↔K230 使用 UART 115200 8N1。 |

> 不推荐 L298N，不推荐无编码器电机，不使用 ST-LINK 烧 MSPM0。K230 不作为小车主控，只输出视觉结果。

## 2. 推荐阅读顺序

AI 优先读：

1. `hardware/pinmap.md` — 真实引脚映射（代码必须用这些引脚）
2. `firmware/mspm0/config/pid-defaults.md` — 起步 PID 参数
3. `docs/AI-IMPLEMENTATION-PLAN.md` — 完整实现方案和验收标准
4. `docs/k230-vision-plan.md` — K230 视觉协处理器方案
5. `hardware/wiring-guide.md` — 接线步骤
6. `docs/preparation-roadmap.md` — 分阶段训练路线
7. `docs/purchase-and-build-plan.md` — 采购与搭建计划

人先看：

1. `web/index.html` — 项目展示页
2. `hardware/wiring-guide.md` — 接线教程
3. `docs/preparation-roadmap.md` — 训练路线
4. `docs/k230-vision-plan.md` — K230 练手和接入小车方案

## 3. 参考案例

```text
D:\work\参考文件-已实现案例\2026_04_地猛星电赛控制题配套资料\
  ├── 【小车】01_DC_MOTOR_PID_1/   TB6612 基本旋转
  ├── 【小车】02_DC_MOTOR_PID_2/   编码器测速
  ├── 【小车】02_DC_MOTOR_PID_3/   速度 PID 闭环
  └── 【小车】03_PID_car/           灰度循迹（最完整）

D:\work\参考文件-已实现案例\2025_10_k230运行yolov12\
  └── K230 端侧 YOLO/kmodel 推理参考
```

## 4. 文件夹结构

```text
control-car/
├── README.md
├── docs/
│   ├── AI-IMPLEMENTATION-PLAN.md
│   ├── k230-vision-plan.md
│   ├── preparation-roadmap.md
│   └── purchase-and-build-plan.md
├── firmware/
│   └── mspm0/
│       ├── README.md
│       ├── src/control/       # pid.c/h, line_follow.c/h
│       └── config/            # pid-defaults.md, serial-protocol.md, vision-uart-protocol.md
├── hardware/
│   ├── pinmap.md              # 完整引脚表（含 C 宏定义）
│   ├── wiring-guide.md        # 接线教程
│   ├── power-design.md        # 电源方案
│   └── bring-up-checklist.md  # 启动检查表
├── web/
│   └── index.html
├── test-data/
├── report/
└── simulation/
```

## 5. 当前阶段

```text
阶段 0：环境搭建（CCS + XDS110 + 入门教程）          ← 当前
阶段 1：小车最小闭环（TB6612 双电机 + 编码器 + PID + 循迹）
阶段 2：工程化（状态机 + UART 调参 + OLED + CSV 记录）
阶段 3：K230 视觉接入（TF 卡启动 + CanMV + UART 视觉结果）
阶段 4：视觉/云台/靶面扩展（K230 坐标 → MSPM0 云台控制）
```

## 6. 关键原则

- 主控统一使用 **地猛星 MSPM0G3507**。
- 下载调试使用 **XDS110**，不使用 ST-LINK 烧 MSPM0。
- 电机驱动使用 **TB6612FNG**，不推荐 L298N。
- 小车采用 **两轮差速 + 万向轮**。
- 第一目标是稳定闭环，不是炫技。
- 小车底层不稳定时，不堆视觉/云台。
- K230 只做视觉协处理器，MSPM0 做实时运动控制。
- K230 必须独立稳定供电，所有模块 GND 共地。
- K230 训练必须准备 TF 卡；推荐 8GB/16GB Class 10/U1。
- 所有 MSPM0 引脚以 `hardware/pinmap.md` 为唯一来源。
