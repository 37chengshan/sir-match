# 测试数据工作区

本目录用于保存小车调试过程中的 CSV、截图、参数表和调参记录。

## 推荐文件

```text
motor_pwm_speed.csv           不同 PWM 下左右轮速度
line_sensor_calibration.csv   灰度/红外循迹传感器黑白标定
pid_step_response.csv         速度 PID 阶跃响应
line_follow_log.csv           循迹过程中的偏差、速度、PWM、电池电压
fault-log.md                  故障现象、原因、解决方法
```

## CSV 字段建议

### motor_pwm_speed.csv

```csv
time_ms,left_pwm,right_pwm,left_speed_mm_s,right_speed_mm_s,battery_v,note
```

### line_sensor_calibration.csv

```csv
sensor_id,white_value,black_value,threshold,note
```

### pid_step_response.csv

```csv
time_ms,target_speed,left_speed,right_speed,left_pwm,right_pwm,kp,ki,kd
```

调 PID 时不要只凭感觉，要留下数据，方便复盘和迁移到下一题。
