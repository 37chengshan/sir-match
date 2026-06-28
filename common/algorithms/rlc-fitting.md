# RLC 参数拟合

## 1. 作用

RLC 参数拟合用于从扫频数据中估算未知 RLC 电路的中心频率、带宽、Q 值，以及可能的 R、L、C 参数趋势。

G题中它属于“未知黑盒学习建模”的核心能力。

## 2. 输入输出

输入：

```text
freq_hz[]
gain_db[]
phase_deg[]
n
```

输出：

```text
滤波类型
中心频率 f0
-3dB 带宽 BW
品质因数 Q = f0 / BW
参数估算结果
拟合误差
```

## 3. 基本估算方法

### 3.1 中心频率

带通电路：

```text
f0 = 增益最大点对应频率
```

带阻电路：

```text
f0 = 增益最小点对应频率
```

### 3.2 Q 值

```text
Q = f0 / BW
```

其中 `BW` 为 -3dB 带宽。

### 3.3 LC 关系

```text
f0 = 1 / (2π√(LC))
```

如果已知 L，可估算 C；如果已知 C，可估算 L。

## 4. 第一版实现策略

第一版不要追求完整非线性优化，先做解析估算：

1. 对 gain_db 做简单平滑；
2. 找峰值或谷值频率；
3. 找左右 -3dB 交点；
4. 计算带宽和 Q；
5. 根据已知训练黑盒参数反推 L 或 C；
6. 串口输出结果。

## 5. 进阶拟合

如果第一版稳定，再做最小二乘：

```text
minimize Σ |H_measured(f_i) - H_model(f_i, params)|^2
```

注意：MSPM0G3507 上不建议做复杂浮点优化。复杂拟合可放在 MATLAB/Python，单片机只做解析估算和简单局部搜索。

## 6. 推荐接口

```c
typedef struct {
    float f0_hz;
    float bandwidth_hz;
    float q;
    float estimated_r;
    float estimated_l;
    float estimated_c;
    float fit_error;
} RLCFitResult_t;

int rlc_fit_from_bode(
    const float *freq_hz,
    const float *gain_db,
    const float *phase_deg,
    int n,
    RLCFitResult_t *out
);
```

## 7. 常见坑

1. 扫频步长太大，f0 不准；
2. 峰值附近没有局部细扫；
3. 电感实际误差大，标称值和实测值差很多；
4. 面包板寄生电容影响高频段；
5. 运放/ADC 前端带宽影响被误当作黑盒特性；
6. 只用幅频不用相频，某些电路容易误判。

## 8. 最小验收

训练黑盒：

```text
R = 2kΩ
L = 10mH
C = 10nF
```

验收目标：

- 能找到中心频率附近；
- Q 值趋势正确；
- 改变 L 或 C 后，f0 变化方向正确；
- 报告中给出理论值、测量值、误差百分比。
