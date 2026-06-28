/**
 * @file    waveform.h
 * @brief   波形复刻模块 - 逆滤波器 + 谐波合成
 * @note    根据学习到的频率响应, 通过补偿滤波器
 *          使装置输出能复刻目标电路的响应特性
 *
 * 原理:
 *   1. 已知被测电路的频率响应 H(f)
 *   2. 对目标波形做谐波分解(基波+各次谐波)
 *   3. 计算逆传递函数 H_inv(f) = 1/H(f)
 *   4. 用AD9833+衰减网络逐频率输出, 合成复刻波形
 *   或: 用DAC直接输出, 幅度按H_inv补偿
 *
 * 支持波形:
 *   - 正弦波: 单频率, 直接输出
 *   - 矩形波: 奇次谐波合成(1,3,5,7,...次)
 *   - 三角波: 奇次谐波合成(衰减更快)
 *   - 任意周期波: 傅里叶级数分解
 */

#ifndef __WAVEFORM_H
#define __WAVEFORM_H

#include "main.h"
#include "filter_id.h"
#include "rlc_fit.h"

/* ======================== 常量定义 ======================== */
#define MAX_HARMONICS       20          /* 最大谐波数 */
#define WAVEFORM_TABLE_SIZE 256         /* 波形查找表大小 */
#define REPLICATE_POINTS    64          /* 复刻输出点数 */

/* ======================== 数据结构 ======================== */

/* 谐波分量 */
typedef struct {
    float freq;         /* 频率(Hz) */
    float amplitude;    /* 幅度(Vpp) */
    float phase;        /* 相位(度) */
} Harmonic_t;

/* 波形描述 */
typedef struct {
    Harmonic_t harmonics[MAX_HARMONICS];    /* 谐波数组 */
    int        num_harmonics;                /* 谐波数量 */
    float      fundamental_freq;             /* 基波频率(Hz) */
    float      dc_offset;                    /* 直流偏置(V) */
    float      total_vpp;                    /* 总峰峰值 */
    uint8_t    wave_type;                    /* 波形类型 */
} WaveformDesc_t;

/* 复刻配置 */
typedef struct {
    WaveformDesc_t  target;             /* 目标波形 */
    WaveformDesc_t  compensated;        /* 补偿后波形 */
    RLC_Params_t   *circuit_params;     /* 被测电路参数 */
    FreqResponse_t *freq_response;      /* 频率响应数据 */
    float           output_table[WAVEFORM_TABLE_SIZE]; /* 输出查找表 */
    uint8_t         ready;              /* 复刻就绪标志 */
} WaveformReplicator_t;

/* ======================== 函数声明 ======================== */

/**
 * @brief  初始化波形复刻器
 * @param  wr: WaveformReplicator_t指针
 */
void Waveform_Init(WaveformReplicator_t *wr);

/**
 * @brief  设置目标波形为正弦波
 * @param  wr: 复刻器指针
 * @param  freq: 频率(Hz)
 * @param  vpp: 峰峰值(V)
 */
void Waveform_SetSine(WaveformReplicator_t *wr, float freq, float vpp);

/**
 * @brief  设置目标波形为矩形波
 * @param  wr: 复刻器指针
 * @param  freq: 频率(Hz)
 * @param  vpp: 峰峰值(V)
 * @param  duty: 占空比(0~1)
 */
void Waveform_SetSquare(WaveformReplicator_t *wr, float freq, float vpp, float duty);

/**
 * @brief  设置目标波形为三角波
 * @param  wr: 复刻器指针
 * @param  freq: 频率(Hz)
 * @param  vpp: 峰峰值(V)
 */
void Waveform_SetTriangle(WaveformReplicator_t *wr, float freq, float vpp);

/**
 * @brief  设置自定义波形(谐波数组)
 * @param  wr: 复刻器指针
 * @param  harmonics: 谐波数组
 * @param  num: 谐波数量
 * @param  dc: 直流偏置
 */
void Waveform_SetCustom(WaveformReplicator_t *wr, const Harmonic_t *harmonics,
                        int num, float dc);

/**
 * @brief  计算逆滤波补偿
 * @param  wr: 复刻器指针(使用freq_response做补偿)
 */
void Waveform_Compensate(WaveformReplicator_t *wr);

/**
 * @brief  生成输出查找表
 * @param  wr: 复刻器指针
 */
void Waveform_GenerateTable(WaveformReplicator_t *wr);

/**
 * @brief  获取查找表中的一个输出值
 * @param  wr: 复刻器指针
 * @param  index: 索引(0 ~ WAVEFORM_TABLE_SIZE-1)
 * @return 输出电压值(V)
 */
float Waveform_GetOutput(WaveformReplicator_t *wr, uint16_t index);

/**
 * @brief  计算矩形波傅里叶级数
 * @param  freq: 基波频率
 * @param  vpp: 峰峰值
 * @param  duty: 占空比
 * @param  harmonics: 输出谐波数组
 * @param  max_harmonics: 最大谐波数
 * @return 实际谐波数
 */
int Waveform_SquareHarmonics(float freq, float vpp, float duty,
                              Harmonic_t *harmonics, int max_harmonics);

/**
 * @brief  计算三角波傅里叶级数
 */
int Waveform_TriangleHarmonics(float freq, float vpp,
                                Harmonic_t *harmonics, int max_harmonics);

/**
 * @brief  通过AD9833逐次输出谐波分量(复刻输出)
 * @param  wr: 复刻器指针
 * @note   每个谐波单独输出一段时间, 需要外部时序控制
 */
void Waveform_OutputViaDDS(WaveformReplicator_t *wr);

/**
 * @brief  评估复刻质量(波形误差)
 * @param  wr: 复刻器指针
 * @return 误差百分比(0~100)
 */
float Waveform_EvaluateQuality(WaveformReplicator_t *wr);

#endif /* __WAVEFORM_H */
