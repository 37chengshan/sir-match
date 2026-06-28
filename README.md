# sir-match

电赛备赛结构化仓库。当前主线是 **TI杯 / 全国大学生电子设计竞赛 G题：电路模型探究装置**，训练平台统一切换为 **TI MSPM0G3507**。

## 仓库目标

本仓库不是只存一个临时方案，而是作为电赛备赛的长期资料库：

- 每个题目一个独立目录；
- 通用要求、通用算法、通用硬件模块集中管理；
- 方案、采购清单、仿真、固件、测试数据和报告资料分层存放；
- 方便人和 agent 快速定位文件，避免根目录堆满零散文档。

## 推荐目录结构

```text
sir-match/
├── README.md
├── CLAUDE.md
├── agent/
│   └── CLAUDE.md
├── docs/
│   ├── 00-overall-requirements.md
│   └── 01-training-plan.md
├── common/
│   ├── README.md
│   ├── algorithms/
│   │   └── README.md
│   └── hardware/
│       └── README.md
├── problems/
│   ├── README.md
│   ├── _template/
│   │   └── README.md
│   └── G-circuit-model/
│       ├── README.md
│       ├── docs/
│       │   └── solution-MSPM0G3507.md
│       ├── web/
│       │   └── index.html
│       ├── firmware/
│       │   └── mspm0/
│       ├── simulation/
│       │   └── matlab/
│       ├── hardware/
│       ├── test-data/
│       └── report/
├── matlab/        # 旧位置：后续逐步迁入对应题目或 common
└── stm32/         # 旧 STM32 方案代码，仅保留作历史参考
```

## 当前主线题目

| 题目 | 文件夹 | 主控平台 | 当前状态 |
|---|---|---|---|
| G题：电路模型探究装置 | `problems/G-circuit-model/` | MSPM0G3507 | 方案、采购、训练计划已结构化 |

## 当前先做什么

1. 先看 `docs/01-training-plan.md`，按阶段推进。
2. 买第一批必备器材：MSPM0G3507、AD9833、OLED、运放、阻容感、电源、面包板、USB转串口。
3. 先跑通最小闭环：AD9833 输出 → 模型电路 → MSPM0G3507 双 ADC 采样 → Goertzel 算幅相 → OLED/串口显示。
4. 不要一开始做复杂 UI、大 FFT、完整报告；先让系统可测、可调、可复现。

## 文件放置规则

- 新题目：放入 `problems/<题目编号-英文短名>/`。
- 题目专用方案：放入 `problems/<题目>/docs/`。
- 题目专用固件：放入 `problems/<题目>/firmware/`。
- 题目专用仿真：放入 `problems/<题目>/simulation/`。
- 通用算法：放入 `common/algorithms/`。
- 通用硬件模块：放入 `common/hardware/`。
- 备赛计划、总要求、采购原则：放入 `docs/`。
