# PID 默认参数与调参记录

> 起跑参数来自 `参考文件-已实现案例/【小车】02_DC_MOTOR_PID_3/motor.c` 中已验证的增量式 PI 实现。
> 真实参数必须根据车重、轮径、地面材质重新整定。

## 1. PID 类型

**增量式 PI**（参考案例实际实现）：

```
error = target_speed - measured_speed
duty += kp * (error - last_error) + ki * error
```

特点：
- 输出直接累加到 PWM 占空比，无需存储"绝对位置"
- 天然抗积分饱和（没有长时积分累积）
- 适合嵌入式定时中断驱动

## 2. 控制结构

```
循迹偏差 line_error
→ 转向 PID（输出左右轮速度差）
→ 左右轮目标速度

左右轮目标速度
→ 左右轮速度 PID
→ 左右轮 PWM 输出
```

## 3. 起步参数

来自参考案例实测：

| 控制器 | kp | ki | kd | 说明 |
|--------|---:|---:|---:|------|
| 左轮速度 PID | **0.5** | **0.4** | 0.0 | 先 PI，暂不加 D |
| 右轮速度 PID | **0.5** | **0.4** | 0.0 | 与左轮同参数起跑 |
| 循迹转向 PID | 1.2 | 0.0 | 0.1 | 先 P+D，通常不加 I |

## 4. 控制周期与限幅

| 参数 | 值 | 说明 |
|------|---:|------|
| 控制频率 | **1 kHz** | TIMA0, 40MHz/40000 |
| PWM 周期 | 4000 | 40MHz/4000 = 10kHz |
| PWM 最大占空比 | 4000 | 对应 100% |
| 起步基础速度 | 125 mm/s | 低速安全起跑 |
| 稳定后基础速度 | 200~300 mm/s | 参考案例用 300 |
| 转向修正上限 | 基础速度的 50% | 防止急弯翻车 |

## 5. PID 结构体（建议）

```c
typedef struct {
    float kp;
    float ki;
    float kd;
    float setpoint;
    float integral;
    float prev_error;
    float output;
    float output_min;
    float output_max;
    float integral_limit;
} pid_t;

void pid_init(pid_t *pid, float kp, float ki, float kd,
              float out_min, float out_max);
void pid_reset(pid_t *pid);
float pid_update(pid_t *pid, float measured, float dt);
void pid_set_gains(pid_t *pid, float kp, float ki, float kd);
```

## 6. 调参顺序

1. 不循迹，只测左右轮编码器速度
2. 先调速度 P，让速度跟上目标但不振荡
3. 加少量 I，减少稳态误差（参考 ki=0.4）
4. 暂不加 D
5. 低速跑开环循迹（参考 125 mm/s）
6. 调循迹 Kp，让小车能回到线上
7. 加 Kd 减少左右摇摆
8. 最后逐步提高基础速度

## 7. 常见现象

| 现象 | 可能原因 | 调整 |
|------|---------|------|
| 左右摆动大 | 循迹 Kp 太大或速度太快 | 降 Kp 或降基础速度 |
| 过弯冲出线 | 基础速度太高，Kd 不够 | 降速，适当加 Kd |
| 直线偏航 | 左右轮速度不一致 | 先调速度环和方向系数 |
| 反应很慢 | Kp 太小 | 增大 Kp |
| 速度超调振荡 | Kp 太大或 Ki 太大 | 先降 Kp，再调 Ki |
| 长时间偏一边 | 机械偏差或稳态误差 | 检查轮子，加少量 I |
| PWM 输出不变 | 积分未生效 | 检查 ki > 0 和积分限幅 |

## 8. 调参记录模板

```text
日期：
场地：
轮胎材质：
电池电压：
基础速度 (mm/s)：
左轮 kp/ki/kd：
右轮 kp/ki/kd：
循迹 kp/ki/kd：
现象：
修改：
结果：
```
