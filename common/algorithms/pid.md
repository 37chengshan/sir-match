# PID 控制器

## 1. 作用

PID 用于把系统输出稳定到目标值。电赛中最常见的用途：

- G题：自动调节 AD9833 输出幅度，使被测点稳定到目标 Vpp；
- 小车：速度闭环、转向闭环、循迹偏差修正；
- 电源题：电压/电流闭环；
- 温控题：温度闭环。

## 2. 控制形式

优先使用位置式 PID：

```text
error = target - measured
integral += error * dt
output = kp * error + ki * integral + kd * (error - last_error) / dt
```

对电机速度控制，也可以使用增量式 PID，但第一版统一用位置式，方便调试。

## 3. 推荐接口

```c
typedef struct {
    float kp;
    float ki;
    float kd;
    float out_min;
    float out_max;
    float integral_min;
    float integral_max;
    float integral;
    float last_error;
    uint8_t first_run;
} PID_t;

void pid_init(PID_t *pid, float kp, float ki, float kd,
              float out_min, float out_max);

float pid_update(PID_t *pid, float target, float measured, float dt_s);

void pid_reset(PID_t *pid);
```

## 4. 必须带的保护

| 保护 | 原因 |
|---|---|
| 输出限幅 | 数字电位器、电机 PWM、DAC 都有上下限 |
| 积分限幅 | 防止长时间误差造成积分爆炸 |
| 首次运行处理 | 避免 D 项第一帧突变 |
| dt 检查 | dt 为 0 或异常会导致 D 项爆炸 |
| 死区 | 小误差时避免来回抖动 |

## 5. G题自动调幅建议

G题目标是让输出峰峰值稳定到 2Vpp。控制对象可以是数字电位器值或可变增益放大器控制量。

建议第一版不用复杂 PID，先用分段逼近：

```text
误差很大：大步调节
误差中等：小步调节
误差很小：保持
```

跑稳后再切 PID：

```text
kp: 先从小值开始
ki: 慢慢加，消除静差
kd: 第一版可以为 0
```

## 6. 小车 PID 建议

小车一般分两层：

```text
速度 PID：目标速度 - 编码器速度 → 左右轮 PWM
转向 PID：循迹偏差 → 左右轮差速修正
```

不要一开始把速度 PID 和转向 PID 混成一个大 PID。先单独让左右轮速度闭环稳定，再叠加循迹偏差。

## 7. 调参顺序

1. `ki = 0, kd = 0`，只调 `kp`，直到能响应但不剧烈振荡；
2. 加一点 `ki`，消除稳态误差；
3. 如有明显超调，再小量加 `kd`；
4. 加输出限幅和积分限幅；
5. 记录最终参数和测试条件。

## 8. 最小验收

G题：

- 目标 2Vpp；
- 频率切换后能重新收敛；
- 稳态误差尽量 < ±5%；
- 不出现持续振荡。

小车：

- 轮子空载速度能稳定；
- 左右轮同目标速度时偏差可接受；
- 循迹时不明显蛇形失控。

## 9. 放入题目工程的位置

```text
G-circuit-model/firmware/mspm0/Core/Algorithms/pid.c
control-car/firmware/mspm0/Core/Algorithms/pid.c
```
