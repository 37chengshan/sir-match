# 电源设计与抗干扰说明

> 目标：解决小车最常见的问题——电机一转，开发板复位；传感器误判；串口/OLED 异常。

## 1. 原理图中的电源链路

```text
电池 BAT / P3
→ D1 SS34
→ SW1 总开关
→ 7.4V 母线
→ U6 降压
→ 5V
```

板上主要电源：

| 电源 | 来源 | 用途 |
|---|---|---|
| `7.4V` | 2 节锂电池 | 电机驱动、电机电源 |
| `5V` | U6 降压输出 | 超声波、OLED/外设、扩展模块 |
| `3V3` | MSPM0/板载 3.3V | MCU IO、nRF24L01、逻辑信号 |
| `GND` | 公共地 | 所有模块共同参考地 |

## 2. ADC 电池电压采样

原理图采用三只 10k 电阻分压：

```text
7.4V → 10k → 10k → ADC → 10k → GND
```

换算公式：

```text
V_adc = V_bat / 3
V_bat = V_adc * 3
```

代码中建议：

```c
float battery_voltage = adc_voltage * 3.0f * battery_calibration_gain;
```

`battery_calibration_gain` 由万用表实测校准。

## 3. 电机干扰处理

### 3.1 必做

1. 电机供电和逻辑供电分清；
2. 所有模块必须共地；
3. 电机线尽量短；
4. 电池、降压模块、开发板、传感器线都要固定；
5. 电机驱动附近加 470uF~1000uF 电解电容；
6. 5V 输出处加 100uF~470uF 电容；
7. MCU 旁边保持已有去耦电容。

### 3.2 建议

- 电机线和 I2C/循迹线分开走线；
- OLED/I2C 线不要太长；
- 循迹传感器不要和电机线绑在一起；
- 如果串口乱码，先检查 GND 和电源纹波。

## 4. 低电压保护建议

2 节 18650 标称 7.4V，满电约 8.4V。训练阶段建议：

| 电池电压 | 状态 | 软件动作 |
|---:|---|---|
| > 7.2V | 正常 | 允许正常跑车 |
| 6.8V~7.2V | 偏低 | OLED 提示，限制最大 PWM |
| < 6.8V | 低电压 | 停车、蜂鸣器提示 |

阈值按电池类型和实验室要求调整。

## 5. 上电自检建议

固件启动后先做：

1. 读取 ADC 电池电压；
2. 检查电压是否低于阈值；
3. OLED 显示电池电压；
4. 蜂鸣器提示启动完成；
5. 默认进入 IDLE，电机输出为 0。

伪代码：

```c
void app_power_on_check(void) {
    motor_stop_all();
    float vbat = battery_read_voltage();
    oled_show_battery(vbat);

    if (vbat < BATTERY_STOP_VOLTAGE) {
        app_set_state(CAR_STATE_ERROR_LOW_BATTERY);
        buzzer_beep_error();
    } else {
        app_set_state(CAR_STATE_IDLE);
        buzzer_beep_ok();
    }
}
```

## 6. 禁止事项

- 不要让电机从开发板 5V 引脚取电；
- 不要让舵机从开发板 5V 引脚取大电流；
- 不要带电插拔主控板；
- 不要在没有电压检查的情况下直接落地跑车；
- 不要把 5V 传感器输出直接接入 3.3V IO，除非确认输出兼容。
