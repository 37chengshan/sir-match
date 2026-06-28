# 硬件资料

这里放 G题「电路模型探究装置」专用硬件资料。

## 计划内容

```text
hardware/
├── README.md
├── schematic/       原理图
├── pcb/             PCB工程和Gerber
├── wiring/          接线图
└── bom/             题目专用BOM
```

## 当前硬件模块

- MSPM0G3507 主控板；
- AD9833 DDS 信号源；
- 数字电位器幅度控制；
- 运放缓冲与偏置；
- Vin/Vout 双路 ADC 前端；
- DAC 波形复现输出；
- 已知 RC 模型电路；
- 未知 RLC 黑盒电路；
- OLED/ST7789 显示。
