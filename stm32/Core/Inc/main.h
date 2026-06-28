/**
 * @file    main.h
 * @brief   电路模型探究装置 - 主程序头文件
 * @note    STM32F407VET6 | 电赛G题 | 168MHz | FPU+DSP | 192KB SRAM
 *
 * 硬件引脚分配:
 *   ── SPI1 (AD9833) ──
 *     PA5(SCK)  - 时钟
 *     PA7(MOSI) - 数据
 *     PB2       - FSYNC (软件片选)
 *   ── SPI2 (ST7789 TFT) ──
 *     PB13(SCK)  - 时钟
 *     PB15(MOSI) - 数据
 *     PB12       - CS  (片选)
 *     PB14       - DC  (数据/命令)
 *     PA6        - RST (复位)
 *     PB3        - BLK (背光PWM, TIM2_CH2)
 *   ── ADC1 ──
 *     PA0(CH0)   - 信号输入 (经运放调理)
 *     PA1(CH1)   - 参考电压检测
 *   ── DAC1 ──
 *     PA4        - 波形复刻输出 (经运放缓冲)
 *   ── 按键 ──
 *     PC0        - 学习键
 *     PC1        - 设置键
 *     PC2        - 启动键
 *   ── 其他 ──
 *     PC13       - LED指示灯
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ======================== 系统参数 ======================== */
#define SYS_CLK_FREQ        168000000UL  /* 系统时钟 168MHz */
#define HCLK_FREQ           168000000UL  /* AHB时钟 */
#define APB1_TIMER_FREQ      84000000UL  /* APB1定时器 84MHz (PCLK1×2) */
#define APB2_TIMER_FREQ     168000000UL  /* APB2定时器 168MHz (PCLK2×2) */

/* ======================== 信号源参数 ======================== */
#define FREQ_MIN_HZ         10           /* 最低输出频率 */
#define FREQ_MAX_HZ         12500000UL   /* AD9833最高频率 12.5MHz */
#define FREQ_STEP_HZ        100          /* 频率步长 100Hz */
#define DEFAULT_FREQ_HZ     1000         /* 默认频率 1kHz */
#define VPP_TARGET_DEFAULT  2.0f         /* 默认目标峰峰值 2V */
#define VPP_MIN             0.1f         /* 最小输出幅度 */
#define VPP_MAX             3.5f         /* 最大输出幅度 */
#define DAC_VREF            3.3f         /* DAC参考电压 */

/* ======================== ADC参数 ======================== */
#define ADC_RESOLUTION      4096         /* 12位ADC */
#define ADC_VREF            3.3f         /* ADC参考电压 */
#define ADC_OVERSAMPLE      16           /* 过采样倍数 (等效14-bit) */
#define ADC_SAMPLE_RATE     2000000      /* ADC采样率 2MSPS (F407可达2.4M) */
#define ADC_BUF_SIZE        1024         /* ADC DMA缓冲区大小 (双通道×512) */

/* ======================== DAC参数 ======================== */
#define DAC_RESOLUTION      4096         /* 12位DAC */
#define DAC_BUF_SIZE        512          /* DAC DMA缓冲区大小 */
#define DAC_SAMPLE_RATE     200000       /* DAC更新率 200kSPS (50kHz×4) */

/* ======================== 采集参数 ======================== */
#define SAMPLES_PER_FREQ    512          /* 每个频率点采样数(用于Goertzel) */
#define SWEEP_POINTS_MIN    10           /* 扫频最少点数 */
#define SWEEP_POINTS_MAX    200          /* 扫频最多点数 */
#define SWEEP_FREQ_LOW      100          /* 扫频最低频率 100Hz */
#define SWEEP_FREQ_HIGH     50000        /* 扫频最高频率 50kHz */
#define FFT_SIZE            1024         /* FFT点数(用于谐波分析) */

/* ======================== 按键定义 (F407VET6) ======================== */
#define KEY_LEARN_PIN       GPIO_PIN_0
#define KEY_LEARN_PORT      GPIOC
#define KEY_SET_PIN         GPIO_PIN_1
#define KEY_SET_PORT        GPIOC
#define KEY_START_PIN       GPIO_PIN_2
#define KEY_START_PORT      GPIOC
#define KEY_DEBOUNCE_MS     50           /* 按键消抖时间 */

/* ======================== LED定义 ======================== */
#define LED_PIN             GPIO_PIN_13
#define LED_PORT            GPIOC

/* ======================== 滤波器类型 ======================== */
#define FILTER_TYPE_UNKNOWN     0
#define FILTER_TYPE_LPF         1   /* 低通 */
#define FILTER_TYPE_HPF         2   /* 高通 */
#define FILTER_TYPE_BPF         3   /* 带通 */
#define FILTER_TYPE_BSF         4   /* 带阻/陷波 */

/* ======================== 波形类型 ======================== */
#define WAVE_TYPE_SINE          0
#define WAVE_TYPE_SQUARE        1
#define WAVE_TYPE_TRIANGLE      2
#define WAVE_TYPE_CUSTOM        3

/* ======================== 工作模式 ======================== */
#define MODE_IDLE               0   /* 空闲 */
#define MODE_SWEEP              1   /* 扫频测量 */
#define MODE_AUTO_AMP           2   /* 自动幅度控制 */
#define MODE_LEARN              3   /* 自学习模式 */
#define MODE_REPLICATE          4   /* 波形复刻模式 */
#define MODE_CALIBRATE          5   /* 自校准模式 */

/* ======================== PI常量 ======================== */
#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif
#ifndef M_2PI
#define M_2PI   6.28318530717958647692f
#endif

/* ======================== 函数声明 ======================== */
void SystemClock_Config(void);
void Error_Handler(void);
void delay_us(uint32_t us);
uint32_t get_tick_ms(void);

/* ======================== 外设句柄(全局) ======================== */
extern SPI_HandleTypeDef  hspi1;     /* AD9833 */
extern SPI_HandleTypeDef  hspi2;     /* ST7789 TFT */
extern ADC_HandleTypeDef  hadc1;     /* 信号采集 */
extern DAC_HandleTypeDef  hdac;      /* 波形输出 */
extern TIM_HandleTypeDef  htim2;     /* ADC触发定时器 */
extern TIM_HandleTypeDef  htim3;     /* BLK PWM / 系统滴答 */
extern DMA_HandleTypeDef  hdma_adc1; /* ADC DMA */
extern DMA_HandleTypeDef  hdma_dac1; /* DAC DMA */
extern DMA_HandleTypeDef  hdma_spi2_tx; /* ST7789 SPI DMA */

/* ======================== 全局状态 ======================== */
typedef struct {
    /* 工作状态 */
    uint8_t  mode;              /* 当前工作模式 */
    uint32_t freq_hz;           /* 当前输出频率 */
    float    target_vpp;        /* 目标峰峰值 */
    float    measured_vpp;      /* 实测峰峰值 */
    float    measured_phase;    /* 实测相位(度) */

    /* 建模结果 */
    uint8_t  filter_type;       /* 识别的滤波器类型 */
    float    cutoff_freq;       /* 截止频率(Hz) */
    float    quality_factor;    /* 品质因数Q */
    float    R_val;             /* 电阻值(Ω) */
    float    L_val;             /* 电感值(H) */
    float    C_val;             /* 电容值(F) */

    /* 波形复刻 */
    uint8_t  wave_type;         /* 波形类型 */
    float    wave_duty;         /* 占空比(矩形波: 0.1~0.5) */

    /* 状态标志 */
    uint8_t  sweep_ready;       /* 扫频数据就绪 */
    uint8_t  learn_done;        /* 学习完成标志 */
    uint8_t  replicate_ready;   /* 复刻就绪 */
    uint8_t  calibration_done;  /* 校准完成 */

    /* 运行时数据 */
    uint32_t tick_ms;           /* 系统运行毫秒 */
    uint32_t loop_count;        /* 主循环计数 */
} SystemState_t;

extern SystemState_t g_sys;

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
