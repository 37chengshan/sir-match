# sir-match

电赛备赛结构化仓库。当前主线题目是 **G题：电路模型探究装置**，训练平台统一为 **TI MSPM0G3507**。

## 仓库结构原则

本仓库只保留四类顶层入口：

1. 根目录说明文件：`README.md`、`CLAUDE.md`、`AGENTS.md`；
2. `common/`：通用算法、通用硬件模块、通用测试方法；
3. `比赛文档/`：总要求、训练计划、采购原则、官方资料摘要；
4. 每个题目一个顶层文件夹，例如 `G-circuit-model/`、`control-car/`。

以后不要再使用二级 `problems/题目名/` 结构，新题目直接在仓库顶层新建文件夹。

## 当前推荐结构

```text
sir-match/
├── README.md
├── CLAUDE.md
├── AGENTS.md
├── common/
│   ├── README.md
│   ├── algorithms/
│   │   └── README.md
│   └── hardware/
│       └── README.md
├── 比赛文档/
│   ├── README.md
│   ├── 00-总要求.md
│   └── 01-MSPM0G3507训练计划.md
├── G-circuit-model/
│   ├── README.md
│   ├── docs/
│   │   └── solution-MSPM0G3507.md
│   ├── firmware/
│   │   └── mspm0/
│   ├── simulation/
│   │   └── matlab/
│   ├── hardware/
│   ├── test-data/
│   ├── report/
│   └── web/
│       └── index.html
└── control-car/
    ├── README.md
    ├── docs/
    │   ├── AI-IMPLEMENTATION-PLAN.md
    │   └── purchase-and-build-plan.md
    ├── firmware/
    │   └── mspm0/
    │       └── README.md
    ├── hardware/
    │   └── README.md
    ├── simulation/
    │   └── README.md
    ├── test-data/
    │   └── README.md
    ├── report/
    │   └── README.md
    └── web/
        └── index.html
```

## 当前主线题目

| 题目 | 顶层文件夹 | 主控平台 | 当前状态 |
|---|---|---|---|
| G题：电路模型探究装置 | `G-circuit-model/` | MSPM0G3507 | 方案、采购、训练计划已结构化 |
| 小车训练平台：循迹/PID/通信/显示积木库 | `control-car/` | MSPM0G3507 | 采购计划、AI 实施计划、展示页已创建 |

## 当前先做什么

1. 看 `比赛文档/01-MSPM0G3507训练计划.md`。
2. G题方向：买第一批必备器材：MSPM0G3507、AD9833、OLED、运放、阻容感、电源、面包板、USB转串口。
3. 小车方向：看 `control-car/web/index.html` 和 `control-car/docs/purchase-and-build-plan.md`，优先购买 MSPM0G3507、小车 PCB、N20 编码器电机、TB6612、8路灰度、电源和调试工具。
4. 在 `G-circuit-model/firmware/mspm0/` 新建真正的 MSPM0G3507 工程。
5. G题先跑通最小闭环：AD9833 输出 → 模型电路 → MSPM0G3507 双 ADC 采样 → Goertzel 算幅相 → OLED/串口显示。
6. 小车先跑通最小闭环：PWM 输出 → TB6612 电机控制 → 编码器测速 → 灰度循迹 → 速度 PID/转向 PID → OLED/串口调参。

## 顶层题目文件夹规则

新题目直接建在根目录：

```text
A-topic-name/
B-topic-name/
G-circuit-model/
control-car/
power-converter/
```

每个题目内部结构统一：

```text
<topic>/
├── README.md
├── docs/          方案、评分拆解、采购清单
├── firmware/      固件工程
├── simulation/    MATLAB/Python 仿真
├── hardware/      原理图、PCB、接线图
├── test-data/     CSV、示波器截图、标定结果
├── report/        报告素材
└── web/           展示页
```

## 文件放置规则

- 通用算法：`common/algorithms/`
- 通用硬件模块：`common/hardware/`
- 总计划和总要求：`比赛文档/`
- 题目专用内容：对应顶层题目文件夹
- agent/Claude/Codex 说明：根目录 `CLAUDE.md` 和 `AGENTS.md`

## 已清理内容

旧的 `stm32/`、`matlab/`、`docs/`、`problems/`、`agent/` 和根目录旧方案文件不再作为新结构的一部分。后续所有新增内容按本 README 放置。
