/**
 * @file    main.h
 * @brief   电路模型探究装置 - 主程序头文件
 * @note    STM32F401CCU6 | 电赛G题
 *
 * 硬件引脚分配:
 *   SPI1 (共享总线):
 *     PA5(SCK)  - 共享时钟
 *     PA7(MOSI) - 共享数据
 *     PA4(FSYNC) - AD9833 片选
 *     PB0       - ST7789 TFT 片选(CS)
 *     PB1       - ST7789 数据/命令(DC)
 *     PA6       - ST7789 复位(RST)
 *     PB3       - ST7789 背光(BLK, PWM控制)
 *   ADC1_CH0:       PA0 (信号输入)
 *   ADC1_CH1:       PA1 (参考电压检测)
 *   KEY:            PB12(学习), PB13(设置), PB14(启动)
 *   LED:            PC13
 *   DAC_OUT:        PA4 (复用为DAC输出, 与AD9833分时)
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
#define SYS_CLK_FREQ        84000000UL   /* 系统时钟 84MHz */
#define APB1_TIMER_FREQ     84000000UL   /* APB1定时器时钟 */
#define APB2_TIMER_FREQ     84000000UL   /* APB2定时器时钟 */

/* ======================== 信号源参数 ======================== */
#define FREQ_MIN_HZ         10           /* 最低输出频率 */
#define FREQ_MAX_HZ         12500000UL   /* AD9833最高频率 12.5MHz */
#define FREQ_STEP_HZ        100          /* 频率步长 100Hz */
#define DEFAULT_FREQ_HZ     1000         /* 默认频率 1kHz */
#define VPP_TARGET_DEFAULT  2.0f         /* 默认目标峰峰值 2V */
#define VPP_MIN             0.1f         /* 最小输出幅度 */
#define VPP_MAX             3.5f         /* 最大输出幅度(受限于供电) */

/* ======================== ADC参数 ======================== */
#define ADC_RESOLUTION      4096         /* 12位ADC */
#define ADC_VREF            3.3f         /* ADC参考电压 */
#define ADC_OVERSAMPLE      16           /* 过采样倍数 */
#define ADC_SAMPLE_RATE     1000000      /* ADC采样率 1MSPS */
#define ADC_BUF_SIZE        1024         /* ADC DMA缓冲区大小 */

/* ======================== 采集参数 ======================== */
#define SAMPLES_PER_FREQ    256          /* 每个频率点采样数(用于Goertzel) */
#define SWEEP_POINTS_MIN    10           /* 扫频最少点数 */
#define SWEEP_POINTS_MAX    200          /* 扫频最多点数 */
#define SWEEP_FREQ_LOW      100          /* 扫频最低频率 100Hz */
#define SWEEP_FREQ_HIGH     50000        /* 扫频最高频率 50kHz */

/* ======================== 按键定义 ======================== */
#define KEY_LEARN_PIN       GPIO_PIN_12
#define KEY_LEARN_PORT      GPIOB
#define KEY_SET_PIN         GPIO_PIN_13
#define KEY_SET_PORT        GPIOB
#define KEY_START_PIN       GPIO_PIN_14
#define KEY_START_PORT      GPIOB

#define KEY_DEBOUNCE_MS     50           /* 按键消抖时间 */

/* ======================== LED定义 ======================== */
#define LED_PIN             GPIO_PIN_13
#define LED_PORT            GPIOC

/* ======================== OLED参数 ======================== */
#define OLED_I2C_ADDR       0x78         /* SSD1306 I2C地址 */
#define OLED_WIDTH          128
#define OLED_HEIGHT         64

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
extern SPI_HandleTypeDef  hspi1;
extern I2C_HandleTypeDef  hi2c1;
extern ADC_HandleTypeDef  hadc1;
extern TIM_HandleTypeDef  htim2;
extern TIM_HandleTypeDef  htim3;
extern DMA_HandleTypeDef  hdma_adc1;

/* ======================== 全局状态 ======================== */
typedef struct {
    uint8_t  mode;              /* 当前工作模式 */
    uint32_t freq_hz;           /* 当前输出频率 */
    float    target_vpp;        /* 目标峰峰值 */
    float    measured_vpp;      /* 实测峰峰值 */
    float    measured_phase;    /* 实测相位(度) */
    uint8_t  filter_type;       /* 识别的滤波器类型 */
    float    cutoff_freq;       /* 截止频率 */
    float    quality_factor;    /* 品质因数Q */
    float    R_val, L_val, C_val; /* RLC参数 */
    uint8_t  wave_type;         /* 波形类型 */
    uint8_t  sweep_ready;       /* 扫频数据就绪 */
    uint8_t  learn_done;        /* 学习完成标志 */
} SystemState_t;

extern SystemState_t g_sys;

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
