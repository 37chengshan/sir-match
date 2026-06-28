# CLAUDE.md

## 项目是什么

这是电赛备赛仓库，目标是把不同赛题的方案、固件、仿真、采购清单和报告资料结构化管理。当前主线题目是：

- `problems/G-circuit-model/`：G题「电路模型探究装置」，主控平台改为 **TI MSPM0G3507**。

## 先怎么查

1. 先读根目录 `README.md`，确认仓库总结构。
2. 再读 `docs/00-overall-requirements.md`，确认通用要求。
3. 再读 `docs/01-training-plan.md`，确认当前训练计划。
4. 处理具体题目时，只进入对应题目文件夹，例如 `problems/G-circuit-model/`。
5. 查通用算法、硬件积木、采购规范时，看 `common/`。

## 文件应该放哪里

- 每个赛题单独放在 `problems/<题目英文短名>/`。
- 通用要求、备赛计划、采购总表放在 `docs/`。
- 通用算法、通用硬件模块、通用测试流程放在 `common/`。
- 给 agent / Claude / Codex 的简短说明放在 `agent/`。
- 旧 STM32 代码暂时保留在 `stm32/`，只作为历史参考；新 MSPM0G3507 工程后续应放到 `problems/G-circuit-model/firmware/mspm0/`。

## 注意

- 不要再把新题目文档散放在仓库根目录。
- 不要把具体题目的专用代码放进 `common/`。
- 不要把通用算法重复复制到每个题目文件夹；先放 `common/algorithms/`，题目里只引用。
