# G题：电路模型探究装置

## 1. 题目一句话概括

制作一个可以产生测试信号、测量模型电路输入输出、学习未知电路频率响应，并复现未知电路输出的 **电路数字孪生装置**。

当前方案主控统一使用：**TI MSPM0G3507**。

## 2. 当前方案

```text
MSPM0G3507 主控
  ├── SPI 控制 AD9833 输出 1kHz~50kHz 正弦信号
  ├── 双 ADC 同步采样 Vin / Vout
  ├── Goertzel 提取幅值和相位
  ├── 识别低通 / 高通 / 带通 / 带阻
  ├── 拟合 RC / RLC 参数
  └── DAC 查表输出复现波形
```

## 3. 目录说明

```text
G-circuit-model/
├── README.md
├── docs/
│   └── solution-MSPM0G3507.md
├── web/
│   └── index.html
├── firmware/
│   └── mspm0/        # 后续新建 MSPM0G3507 工程
├── simulation/
│   └── matlab/       # 后续迁入 MATLAB 仿真
├── hardware/         # 原理图、PCB、接线图
├── test-data/        # CSV、示波器截图、标定数据
└── report/           # 报告素材和最终报告
```

## 4. 最小可运行闭环

第一阶段只追求跑通这个链路：

```text
AD9833 输出正弦 → 已知 RC 模型电路 → MSPM0G3507 ADC 采样 → 计算 Vpp → OLED/串口显示
```

第二阶段扩展为：

```text
AD9833 扫频 → Vin/Vout 双通道采样 → Goertzel 算幅相 → 串口导出 Bode 数据
```

第三阶段扩展为：

```text
学习得到 H(jω) / h(n) → MSPM0G3507 DAC 输出复现波形
```

## 5. 当前优先级

| 优先级 | 任务 | 说明 |
|---|---|---|
| P0 | 买第一批器材 | MSPM0G3507、AD9833、OLED、运放、阻容感、电源 |
| P0 | 建 MSPM0G3507 工程 | 放到 `firmware/mspm0/` |
| P0 | 跑通 AD9833 | 1kHz、5kHz、10kHz、50kHz 可切换 |
| P0 | 跑通 ADC 采样 | 先单通道，再双通道 |
| P1 | 做自动幅度控制 | 数字电位器 + PID |
| P1 | 做扫频建模 | Goertzel 幅相提取 |
| P2 | 做波形复现 | DAC 查表输出、谐波法 |
| P2 | 做报告图表 | MATLAB 画 Bode 图、误差表 |

## 6. 重要注意

- 不再以 STM32F407 作为最终主控方案；旧 `stm32/` 仅作历史参考。
- MSPM0 不要使用 ST-LINK 下载。
- 先跑硬件闭环，再优化算法。
- 频响建模必须采 `Vin` 和 `Vout` 两路，不能只采输出。
