# OLED / ST7789 显示模块

## 1. 作用

显示模块用于现场调试和状态展示。

优先级：

1. 0.96 寸 OLED：先调通，简单稳定；
2. ST7789 彩屏：后期显示 Bode 曲线、菜单和图表。

## 2. OLED 适合显示

- 当前模式；
- 频率；
- Vin/Vout；
- 增益；
- 相位；
- PID 误差；
- 错误码。

## 3. ST7789 适合显示

- Bode 幅频曲线；
- 相频曲线；
- 滤波器识别结果；
- 参数拟合结果；
- 小车调参界面。

## 4. 接线注意

OLED I2C：

```text
VCC → 3.3V 或 5V，按模块说明
GND → GND
SCL → MSPM0 I2C SCL
SDA → MSPM0 I2C SDA
```

ST7789 SPI：

```text
SCLK → SPI SCLK
MOSI → SPI MOSI
CS   → GPIO
DC   → GPIO
RST  → GPIO
BLK  → GPIO/PWM 或接高电平
```

## 5. 调试策略

第一阶段只用 OLED，不做复杂 UI：

```text
line1: mode
line2: freq
line3: Vin Vout
line4: gain phase
```

等核心功能跑通后再上 ST7789。

## 6. 常见坑

- I2C 地址不是 0x3C；
- SCL/SDA 没接上拉；
- SPI 模式或 DC 引脚错误；
- 彩屏刷新占用太多时间，影响采样；
- UI 太复杂，拖慢主循环；
- 显示任务和实时采样没有解耦。

## 7. 最小验收

- 上电显示项目名；
- 串口命令改变频率后，屏幕同步刷新；
- 出错时能显示错误码；
- 采样期间显示不阻塞关键中断。

## 8. 建议错误码

```text
E01: AD9833 init failed
E02: ADC timeout
E03: DAC output overflow
E04: amplitude not stable
E05: model identify failed
E06: UART export failed
```
