# 滤波器类型识别

## 1. 作用

根据扫频得到的幅频/相频数据，判断未知电路属于：

- 低通 LPF；
- 高通 HPF；
- 带通 BPF；
- 带阻 BSF；
- 未知/异常。

## 2. 输入数据

推荐输入：

```text
freq_hz[]      频率数组
gain_db[]      增益 dB
phase_deg[]    相位，第一版可选
n              点数
```

G题里这些数据来自：

```text
AD9833 扫频 → ADC采 Vin/Vout → Goertzel → |H| 和 phase
```

## 3. 基本判据

| 类型 | 幅频特征 | 相频特征 |
|---|---|---|
| 低通 | 低频增益高，高频衰减 | 相位一般随频率下降 |
| 高通 | 低频衰减，高频增益高 | 相位低频变化明显 |
| 带通 | 中间有峰值，两侧衰减 | 峰值附近相位快速变化 |
| 带阻 | 中间有谷值，两侧较高 | 谷值附近相位可能突变 |

## 4. 第一版算法

第一版不要复杂机器学习，直接用规则法：

```text
low_avg  = 低频前 20% 点平均增益
mid_peak = 全部频点最大增益
mid_min  = 全部频点最小增益
high_avg = 高频后 20% 点平均增益

if low_avg - high_avg > threshold:
    lowpass
elif high_avg - low_avg > threshold:
    highpass
elif mid_peak - max(low_avg, high_avg) > threshold:
    bandpass
elif min(low_avg, high_avg) - mid_min > threshold:
    bandstop
else:
    unknown
```

推荐阈值第一版取 6dB，后续根据实测调整。

## 5. 推荐接口

```c
typedef enum {
    FILTER_UNKNOWN = 0,
    FILTER_LOWPASS,
    FILTER_HIGHPASS,
    FILTER_BANDPASS,
    FILTER_BANDSTOP
} FilterType_t;

FilterType_t filter_identify(
    const float *freq_hz,
    const float *gain_db,
    const float *phase_deg,
    int n
);
```

## 6. G题报告建议

报告里不要只写“识别为带通”，要同时给出：

- 幅频曲线；
- 相频曲线；
- 低频平均增益；
- 高频平均增益；
- 峰值/谷值频率；
- 判断阈值；
- 误差来源。

## 7. 常见坑

1. 扫频点太少，峰值/谷值被跳过；
2. 高频前端衰减被误认为被测电路衰减；
3. 输入 Vin 幅度本身随频率变化，但只看 Vout；
4. 没有做 dB 归一化；
5. 面包板寄生参数导致高频异常。

## 8. 最小验收

用 4 个训练黑盒测试：

- RC 低通；
- RC 高通；
- RLC 带通；
- RLC 带阻。

识别准确率至少达到 3/4，才进入参数拟合阶段。
