# 通用硬件模块库

这里放不同题目都能复用的硬件积木说明。

## 当前优先硬件积木

| 模块 | 说明 | 当前用途 |
|---|---|---|
| MSPM0G3507 主控 | TI 比赛指定/推荐平台之一 | 当前主平台 |
| AD9833 DDS | SPI 控制正弦/方波/三角波输出 | 信号源 |
| ADC 前端 | 1.65V 偏置、限幅、缓冲、抗混叠 | Vin/Vout 采样 |
| DAC 输出 | 查表波形输出、RC 重构、运放缓冲 | 波形复现 |
| 数字电位器 | SPI/I2C 控制幅度 | 自动幅度控制 |
| OLED/ST7789 | 状态显示、Bode 曲线显示 | 人机交互 |
| 电源模块 | DC-DC、LDO、模拟/数字分区 | 稳定供电 |

## 推荐文档结构

```text
common/hardware/
├── README.md
├── mspm0g3507-board.md
├── ad9833-dds.md
├── adc-front-end.md
├── dac-output.md
├── digital-potentiometer.md
├── display-oled-st7789.md
└── power.md
```

## 硬件文档必须写清楚

每个硬件模块文档至少包含：

1. 作用；
2. 推荐型号；
3. 接线方式；
4. 电压逻辑；
5. 常见坑；
6. 最小测试代码或测试步骤；
7. 在哪些题目中使用。
