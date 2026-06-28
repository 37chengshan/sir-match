# CLAUDE.md

## 项目是什么

这是电赛备赛仓库。当前重点顶层文件夹：

- `G-circuit-model/`：G题「电路模型探究装置」
- `control-car/`：MSPM0G3507 循迹小车训练平台
- 主控平台：TI MSPM0G3507

## 先怎么查

1. 先读根目录 `README.md`，确认仓库结构。
2. 再读 `比赛文档/00-总要求.md`，确认通用要求。
3. 再读 `比赛文档/01-MSPM0G3507训练计划.md`，确认当前训练计划。
4. 处理 G 题时，只进入 `G-circuit-model/`。
5. 处理循迹小车、电机、编码器、PID、通信、显示、控制类训练时，进入 `control-car/`。
6. 处理小车任务时优先读 `control-car/docs/AI-IMPLEMENTATION-PLAN.md`。
7. 查通用算法、通用硬件积木时，看 `common/`。

## 文件应该放哪里

```text
README.md / CLAUDE.md / AGENTS.md    根目录说明
common/                              通用算法、通用硬件、通用测试方法
比赛文档/                            总要求、训练计划、采购原则、官方资料摘要
G-circuit-model/                     G题专用方案、固件、仿真、硬件、报告
control-car/                         小车训练平台、循迹、PID、电机、通信、显示
<new-topic>/                         以后每个新题目都在顶层新建
```

## 处理原则

- 新题目直接在仓库顶层创建文件夹，不再放入 `problems/`。
- 不要再向根目录添加零散方案文档或 HTML。
- 题目专用资料放进对应题目文件夹。
- 通用算法和通用硬件模块放进 `common/`。
- 总要求、总计划、比赛规则摘要放进 `比赛文档/`。
- MSPM0 不要用 ST-LINK 下载。
- `control-car/` 的第一目标是稳定完成 MSPM0G3507 小车底层闭环：PWM 电机控制、编码器测速、灰度循迹、速度 PID、转向 PID、OLED/串口调参。
