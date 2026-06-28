/**
 * @file    goertzel.h
 * @brief   Goertzel算法 - 单频率DFT幅度/相位提取
 * @note    比完整FFT高效得多, 特别适合扫频应用
 *          计算单个频率bin的DFT, 复杂度O(N)
 *
 * 算法原理:
 *   对N个采样点, 计算目标频率k处的DFT:
 *   coeff = 2*cos(2*pi*k/N)
 *   s[n] = x[n] + coeff*s[n-1] - s[n-2]
 *   |X[k]|^2 = s1^2 + s2^2 - coeff*s1*s2
 *
 * 使用方法:
 *   1. Goertzel_Init() 设置目标频率和采样参数
 *   2. 逐个喂入采样值 Goertzel_Process()
 *   3. Goertzel_GetResult() 获取幅度和相位
 */

#ifndef __GOERTZEL_H
#define __GOERTZEL_H

#include "main.h"

/* ======================== 数据结构 ======================== */

typedef struct {
    float coeff;        /* 2*cos(2*pi*k/N) - Goertzel系数 */
    float s0;           /* 当前状态 s[n] */
    float s1;           /* 前一状态 s[n-1] */
    float s2;           /* 前前状态 s[n-2] */
    int   N;            /* 总采样点数 */
    int   count;        /* 已处理样本计数 */
    float w_real;       /* 旋转因子实部 cos(2*pi*k/N) */
    float w_imag;       /* 旋转因子虚部 sin(2*pi*k/N) */
    float magnitude;    /* 计算得到的幅度 */
    float phase;        /* 计算得到的相位(弧度) */
    float power;        /* 功率 |X[k]|^2 */
} Goertzel_t;

/* ======================== 函数声明 ======================== */

/**
 * @brief  初始化Goertzel滤波器
 * @param  g: Goertzel结构体指针
 * @param  target_freq: 目标频率(Hz)
 * @param  sample_rate: 采样率(Hz)
 * @param  num_samples: 总采样点数N
 */
void Goertzel_Init(Goertzel_t *g, float target_freq, float sample_rate, int num_samples);

/**
 * @brief  重置Goertzel状态(保留配置)
 * @param  g: Goertzel结构体指针
 */
void Goertzel_Reset(Goertzel_t *g);

/**
 * @brief  喂入一个采样值
 * @param  g: Goertzel结构体指针
 * @param  sample: 输入采样值(电压值)
 */
void Goertzel_Process(Goertzel_t *g, float sample);

/**
 * @brief  批量喂入采样值
 * @param  g: Goertzel结构体指针
 * @param  samples: 采样值数组
 * @param  count: 样本数
 */
void Goertzel_ProcessBlock(Goertzel_t *g, const float *samples, int count);

/**
 * @brief  计算最终结果(幅度和相位)
 * @note   必须在处理完所有N个样本后调用
 * @param  g: Goertzel结构体指针
 */
void Goertzel_Finalize(Goertzel_t *g);

/**
 * @brief  获取幅度
 * @param  g: Goertzel结构体指针
 * @return 归一化幅度 (实际幅度 = magnitude / N)
 */
float Goertzel_GetMagnitude(Goertzel_t *g);

/**
 * @brief  获取峰峰值
 * @param  g: Goertzel结构体指针
 * @return 峰峰值电压 (假设满N个样本)
 */
float Goertzel_GetVpp(Goertzel_t *g);

/**
 * @brief  获取相位(度)
 * @param  g: Goertzel结构体指针
 * @return 相位角 (-180 ~ +180 度)
 */
float Goertzel_GetPhaseDeg(Goertzel_t *g);

/**
 * @brief  一次性计算: 从缓冲区直接得出结果
 * @param  samples: 采样值数组
 * @param  num_samples: 样本数
 * @param  target_freq: 目标频率(Hz)
 * @param  sample_rate: 采样率(Hz)
 * @param  out_mag: 输出幅度指针
 * @param  out_phase: 输出相位指针(度)
 */
void Goertzel_Compute(const float *samples, int num_samples,
                      float target_freq, float sample_rate,
                      float *out_mag, float *out_phase);

#endif /* __GOERTZEL_H */
