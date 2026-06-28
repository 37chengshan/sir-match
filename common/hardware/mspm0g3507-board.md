# MSPM0G3507 主控板

## 1. 作用

MSPM0G3507 是当前电赛训练的主控平台，用于：

- G题电路模型探究装置；
- control-car 循迹小车；
- 其他 TI MSPM0 指定平台题目。

## 2. 推荐板卡

优先级：

1. LP-MSPM0G3507 LaunchPad；
2. 立创·天猛星 MSPM0G3507；
3. 其他 MSPM0G3507 核心板。

建议买 2 块：一块主用，一块备用。

## 3. 开发环境

- Code Composer Studio Theia；
- MSPM0 SDK；
- SysConfig；
- XDS110 / 板载调试器；
- 串口助手。

## 4. 下载注意

不要用 ST-LINK 下载 MSPM0。使用官方支持的 XDS110、LaunchPad 板载调试器或开发板文档指定方式。

## 5. 外设分配建议

| 功能 | 外设 |
|---|---|
| AD9833 | SPI + GPIO CS |
| OLED | I2C |
| ST7789 | SPI + GPIO DC/RST/CS |
| Vin采样 | ADC0 |
| Vout采样 | ADC1 |
| 波形复现 | DAC0 |
| 串口调试 | UART0 |
| 按键/LED | GPIO |
| 电机 PWM | Timer PWM |
| 编码器 | Timer / GPIO interrupt |

实际引脚必须以所用板卡原理图和 SysConfig 为准。

## 6. 最小测试顺序

1. 点灯；
2. 按键；
3. 串口打印；
4. I2C OLED；
5. SPI AD9833；
6. ADC 单通道；
7. DAC 输出；
8. DMA；
9. 定时器中断 / PWM。

## 7. 常见坑

- 下载器选错；
- 3.3V 和 5V 模块混接；
- ADC 输入没有限幅；
- 模拟地线和电机地线乱接；
- SysConfig 修改后代码没有重新生成；
- 串口 TX/RX 交叉接错。

## 8. 验收标准

- 能稳定下载程序；
- 串口每秒输出状态；
- OLED 能显示；
- ADC 能读取可变电压；
- DAC 能输出查表波形；
- 复位、断电重启后行为一致。
