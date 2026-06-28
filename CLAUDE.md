# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

TI Cup 2025 全国大学生电子设计竞赛 **G题：电路模型探究装置** — MCU固件 + MATLAB仿真。

- 主控: **STM32F407VET6** @ 168MHz, FPU+DSP, 192KB SRAM
- 信号源: **AD9833** DDS (SPI1), 最高12.5MHz
- 显示: **ST7789** 240×240 TFT (SPI2+DMA)
- 采集: **ADC1** 2MSPS, 12-bit, TIM2触发, DMA双缓冲
- 输出: **DAC1** 200kSPS, 12-bit, TIM6触发, DMA循环
- 工具链: ARM GCC / Keil MDK (裸机HAL库, 无RTOS)

## Project Structure

```
/
├── stm32/Core/
│   ├── Inc/           # 头文件 (HAL驱动 + 功能模块)
│   │   ├── main.h            # 系统参数、引脚定义、全局状态结构体
│   │   ├── ad9833.h          # DDS信号源驱动
│   │   ├── adc_driver.h      # ADC采集驱动 (DMA+TIM触发)
│   │   ├── dac_driver.h      # DAC波形输出驱动 (DMA+TIM触发)
│   │   ├── st7789.h          # TFT显示驱动
│   │   ├── pid.h             # PID控制器 (增量式)
│   │   ├── goertzel.h        # Goertzel算法 (单频率DFT)
│   │   ├── filter_id.h       # 滤波器类型识别 (LPF/HPF/BPF/BSF)
│   │   ├── rlc_fit.h         # RLC参数拟合
│   │   └── waveform.h        # 波形复刻 (逆滤波器+谐波合成)
│   └── Src/           # 实现文件 (暂无)
│       ├── main.c            # 主程序、状态机、外设初始化
│       ├── ad9833.c          # AD9833驱动
│       ├── adc_driver.c      # ADC驱动
│       ├── dac_driver.c      # DAC驱动
│       ├── st7789.c          # ST7789显示驱动
│       ├── pid.c             # PID控制器
│       ├── goertzel.c        # Goertzel算法
│       ├── filter_id.c       # 滤波类型识别
│       ├── rlc_fit.c         # RLC参数拟合
│       └── waveform.c        # 波形复刻
├── matlab/
│   ├── transfer_function_analysis.m   # 已知模型电路传递函数分析
│   ├── goertzel_verification.m        # Goertzel算法验证 + STM32测试向量生成
│   ├── pid_design.m                   # PID控制器设计与仿真
│   ├── rlc_fitting.m                  # RLC参数拟合算法验证
│   └── waveform_replication.m         # 波形复刻逆滤波器仿真
├── 电路模型探究装置-赛题.md           # 赛题原文及评分标准
├── 电路模型探究装置-最终方案.md       # 最终方案文档
├── 电路模型探究装置-高分方案.md       # 高分方案文档 (已删除)
├── 第三轮多维度审核报告.md            # 审核报告 (已删除)
└── 电赛方案介绍.html                   # 方案展示页面
```

## State Machine

主控状态机流转: `CALIBRATE → IDLE → SWEEP / AUTO_AMP / LEARN / REPLICATE`

- **CALIBRATE**: ADC零点校准 + DAC中点设置 (上电自动)
- **IDLE**: 空闲显示, 等待按键
- **SWEEP**: 对数扫频 (100Hz~50kHz), Goertzel逐点提取幅相
- **AUTO_AMP**: PID闭环调节, 使输出稳定在目标Vpp
- **LEARN**: 自动扫频 → 滤波类型识别 → RLC参数拟合
- **REPLICATE**: 谐波分解 → 逆滤波补偿 → DAC/AD9833输出

## MATLAB Workflow

每个MATLAB脚本独立运行, 输出C语言格式的查找表/测试向量供STM32参考:
1. `transfer_function_analysis.m` — 验证已知模型传递函数, 生成频率-幅度查找表
2. `goertzel_verification.m` — 验证Goertzel算法精度, 输出测试信号数组
3. `pid_design.m` — Ziegler-Nichols整定, 仿真控制系统
4. `rlc_fitting.m` — 从频响数据反推RLC参数, 含解析法和局部优化
5. `waveform_replication.m` — 验证逆滤波器补偿效果

## Build & Development

- **STM32**: 使用STM32CubeMX生成HAL初始化代码, Keil MDK或ARM GCC编译
- **MATLAB**: 直接在 MATLAB 环境运行 `.m` 文件 (需 Control System Toolbox)
- 当前为裸机HAL库开发, 无RTOS, 无测试框架
