# problems 题目目录

每一个电赛题目都必须单独放一个文件夹，避免方案、代码、仿真和报告混在一起。

## 当前题目

| 题目 | 文件夹 | 状态 |
|---|---|---|
| G题：电路模型探究装置 | `G-circuit-model/` | 当前主线，已切换 MSPM0G3507 平台 |
| 题目模板 | `_template/` | 新题目复制这个结构 |

## 新题目命名规则

```text
<题目编号>-<英文短名>
```

示例：

```text
G-circuit-model
E-auto-aiming-car
F-signal-processing
control-car
power-converter
```

## 每个题目推荐结构

```text
problems/<topic>/
├── README.md
├── docs/
│   ├── problem.md
│   ├── scoring.md
│   ├── solution.md
│   └── purchase-list.md
├── firmware/
│   └── <platform>/
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

## 放置原则

- 题目专用内容放题目目录。
- 多题复用内容放 `common/`。
- 总计划、总采购原则、总要求放 `docs/`。
