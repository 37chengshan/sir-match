# control-car：地猛星 MSPM0G3507 循迹小车训练平台

> 定位：基于地猛星 MSPM0G3507 + TB6612 的小车控制积木平台。
> 目标：沉淀可复用的电源、电机、编码器、循迹、PID、通信、显示模块。

## 1. 硬件方案

| 模块 | 型号 | 说明 |
|------|------|------|
| 主控 | 地猛星 MSPM0G3507 | 小板，适合小车集成 |
| 电机驱动 | TB6612FNG | 双路直流电机驱动 |
| 电机 | N20 减速电机 ×2 | 带 AB 相霍尔编码器，260线，减速比 1:20 |
| 轮胎 | 直径 67mm | 两轮差速 + 万向轮 |
| 循迹 | 5 路灰度传感器 | 数字量读取 |
| 显示 | I2C OLED | SSD1306 / SH1106 |
| 供电 | 2S 锂电池 7.4V | TB6612 VM 直连电池，逻辑电经电源模块 |
| 调试 | XDS110 + UART | 115200 8N1 |

> 不推荐 L298N，不推荐无编码器电机，不使用 ST-LINK。

## 2. 推荐阅读顺序

AI 优先读：
1. `hardware/pinmap.md` — 真实引脚映射（代码必须用这些引脚）
2. `firmware/mspm0/config/pid-defaults.md` — 起步 PID 参数
3. `docs/AI-IMPLEMENTATION-PLAN.md` — 完整实现方案和验收标准
4. `hardware/wiring-guide.md` — 接线步骤
5. `docs/preparation-roadmap.md` — 4 阶段训练路线

人先看：
1. `web/index.html` — 项目展示页
2. `hardware/wiring-guide.md` — 接线教程
3. `docs/preparation-roadmap.md` — 训练路线

## 3. 参考案例

```
D:\work\参考文件-已实现案例\2026_04_地猛星电赛控制题配套资料\
  ├── 【小车】01_DC_MOTOR_PID_1/   TB6612 基本旋转
  ├── 【小车】02_DC_MOTOR_PID_2/   编码器测速
  ├── 【小车】02_DC_MOTOR_PID_3/   速度 PID 闭环
  └── 【小车】03_PID_car/           灰度循迹（最完整）
```

## 4. 文件夹结构

```text
control-car/
├── README.md
├── docs/
│   ├── AI-IMPLEMENTATION-PLAN.md
│   ├── preparation-roadmap.md
│   └── purchase-and-build-plan.md
├── firmware/
│   └── mspm0/
│       ├── README.md
│       ├── src/control/       # pid.c/h, line_follow.c/h
│       └── config/            # pid-defaults.md, serial-protocol.md
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
阶段 3：扩展（MPU6050 + 陀螺仪 + 云台 + 视觉）
```

## 6. 关键原则

- 主控统一使用 **地猛星 MSPM0G3507**
- 下载调试使用 **XDS110**，不使用 ST-LINK
- 电机驱动使用 **TB6612FNG**，不推荐 L298N
- 小车采用 **两轮差速 + 万向轮**
- 第一目标是稳定闭环，不是炫技
- 小车底层不稳定时，不堆视觉/云台
- 所有引脚以 `hardware/pinmap.md` 为唯一来源
