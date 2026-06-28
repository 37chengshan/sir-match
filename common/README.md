# common 通用积木库

`common/` 是电赛项目的公共积木层，只放跨题目复用的算法、硬件、接口规范和测试方法。任何单一题目的最终方案、专用接线图、专用固件工程都不要放在这里。

## 1. 目录定位

```text
common/
├── README.md
├── algorithms/       通用算法：Goertzel、PID、FFT、拟合、系统辨识
├── hardware/         通用硬件：MSPM0G3507、AD9833、ADC前端、DAC输出、电源
└── test-methods/     通用测试方法：示波器、CSV、标定、误差分析，后续新增
```

## 2. 当前优先服务的题目

| 顶层题目文件夹 | 复用内容 |
|---|---|
| `G-circuit-model/` | AD9833、ADC前端、DAC输出、Goertzel、滤波器识别、RLC拟合、m序列互相关 |
| `control-car/` | MSPM0G3507、PWM、电机驱动、编码器测速、PID、OLED/串口调参、电源抗干扰 |

## 3. 放入 common 的判断标准

可以放入 `common/`：

- 两个及以上题目可能复用；
- 不依赖某个题目的具体引脚；
- 可以被移植到 MSPM0G3507、STM32 或其他平台；
- 能抽象成接口、模块说明、测试流程或算法说明。

不应放入 `common/`：

- 某题最终方案；
- 某题专用 BOM；
- 某题专用接线图；
- 某题最终报告；
- 某题固件工程入口；
- 某题示波器截图和 CSV 原始数据。

## 4. 通用模块成熟度分级

| 等级 | 含义 | 文件要求 |
|---|---|---|
| L0 概念 | 只有思路，未验证 | 写清用途、输入输出、风险 |
| L1 仿真 | 已在 MATLAB/Python 验证 | 给出测试数据、误差指标 |
| L2 单模块实测 | 单独接线可跑 | 给出接线、最小测试步骤 |
| L3 集成可用 | 已在题目工程使用 | 给出接口、依赖、验收标准 |
| L4 比赛固化 | 现场可复用 | 给出故障排查、替代方案、报告素材 |

## 5. 通用文档最低标准

每个通用算法或硬件文档至少写清楚：

1. 作用；
2. 适用题目；
3. 输入输出；
4. 推荐接口；
5. 最小验证方法；
6. 常见坑；
7. 后续迁移到题目工程时应该放在哪里。

## 6. 当前公共模块索引

### 算法

| 文件 | 模块 | 当前用途 |
|---|---|---|
| `algorithms/goertzel.md` | 单频幅相提取 | G题频响测量 |
| `algorithms/pid.md` | PID闭环控制 | G题调幅、小车速度/转向闭环 |
| `algorithms/fft-harmonic.md` | FFT/谐波分析 | G题矩形波复现 |
| `algorithms/filter-identification.md` | 滤波器类型识别 | G题低通/高通/带通/带阻判断 |
| `algorithms/rlc-fitting.md` | RLC参数拟合 | G题未知黑盒建模 |
| `algorithms/m-sequence-correlation.md` | m序列互相关 | G题时域系统辨识冲高分 |

### 硬件

| 文件 | 模块 | 当前用途 |
|---|---|---|
| `hardware/mspm0g3507-board.md` | MSPM0G3507主控平台 | G题、小车训练 |
| `hardware/ad9833-dds.md` | AD9833信号源 | G题扫频 |
| `hardware/adc-front-end.md` | ADC采样前端 | G题Vin/Vout采样 |
| `hardware/dac-output.md` | DAC查表输出 | G题波形复现 |
| `hardware/digital-potentiometer.md` | 数字电位器 | G题自动幅度控制 |
| `hardware/display-oled-st7789.md` | 显示模块 | 状态显示、调参、曲线显示 |
| `hardware/power.md` | 电源模块 | 全题通用供电与抗干扰 |

## 7. 使用原则

题目工程引用 `common/` 时，只复制“接口思想”和“测试方法”，不要直接把公共文档当成最终代码。真正的题目固件应放在对应题目目录，例如：

```text
G-circuit-model/firmware/mspm0/
control-car/firmware/mspm0/
```
