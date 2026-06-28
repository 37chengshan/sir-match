# MATLAB 仿真

这里放 G题专用 MATLAB 仿真。

## 后续迁移目标

原根目录 `matlab/` 下与 G题直接相关的脚本，后续应逐步迁移到这里：

```text
simulation/matlab/
├── transfer_function_analysis.m
├── goertzel_verification.m
├── pid_design.m
├── rlc_fitting.m
└── waveform_replication.m
```

## 仿真用途

- 验证已知模型传递函数；
- 生成 Goertzel 测试向量；
- 验证自动幅度控制参数；
- 验证 RLC 参数拟合；
- 验证 DAC 波形复现算法。

## 原则

- 与题目强相关的仿真放这里。
- 可多题复用的算法说明放 `common/algorithms/`。
