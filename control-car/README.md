# control-car：MSPM0G3507 循迹小车训练平台

> 定位：这是电赛备赛中的 **MSPM0G3507 小车控制积木平台**，目标不是只做一个能跑的视频，而是沉淀可复用的电源、电机、循迹、编码器、PID、通信、显示、数据采集与调试模块。

## 1. 本文件夹解决什么问题

围绕老师提供的循迹小车 PCB 和 TI MSPM0G3507 平台，提前搭建一套可持续迭代的小车训练系统：

1. 能稳定完成两轮差速小车的基本运动控制；
2. 能读取灰度/红外循迹传感器，完成黑线循迹；
3. 能读取编码器，完成速度闭环；
4. 能实现速度 PID、转向 PID、串口/OLED 调参；
5. 能把电源、电机、循迹、通信、显示等模块封装成以后电赛可复用的“积木”；
6. 后续可扩展云台、视觉、IMU、超声波、蓝牙/2.4G 通信等高分模块。

## 2. 推荐阅读顺序

AI/Claude/Codex 优先读：

1. `docs/AI-IMPLEMENTATION-PLAN.md`：给 AI 执行用的详细实现方案、任务拆分、目录规范、接口约定、验收标准。
2. `docs/purchase-and-build-plan.md`：采购清单、分阶段购买策略、模块优先级、避坑说明。
3. `web/index.html`：给人看的项目介绍页，可以直接在浏览器打开。

人先看：

1. `web/index.html`
2. `docs/purchase-and-build-plan.md`
3. `docs/AI-IMPLEMENTATION-PLAN.md`

## 3. 文件夹结构

```text
control-car/
├── README.md
├── docs/
│   ├── AI-IMPLEMENTATION-PLAN.md
│   └── purchase-and-build-plan.md
├── firmware/
│   └── mspm0/
├── hardware/
├── simulation/
├── test-data/
├── report/
└── web/
    └── index.html
```

## 4. 当前阶段目标

当前只做第一阶段：

```text
MSPM0G3507 基础外设
→ TB6612 电机控制
→ 编码器测速
→ 灰度循迹
→ 速度 PID
→ 转向 PID
→ OLED/串口调参
```

暂时不要一上来堆视觉、云台和复杂上位机。小车底层不稳定时，任何高级模块都会变成调试负担。

## 5. 关键原则

- 主控统一使用 **TI MSPM0G3507**。
- MSPM0 不使用 ST-LINK 下载。
- 小车优先采用 **两轮差速 + 万向轮**。
- 电机建议直接使用 **N20 带编码器减速电机**。
- 电机驱动优先使用 **TB6612FNG**，不推荐 L298N。
- 第一目标是稳定闭环，不是炫技。
- 采购按“必需 → 推荐 → 冲高分”三阶段推进。
- 任何光源/激光相关测试必须经过老师或实验室安全确认；早期用普通 LED/低风险指示光源替代即可。
