/**
 * @file    filter_id.h
 * @brief   滤波器类型识别与频率响应分析
 * @note    基于扫频数据, 自动识别未知电路的滤波特性
 *
 * 识别方法:
 *   1. 多点扫频获取幅频响应曲线
 *   2. 分析高/低频段增益变化趋势
 *   3. 寻找-3dB截止频率和中心频率
 *   4. 判定类型: LPF / HPF / BPF / BSF(带阻)
 *
 * 特征提取:
 *   - 低频增益: 100Hz处的增益
 *   - 高频增益: 50kHz处的增益
 *   - 最大增益及对应频率
 *   - -3dB带宽
 *   - 斜率(dB/oct)
 */

#ifndef __FILTER_ID_H
#define __FILTER_ID_H

#include "main.h"

/* ======================== 常量定义 ======================== */
#define FREQ_RESP_MAX_POINTS    200     /* 最大扫频点数 */
#define GAIN_TOLERANCE_DB       1.0f    /* 增益判定容差(dB) */
#define SLOPE_THRESHOLD_DB      6.0f    /* 斜率阈值(dB/oct) */
#define MIN_GAIN_RANGE_DB       3.0f    /* 最小增益动态范围(dB) */

/* ======================== 数据结构 ======================== */

/* 单频率点测量数据 */
typedef struct {
    float freq;         /* 频率(Hz) */
    float gain;         /* 增益(线性, Vout/Vin) */
    float gain_db;      /* 增益(dB) */
    float phase_deg;    /* 相移(度) */
} FreqPoint_t;

/* 频率响应数据集 */
typedef struct {
    FreqPoint_t points[FREQ_RESP_MAX_POINTS];   /* 测量点数组 */
    int         num_points;                      /* 实际点数 */
    float       ref_amplitude;                   /* 参考输入幅度(V) */
} FreqResponse_t;

/* 滤波器识别结果 */
typedef struct {
    uint8_t  type;              /* 滤波器类型 */
    float    cutoff_low;        /* 下截止频率(HPF/BSF用) */
    float    cutoff_high;       /* 上截止频率(LP/BPF用) */
    float    center_freq;       /* 中心频率(BPF/BSF用) */
    float    bandwidth;         /* 带宽 */
    float    quality_factor;    /* 品质因数Q */
    float    max_gain_db;       /* 最大增益(dB) */
    float    min_gain_db;       /* 最小增益(dB) */
    float    dc_gain_db;        /* DC/低频增益(dB) */
    float    hf_gain_db;        /* 高频增益(dB) */
    float    slope_low;         /* 低频段斜率(dB/oct) */
    float    slope_high;        /* 高频段斜率(dB/oct) */
    uint8_t  confidence;        /* 置信度(0~100) */
} FilterResult_t;

/* ======================== 函数声明 ======================== */

/**
 * @brief  初始化频率响应数据集
 * @param  resp: FreqResponse_t指针
 * @param  ref_amp: 参考输入幅度(V)
 */
void FreqResp_Init(FreqResponse_t *resp, float ref_amp);

/**
 * @brief  添加一个频率测量点
 * @param  resp: FreqResponse_t指针
 * @param  freq: 频率(Hz)
 * @param  vout: 输出幅度(Vpp)
 * @param  phase: 相移(度)
 */
void FreqResp_AddPoint(FreqResponse_t *resp, float freq, float vout, float phase);

/**
 * @brief  识别滤波器类型
 * @param  resp: 扫频数据
 * @param  result: 识别结果输出
 */
void FilterID_Analyze(const FreqResponse_t *resp, FilterResult_t *result);

/**
 * @brief  获取滤波器类型名称字符串
 * @param  type: 滤波器类型
 * @return 类型名称(中文)
 */
const char* FilterID_GetTypeName(uint8_t type);

/**
 * @brief  计算两个频率之间的斜率(dB/octave)
 * @param  resp: 扫频数据
 * @param  idx_low: 低频点索引
 * @param  idx_high: 高频点索引
 * @return 斜率(dB/oct)
 */
float FilterID_CalcSlope(const FreqResponse_t *resp, int idx_low, int idx_high);

/**
 * @brief  寻找-3dB频率点
 * @param  resp: 扫频数据
 * @param  ref_gain_db: 参考增益(dB)
 * @param  from_low: 1=从低频搜索, 0=从高频搜索
 * @return -3dB频率(Hz), 未找到返回0
 */
float FilterID_Find3dBFreq(const FreqResponse_t *resp, float ref_gain_db, uint8_t from_low);

/**
 * @brief  寻找最大增益及对应频率
 * @param  resp: 扫频数据
 * @param  out_freq: 输出最大增益对应的频率
 * @param  out_gain_db: 输出最大增益(dB)
 */
void FilterID_FindPeak(const FreqResponse_t *resp, float *out_freq, float *out_gain_db);

#endif /* __FILTER_ID_H */
