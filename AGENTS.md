# AGENTS.md

## 项目定位

这是电赛备赛仓库。当前重点顶层题目文件夹：

```text
G-circuit-model/
control-car/
```

- `G-circuit-model/`：G题「电路模型探究装置」，平台统一为 **MSPM0G3507**。
- `control-car/`：MSPM0G3507 循迹小车训练平台，用于沉淀电源、电机、编码器、循迹、PID、通信、显示等控制类通用积木。

## 查找顺序

1. `README.md`：仓库总结构。
2. `比赛文档/00-总要求.md`：通用要求。
3. `比赛文档/01-MSPM0G3507训练计划.md`：当前训练计划。
4. 处理 G 题时，进入 `G-circuit-model/README.md`。
5. 处理小车/控制/PID/循迹/电机训练时，进入 `control-car/README.md`，再读 `control-car/docs/AI-IMPLEMENTATION-PLAN.md`。
6. 查通用算法和硬件模块时，看 `common/`。

## 文件归档规则

- 题目文件夹必须在仓库顶层。
- 通用内容放 `common/`。
- 比赛总要求和训练计划放 `比赛文档/`。
- 不再使用 `problems/` 二级题目目录。
- 不再使用 `agent/` 目录。
- 不再保留旧 `stm32/` 和旧根目录方案文件作为主线。
- MSPM0 不要用 ST-LINK 下载。
