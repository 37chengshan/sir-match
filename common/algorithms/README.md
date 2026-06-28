# 通用算法库

这里放电赛不同题目可复用的算法说明、接口规范和后续代码。

## 当前优先算法

| 算法 | 用途 | 当前题目 |
|---|---|---|
| Goertzel | 单频点幅值和相位提取 | G题电路模型探究装置 |
| PID | 自动幅度控制、电机控制、闭环调节 | G题、控制类题 |
| FFT/谐波分析 | 矩形波、周期波分解 | G题波形复现 |
| 滤波器类型识别 | 低通/高通/带通/带阻判断 | G题建模 |
| RLC参数拟合 | 从频响估算 R、L、C、Q | G题建模 |
| m序列互相关 | 时域系统辨识、冲激响应估计 | G题冲高分 |

## 推荐接口风格

通用算法尽量做到和平台无关：

```c
// 输入 float 数组，输出幅值和相位
void goertzel_compute(
    const float *samples,
    int n,
    float target_freq,
    float sample_rate,
    float *magnitude,
    float *phase_deg
);
```

平台相关内容，例如 ADC、DMA、定时器、UART，不放在算法文件里，由题目工程的 `Drivers/` 负责。

## 后续建议文件

```text
common/algorithms/
├── README.md
├── goertzel.md
├── pid.md
├── fft-harmonic.md
├── filter-identification.md
├── rlc-fitting.md
└── m-sequence-correlation.md
```
