# MSPM0G3507 固件工程

这里放 G题「电路模型探究装置」的 MSPM0G3507 固件工程。

## 目标工程结构

```text
firmware/mspm0/
├── README.md
├── Core/
│   ├── App/
│   │   └── app_state_machine.c
│   ├── Drivers/
│   │   ├── ad9833_mspm0.c
│   │   ├── adc_dual_dma.c
│   │   ├── dac_wave.c
│   │   ├── display_oled.c
│   │   └── mcp41010.c
│   ├── Algorithms/
│   │   ├── goertzel.c
│   │   ├── filter_id.c
│   │   └── rlc_fit.c
│   └── Config/
│       └── board_config.h
└── Tools/
    └── serial_csv_logger.py
```

## 开发环境

- Code Composer Studio Theia
- MSPM0 SDK
- SysConfig
- XDS110 或板载调试器

## 注意

- 不使用 ST-LINK。
- 新代码以 MSPM0G3507 为准。
