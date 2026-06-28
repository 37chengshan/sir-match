# common/algorithms 通用算法库

这里放平台无关、题目可复用的算法说明和接口规范。算法文件只描述数学逻辑和接口，不绑定具体 ADC、DMA、GPIO、UART、屏幕驱动。

## 1. 总原则

算法层只处理数据：

```text
输入数组 / 参数 → 计算 → 输出结果
```

不要在算法层直接操作：

- ADC；
- DMA；
- Timer；
- UART；
- SPI；
- OLED / TFT；
- GPIO。

这些平台相关内容放到题目工程的 `Drivers/` 或 `App/`。

## 2. 当前算法索引

| 文件 | 算法 | 成熟度 | 主要服务题目 | 作用 |
|---|---|---|---|---|
| `goertzel.md` | Goertzel 单频 DFT | L1→L2 | G题 | 提取指定频点幅值和相位 |
| `pid.md` | PID 控制 | L1→L3 | G题、小车 | 自动调幅、速度闭环、转向闭环 |
| `fft-harmonic.md` | FFT/谐波分析 | L1 | G题 | 矩形波/周期波分解与复现 |
| `filter-identification.md` | 滤波器类型识别 | L0→L1 | G题 | 判断低通、高通、带通、带阻 |
| `rlc-fitting.md` | RLC 参数拟合 | L0→L1 | G题 | 从频响估算 f0、Q、R/L/C |
| `m-sequence-correlation.md` | m序列互相关 | L0 | G题冲高分 | 估计冲激响应 h(n) |

## 3. 推荐 C 接口风格

### 3.1 数据类型

```c
typedef struct {
    float magnitude;
    float phase_deg;
    float real;
    float imag;
} ComplexMeasure_t;

typedef struct {
    float kp;
    float ki;
    float kd;
    float out_min;
    float out_max;
    float integral;
    float last_error;
} PID_t;
```

### 3.2 Goertzel 接口

```c
int goertzel_compute(
    const float *samples,
    int n,
    float target_freq_hz,
    float sample_rate_hz,
    ComplexMeasure_t *out
);
```

### 3.3 PID 接口

```c
void pid_init(PID_t *pid, float kp, float ki, float kd, float out_min, float out_max);
float pid_update(PID_t *pid, float target, float measured, float dt_s);
void pid_reset(PID_t *pid);
```

### 3.4 滤波识别接口

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

## 4. 算法验收标准

| 算法 | 最小验收 |
|---|---|
| Goertzel | 对纯正弦幅值误差 < 3%，相位误差 < 5° |
| PID | 阶跃目标下无持续振荡，能稳定收敛 |
| FFT/谐波 | 能正确识别基波和前 5 个有效谐波 |
| 滤波识别 | 训练数据中能区分 LPF/HPF/BPF/BSF |
| RLC拟合 | 中心频率误差 < 10%，Q 值趋势正确 |
| m序列互相关 | 冲激响应趋势正确，可重构主要输出波形 |

## 5. 迁移到题目工程的位置

公共算法文档在 `common/algorithms/`，具体实现放到题目工程：

```text
G-circuit-model/firmware/mspm0/Core/Algorithms/
control-car/firmware/mspm0/Core/Algorithms/
```

## 6. 当前优先级

第一优先级：

1. `goertzel.md`
2. `pid.md`
3. `filter-identification.md`

第二优先级：

1. `rlc-fitting.md`
2. `fft-harmonic.md`

第三优先级：

1. `m-sequence-correlation.md`
