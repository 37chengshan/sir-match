# sir-match

电赛备赛结构化仓库。主控平台统一为 **TI MSPM0G3507**（地猛星开发板），小车/云台视觉平台统一为 **立创·庐山派 K230 / CanMV K230**。

## 仓库结构

```text
sir-match/
├── README.md / CLAUDE.md / AGENTS.md
├── common/              通用算法、硬件模块、测试方法
├── 比赛文档/            总要求、训练计划、器材清单
├── G-circuit-model/     G题「电路模型探究装置」
└── control-car/         MSPM0G3507 循迹小车 + 云台训练平台
```

## 当前主线

| 项目 | 文件夹 | 平台 | 核心内容 |
|------|--------|------|---------|
| G题：电路模型探究 | `G-circuit-model/` | MSPM0G3507 | AD9833 信号源 + 双 ADC + Goertzel 幅相 |
| **小车 + 云台训练** | `control-car/` | 地猛星 MSPM0G3507 + 庐山派 K230 | TB6612 电机 + 编码器 + 灰度循迹 + PID + K230 视觉 + 步进云台 |

## control-car 全貌

### 平台组成

```text
地猛星 MSPM0G3507
├── 小车底盘：两轮差速 + 万向轮
│   ├── TB6612FNG 双路电机驱动
│   ├── N20 编码器减速电机 ×2（260线, 67mm轮径）
│   └── 5路/8路灰度循迹传感器
├── 云台：二维步进电机
│   ├── DCC-100v3 开环驱动（定时器脉冲）
│   └── DCC-101v1 闭环驱动（UART 协议, 备选）
└── 视觉：庐山派 K230 / CanMV K230
    ├── 摄像头预览、色块、黑线、二维码、AprilTag
    ├── YOLO/kmodel 端侧推理（进阶）
    └── UART 视觉结果输出 → MSPM0

调试工具：XDS110 仿真器 + OLED I2C + UART 115200 + 按键/旋钮
```

### 训练路线

| 阶段 | 目标 | 关键内容 |
|------|------|---------|
| 0 环境搭建 | CCS Theia + XDS110 + LED/OLED/UART | 基础外设教程 |
| 1 小车闭环 | TB6612 双电机 + 编码器 + PID + 循迹 | 参考案例 01→03 |
| 2 工程化 | 状态机 + UART 调参 + OLED + CSV 记录 | 可复现、可调参 |
| 3 K230 视觉扩展 | K230 识别结果 → UART → MSPM0 | 视觉循迹、色块/二维码/AprilTag、靶面识别 |
| 4 云台/瞄准扩展 | MSPM0 控制步进云台 | K230 输出坐标，MSPM0 做云台控制 |

### 已采购 / 已纳入主线器材

- 地猛星 MSPM0G3507 ×2
- XDS110 仿真器
- 两轮差速小车（N20 编码器电机 + TB6612 + 5路灰度）
- 2S 锂电池 + 充电器
- 立创·庐山派 K230 / CanMV K230
- K230 必需：TF 卡 8GB/16GB Class 10/U1 + TF 读卡器 + USB-C 数据线
- USB 摄像头（PC 端算法预研/备用，不作为比赛现场主线）
- 42 步进电机二维云台 + DCC-100v3 驱动
- OLED、杜邦线、面包板、电源模块等

完整清单见 `比赛文档/电赛备赛器材清单.md`。

## 参考已实现案例

```text
D:\work\参考文件-已实现案例\
├── 2026_04_地猛星电赛控制题配套资料/    MSPM0G3507 CCS 工程（小车←→云台）
├── 2025_09_yolo全系列使用教程/          YOLO 训练 pipeline
├── 2025_10_k230运行yolov12/            K230 端侧推理
├── 2026_06_电赛视觉资料/              双路 MJPEG 网页推流
└── DCC-100双路步进电机驱动模块资料/     步进驱动文档 + STM32 参考
```

## 快速开始

1. 看 `比赛文档/01-MSPM0G3507训练计划.md`
2. 小车方向：读 `control-car/README.md` → `control-car/hardware/pinmap.md` → `control-car/docs/AI-IMPLEMENTATION-PLAN.md`
3. K230 视觉方向：读 `control-car/docs/k230-vision-plan.md`
4. G题方向：读 `G-circuit-model/README.md`

## 多人协作

```text
同步 main → 建分支 → 修改 → commit → push → 开 PR → CI 检查 → Review → 合并
```

详见 `CONTRIBUTING.md` 和 `GOVERNANCE.md`。

## 已清理

旧的 `stm32/`、`matlab/`、`docs/`、`problems/`、`agent/` 已删除。新题目直接在顶层建文件夹。
