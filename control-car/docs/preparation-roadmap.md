# 前期准备路线：MSPM0G3507 循迹小车

## 1. 先读这些文件

```text
control-car/hardware/original-pcb-summary.md
control-car/hardware/pinmap.md
control-car/hardware/wiring-guide.md
control-car/hardware/power-design.md
control-car/hardware/bring-up-checklist.md
control-car/firmware/mspm0/config/pid-defaults.md
control-car/firmware/mspm0/tests/bringup-plan.md
```

## 2. 第一阶段：开发环境

目标：能下载、能串口、能显示。

任务：

1. 安装 CCS Theia；
2. 安装 MSPM0 SDK；
3. 打开官方 GPIO 例程；
4. 跑通 LED；
5. 跑通 UART；
6. 跑通 I2C OLED；
7. 保存工程模板。

## 3. 第二阶段：板卡基础测试

目标：确认 PCB 硬件可用。

任务：

1. 测 7.4V、5V、3V3；
2. 读 ADC 电池电压；
3. 测 START/MODE 按键；
4. 测 LED_L/LED_M/LED_R；
5. 测 BUZZER；
6. 打印所有关键状态。

## 4. 第三阶段：运动底层

目标：确认 LQ/RQ 两个通道能稳定使用。

任务：

1. 先架空测试；
2. 小 PWM 起步；
3. 记录左右方向；
4. 找出最小可转 PWM；
5. 设置安全最大 PWM；
6. 写入方向系数。

## 5. 第四阶段：循迹输入

目标：把黑白状态变成偏差值。

任务：

1. 确认 5 路/8 路传感器顺序；
2. 记录黑线输出电平；
3. 计算 `line_error`；
4. OLED/串口显示传感器状态；
5. 处理丢线情况。

## 6. 第五阶段：闭环控制

目标：让小车稳定循迹。

任务：

1. 先调低速；
2. 先开环循迹；
3. 再调循迹 PID；
4. 若有编码器，再加入速度 PID；
5. 记录每次参数和现象。

## 7. 当前已有前期代码

```text
control-car/firmware/mspm0/src/control/pid.h
control-car/firmware/mspm0/src/control/pid.c
control-car/firmware/mspm0/src/control/line_follow.h
control-car/firmware/mspm0/src/control/line_follow.c
```

这些代码是平台无关的控制算法，可以先在本地 C 工程或 MSPM0 工程中复用。

## 8. 下一个最小交付

```text
1. 建 MSPM0 CCS Theia 工程；
2. 将 pid.c/h 和 line_follow.c/h 放入工程；
3. 写 board 层，把 pinmap.md 的网络名映射到 SysConfig 生成的真实引脚；
4. 写 app 状态机；
5. 写 UART/OLED 调试输出；
6. 按 bringup-plan.md 逐项测试。
```
