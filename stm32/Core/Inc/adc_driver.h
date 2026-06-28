/**
 * @file    adc_driver.h
 * @brief   ADC采集驱动 - 信号采集与电压测量
 * @note    使用ADC1 + DMA + TIM触发
 *          PA0: 信号输入(经运放调理)
 *          PA1: 参考电压检测
 *
 * 信号调理链:
 *   被测电路输出 -> OPA2134(增益+偏置) -> PA0(ADC)
 *   偏置到VREF/2 = 1.65V, 使交流信号在0~3.3V范围内
 */

#ifndef __ADC_DRIVER_H
#define __ADC_DRIVER_H

#include "main.h"

/* ======================== ADC配置 ======================== */
#define ADC_CHANNELS        2           /* 使用2个ADC通道 */
#define ADC_DMA_BUF_SIZE    (ADC_BUF_SIZE * ADC_CHANNELS)

/* ======================== 电压转换 ======================== */
#define ADC_TO_VOLTAGE(x)   ((float)(x) * ADC_VREF / ADC_RESOLUTION)
#define VOLTAGE_OFFSET      1.65f       /* 偏置电压(信号调理后) */

/* ======================== 数据结构 ======================== */

/* 采集结果 */
typedef struct {
    float    dc_offset;         /* 直流偏置(V) */
    float    ac_rms;            /* 交流有效值(V) */
    float    vpp;               /* 峰峰值(V) */
    float    freq_est;          /* 估算频率(Hz) */
    uint32_t raw_peak;          /* 原始峰值 */
    uint32_t raw_valley;        /* 原始谷值 */
} ADC_Measure_t;

/* ======================== 函数声明 ======================== */

/**
 * @brief  初始化ADC + DMA + 触发定时器
 */
void ADC_Driver_Init(void);

/**
 * @brief  启动DMA连续采集
 */
void ADC_StartContinuous(void);

/**
 * @brief  停止DMA采集
 */
void ADC_StopContinuous(void);

/**
 * @brief  单次阻塞采集一个通道
 * @param  channel: ADC通道号
 * @return ADC原始值 (0~4095)
 */
uint16_t ADC_ReadSingle(uint32_t channel);

/**
 * @brief  获取DMA缓冲区中最新的N个样本(指定通道)
 * @param  buf: 输出缓冲区
 * @param  count: 样本数
 * @param  channel: 通道索引(0或1)
 * @return 实际读取的样本数
 */
uint16_t ADC_GetSamples(float *buf, uint16_t count, uint8_t channel);

/**
 * @brief  测量信号参数(DC偏置、RMS、Vpp)
 * @param  result: 测量结果结构体
 * @param  num_samples: 使用的样本数
 */
void ADC_Measure(ADC_Measure_t *result, uint16_t num_samples);

/**
 * @brief  配置ADC触发定时器(改变采样率)
 * @param  sample_rate: 目标采样率(Hz)
 */
void ADC_SetSampleRate(uint32_t sample_rate);

/**
 * @brief  DMA半传输完成回调
 */
void ADC_HalfTransferCallback(void);

/**
 * @brief  DMA传输完成回调
 */
void ADC_TransferCompleteCallback(void);

/**
 * @brief  检查是否有新数据
 * @return 1=有新数据, 0=无
 */
uint8_t ADC_DataReady(void);

/**
 * @brief  清除数据就绪标志
 */
void ADC_ClearReady(void);

#endif /* __ADC_DRIVER_H */
