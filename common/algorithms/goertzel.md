# Goertzel 单频幅相提取

## 1. 作用

Goertzel 算法用于从一段采样数据中提取某一个目标频率的幅值和相位。相比完整 FFT，它更适合 G题这种逐点扫频场景。

典型应用：

```text
AD9833 设置频率 f
ADC 同步采 Vin / Vout
Goertzel 分别提取 Vin、Vout 在 f 点的幅值和相位
计算 |H| = |Vout| / |Vin|
计算 phase = phase_out - phase_in
```

## 2. 适用题目

- `G-circuit-model/`：频响测量、Bode 数据生成；
- 其他信号处理题：单频检测、幅相估计、锁频检测。

## 3. 输入输出

输入：

- 采样数组 `samples[]`；
- 点数 `n`；
- 目标频率 `target_freq_hz`；
- 采样率 `sample_rate_hz`。

输出：

- 实部；
- 虚部；
- 幅值；
- 相位角。

## 4. 推荐接口

```c
typedef struct {
    float real;
    float imag;
    float magnitude;
    float phase_deg;
} ComplexMeasure_t;

int goertzel_compute(
    const float *samples,
    int n,
    float target_freq_hz,
    float sample_rate_hz,
    ComplexMeasure_t *out
);
```

返回值建议：

```text
0  成功
-1 参数错误
-2 目标频率超过奈奎斯特频率
```

## 5. G题使用方式

不要只对 `Vout` 做 Goertzel。正确做法：

```text
Vin_samples  → Goertzel → Mag_in, Phase_in
Vout_samples → Goertzel → Mag_out, Phase_out
Gain = Mag_out / Mag_in
Phase = Phase_out - Phase_in
```

这样得到的才是被测电路的传递函数频响。

## 6. 采样窗口建议

| 项目 | 建议 |
|---|---|
| 采样周期 | 尽量覆盖整数个信号周期 |
| 采样点数 | 256 / 512 / 1024 优先 |
| 采样率 | 至少大于最高目标频率的 10 倍，最低不少于 3 倍 |
| 输入处理 | 先减去 ADC 偏置值，再进入算法 |
| 窗函数 | 第一版不用，后续可加 Hann 降低频谱泄漏 |

## 7. 常见坑

1. 没有减去 1.65V ADC 偏置，导致直流分量污染结果；
2. 采样窗口不稳定，导致相位跳变；
3. 只测 Vout，不测 Vin，导致相位没有参考；
4. 目标频率和实际 AD9833 输出频率不一致；
5. 采样率和算法里填写的 `sample_rate_hz` 不一致；
6. 高频段采样前端带宽不足，导致误判为电路衰减。

## 8. 最小验收

用函数信号源或 AD9833 输入 1kHz 正弦：

- 幅值误差 < 3%；
- 相位重复测量波动 < 5°；
- 频率切换到 5kHz、10kHz、50kHz 后趋势正常；
- Vin/Vout 同频同相信号时，相位差应接近 0°。

## 9. 放入题目工程的位置

```text
G-circuit-model/firmware/mspm0/Core/Algorithms/goertzel.c
G-circuit-model/firmware/mspm0/Core/Algorithms/goertzel.h
```
