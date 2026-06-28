/**
 * @file    rlc_fit.h
 * @brief   RLC参数拟合 - 从频率响应估算电路参数
 * @note    根据滤波器类型和频率特征, 估算等效R、L、C值
 *
 * 模型:
 *   低通RC:  fc = 1/(2πRC)
 *   高通RC:  fc = 1/(2πRC)
 *   带通RLC: f0 = 1/(2π√(LC)), Q = √(L/C)/R
 *   带阻RLC: f0 = 1/(2π√(LC)), Q = √(L/C)/R
 *
 * 拟合策略:
 *   1. 解析估算法: 从f0和Q直接计算(快速)
 *   2. 局部优化法: 基于解析值做Levenberg-Marquardt微调(精确)
 */

#ifndef __RLC_FIT_H
#define __RLC_FIT_H

#include "main.h"
#include "filter_id.h"

/* ======================== 数据结构 ======================== */

/* RLC等效电路参数 */
typedef struct {
    float R;            /* 电阻(Ω) */
    float L;            /* 电感(H) */
    float C;            /* 电容(F) */
    float fc;           /* 截止/中心频率(Hz) */
    float Q;            /* 品质因数 */
    float BW;           /* 带宽(Hz) */
    uint8_t topology;   /* 电路拓扑 */
    float fit_error;    /* 拟合误差 */
    uint8_t valid;      /* 参数有效标志 */
} RLC_Params_t;

/* 拓扑类型 */
#define TOPOLOGY_RC_LPF     1   /* RC低通 */
#define TOPOLOGY_RC_HPF     2   /* RC高通 */
#define TOPOLOGY_RLC_BPF    3   /* RLC带通 */
#define TOPOLOGY_RLC_BSF    4   /* RLC带阻(串联) */
#define TOPOLOGY_RLC_LPF    5   /* RLC低通(二阶) */
#define TOPOLOGY_RLC_HPF    6   /* RLC高通(二阶) */
#define TOPOLOGY_UNKNOWN    0

/* ======================== 函数声明 ======================== */

/**
 * @brief  初始化RLC参数结构
 * @param  params: RLC_Params_t指针
 */
void RLC_Init(RLC_Params_t *params);

/**
 * @brief  从滤波器识别结果进行RLC参数拟合
 * @param  filter_result: 滤波器识别结果
 * @param  freq_resp: 扫频数据(用于优化)
 * @param  params: RLC参数输出
 */
void RLC_Fit(const FilterResult_t *filter_result,
             const FreqResponse_t *freq_resp,
             RLC_Params_t *params);

/**
 * @brief  解析估算法(快速)
 * @param  type: 滤波器类型
 * @param  fc: 截止/中心频率(Hz)
 * @param  Q: 品质因数
 * @param  params: RLC参数输出
 */
void RLC_AnalyticEstimate(uint8_t type, float fc, float Q, RLC_Params_t *params);

/**
 * @brief  局部优化拟合(精确)
 * @param  params: 初始估计值, 优化后覆盖
 * @param  freq_resp: 扫频数据
 */
void RLC_LocalOptimize(RLC_Params_t *params, const FreqResponse_t *freq_resp);

/**
 * @brief  根据RLC参数计算理论频率响应
 * @param  params: RLC参数
 * @param  freq: 测试频率(Hz)
 * @param  out_gain: 输出增益(线性)
 * @param  out_phase: 输出相位(度)
 */
void RLC_TheoryResponse(const RLC_Params_t *params, float freq,
                         float *out_gain, float *out_phase);

/**
 * @brief  计算拟合误差(均方根)
 * @param  params: RLC参数
 * @param  freq_resp: 实测扫频数据
 * @return 归一化均方根误差
 */
float RLC_FitError(const RLC_Params_t *params, const FreqResponse_t *freq_resp);

/**
 * @brief  格式化参数为字符串(用于显示)
 * @param  params: RLC参数
 * @param  buf: 输出字符串缓冲区
 * @param  buf_size: 缓冲区大小
 */
void RLC_FormatString(const RLC_Params_t *params, char *buf, int buf_size);

#endif /* __RLC_FIT_H */
