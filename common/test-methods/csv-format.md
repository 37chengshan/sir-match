# 串口 CSV 数据格式

## 1. 作用

统一 CSV 格式，方便把 MSPM0G3507 串口输出直接导入 MATLAB、Python 或 Excel 做图和报告。

## 2. G题频响 CSV

推荐表头：

```csv
time_ms,mode,freq_hz,vin_vpp,vout_vpp,gain,gain_db,phase_deg,amp_ctrl,error_code,note
```

示例：

```csv
1200,SWEEP,1000,2.000,1.840,0.920,-0.724,-12.3,128,0,known_rc
```

字段说明：

| 字段 | 含义 |
|---|---|
| time_ms | 系统时间 |
| mode | 当前模式 |
| freq_hz | 测试频率 |
| vin_vpp | 输入峰峰值 |
| vout_vpp | 输出峰峰值 |
| gain | 线性增益 |
| gain_db | dB 增益 |
| phase_deg | 相位差 |
| amp_ctrl | 数字电位器/PID输出 |
| error_code | 错误码 |
| note | 备注 |

## 3. 小车调试 CSV

推荐表头：

```csv
time_ms,mode,left_pwm,right_pwm,left_speed,right_speed,line_error,pid_turn,pid_speed,error_code
```

## 4. 串口输出原则

- 第一行输出表头；
- 每行一个采样点或一个扫频点；
- 不要混入中文日志；
- 错误信息用 error_code 表示；
- 大量数据导出时降低 OLED 刷新频率。

## 5. 报告使用

CSV 应用于：

- Bode 幅频图；
- 相频图；
- PID 收敛曲线；
- 误差分析表；
- 波形复现对比图。
