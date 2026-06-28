# 测试数据

这里放 G题测试数据。

## 推荐结构

```text
test-data/
├── README.md
├── csv/             串口导出的扫频数据
├── scope-images/    示波器截图
├── calibration/     ADC零点、幅度校准、相位校准
└── notes/           测试记录
```

## CSV 推荐格式

```csv
freq_hz,vin_vpp,vout_vpp,gain,gain_db,phase_deg,note
1000,2.000,1.840,0.920,-0.724,-12.3,known_rc
```

## 必须保存的数据

- 已知 RC 模型理论曲线；
- 已知 RC 模型实测曲线；
- 未知 RLC 黑盒扫频数据；
- 自动幅度控制收敛数据；
- DAC 波形复现示波器截图；
- 误差分析表。
