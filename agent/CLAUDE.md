# Agent Guide

## 项目是什么

这是电赛备赛仓库。核心目标是把题目方案、采购清单、算法、固件、仿真、测试数据和报告资料按目录组织起来，方便人和 agent 协作。

当前重点题目：

- `problems/G-circuit-model/`：G题「电路模型探究装置」
- 指定训练平台：`MSPM0G3507`
- 主要模块：AD9833 信号源、双 ADC 同步采样、Goertzel 幅相测量、DAC 波形复现、MATLAB 验证

## 怎么查

1. 总览先查 `README.md`。
2. 总要求查 `docs/00-overall-requirements.md`。
3. 当前执行计划查 `docs/01-training-plan.md`。
4. 具体题目只查对应题目文件夹，例如 `problems/G-circuit-model/`。
5. 通用算法查 `common/algorithms/`。
6. 通用硬件模块查 `common/hardware/`。
7. 旧 `stm32/` 和旧 `matlab/` 目录先当历史参考，不要直接当最终 MSPM0G3507 方案。

## 文件放哪里

```text
problems/<题目>/docs/          题目方案、评分拆解、采购清单
problems/<题目>/firmware/      题目专用固件
problems/<题目>/simulation/    MATLAB/Python 仿真
problems/<题目>/hardware/      原理图、PCB、接线图
problems/<题目>/test-data/     示波器截图、CSV、标定数据
problems/<题目>/report/        最终报告素材
common/algorithms/             通用算法
common/hardware/               通用硬件模块
common/test-methods/           通用测试方法
docs/                          总要求、总计划、采购原则、训练路线
agent/                         给 agent 的最小说明
```

## 处理原则

- 新题目必须新建独立文件夹。
- 通用资料不要复制到每个题目里，统一放 `common/`。
- 题目专用资料不要放 `common/`。
- 不要再向仓库根目录新增零散中文文档。
- 修改方案前先确认主控平台是否仍为 MSPM0G3507。
