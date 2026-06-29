# MSPM0G3507 固件工作区

> 引脚映射：`control-car/hardware/pinmap.md`
> PID 参数：`control-car/firmware/mspm0/config/pid-defaults.md`
> 参考案例：`D:\work\参考文件-已实现案例\2026_04_地猛星电赛控制题配套资料\`

## 1. 硬件平台

- 主控：地猛星 MSPM0G3507 开发板
- 电机驱动：TB6612FNG（双路直流电机）
- 编码器：AB 相霍尔编码器，260 线，减速比 1:20，轮径 67mm
- 循迹：5 路灰度传感器（数字量）
- 显示：I2C OLED（SSD1306 / SH1106）
- 调试：XDS110 仿真器 + UART 串口 115200 8N1

## 2. 开发环境

- CCS Theia + MSPM0 SDK + SysConfig
- SysConfig 自动生成 `ti_msp_dl_config.c/h`（引脚/时钟/外设初始化）
- 用户代码放在 `user_driver/` 目录，不要直接改 SysConfig 生成文件

## 3. 代码目录规划

```text
control-car/firmware/mspm0/
├── README.md
├── src/
│   ├── app/                状态机、任务调度
│   ├── board/              时钟、SysConfig 适配
│   ├── drivers/            电机(TB6612)、编码器、循迹、OLED、按键
│   ├── control/            PID、速度环、转向环
│   ├── protocol/           UART 调参协议
│   └── utils/              环形缓冲、滤波
├── config/
│   ├── pid-defaults.md     PID 起步参数和调参记录
│   └── serial-protocol.md  UART 命令协议
└── tests/
    └── bringup-plan.md     启动测试计划
```

## 4. 开发原则

1. 引脚全部用 `control-car/hardware/pinmap.md` 中的宏定义，不要硬编码
2. 写代码前先看参考案例中对应工程的实现
3. 不要把全部逻辑塞进 `main.c`，按分层拆分
4. 不确认的参数用 `TODO_MEASURE` 标注，不要编造

## 5. 第一版固件验收

- [ ] CCS 工程能编译下载
- [ ] 串口能输出 BOOT 日志
- [ ] OLED 能显示当前模式和状态
- [ ] 左右电机能独立正反转和 PWM 调速
- [ ] 编码器能读到方向和速度
- [ ] 灰度传感器能输出黑白状态和偏差
- [ ] 速度 PID 闭环稳定
- [ ] 可通过串口调整 PID 参数
