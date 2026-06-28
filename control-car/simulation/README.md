# 仿真工作区

本目录用于放置循迹小车控制算法的 MATLAB/Python 仿真材料。

## 后续可做内容

1. 速度 PID 阶跃响应仿真；
2. 转向 PID 参数扫描；
3. 循迹传感器权重模型；
4. 弯道降速策略仿真；
5. 轮径、轮距、速度误差对轨迹的影响分析。

## 建议数据来源

- `test-data/motor_pwm_speed.csv`
- `test-data/line_sensor_calibration.csv`
- `test-data/pid_step_response.csv`

仿真只用于辅助调参，最终参数必须在真实小车上复测。
