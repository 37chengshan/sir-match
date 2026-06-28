# FFT / 谐波分析

## 1. 作用

FFT / 谐波分析用于把周期信号分解成多个频率分量。G题波形复现中，矩形波和其他周期信号可以拆成基波与若干谐波，再分别乘以模型频率响应，最后合成输出波形。

## 2. G题使用路线

```text
输入周期信号 x(t)
    ↓
提取基波频率 f0
    ↓
计算有效谐波：f0, 3f0, 5f0...
    ↓
每个谐波乘以 H(jw)
    ↓
合成 y(t)
    ↓
DAC 查表输出
```

## 3. 第一版建议

不要一开始做完整大 FFT。MSPM0G3507 资源有限，第一版优先做“已知周期信号谐波合成”：

- 正弦波：只处理 1 个频率分量；
- 矩形波：只取前 5~10 个奇次谐波；
- 三角波：谐波衰减快，取前 5 个即可；
- 任意周期波：后续再考虑 FFT。

## 4. 矩形波谐波公式

50% 占空比理想矩形波只含奇次谐波：

```text
x(t) = 4A/pi * [sin(wt) + 1/3 sin(3wt) + 1/5 sin(5wt) + ...]
```

如果占空比不是 50%，谐波系数需要按傅里叶级数重新计算。

## 5. 推荐接口

```c
typedef struct {
    float freq_hz;
    float amplitude;
    float phase_deg;
} Harmonic_t;

int harmonic_square_wave(
    float fundamental_hz,
    float amplitude,
    int max_order,
    Harmonic_t *out,
    int max_count
);

int waveform_synthesize(
    const Harmonic_t *harmonics,
    int harmonic_count,
    float sample_rate_hz,
    float *out_samples,
    int n
);
```

## 6. 频响补偿

每个谐波都要乘以模型频响：

```text
Y(kf0) = X(kf0) * H(j*k*w0)
```

如果目标是让探究装置复现未知电路输出，则按学习得到的 H 直接合成输出。

如果目标是补偿某个输出链路，则需要除以输出链路响应，但这会放大噪声，必须限幅。

## 7. 常见坑

1. 谐波频率超过系统带宽，强行复现会失真；
2. DAC 更新率不足，矩形波边沿不可能完全理想；
3. 输出滤波太强会削掉高频谐波；
4. 谐波叠加后幅值可能超出 DAC 范围，必须归一化；
5. 没有考虑相位，导致复现波形位置错位。

## 8. 最小验收

- 正弦波复现趋势正确；
- 8kHz 矩形波可用前 5 个奇次谐波合成出基本形状；
- 示波器能看到复现波形与理论趋势一致；
- 输出不削顶、不明显超出 DAC 范围。
