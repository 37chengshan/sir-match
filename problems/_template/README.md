# 题目模板

复制本目录结构创建新题目。

## 基本信息

- 题目编号：
- 题目名称：
- 主控平台：
- 题目类型：控制 / 信号 / 电源 / 模拟 / 小车 / 视觉 / 综合
- 当前状态：调研 / 采购 / 硬件搭建 / 软件开发 / 联调 / 报告

## 推荐目录

```text
<topic>/
├── README.md
├── docs/
│   ├── problem.md          # 题目原文和约束
│   ├── scoring.md          # 得分点拆解
│   ├── solution.md         # 总体方案
│   └── purchase-list.md    # 采购清单
├── firmware/
│   └── <platform>/         # 固件工程
├── simulation/
│   ├── matlab/
│   └── python/
├── hardware/
│   ├── schematic/
│   ├── pcb/
│   └── wiring/
├── test-data/
│   ├── csv/
│   └── scope-images/
├── report/
└── web/
```

## README 必须写

1. 题目一句话概括；
2. 目标分数；
3. 主控和关键器件；
4. 最小可运行闭环；
5. 当前进度；
6. 下一步任务。
